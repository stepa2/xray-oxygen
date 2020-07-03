#include	"stdafx.h"

#include	"interactive_animation.h"

#include	"../xrphysics/physicsshell.h"
#include	"../xrphysics/ExtendedGeom.h"
#include	"../xrphysics/mathutils.h"

#include	"../Include/xrRender/KinematicsAnimated.h"

interactive_animation::interactive_animation(CPhysicsShellHolder* O, CBlend* b) : physics_shell_animated(O, false), blend(b)
{
}

interactive_animation::~interactive_animation()
{
}
static float depth = 0;
bool interactive_animation::collide()
{
//	depth = 0;
	physics_shell->CollideAll();
	return depth > 0.05;
}

bool interactive_animation::update(const Fmatrix &xrorm)
{
	if (!blend)
		return false;

	if (!inherited::update(xrorm))
		return false;

	if (blend->playing && collide())
	{
		if (blend->Callback)
			blend->Callback(blend);

		blend->blendPower *= 0.5f;
		blend->playing = FALSE;
	}
	else if (!blend->playing)
	{
		blend = 0;
		return false;
	}

	return true;
}

void interactive_animation::contact_callback(bool& do_colide, bool bo1, dContact& c, SGameMtl* /*material_1*/, SGameMtl* /*material_2*/)
{
	dxGeomUserData* gd1 = nullptr, * gd2 = nullptr;
	get_user_data(gd1, gd2, bo1, c.geom);
	VERIFY(gd1);

	if (!gd2 || gd2->ph_ref_object != gd1->ph_ref_object)
		depth = std::max(depth, c.geom.depth);
}

void interactive_animation::create_shell(CPhysicsShellHolder* O)
{
	inherited::create_shell(O);
	physics_shell->add_ObjectContactCallback(contact_callback);
}