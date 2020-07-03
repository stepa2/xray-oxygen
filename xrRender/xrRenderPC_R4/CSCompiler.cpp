////////////////////////////////////////////////////////////////////////////
//	Created		: 22.05.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CSCompiler.h"
#include "ComputeShader.h"
#include "..\xrRender\dxRenderDeviceRender.h"

ENGINE_API BOOL isGraphicDebugging;

CSCompiler::CSCompiler(ComputeShader& target):
	m_Target(target), m_cs(0)
{
}

CSCompiler& CSCompiler::begin(const char* name)
{
	compile(name);
	return *this;
}

CSCompiler& CSCompiler::defSampler(LPCSTR ResourceName)
{
	D3D11_SAMPLER_DESC	desc;
    std::memset(&desc,0,sizeof(desc));

	if (0 == xr_strcmp(ResourceName, "smp_nofilter"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		return defSampler(ResourceName, desc);
	}
	else if (0 == xr_strcmp(ResourceName, "smp_rtlinear"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		return defSampler(ResourceName, desc);
	}
	else if (0 == xr_strcmp(ResourceName, "smp_linear"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		return defSampler(ResourceName, desc);
	}
	else if (0 == xr_strcmp(ResourceName, "smp_base"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D11_FILTER_ANISOTROPIC;
		desc.MaxAnisotropy = 8;
		return defSampler(ResourceName, desc);
	}
	else if (0 == xr_strcmp(ResourceName, "smp_material"))
	{
		desc.AddressU = desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		return defSampler(ResourceName, desc);
	}
	else if (0 == xr_strcmp(ResourceName, "smp_smap"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		desc.ComparisonFunc = D3D_COMPARISON_LESS_EQUAL;
		return defSampler(ResourceName, desc);
	}
	else if (0 == xr_strcmp(ResourceName, "smp_jitter"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		return defSampler(ResourceName, desc);
	}

	VERIFY(0);

	return *this;
}

CSCompiler& CSCompiler::defSampler(LPCSTR ResourceName, const D3D_SAMPLER_DESC& def)
{
	VERIFY(ResourceName);

	ref_constant C			= m_constants.get(ResourceName);
	if (!C)					return	*this;

	R_ASSERT				(C->type == RC_sampler);
	u32 stage				= C->samp.index;

	if (stage >= m_Samplers.size())
		m_Samplers.resize(stage+1);

	R_CHK(HW.pDevice->CreateSamplerState(&def, &m_Samplers[stage]));

	return *this;
}

void fix_texture_name(LPSTR);

CSCompiler& CSCompiler::defOutput(LPCSTR ResourceName,	ref_rt rt)
{
	VERIFY(ResourceName);
	if (!rt) return *this;

	ref_constant C			= m_constants.get(ResourceName);
	if (!C)					return *this;

	R_ASSERT				(C->type == RC_dx11UAV);
	u32 stage				= C->samp.index;

	if (stage >= m_Textures.size())
		m_Textures.resize(stage+1);

	m_Outputs[stage] = rt->pUAView; //!!!dangerous view can be deleted

	return *this;
}

CSCompiler& CSCompiler::defTexture(LPCSTR ResourceName,	ref_texture texture)
{
	VERIFY(ResourceName);
	if (!texture) return *this;

	// Find index
	ref_constant C			= m_constants.get(ResourceName);
	if (!C)					return *this;

	R_ASSERT				(C->type == RC_dx10texture);
	u32 stage				= C->samp.index;

	if (stage >= m_Textures.size())
		m_Textures.resize(stage+1);

	m_Textures[stage] = texture->get_SRView(); //!!!dangerous view can be deleted

	return *this;
}

void CSCompiler::end()
{
	for (size_t i=0; i<m_Textures.size(); ++i)
		m_Textures[i]->AddRef();

	for (size_t i=0; i<m_Outputs.size(); ++i)
		m_Outputs[i]->AddRef();

	//Samplers create by us, thou they should not be AddRef'ed

	m_Target.Construct(m_cs, DEV->_CreateConstantTable(m_constants), m_Samplers, m_Textures, m_Outputs);
}

void CSCompiler::compile(const char* name)
{
	if (0==stricmp(name, "null"))
	{
		m_cs = 0;
		return;
	}

	string_path					cname;
	xr_strconcat				(cname, ::Render->getShaderPath(), "cs_", name,".hlsl");
	FS.update_path				(cname,	"$game_shaders$", cname);

	IReader* file				= FS.r_open(cname);
	R_ASSERT2					( file, cname );

	// Select target
	LPCSTR						c_target	= "cs_5_0";
	LPCSTR						c_entry		= "main";

    DWORD shaderCompileFlags = D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
    if (isGraphicDebugging)
    {
        shaderCompileFlags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION | D3D10_SHADER_PREFER_FLOW_CONTROL;
    }
	HRESULT	const _hr			= ::Render->shader_compile(name,(DWORD const*)file->pointer(),file->length(), c_entry, c_target, shaderCompileFlags, (void*&)m_cs );

	FS.r_close					( file );

	VERIFY(SUCCEEDED(_hr));

	CHECK_OR_EXIT				(
		!FAILED(_hr),
		make_string("Your video card doesn't meet game requirements.\n\nTry to lower game settings.")
	);
}
