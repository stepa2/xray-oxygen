#include "stdafx.h"
#pragma hdrstop

#include "Blender_SSAO.h"

CBlender_SSAO_noMSAA::CBlender_SSAO_noMSAA	()	{	description.CLS		= 0;	}
CBlender_SSAO_noMSAA::~CBlender_SSAO_noMSAA	()	{	}

void	CBlender_SSAO_noMSAA::Compile			(CBlender_Compile& C)
{
	IBlender::Compile		(C);

	switch (C.iElement)
	{
	case 0:		// calculate SSAO
		C.r_Pass			("combine_1",		"ssao_calc_nomsaa",	false,	false,	false);
		C.r_Stencil			(TRUE, D3D11_COMPARISON_LESS_EQUAL, 0xFF);	// stencil should be >= 1
		C.r_StencilRef		(0x01);
		C.r_CullMode		(D3D11_CULL_NONE);

		C.r_dx10Texture		("s_position",	r2_RT_P);
		C.r_dx10Texture		("s_normal",	r2_RT_N);
		C.r_dx10Texture		("s_tonemap",	r2_RT_luminance_cur	);
		C.r_dx10Texture		("s_half_depth",r2_RT_half_depth	);

		jitter(C);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 1:		// depth downsample for HBAO
		C.r_Pass			("combine_1",		"depth_downs",	false,	false,	false);
		C.r_CullMode		(D3D11_CULL_NONE);

		C.r_dx10Texture		("s_position",	r2_RT_P);
		C.r_dx10Texture		("s_normal",	r2_RT_N);
		C.r_dx10Texture		("s_tonemap",	r2_RT_luminance_cur	);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	}
}

CBlender_SSAO_MSAA::CBlender_SSAO_MSAA	()	{	description.CLS		= 0;	}
CBlender_SSAO_MSAA::~CBlender_SSAO_MSAA	()	{	}

void	CBlender_SSAO_MSAA::Compile(CBlender_Compile& C)
{
	IBlender::Compile		(C);

	if( Name )
		::Render->m_MSAASample = atoi_17( Definition );
	else
		::Render->m_MSAASample = -1;

	switch (C.iElement)
	{
	case 0:	// combine
		C.r_Pass			("combine_1",		"ssao_calc_msaa",		false,	false,	false);
		C.r_Stencil			(TRUE, D3D11_COMPARISON_EQUAL, 0x81);	// stencil should be >= 1
		C.r_StencilRef		(0x81);
		C.r_CullMode		(D3D11_CULL_NONE);

		C.r_dx10Texture		("s_position",		r2_RT_P				);
		C.r_dx10Texture		("s_normal",		r2_RT_N				);

		jitter(C);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	}
	::Render->m_MSAASample = -1;
}    
