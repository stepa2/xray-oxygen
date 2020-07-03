#include "stdafx.h"
#include "alife_space.h"
#include "hit.h"
#include "../xrEngine/bone.h"
#include "xrMessages.h"
#include "../xrGame/Level.h"
#include "../xrPhysics/mathutils.h"
#include "../xrEngine/xr_object.h"

SHit::SHit(float powerA, Fvector &dirA, CObject *whoA, u16 elementA, Fvector p_in_bone_spaceA, //-V730
	float impulseA, ALife::EHitType hit_typeA, float armor_piercingA, bool AimBullet)
{
	power = powerA;
	dir.set(dirA);
	who = whoA;
	if (whoA)
		whoID = whoA->ID();
	else
		whoID = 0;
	boneID = elementA;
	p_in_bone_space.set(p_in_bone_spaceA);
	impulse = impulseA;

	hit_type = hit_typeA;
	armor_piercing = armor_piercingA;
	PACKET_TYPE = 0;
	BulletID = 0;
	SenderID = 0;
	aim_bullet = AimBullet;
	add_wound = true;
	BulletFlightSpeed = 0.0f;
	BulletFlightDist = 0.0f;
}

SHit::SHit()
{
	invalidate();
}

void SHit::invalidate()
{
	Time					= 0;
	PACKET_TYPE				= 0;
	DestID					= 0;

	power					=-phInfinity;
	dir						.set(-phInfinity,-phInfinity,-phInfinity);
	who						=nullptr;
	whoID					= 0;
	weaponID				= 0;

	boneID					=BI_NONE;
	p_in_bone_space		.set(-phInfinity,-phInfinity,-phInfinity);

	impulse					=-phInfinity;
	hit_type				=ALife::eHitTypeMax;

	armor_piercing			= 0.0f;	
	BulletID				= 0;
	SenderID				= 0;
	aim_bullet				= false;
	add_wound				= false;
	BulletFlightSpeed = 0.0f;
	BulletFlightDist = 0.0f;
}

void	SHit::GenHeader				(u16 PacketType, u16 ID)
{
	DestID = ID;
	PACKET_TYPE = PacketType;
	Time = Level().timeServer();
};

void SHit::Read_Packet				(NET_Packet	Packet)
{
	u16 type_dummy;	
	Packet.r_begin			(type_dummy);
	Packet.r_u32			(Time);
	Packet.r_u16			(PACKET_TYPE);
	Packet.r_u16			(DestID);
	Read_Packet_Cont		(Packet);
};

void SHit::Read_Packet_Cont		(NET_Packet	Packet)
{

	Packet.r_u16			(whoID);
	Packet.r_u16			(weaponID);
	Packet.r_dir			(dir);
	Packet.r_float			(power);
	Packet.r_u16			(boneID);
	Packet.r_vec3			(p_in_bone_space);
	Packet.r_float			(impulse);
	
	aim_bullet				= Packet.r_u16()!=0;
	hit_type				= (ALife::EHitType)Packet.r_u16();	//hit type

	if (hit_type == ALife::eHitTypeFireWound)
	{
		Packet.r_float(armor_piercing);
		Packet.r_float(BulletFlightSpeed);
		Packet.r_float(BulletFlightDist);
	}
}

void SHit::Write_Packet_Cont		(NET_Packet	&Packet)
{
	Packet.w_u16		(whoID);
	Packet.w_u16		(weaponID);
	Packet.w_dir		(dir);
	Packet.w_float		(power);
	Packet.w_u16		(boneID);
	Packet.w_vec3		(p_in_bone_space);
	Packet.w_float		(impulse);	
	Packet.w_u16		(aim_bullet!=0);
	Packet.w_u16		(u16(hit_type&0xffff));	
	if (hit_type == ALife::eHitTypeFireWound)
	{
		Packet.w_float(armor_piercing);
		Packet.w_float(BulletFlightSpeed);
		Packet.w_float(BulletFlightDist);
	}
}
void SHit::Write_Packet			(NET_Packet	&Packet)
{
	Packet.w_begin	(M_EVENT);
	Packet.w_u32		(Time);
	Packet.w_u16		(u16(PACKET_TYPE&0xffff));
	Packet.w_u16		(u16(DestID&0xffff));

	Write_Packet_Cont (Packet);	
};

#ifdef DEBUG
void SHit::_dump()
{
	Msg("SHit::_dump()---begin");
	Msg("power=%f", power);
	Msg("impulse=%f", impulse);
	Log("dir=",dir);
	Msg("whoID=%hu",whoID);
	Msg("weaponID==%hu",weaponID);
	Msg("element==%hu",boneID);
	Log("p_in_bone_space=",p_in_bone_space);
	Msg("hit_type=%d",(int)hit_type);
	Msg("armor_piercing=%f",armor_piercing);
	Msg("SHit::_dump()---end");
}
#endif
