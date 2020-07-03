#include "stdafx.h"
#include "compiler.h"

#include "cl_intersect.h"

CDB::MODEL			Level;
CDB::COLLIDER		XRC;

Nodes				g_nodes;
xr_vector<SCover>	g_covers_palette;

Lights				g_lights;
SAIParams			g_params;
Fbox				LevelBB;
//Vectors				Emitters;

void vertex::PointLF(Fvector& D)
{
	Fvector	d;	d.set(0,-1,0);
	Fvector	v	= Pos;	
	float	s	= g_params.fPatchSize/2;
	v.x			-= s;
	v.z			+= s;
	Plane.intersectRayPoint(v,d,D);
}

void vertex::PointFR(Fvector& D)
{
	Fvector	d;	d.set(0,-1,0);
	Fvector	v	= Pos;	
	float	s	= g_params.fPatchSize/2;
	v.x			+= s;
	v.z			+= s;
	Plane.intersectRayPoint(v,d,D);
}

void vertex::PointRB(Fvector& D)
{
	Fvector	d;	d.set(0,-1,0);
	Fvector	v	= Pos;	
	float	s	= g_params.fPatchSize/2;
	v.x			+= s;
	v.z			-= s;
	Plane.intersectRayPoint(v,d,D);
}

void vertex::PointBL(Fvector& D)
{
	Fvector	d;	d.set(0,-1,0);
	Fvector	v	= Pos;	
	float	s	= g_params.fPatchSize/2;
	v.x			-= s;
	v.z			-= s;
	Plane.intersectRayPoint(v,d,D);
}

void	mem_Optimize	()
{
	Memory.mem_compact	();
	Msg("* Memory usage: %d M",Memory.mem_usage()/(1024*1024));
}

void xrCompiler(LPCSTR name, bool draft_mode, bool pure_covers, LPCSTR out_name)
{
	Logger.Phase("Loading level...");
	xrLoad(name, draft_mode);
	mem_Optimize();

	if (!draft_mode)
	{
		Logger.Phase("Lighting nodes...");
		mem_Optimize();

		Logger.Phase("Calculating coverage...");
		xrCover(pure_covers);
		mem_Optimize();
	}
	/////////////////////////////////////
	Logger.Phase("Saving nodes...");
	xrSaveNodes(name, out_name);
	mem_Optimize();
}