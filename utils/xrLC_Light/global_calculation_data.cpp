#include "stdafx.h"
#include "global_calculation_data.h"
#include "serialize.h"
#include "../shader_xrlc.h"

global_claculation_data	gl_data;

template <class T>
void transfer(const char *name, xr_vector<T> &dest, IReader& F, u32 chunk)
{
	IReader*	O	= F.open_chunk(chunk);
	u32		count	= O?(O->length()/sizeof(T)):0;
	Logger.clMsg			("* %16s: %d",name,count);
	if (count)  
	{
		dest.reserve(count);
		dest.insert	(dest.begin(), (T*)O->pointer(), (T*)O->pointer() + count);
	}
	if (O)		O->close	();
}

extern u32*		Surface_Load	(char* name, u32& w, u32& h);
extern void		Surface_Init	();

// 
bool is_thm_missing = false;
bool is_tga_missing = false;

#include <memory>
void global_claculation_data::xrLoad()
{
	string_path					N;
	FS.update_path				( N, "$game_data$", "shaders_xrlc.xr" );
	g_shaders_xrlc				= xr_new<Shader_xrLC_LIB> ();
	g_shaders_xrlc->Load		( N );

	// Load CFORM
	{
		FS.update_path			(N,"$level$","level.cform");
		IReader*			fs = FS.r_open("$level$","level.cform");
		
		hdrCFORM			H;
		fs->r				(&H,sizeof(hdrCFORM));
		R_ASSERT			(CFORM_CURRENT_VERSION==H.version);
		
		Fvector*	verts	= (Fvector*)fs->pointer();
		CDB::TRI*	tris = (CDB::TRI*)(verts + H.vertcount);
		RCAST_Model.build	( verts, H.vertcount, tris, H.facecount );
		Msg("* Level CFORM: %dK",RCAST_Model.memory()/1024);

		g_rc_faces.resize	(H.facecount);

		IReader* Face_fs = FS.r_open("$level$", "build.rc_faces");
		R_ASSERT2(Face_fs, "can`t load build.rc_faces");

		Face_fs->open_chunk(0);
		for (auto &it : g_rc_faces)
		{
			it.reserved = Face_fs->r_u16();
			it.dwMaterial = Face_fs->r_u16();
			it.dwMaterialGame = Face_fs->r_u32();
			Face_fs->r_fvector2(it.t[0]);
			Face_fs->r_fvector2(it.t[1]);
			Face_fs->r_fvector2(it.t[2]);
		}
		Face_fs->close();
		LevelBB.set			(H.aabb);
	}
	
	{
		slots_data.Load( );
	}
	// Lights
	{
		IReader*			fs = FS.r_open("$level$","build.lights");
		IReader*			F;	u32 cnt; R_Light* L;

		// rgb
		F		=			fs->open_chunk		(0);
		cnt		=			F->length()/sizeof(R_Light);
		L		=			(R_Light*)F->pointer();
		g_lights.rgb.assign	(L,L+cnt);
		F->close			();

		// hemi
		F		=			fs->open_chunk		(1);
		cnt		=			F->length()/sizeof(R_Light);
		L		=			(R_Light*)F->pointer();
		g_lights.hemi.assign(L,L+cnt);
		F->close			();

		// sun
		F		=			fs->open_chunk		(2);
		cnt		=			F->length()/sizeof(R_Light);
		L		=			(R_Light*)F->pointer();
		g_lights.sun.assign	(L,L+cnt);
		F->close			();

		FS.r_close			(fs);
	}

	
	// Load level data
	{
		IReader*	fs		= FS.r_open ("$level$","build.prj");
		IReader*	F;

		// Version
		u32 version;
		fs->r_chunk			(EB_Version,&version);

		bool oxyVersion = (XRCL_CURRENT_VERSION == version) || (version == 18);
		R_ASSERT2(oxyVersion, "xrLC don't support a current version. Sorry.");

		// Header
		fs->r_chunk			(EB_Parameters,&g_params);

		// Load level data
		transfer("materials",	g_materials,			*fs,		EB_Materials);
		transfer("shaders_xrlc",g_shader_compile,		*fs,		EB_Shaders_Compile);
		post_process_materials( *g_shaders_xrlc, g_shader_compile, g_materials );
		// process textures
		Logger.Status			("Processing textures...");
		{
			Surface_Init		();
			F = fs->open_chunk	(EB_Textures);
			u32 tex_count = F->length() / sizeof(help_b_texture);
			for (u32 t=0; t<tex_count; t++)
			{
				Logger.Progress		(float(t)/float(tex_count));
				
				help_b_texture	TEX;
				F->r(&TEX, sizeof(TEX));
				b_BuildTexture	BT;
				std::memcpy(&BT, &TEX, sizeof(TEX) - 4);
				BT.pSurface = nullptr;
				
				// load thumbnail
				LPSTR N			= BT.name;
				if (strchr(N,'.')) *(strchr(N,'.')) = 0;
				strlwr			(N);

				if (0==xr_strcmp(N,"level_lods"))	
				{
					// HACK for merged lod textures
					BT.dwWidth	= 1024;
					BT.dwHeight	= 1024;
					BT.bHasAlpha= TRUE;
					BT.THM.SetHasSurface(FALSE);
				} 
				else
				{
					xr_strcat				(N,sizeof(BT.name),".thm");
					IReader* THM			= FS.r_open("$game_textures$",N);
					if (!THM) 
					{
						Logger.clMsg("can't find thm: %s", N);
						is_thm_missing = true;
						continue;
						
					}
					// version
					u32 version				= 0;
					R_ASSERT				(THM->r_chunk(THM_CHUNK_VERSION,&version));

					// analyze thumbnail information
					R_ASSERT(THM->find_chunk(THM_CHUNK_TEXTUREPARAM));
					THM->r                  (&BT.THM.fmt,sizeof(STextureParams::ETFormat));
					BT.THM.flags.assign		(THM->r_u32());
					BT.THM.border_color		= THM->r_u32();
					BT.THM.fade_color		= THM->r_u32();
					BT.THM.fade_amount		= THM->r_u32();
					BT.THM.mip_filter		= THM->r_u32();
					BT.THM.width			= THM->r_u32();
					BT.THM.height           = THM->r_u32();
					bool bLOD = false;
					if (N[0]=='l' && N[1]=='o' && N[2]=='d' && N[3]=='\\') bLOD = true;

					// load surface if it has an alpha channel or has "implicit lighting" flag
					BT.dwWidth				= BT.THM.width;
					BT.dwHeight				= BT.THM.height;
					BT.bHasAlpha			= BT.THM.HasAlphaChannel();
					BT.THM.SetHasSurface(FALSE);
					if (!bLOD) 
					{
						if (BT.bHasAlpha || BT.THM.flags.test(STextureParams::flImplicitLighted))
						{
							Logger.clMsg		("- loading: %s",N);
							u32			w=0, h=0;
							BT.pSurface		= Surface_Load(N,w,h);
							BT.THM.SetHasSurface(TRUE);
							
							if (!BT.pSurface)
							{
								Logger.clMsg("can't find tga texture: %s", N);
								is_tga_missing = true;
								continue;
							}
							if ((w != BT.dwWidth) || (h != BT.dwHeight))
								Msg		("! THM doesn't correspond to the texture: %dx%d -> %dx%d", BT.dwWidth, BT.dwHeight, w, h);
							BT.Vflip	();
						} else {
							// Free surface memory
						}
					}
				}

				// save all the stuff we've created
				g_textures.push_back	(BT);
			}
			R_ASSERT2(!is_thm_missing, "Some of required thm's are missing. See log for details.");
			R_ASSERT2(!is_tga_missing, "Some of required tga are missing. See log for details.");
		}
	}
}

void read(IReader &r, CDB::MODEL &m);

void global_claculation_data::read(IReader &r)
{
	g_lights.read(r);
	R_ASSERT(!g_shaders_xrlc);
	g_shaders_xrlc = xr_new<Shader_xrLC_LIB>();
	r_pod_vector(r, g_shaders_xrlc->Library());
	r_pod(r, g_params);
	r_pod_vector(r, g_materials);
	r_vector(r, g_textures);
	::read(r, RCAST_Model);
	r_pod(r, LevelBB);
	slots_data.read(r);
	r_pod_vector(r, g_shader_compile);
	r_pod_vector(r, g_rc_faces);
}

void write(IWriter &w, const CDB::MODEL &m);
void global_claculation_data::write(IWriter &w) const
{
	g_lights.write(w);
	R_ASSERT(g_shaders_xrlc);
	w_pod_vector(w, g_shaders_xrlc->Library());
	w_pod(w, g_params);
	w_pod_vector(w, g_materials);
	w_vector(w, g_textures);
	::write(w, RCAST_Model);
	w_pod(w, LevelBB);
	slots_data.write(w);
	w_pod_vector(w, g_shader_compile);
	w_pod_vector(w, g_rc_faces);
}