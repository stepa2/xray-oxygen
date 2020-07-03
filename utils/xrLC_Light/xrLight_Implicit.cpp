#include "stdafx.h"
#include "xrlight_implicit.h"
#include "xrLight_ImplicitDeflector.h"
#include "xrlight_implicitrun.h"
#include "tga.h"
#include "light_point.h"
#include "xrdeflector.h"
#include "xrLC_GlobalData.h"
#include "xrface.h"
#include "xrlight_implicitcalcglobs.h"
#include "../../xrcdb/xrcdb.h"
#include "xrHardwareLight.h"

extern "C" bool __declspec(dllimport) __stdcall DXTCompress(LPCSTR out_name, u8* raw_data, u8* normal_map, u32 w, u32 h, u32 pitch, STextureParams* fmt, u32 depth);
using Implicit = xr_map<u32, ImplicitDeflector>;

void ImplicitExecute::read(IReader &r)
{
	y_start = r.r_u32();
	y_end = r.r_u32();
}
void ImplicitExecute::write(IWriter	&w) const
{
	R_ASSERT(y_start != (u32(-1)));
	R_ASSERT(y_end != (u32(-1)));
	w.w_u32(y_start);
	w.w_u32(y_end);
}

ImplicitCalcGlobs cl_globs;


void	ImplicitExecute::receive_result(IReader	&r)
{
	R_ASSERT(y_start != (u32(-1)));
	R_ASSERT(y_end != (u32(-1)));
	ImplicitDeflector&		defl = cl_globs.DATA();
	for (u32 V = y_start; V < y_end; V++)
		for (u32 U = 0; U < defl.Width(); U++)
		{

			r_pod<base_color>(r, defl.Lumel(U, V));
			r_pod<u8>(r, defl.Marker(U, V));

		}
}
void	ImplicitExecute::send_result(IWriter	&w) const
{
	R_ASSERT(y_start != (u32(-1)));
	R_ASSERT(y_end != (u32(-1)));
	ImplicitDeflector&		defl = cl_globs.DATA();
	for (u32 V = y_start; V < y_end; V++)
		for (u32 U = 0; U < defl.Width(); U++)
		{
			w_pod<base_color>(w, defl.Lumel(U, V));
			w_pod<u8>(w, defl.Marker(U, V));
		}
}


void ImplicitExecute::Execute()
{
	R_ASSERT(y_start != (u32(-1)));
	R_ASSERT(y_end != (u32(-1)));
	//R_ASSERT				(DATA);
	ImplicitDeflector&		defl = cl_globs.DATA();
	CDB::COLLIDER			DB;

	// Setup variables
	Fvector2	dim, half;
	dim.set(float(defl.Width()), float(defl.Height()));
	half.set(.5f / dim.x, .5f / dim.y);

	// Jitter data
	Fvector2	JS;
	JS.set(.499f / dim.x, .499f / dim.y);
	u32			Jcount;
	Fvector2*	Jitter;
	Jitter_Select(Jitter, Jcount);

	// Lighting itself
	DB.ray_options(0);
	for (u32 V = y_start; V < y_end; V++)
	{
		for (u32 U = 0; U < defl.Width(); U++)
		{
			base_color_c	C;
			u32				Fcount = 0;
			defl.Marker(U, V) = 0;

			try {
				for (u32 J = 0; J < Jcount; J++)
				{
					// LUMEL space
					Fvector2				P;
					P.x = float(U) / dim.x + half.x + Jitter[J].x * JS.x;
					P.y = float(V) / dim.y + half.y + Jitter[J].y * JS.y;
					xr_vector<Face*>& space = cl_globs.Hash().query(P.x, P.y);

					// World space
					Fvector wP, wN, B;
					for (Face* F : space)
					{
						_TCF&	tc = F->tc[0];
						if (tc.isInside(P, B))
						{
							// We found triangle and have barycentric coords
							Vertex	*V1 = F->v[0];
							Vertex	*V2 = F->v[1];
							Vertex	*V3 = F->v[2];
							wP.from_bary(V1->P, V2->P, V3->P, B);
							wN.from_bary(V1->N, V2->N, V3->N, B);
							wN.normalize();
							if (xrHardwareLight::IsEnabled())
							{
								defl.lmap.SurfaceLightRequests.emplace_back(U, V, wP, wN, F);
								defl.Marker(U, V) = 255;
							}
							else
							{
								LightPoint(&DB, inlc_global_data()->RCAST_Model(), C, wP, wN, inlc_global_data()->L_static(), (inlc_global_data()->b_nosun() ? LP_dont_sun : 0), F);
							}
							Fcount++;
						}
					}
				}
			}
			catch (...)
			{
				Logger.clMsg("* THREAD #%d: Access violation. Possibly recovered.");//,thID
			}

			if (!xrHardwareLight::IsEnabled())
			{
				if (Fcount) {
					// Calculate lighting amount
					C.scale(Fcount);
					C.mul(.5f);
					defl.Lumel(U, V)._set(C);
					defl.Marker(U, V) = 255;
				}
				else {
					defl.Marker(U, V) = 0;
				}
			}
		}
		//		thProgress	= float(V - y_start) / float(y_end-y_start);
	}

	if (xrHardwareLight::IsEnabled())
	{
		//cast and finalize
		if (defl.lmap.SurfaceLightRequests.empty())
		{
			return;
		}
		xrHardwareLight& HardwareCalculator = xrHardwareLight::Get();

		//pack that shit in to task, but remember order
		xr_vector <RayRequest> RayRequests;
		u32 SurfaceCount = defl.lmap.SurfaceLightRequests.size();
		RayRequests.reserve(SurfaceCount);
		for (int SurfaceID = 0; SurfaceID < SurfaceCount; ++SurfaceID)
		{
			LightpointRequest& LRequest = defl.lmap.SurfaceLightRequests[SurfaceID];
			RayRequests.push_back(RayRequest{ LRequest.Position, LRequest.Normal, LRequest.FaceToSkip });
		}

		xr_vector<base_color_c> FinalColors;
		HardwareCalculator.PerformRaycast(RayRequests, (inlc_global_data()->b_nosun() ? LP_dont_sun : 0) | LP_UseFaceDisable, FinalColors);
		//HardwareCalculator.PerformRaycast(RayRequests, LP_dont_sun + LP_dont_hemi + LP_UseFaceDisable, FinalColors);

		//finalize rays

		//all that we must remember - we have fucking jitter. And that we don't have much time, because we have tons of that shit
		u32 SurfaceRequestCursor = 0;
		u32 AlmostMaxSurfaceLightRequest = defl.lmap.SurfaceLightRequests.size() - 1;
		for (u32 V = 0; V < defl.lmap.height; V++)
		{
			for (u32 U = 0; U < defl.lmap.width; U++)
			{
				LightpointRequest& LRequest = defl.lmap.SurfaceLightRequests[SurfaceRequestCursor];

				if (LRequest.X == U && LRequest.Y == V)
				{
					//accumulate all color and draw to the lmap
					base_color_c ReallyFinalColor;
					int ColorCount = 0;
					for (;;)
					{
						LRequest = defl.lmap.SurfaceLightRequests[SurfaceRequestCursor];

						if (LRequest.X != U || LRequest.Y != V || SurfaceRequestCursor == AlmostMaxSurfaceLightRequest)
						{
							ReallyFinalColor.scale(ColorCount);
							ReallyFinalColor.mul(0.5f);
							defl.Lumel(U, V)._set(ReallyFinalColor);
							break;
						}

						base_color_c& CurrentColor = FinalColors[SurfaceRequestCursor];
						ReallyFinalColor.add(CurrentColor);

						++SurfaceRequestCursor;
						++ColorCount;
					}
				}
			}
		}

		defl.lmap.SurfaceLightRequests.clear();
	}

}

xrCriticalSection implicit_net_lock;
void ImplicitLightingTreadNetExec(void *p);
void XRLC_LIGHT_API ImplicitNetWait()
{
	implicit_net_lock.Enter();
	implicit_net_lock.Leave();
}

static xr_vector<u32> not_clear;
void ImplicitLightingExec(u32 thCount)
{
	Implicit calculator;

	cl_globs.Allocate();
	not_clear.clear();
	// Sorting
	Logger.Status("Sorting faces...");
	size_t Iterator = 0;
	for (Face* F: inlc_global_data()->g_faces())
	{
		Iterator++;
		if (F->pDeflector)				continue;
		if (!F->hasImplicitLighting())	continue;

		Logger.Progress(inlc_global_data()->g_faces().size() / Iterator);
		b_material&		M = inlc_global_data()->materials()[F->dwMaterial];
		u32				Tid = M.surfidx;
		b_BuildTexture*	T = &(inlc_global_data()->textures()[Tid]);

		auto it = calculator.find(Tid);
		if (it == calculator.end())
		{
			ImplicitDeflector	ImpD;
			ImpD.texture = T;
			ImpD.faces.push_back(F);
			calculator.insert(std::make_pair(Tid, ImpD));
			not_clear.push_back(Tid);
		}
		else
		{
			ImplicitDeflector&	ImpD = it->second;
			ImpD.faces.push_back(F);
		}
	}


	// Lighing
	for (auto imp : calculator)
	{
		ImplicitDeflector& defl = imp.second;
		Logger.Status("Lighting implicit map '%s'...", defl.texture->name);
		Logger.Progress(0);
		defl.Allocate();

		// Setup cache
		Logger.Progress(0);
		cl_globs.Initialize(defl);
		RunImplicitMultithread(defl, thCount);

		defl.faces.clear();

		// Expand
		Logger.Status("Processing lightmap...");
		for (u32 ref = 254; ref > 0; ref--)	if (!ApplyBorders(defl.lmap, ref)) break;

		Logger.Status("Mixing lighting with texture...");
		{
			b_BuildTexture& TEX = *defl.texture;
			VERIFY(TEX.pSurface);
			u32*			color = TEX.pSurface;
			for (u32 V = 0; V < defl.Height(); V++) {
				for (u32 U = 0; U < defl.Width(); U++) {
					// Retreive Texel
					float	h = defl.Lumel(U, V).h._r();
					u32 &C = color[V*defl.Width() + U];
					C = subst_alpha(C, u8_clr(h));
				}
			}
		}

		xr_vector<u32>				packed;
		defl.lmap.Pack(packed);
		defl.Deallocate();


		// base
		Logger.Status("Saving base...");
		{
			string_path				name, out_name;
			sscanf(strstr(Core.Params, "-f") + 2, "%s", name);
			R_ASSERT(name[0] && defl.texture);
			b_BuildTexture& TEX = *defl.texture;
			xr_strconcat(out_name, name, "\\", TEX.name, ".dds");
			FS.update_path(out_name, "$game_levels$", out_name);
			Logger.clMsg("Saving texture '%s'...", out_name);
			createPath(out_name, true);
			BYTE* raw_data = LPBYTE(TEX.pSurface);
			u32	w = TEX.dwWidth;
			u32	h = TEX.dwHeight;
			u32	pitch = w * 4;
			STextureParams			fmt = TEX.THM;
			fmt.fmt = STextureParams::tfDXT5;
			fmt.flags.set(STextureParams::flDitherColor, false);
			fmt.flags.set(STextureParams::flGenerateMipMaps, false);
			fmt.flags.set(STextureParams::flBinaryAlpha, false);
			DXTCompress(out_name, raw_data, nullptr, w, h, pitch, &fmt, 4);
		}

		// lmap
		Logger.Status("Saving lmap...");
		{
			string_path				name, out_name;
			sscanf(strstr(GetCommandLine(), "-f") + 2, "%s", name);
			b_BuildTexture& TEX = *defl.texture;
			xr_strconcat(out_name, name, "\\", TEX.name, "_lm.dds");
			FS.update_path(out_name, "$game_levels$", out_name);
			Logger.clMsg("Saving texture '%s'...", out_name);
			createPath(out_name, true);
			BYTE* raw_data = LPBYTE(&*packed.begin());
			u32	w = TEX.dwWidth;
			u32	h = TEX.dwHeight;
			u32	pitch = w * 4;
			STextureParams			fmt;
			fmt.fmt = STextureParams::tfRGBA;
			fmt.flags.set(STextureParams::flDitherColor, false);
			fmt.flags.set(STextureParams::flGenerateMipMaps, false);
			fmt.flags.set(STextureParams::flBinaryAlpha, false);
			DXTCompress(out_name, raw_data, nullptr, w, h, pitch, &fmt, 4);
		}
	}
	not_clear.clear();
	cl_globs.Deallocate();
	calculator.clear();
}

void ImplicitLightingTreadNetExec(void *p)
{
	xrCriticalSectionGuard guard(implicit_net_lock);
	ImplicitLightingExec(2);
}

void ImplicitLighting(u32 thCount)
{
	if (g_params().m_quality != ebqDraft)
	{
		ImplicitLightingExec(thCount);
		return;
	}
}