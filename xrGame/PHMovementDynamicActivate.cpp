
#include "stdafx.h"

#include "phmovementcontrol.h"

#include "../xrphysics/phcharacter.h"
#include "../xrphysics/iphysicsshellholder.h"
bool CPHMovementControl:: ActivateBoxDynamic(DWORD id,int num_it/*=8*/,int num_steps/*5*/,float resolve_depth/*=0.01f*/)
{
	bool  character_exist=CharacterExist();
	if(character_exist&&trying_times[id]!=u32(-1))
	{
		Fvector character_body_pos;
		m_character->get_body_position( character_body_pos );
		Fvector dif;dif.sub(trying_poses[id],character_body_pos);
		if(Device.dwTimeGlobal-trying_times[id]<500&&dif.magnitude()<0.05f)
																	return false;
	}

	if(!m_character|| m_character->PhysicsRefObject()->ObjectPPhysicsShell())
		return false;

	DWORD old_id=BoxID();

	bool character_disabled=character_exist && !m_character->IsEnabled();

	if( character_exist && id==old_id )
		return true;

	if(!character_exist)
	{
		CreateCharacter();
	}

	Fvector vel;
	Fvector pos;

	GetCharacterVelocity(vel);
 
	GetCharacterPosition(pos);

	bool ret =	::ActivateBoxDynamic( this, character_exist,id, num_it, num_steps, resolve_depth );

	if(!ret)
	{	
		if(!character_exist)
			DestroyCharacter();
		else if(character_disabled)
			m_character->Disable();

		ActivateBox(old_id);
		SetVelocity(vel);
		m_character->fix_body_rotation();
		SetPosition(pos);
	}
	else
	{
		ActivateBox(id);
	}

	SetVelocity(vel);

	if(!ret&&character_exist)
	{
		trying_times[id]=Device.dwTimeGlobal;
		m_character->GetBodyPosition(trying_poses[id]);
	}
	else
	{
		trying_times[id]=u32(-1);
	}
	return ret;
}
