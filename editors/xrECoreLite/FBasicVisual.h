#pragma once
// The class itself
class CKinematicsAnimated;
class CKinematics;
class IParticleCustom;

#include "../../xrEngine/vis_common.h"
#include "../../Include/xrRender/RenderVisual.h"
#include <dx/d3d9.h>

#define ref_geom void*
#define ref_shader void*
#define VLOAD_NOVERTICES		(1<<0)


// The class itself
class ECORE_API dxRender_Visual //: public IRenderVisual
{
public:
    ogf_desc					desc		;
	shared_str					dbg_name	;
	virtual shared_str	_BCL	getDebugName() { return dbg_name; }
public:
	// Common data for rendering
	u32							Type		;				// visual's type
	vis_data					vis			;				// visibility-data
	ref_shader					shader		;				// pipe state, shared

	virtual void				Render						(float LOD)		{};		// LOD - Level Of Detail  [0..1], Ignored
	virtual void				Load						(const char* N, IReader *data, u32 dwFlags);
	virtual void				Release						();						// Shared memory release
	virtual void				Copy						(dxRender_Visual* from);
	virtual void				Spawn						()				{};
	virtual void				Depart						()				{};

	virtual vis_data&	_BCL	getVisData() { return vis;}
	virtual u32					getType()	 { return Type;}

	dxRender_Visual				();
	virtual ~dxRender_Visual		();
};
