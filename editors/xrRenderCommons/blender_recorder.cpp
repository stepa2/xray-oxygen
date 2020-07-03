// Blender_Recorder.cpp: implementation of the CBlender_Compile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#include "ResourceManager.h"
#include "Blender_Recorder.h"
#include "Blender.h"

#include "dxRenderDeviceRender.h"

static int ParseName(LPCSTR N)
{
	if (!xr_strcmp(N, "$null"))  return -1;
	if (!xr_strcmp(N, "$base0")) return	0;
	if (!xr_strcmp(N, "$base1")) return	1;
	if (!xr_strcmp(N, "$base2")) return	2;
	if (!xr_strcmp(N, "$base3")) return	3;
	if (!xr_strcmp(N, "$base4")) return	4;
	if (!xr_strcmp(N, "$base5")) return	5;
	if (!xr_strcmp(N, "$base6")) return	6;
	if (!xr_strcmp(N, "$base7")) return	7;

	return -1;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlenderCompiler::CBlenderCompiler()
{
}
CBlenderCompiler::~CBlenderCompiler()
{
}
void CBlenderCompiler::_cpp_Compile(ShaderElement* _SH)
{
	SH = _SH;
	RS.Invalidate();

	//	TODO: Check if we need such wired system for
	//	base texture name detection. Perhapse it's done for
	//	optimization?

	// Analyze possibility to detail this shader
	detail_texture = NULL;
	detail_scaler = NULL;
	const char* base;
	if (bDetail && BT->canBeDetailed())
	{
		//
		xrStringVec& lst = L_textures;
		int id = ParseName(BT->oT_Name);
		base = BT->oT_Name;
		if (id >= 0)
		{
			if (id >= int(lst.size()))
				Debug.fatal(DEBUG_INFO, "Not enought textures for shader. Base texture: '%s'.", *lst[0]);

			base = *lst[id];
		}
		//.		if (!dxRenderDeviceRender::Instance().Resources->_GetDetailTexture(base,detail_texture,detail_scaler))	bDetail	= FALSE;

		if (!dxRenderDeviceRender::Instance().Resources->m_textures_description.GetDetailTexture(base, detail_texture, detail_scaler))
			bDetail = FALSE;
	}
	else
	{
		////////////////////
		//	Igor
		//	Need this to correct base to detect steep parallax.
		if (BT->canUseSteepParallax())
		{
			xrStringVec& lst = L_textures;
			int id = ParseName(BT->oT_Name);
			base = BT->oT_Name;
			if (id >= 0)
			{
				if (id >= int(lst.size()))
					Debug.fatal(DEBUG_INFO, "Not enought textures for shader. Base texture: '%s'.", *lst[0]);

				base = *lst[id];
			}
		}
		//	Igor
		////////////////////

		bDetail = FALSE;
	}

	// Validate for R1 or R2
	bDetail_Diffuse = FALSE;
	bDetail_Bump = FALSE;

	if (bDetail)
	{
		dxRenderDeviceRender::Instance().Resources->m_textures_description.GetTextureUsage(base, bDetail_Diffuse, bDetail_Bump);
	}

	bUseSteepParallax = dxRenderDeviceRender::Instance().Resources->m_textures_description.UseSteepParallax(base)
		&& BT->canUseSteepParallax();

	// Compile
	BT->Compile(NULL);		//#TODO: must be render complile
}

ShaderElement* CBlenderCompiler::_spectre_Compile(LPCSTR namesp, LPCSTR name)
{
	return nullptr;
}

void	CBlenderCompiler::SetParams(int iPriority, bool bStrictB2F)
{
	SH->flags.iPriority = iPriority;
	SH->flags.bStrictB2F = bStrictB2F;
	if (bStrictB2F)
	{
#ifdef _EDITOR
		if (1 != (SH->flags.iPriority / 2))
		{
			Log("!If StrictB2F true then Priority must div 2.");
			SH->flags.bStrictB2F = FALSE;
		}
#else
		VERIFY(1 == (SH->flags.iPriority / 2));
#endif
	}
}

//
void CBlenderCompiler::PassBegin()
{
	RS.Invalidate();
	passTextures.clear();
	passMatrices.clear();
	passConstants.clear();
	xr_strcpy(pass_ps, "null");
	xr_strcpy(pass_vs, "null");
	dwStage = 0;
}

void CBlenderCompiler::PassEnd()
{
	// Last Stage - disable
	RS.SetTSS(Stage(), D3DTSS_COLOROP, D3DTOP_DISABLE);
	RS.SetTSS(Stage(), D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	ShaderPass	proto;
	// Create pass
	proto.state = dxRenderDeviceRender::Instance().Resources->_CreateState(RS.GetContainer());
	proto.ps = dxRenderDeviceRender::Instance().Resources->_CreatePS(pass_ps);
	proto.vs = dxRenderDeviceRender::Instance().Resources->_CreateVS(pass_vs);
	ctable.merge(&proto.ps->constants);
	ctable.merge(&proto.vs->constants);
	SetMapping();
	proto.constants = dxRenderDeviceRender::Instance().Resources->_CreateConstantTable(ctable);
	proto.T = dxRenderDeviceRender::Instance().Resources->_CreateTextureList(passTextures);
#ifdef _EDITOR
	proto.M = DEV->_CreateMatrixList(passMatrices);
#endif
	proto.C = dxRenderDeviceRender::Instance().Resources->_CreateConstantList(passConstants);

	ref_pass _pass_ = dxRenderDeviceRender::Instance().Resources->_CreatePass(proto);
	SH->passes.push_back(_pass_);
}

void	CBlenderCompiler::PassSET_PS(LPCSTR name)
{
	xr_strcpy(pass_ps, name);
	strlwr(pass_ps);
}

void CBlenderCompiler::PassSET_VS(LPCSTR name)
{
	xr_strcpy(pass_vs, name);
	strlwr(pass_vs);
}

void CBlenderCompiler::PassSET_ZB(BOOL bZTest, BOOL bZWrite, BOOL bInvertZTest)
{
	if (Pass())	bZWrite = FALSE;
	RS.SetRS(D3DRS_ZFUNC, bZTest ? (bInvertZTest ? D3DCMP_GREATER : D3DCMP_LESSEQUAL) : D3DCMP_ALWAYS);
	RS.SetRS(D3DRS_ZWRITEENABLE, BC(bZWrite));
}

void CBlenderCompiler::PassSET_ablend_mode(BOOL bABlend, u32 abSRC, u32 abDST)
{
	if (bABlend && D3DBLEND_ONE == abSRC && D3DBLEND_ZERO == abDST)
		bABlend = FALSE;

	RS.SetRS(D3DRS_ALPHABLENDENABLE, BC(bABlend));
	RS.SetRS(D3DRS_SRCBLEND, bABlend ? abSRC : D3DBLEND_ONE);
	RS.SetRS(D3DRS_DESTBLEND, bABlend ? abDST : D3DBLEND_ZERO);
}
void CBlenderCompiler::PassSET_ablend_aref(BOOL bATest, u32 aRef)
{
	clamp(aRef, 0u, 255u);
	RS.SetRS(D3DRS_ALPHATESTENABLE, BC(bATest));
	if (bATest)	RS.SetRS(D3DRS_ALPHAREF, u32(aRef));
}

void CBlenderCompiler::PassSET_Blend(BOOL bABlend, u32 abSRC, u32 abDST, BOOL bATest, u32 aRef)
{
	PassSET_ablend_mode(bABlend, abSRC, abDST);

#ifdef DEBUG
	if (strstr(Core.Params, "-noaref"))
	{
		bATest = FALSE;
		aRef = 0;
	}
#endif

	PassSET_ablend_aref(bATest, aRef);
}

void CBlenderCompiler::PassSET_LightFog(BOOL bLight, BOOL bFog)
{
	RS.SetRS(D3DRS_LIGHTING, BC(bLight));
	RS.SetRS(D3DRS_FOGENABLE, BC(bFog));
}

//
void CBlenderCompiler::StageBegin()
{
	StageSET_Address(D3DTADDRESS_WRAP);	// Wrapping enabled by default
}

void CBlenderCompiler::StageEnd()
{
	++dwStage;
}

void CBlenderCompiler::StageSET_Address(u32 adr)
{
	RS.SetSAMP(Stage(), D3DSAMP_ADDRESSU, adr);
	RS.SetSAMP(Stage(), D3DSAMP_ADDRESSV, adr);
}

void CBlenderCompiler::StageSET_XForm(u32 tf, u32 tc)
{
#ifdef _EDITOR
	RS.SetTSS(Stage(), D3DTSS_TEXTURETRANSFORMFLAGS, tf);
	RS.SetTSS(Stage(), D3DTSS_TEXCOORDINDEX, tc);
#endif
}

void CBlenderCompiler::StageSET_Color(u32 a1, u32 op, u32 a2)
{
	RS.SetColor(Stage(), a1, op, a2);
}

void CBlenderCompiler::StageSET_Color3(u32 a1, u32 op, u32 a2, u32 a3)
{
	RS.SetColor3(Stage(), a1, op, a2, a3);
}

void CBlenderCompiler::StageSET_Alpha(u32 a1, u32 op, u32 a2)
{
	RS.SetAlpha(Stage(), a1, op, a2);
}

#if !defined(USE_DX10) && !defined(USE_DX11)
void CBlenderCompiler::StageSET_TMC(LPCSTR T, LPCSTR M, LPCSTR C, int UVW_channel)
{
	Stage_Texture(T);
	Stage_Matrix(M, UVW_channel);
	Stage_Constant(C);
}

void CBlenderCompiler::StageTemplate_LMAP0()
{
	StageSET_Address(D3DTADDRESS_CLAMP);
	StageSET_Color(D3DTA_TEXTURE, D3DTOP_SELECTARG1, D3DTA_DIFFUSE);
	StageSET_Alpha(D3DTA_TEXTURE, D3DTOP_SELECTARG1, D3DTA_DIFFUSE);
	StageSET_TMC("$base1", "$null", "$null", 1);
}

void CBlenderCompiler::Stage_Texture(LPCSTR name, u32, u32 fmin, u32 fmip, u32 fmag)
{
	xrStringVec& lst = L_textures;
	int id = ParseName(name);
	LPCSTR N = name;
	if (id >= 0) {
		if (id >= int(lst.size()))	Debug.fatal(DEBUG_INFO, "Not enought textures for shader. Base texture: '%s'.", *lst[0]);
		N = *lst[id];
	}
	//passTextures.push_back(mk_pair(Stage(), ref_texture(DEV->_CreateTexture(N))));
	//	i_Address				(Stage(),address);
	i_Filter(Stage(), fmin, fmip, fmag);
}
#endif	//	USE_DX10

void CBlenderCompiler::Stage_Matrix(LPCSTR name, int iChannel)
{
	xrStringVec& lst = L_matrices;
	int id = ParseName(name);
	CMatrix*	M = DEV->_CreateMatrix((id >= 0) ? *lst[id] : name);
	passMatrices.push_back(M);

	// Setup transform pipeline
	u32	ID = Stage();

	if (M)
	{
		switch (M->dwMode)
		{
		case CMatrix::modeProgrammable:
			StageSET_XForm(D3DTTFF_COUNT3, D3DTSS_TCI_CAMERASPACEPOSITION | ID);
			break;
		case CMatrix::modeTCM:
			StageSET_XForm(D3DTTFF_COUNT2, D3DTSS_TCI_PASSTHRU | iChannel);
			break;
		case CMatrix::modeC_refl:
			StageSET_XForm(D3DTTFF_COUNT3, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR | ID);
			break;
		case CMatrix::modeS_refl:
			StageSET_XForm(D3DTTFF_COUNT2, D3DTSS_TCI_CAMERASPACENORMAL | ID);
			break;
		default:
			StageSET_XForm(D3DTTFF_DISABLE, D3DTSS_TCI_PASSTHRU | iChannel);
			break;
		}
	}
	else
		// No XForm at all
		StageSET_XForm(D3DTTFF_DISABLE, D3DTSS_TCI_PASSTHRU | iChannel);
}
void CBlenderCompiler::Stage_Constant(LPCSTR name)
{
	xrStringVec& lst = L_constants;
	int id = ParseName(name);
	passConstants.push_back(DEV->_CreateConstant((id >= 0) ? *lst[id] : name));
}