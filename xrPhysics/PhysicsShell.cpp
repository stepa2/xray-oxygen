#include "stdafx.h"
#pragma hdrstop
#include "physicsshell.h"
#include "PHDynamicData.h"
#include "Physics.h"
#include "PHJoint.h"
#include "PHShell.h"
#include "PHJoint.h"
#include "PHJointDestroyInfo.h"
#include "PHSplitedShell.h"

#include "iphysicsshellholder.h"

#include "phvalide.h"

#include "../Include/xrRender/Kinematics.h"
#include "../xrengine/xr_object.h"
#include "../xrengine/bone.h"

extern CPHWorld			*ph_world;
IPhysicsShellEx::~IPhysicsShellEx()
{
}

IPhysicsElementEx*			P_create_Element()
{
	CPHElement* element = xr_new<CPHElement>();
	return element;
}

IPhysicsShellEx*				P_create_Shell()
{
	IPhysicsShellEx* shell = xr_new<CPHShell>();
	return shell;
}

IPhysicsShellEx*				P_create_splited_Shell()
{
	IPhysicsShellEx* shell = xr_new<CPHSplitedShell>();
	return shell;
}

IPhysicsJoint*				P_create_Joint(IPhysicsJoint::enumType type, IPhysicsElementEx* first, IPhysicsElementEx* second)
{
	IPhysicsJoint* joint = xr_new<CPHJoint>(type, first, second);
	return joint;
}

IPhysicsShellEx*	__stdcall P_build_Shell(IPhysicsShellHolder* obj, bool not_active_state, BONE_P_MAP* bone_map, bool not_set_bone_callbacks)
{
	VERIFY(obj);
	phys_shell_verify_object_model(*obj);

	IKinematics* pKinematics = obj->ObjectKinematics();

	IPhysicsShellEx* pPhysicsShell = P_create_Shell();
#ifdef DEBUG
	pPhysicsShell->dbg_obj = obj;
#endif
	pPhysicsShell->build_FromKinematics(pKinematics, bone_map);

	pPhysicsShell->set_PhysicsRefObject(obj);
	pPhysicsShell->mXFORM.set(obj->ObjectXFORM());
	pPhysicsShell->Activate(not_active_state, not_set_bone_callbacks);//,

	pPhysicsShell->SetAirResistance();//0.0014f,1.5f

	return pPhysicsShell;
}

void	fix_bones(LPCSTR	fixed_bones, IPhysicsShellEx* shell)
{
	VERIFY(fixed_bones);
	VERIFY(shell);
	IKinematics	*pKinematics = shell->PKinematics();
	VERIFY(pKinematics);
	int count = _GetItemCount(fixed_bones);
	for (int i = 0; i < count; ++i)
	{
		string64 fixed_bone;
		_GetItem(fixed_bones, i, fixed_bone);
		u16 fixed_bone_id = pKinematics->LL_BoneID(fixed_bone);
		R_ASSERT2(BI_NONE != fixed_bone_id, "wrong fixed bone");
		IPhysicsElementEx* E = shell->get_Element(fixed_bone_id);
		if (E)
			E->Fix();
	}
}
IPhysicsShellEx*	P_build_Shell(IPhysicsShellHolder* obj, bool not_active_state, BONE_P_MAP* p_bone_map, LPCSTR	fixed_bones)
{
	IPhysicsShellEx* pPhysicsShell = 0;

	IKinematics* pKinematics = obj->ObjectKinematics();
	if (fixed_bones)
	{
		int count = _GetItemCount(fixed_bones);
		for (int i = 0; i < count; ++i)
		{
			string64 fixed_bone;
			_GetItem(fixed_bones, i, fixed_bone);
			u16 fixed_bone_id = pKinematics->LL_BoneID(fixed_bone);
			R_ASSERT2(BI_NONE != fixed_bone_id, "wrong fixed bone");
			p_bone_map->insert(std::make_pair(fixed_bone_id, physicsBone()));
		}

		pPhysicsShell = P_build_Shell(obj, not_active_state, p_bone_map);
	}
	else
		pPhysicsShell = P_build_Shell(obj, not_active_state);

	auto i = p_bone_map->begin(), e = p_bone_map->end();
	if (i != e) pPhysicsShell->SetPrefereExactIntegration();
	for (; i != e; i++)
	{
		IPhysicsElementEx* fixed_element = i->second.element;
		R_ASSERT2(fixed_element, "fixed bone has no physics");

		fixed_element->Fix();
	}
	return pPhysicsShell;
}

IPhysicsShellEx*	P_build_Shell(IPhysicsShellHolder* obj, bool not_active_state, LPCSTR	fixed_bones)
{
	U16Vec f_bones;
	if (fixed_bones)
	{
		IKinematics* K = obj->ObjectKinematics();
		VERIFY(K);
		int count = _GetItemCount(fixed_bones);
		for (int i = 0; i < count; ++i) {
			string64		fixed_bone;
			_GetItem(fixed_bones, i, fixed_bone);
			f_bones.push_back(K->LL_BoneID(fixed_bone));
			R_ASSERT2(BI_NONE != f_bones.back(), "wrong fixed bone");
		}
	}
	return P_build_Shell(obj, not_active_state, f_bones);
}

static BONE_P_MAP bone_map = BONE_P_MAP();
IPhysicsShellEx*	P_build_Shell(IPhysicsShellHolder* obj, bool not_active_state, U16Vec& fixed_bones)
{
	bone_map.clear();
	IPhysicsShellEx*			pPhysicsShell = 0;
	if (!fixed_bones.empty())
		for (auto it = fixed_bones.begin(); it != fixed_bones.end(); it++)
			bone_map.insert(std::make_pair(*it, physicsBone()));
	pPhysicsShell = P_build_Shell(obj, not_active_state, &bone_map);

	// fix bones
	auto i = bone_map.begin(), e = bone_map.end();
	if (i != e)
		pPhysicsShell->SetPrefereExactIntegration();
	for (; i != e; i++)
	{
		IPhysicsElementEx* fixed_element = i->second.element;

		if (!fixed_element) continue;
		fixed_element->Fix();
	}
	return pPhysicsShell;
}

IPhysicsShellEx*	P_build_SimpleShell(IPhysicsShellHolder* obj, float mass, bool not_active_state)
{
	IPhysicsShellEx* pPhysicsShell = P_create_Shell();
#ifdef DEBUG
	pPhysicsShell->dbg_obj = (obj);
#endif

	VERIFY(obj);
	VERIFY(obj->ObjectKinematics());

	Fobb obb; obj->ObjectKinematics()->GetBox().get_CD(obb.m_translate, obb.m_halfsize);
	obb.m_rotate.identity();
	IPhysicsElementEx* E = P_create_Element(); R_ASSERT(E); E->add_Box(obb);
	pPhysicsShell->add_Element(E);
	pPhysicsShell->setMass(mass);
	pPhysicsShell->set_PhysicsRefObject(obj);
	if (!obj->has_parent_object())
		pPhysicsShell->Activate(obj->ObjectXFORM(), 0, obj->ObjectXFORM(), not_active_state);
	return pPhysicsShell;
}

void ApplySpawnIniToPhysicShell(CInifile* ini, IPhysicsShellEx* physics_shell, bool fixed)
{
	if (!ini)
		return;
	if (ini->section_exist("physics_common"))
	{
		fixed = fixed || (ini->line_exist("physics_common", "fixed_bones"));
#pragma todo("not ignore static if non realy fixed! ")
		fix_bones(ini->r_string("physics_common", "fixed_bones"), physics_shell);
	}
	if (ini->section_exist("collide"))
	{
		if ((ini->line_exist("collide", "ignore_static") && fixed) || (ini->line_exist("collide", "ignore_static") && ini->section_exist("animated_object")))
		{
			physics_shell->SetIgnoreStatic();
		}
		if (ini->line_exist("collide", "small_object"))
		{
			physics_shell->SetSmall();
		}
		if (ini->line_exist("collide", "ignore_small_objects"))
		{
			physics_shell->SetIgnoreSmall();
		}
		if (ini->line_exist("collide", "ignore_ragdoll"))
		{
			physics_shell->SetIgnoreRagDoll();
		}

		//If need, then show here that it is needed to ignore collisions with "animated_object"
		if (ini->line_exist("collide", "ignore_animated_objects"))
		{
			physics_shell->SetIgnoreAnimated();
		}
	}
	//If next section is available then given "PhysicShell" is classified
	//as animated and we read options for his animation

	if (ini->section_exist("animated_object"))
	{
		//Show that given "PhysicShell" animated
		physics_shell->CreateShellAnimator(ini, "animated_object");
	}
}

void	get_box(const IPhysicsBase*	shell, const	Fmatrix& form, Fvector&	sz, Fvector&	c)
{
	t_get_box(shell, form, sz, c);
}

void __stdcall	destroy_physics_shell(IPhysicsShellEx* &p)
{
	if (p)
		p->Deactivate();
	xr_delete(p);
}

bool bone_has_pysics(IKinematics& K, u16 bone_id)
{
	return K.LL_GetBoneVisible(bone_id) && shape_is_physic(K.GetBoneData(bone_id).get_shape());
}

bool has_physics_collision_shapes(IKinematics& K)
{
	u16 nbb = K.LL_BoneCount();
	for (u16 i = 0; i < nbb; ++i)
		if (bone_has_pysics(K, i))
			return true;
	return false;
}

void	phys_shell_verify_model(IKinematics& K)
{
	VERIFY_FORMAT(has_physics_collision_shapes(K), "Can not create physics shell for model %s because it has no physics collision shapes set", K.getDebugName().c_str());
}

void	phys_shell_verify_object_model(IPhysicsShellHolder& O)
{
	IKinematics* K = O.ObjectKinematics();

	VERIFY_FORMAT(K, "Can not create physics shell for object %s, model %s is not skeleton", O.ObjectName(), O.ObjectNameVisual());

	VERIFY_FORMAT(has_physics_collision_shapes(*K),
		"Can not create physics shell for object %s, model %s has no physics collision shapes set", 
			O.ObjectName(), O.ObjectNameVisual());

	VERIFY2(_valid(O.ObjectXFORM()), "create physics shell: object matrix is not valid");

	VERIFY2(valid_pos(O.ObjectXFORM().c), dbg_valide_pos_string(O.ObjectXFORM().c, &O, "create physics shell").c_str());
}

bool __stdcall	can_create_phys_shell(string1024 &reason, IPhysicsShellHolder& O)
{
	xr_strcpy(reason, "ok");
	bool result = true;
	IKinematics* K = O.ObjectKinematics();
	if (!K)
	{
		xr_sprintf(reason, "Can not create physics shell for object %s, model %s is not skeleton", O.ObjectName(), O.ObjectNameVisual());
		return false;
	}
	if (!has_physics_collision_shapes(*K))
	{
		xr_sprintf(reason, "Can not create physics shell for object %s, model %s has no physics collision shapes set", O.ObjectName(), O.ObjectNameVisual());
		return false;
	}
	if (!_valid(O.ObjectXFORM()))
	{
		xr_strcpy(reason, "create physics shell: object matrix is not valid");
		return false;
	}
	if (!valid_pos(O.ObjectXFORM().c))
	{
#ifdef	DEBUG
		xr_strcpy(reason, dbg_valide_pos_string(O.ObjectXFORM().c, &O, "create physics shell").c_str());
#else
		xr_strcpy(reason, "create physics shell: object position is not valid");
#endif
		return false;
	}
	return result;
}

float NonElasticCollisionEnergy(IPhysicsElementEx *e1, IPhysicsElementEx *e2, const Fvector &norm)// norm - from 2 to 1
{
	VERIFY(e1);
	VERIFY(e2);
	dBodyID b1 = static_cast<CPHElement*> (e1)->get_body();
	VERIFY(b1);
	dBodyID b2 = static_cast<CPHElement*> (e2)->get_body();
	VERIFY(b2);
	return E_NL(b1, b2, cast_fp(norm));
}

void	StaticEnvironmentCB(bool& do_colide, bool bo1, dContact& c, SGameMtl* material_1, SGameMtl* material_2)
{
	dJointID contact_joint = dJointCreateContact(0, ContactGroup, &c);

	if (bo1)
	{
		((CPHIsland*)(retrieveGeomUserData(c.geom.g1)->callback_data))->DActiveIsland()->ConnectJoint(contact_joint);
		dJointAttach(contact_joint, dGeomGetBody(c.geom.g1), 0);
	}
	else
	{
		((CPHIsland*)(retrieveGeomUserData(c.geom.g2)->callback_data))->DActiveIsland()->ConnectJoint(contact_joint);
		dJointAttach(contact_joint, 0, dGeomGetBody(c.geom.g2));
	}
	do_colide = false;
}