#include "stdafx.h"
#include "bolt.h"
#include "../xrParticles/psystem.h"
#include "../xrParticles/ParticlesObject.h"
#include "../xrphysics/PhysicsShell.h"
#include "..\xrEngine\xr_level_controller.h"

CBolt::CBolt() 
{
	m_thrower_id = u16(-1);
}

CBolt::~CBolt() 
{
}

void CBolt::OnH_A_Chield() 
{
	inherited::OnH_A_Chield();
	CObject* o= H_Parent()->H_Parent();
	if(o)SetInitiator(o->ID());
	
}

void CBolt::Throw() 
{
	CMissile					*l_pBolt = smart_cast<CMissile*>(m_fake_missile);
	if(!l_pBolt)				return;
	l_pBolt->set_destroy_time	(u32(m_dwDestroyTimeMax/phTimefactor));
	inherited::Throw			();
	spawn_fake_missile			();
}

bool CBolt::Useful() const
{
	return false;
}

bool CBolt::Action(u16 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;
	return false;
}

void CBolt::activate_physic_shell()
{
	inherited::activate_physic_shell();
	m_pPhysicsShell->SetAirResistance(.0001f);
}

void CBolt::SetInitiator(u16 id)
{
	m_thrower_id=id;
}

u16	CBolt::Initiator()
{
	return m_thrower_id;
}