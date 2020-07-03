/*
	* Authors:
	* Date of creation:
	* Description:
	* Copyright:
*/
#include "files_list.hpp"
#include "bone.h"
#include "xrCore/envelope.h"

#define BONE_VERSION 0x0002
//------------------------------------------------------------------------------
#define BONE_CHUNK_VERSION 0x0001
#define BONE_CHUNK_DEF 0x0002
#define BONE_CHUNK_BIND_POSE 0x0003
#define BONE_CHUNK_MATERIAL 0x0004
#define BONE_CHUNK_SHAPE 0x0005
#define BONE_CHUNK_IK_JOINT 0x0006
#define BONE_CHUNK_MASS 0x0007
#define BONE_CHUNK_FLAGS 0x0008
#define BONE_CHUNK_IK_JOINT_BREAK 0x0009
#define BONE_CHUNK_IK_JOINT_FRICTION 0x0010

CBone::CBone()
{
	flags.zero();
	rest_length = 0;
	SelfID = -1;
	parent = 0;

	ResetData();
}

CBone::~CBone()
{
}

void CBone::ResetData()
{
	IK_data.Reset();
	game_mtl = "default_object";
	shape.Reset();

	mass = 10.f;;
	center_of_mass.set(0.f, 0.f, 0.f);
}

void CBone::Save(IWriter& F)
{
#ifdef _LW_EXPORT
	extern void ReplaceSpaceAndLowerCase(shared_str& s);
	ReplaceSpaceAndLowerCase(name);
	ReplaceSpaceAndLowerCase(parent_name);
#endif

	F.open_chunk(BONE_CHUNK_VERSION);
	F.w_u16(BONE_VERSION);
	F.close_chunk();

	F.open_chunk(BONE_CHUNK_DEF);
	F.w_stringZ(name);
	F.w_stringZ(parent_name);
	F.w_stringZ(wmap);
	F.close_chunk();

	F.open_chunk(BONE_CHUNK_BIND_POSE);
	F.w_fvector3(rest_offset);
	F.w_fvector3(rest_rotate);
	F.w_float(rest_length);
	F.close_chunk();

	SaveData(F);
}

void CBone::Load_0(IReader& F)
{
	F.r_stringZ(name);
	xr_strlwr(name);

	F.r_stringZ(parent_name);
	xr_strlwr(parent_name);

	F.r_stringZ(wmap);
	F.r_fvector3(rest_offset);
	F.r_fvector3(rest_rotate);

	rest_length = F.r_float();
	std::swap(rest_rotate.x, rest_rotate.y);
	Reset();
}

void CBone::Load_1(IReader& F)
{
	R_ASSERT(F.find_chunk(BONE_CHUNK_VERSION));

	u16	ver = F.r_u16();

	if ((ver != 0x0001) && (ver != BONE_VERSION))
		return;

	R_ASSERT(F.find_chunk(BONE_CHUNK_DEF));

	F.r_stringZ(name);
	xr_strlwr(name);

	F.r_stringZ(parent_name);
	xr_strlwr(parent_name);

	F.r_stringZ(wmap);

	R_ASSERT(F.find_chunk(BONE_CHUNK_BIND_POSE));

	F.r_fvector3(rest_offset);
	F.r_fvector3(rest_rotate);
	rest_length = F.r_float();

	if (ver == 0x0001)
		std::swap(rest_rotate.x, rest_rotate.y);

	LoadData(F);
}

void CBone::SaveData(IWriter& F)
{
	F.open_chunk(BONE_CHUNK_DEF);
	F.w_stringZ(name);
	F.close_chunk();

	F.open_chunk(BONE_CHUNK_MATERIAL);
	F.w_stringZ(game_mtl);
	F.close_chunk();

	F.open_chunk(BONE_CHUNK_SHAPE);
	F.w(&shape, sizeof(SBoneShape));
	F.close_chunk();

	F.open_chunk(BONE_CHUNK_FLAGS);
	F.w_u32(IK_data.ik_flags.get());
	F.close_chunk();

	F.open_chunk(BONE_CHUNK_IK_JOINT);
	F.w_u32(IK_data.type);
	F.w(IK_data.limits, sizeof(SJointLimit) * 3);
	F.w_float(IK_data.spring_factor);
	F.w_float(IK_data.damping_factor);
	F.close_chunk();

	F.open_chunk(BONE_CHUNK_IK_JOINT_BREAK);
	F.w_float(IK_data.break_force);
	F.w_float(IK_data.break_torque);
	F.close_chunk();

	F.open_chunk(BONE_CHUNK_IK_JOINT_FRICTION);
	F.w_float(IK_data.friction);
	F.close_chunk();

	F.open_chunk(BONE_CHUNK_MASS);
	F.w_float(mass);
	F.w_fvector3(center_of_mass);
	F.close_chunk();
}

void CBone::LoadData(IReader& F)
{
	R_ASSERT(F.find_chunk(BONE_CHUNK_DEF));

	F.r_stringZ(name); xr_strlwr(name);

	R_ASSERT(F.find_chunk(BONE_CHUNK_MATERIAL));

	F.r_stringZ(game_mtl);

	R_ASSERT(F.find_chunk(BONE_CHUNK_SHAPE));

	F.r(&shape, sizeof(SBoneShape) + 8);

	if (F.find_chunk(BONE_CHUNK_FLAGS))
		IK_data.ik_flags.assign(F.r_u32());

	R_ASSERT(F.find_chunk(BONE_CHUNK_IK_JOINT));

	IK_data.type = (EJointType)F.r_u32();
	F.r(IK_data.limits, sizeof(SJointLimit) * 3);
	IK_data.spring_factor = F.r_float();
	IK_data.damping_factor = F.r_float();

	if (F.find_chunk(BONE_CHUNK_IK_JOINT_BREAK))
	{
		IK_data.break_force = F.r_float();
		IK_data.break_torque = F.r_float();
	}

	if (F.find_chunk(BONE_CHUNK_IK_JOINT_FRICTION))
	{
		IK_data.friction = F.r_float();
	}

	if (F.find_chunk(BONE_CHUNK_MASS))
	{
		mass = F.r_float();
		F.r_fvector3(center_of_mass);
	}
}

void CBone::CopyData(CBone* bone)
{
	game_mtl = bone->game_mtl;
	shape = bone->shape;
	IK_data = bone->IK_data;
	mass = bone->mass;
	center_of_mass = bone->center_of_mass;
}

#include "../../engine.vc2008/xrCDB/xrCDB.h"
#include "../../engine.vc2008/xrSound/Sound.h"
#include "GameMtlLib.h"
#include <xrCore\FS_internal.h>

bool CBone::ExportOGF(IWriter& F)
{
	// check valid

	if (shape.Valid())
		MessageBox(0, "! Bone '%s' has invalid shape.", xr_string("Invalid Shape!" + xr_string(*Name())).c_str(), 0);
	else
	{
		const SEGameMtl* M = GEMLib.GetMaterial(game_mtl.c_str());
		R_ASSERT3(!M, "! Bone '%s' has invalid game material.", *Name());
		R_ASSERT3(!M->Flags.is(SEGameMtl::flDynamic), "! Bone '%s' has non-dynamic game material.", *Name());
	}
	F.w_u32(OGF_IKDATA_VERSION);
	F.w_stringZ(game_mtl);
	F.w(&shape, sizeof(SBoneShape));

	IK_data.Export(F);

	F.w_fvector3(rest_rotate);
	F.w_fvector3(rest_offset);
	F.w_float(mass);
	F.w_fvector3(center_of_mass);
	return true;
}