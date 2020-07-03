#include "stdafx.h"
#include "WeaponRG6.h"
#include "entity.h"
#include "explosiveRocket.h"
#include "level.h"
#include "../xrphysics/MathUtils.h"
#include "actor.h"

#ifdef DEBUG
#	include "phdebug.h"
#endif


CWeaponRG6::~CWeaponRG6()
{
}

BOOL	CWeaponRG6::net_Spawn				(CSE_Abstract* DC)
{
	BOOL l_res = inheritedSG::net_Spawn(DC);
	if (!l_res) return l_res;

	if (iAmmoElapsed && !getCurrentRocket())
	{
		shared_str grenade_name = m_ammoTypes[0];
		shared_str fake_grenade_name = pSettings->r_string(grenade_name, "fake_grenade_name");

		if (fake_grenade_name.size())
		{
			int k=iAmmoElapsed;
			while (k)
			{
				k--;
				inheritedRL::SpawnRocket(fake_grenade_name.c_str(), this);
			}
		}
	}
	

	
	return l_res;
};

void CWeaponRG6::Load(LPCSTR section)
{
	inheritedRL::Load(section);
	inheritedSG::Load(section);
}
#include "inventory.h"
#include "inventoryOwner.h"

bool CWeaponRG6::install_upgrade_impl(LPCSTR section, bool test)
{
	bool result = inheritedSG::install_upgrade_impl( section, test );

	result |= process_if_exists( section, "launch_speed", &CInifile::r_float, m_fLaunchSpeed, test );
	return result;
}

void CWeaponRG6::LaunchGrenade(const Fvector& p1, const Fvector& d1)
{
	if (getRocketCount())
	{
		Fvector p, d;
		p = p1;
		d = d1;

		Fmatrix launch_matrix;
		launch_matrix.identity();
		launch_matrix.k.set(d);
		Fvector::generate_orthonormal_basis(launch_matrix.k,
											launch_matrix.j, launch_matrix.i);
		launch_matrix.c.set(p);

		if (IsZoomed() && smart_cast<CActor*>(H_Parent()))
		{
			H_Parent()->setEnabled(FALSE);
			setEnabled(FALSE);
		
			collide::rq_result RQ;
			BOOL HasPick = Level().ObjectSpace.RayPick(p, d, 300.0f, collide::rqtStatic, RQ, this);

			setEnabled(TRUE);
			H_Parent()->setEnabled(TRUE);

			if (HasPick)
			{
				Fvector Transference;
				Transference.mul(d, RQ.range);
				Fvector res[2];

				u8 canfire0 = TransferenceAndThrowVelToThrowDir(Transference, CRocketLauncher::m_fLaunchSpeed, EffectiveGravity(), res);

				if (canfire0 != 0)
				{
					d = res[0];
				};
			}
		};

		d.normalize();
		d.mul(m_fLaunchSpeed);
		VERIFY2(_valid(launch_matrix),"CWeaponRG6::LaunchGrenade. Invalid launch_matrix");
		CRocketLauncher::LaunchRocket(launch_matrix, d, zero_vel);

		CExplosiveRocket* pGrenade = smart_cast<CExplosiveRocket*>(getCurrentRocket());
		VERIFY(pGrenade);
		pGrenade->SetInitiator(H_Parent()->ID());

		NET_Packet P;
		u_EventGen(P, GE_LAUNCH_ROCKET, ID());
		P.w_u16(u16(getCurrentRocket()->ID()));
		u_EventSend(P);

		dropCurrentRocket();
	}
}

void CWeaponRG6::FireTrace(const Fvector& P, const Fvector& D)
{
	inheritedSG::FireTrace(P, D);
	if (!IsMisfire())
	{
		LaunchGrenade(P, D);
	}
}

u8 CWeaponRG6::AddCartridge		(u8 cnt)
{
	u8 t = inheritedSG::AddCartridge(cnt);
	u8 k = cnt-t;
	shared_str fake_grenade_name = pSettings->r_string(m_ammoTypes[m_ammoType].c_str(), "fake_grenade_name");
	while(k){
		--k;
		inheritedRL::SpawnRocket(*fake_grenade_name, this);
	}
	return k;
}

void CWeaponRG6::OnEvent(NET_Packet& P, u16 type) 
{
	inheritedSG::OnEvent(P,type);

	u16 id;
	switch (type) {
		case GE_OWNERSHIP_TAKE : {
			P.r_u16(id);
			inheritedRL::AttachRocket(id, this);
		} break;
		case GE_OWNERSHIP_REJECT : 
		case GE_LAUNCH_ROCKET : 
			{
			bool bLaunch = (type==GE_LAUNCH_ROCKET);
			P.r_u16						(id);
			inheritedRL::DetachRocket	(id, bLaunch);
		} break;
	}
}


#include "luabind/luabind.hpp"
using namespace luabind;

#pragma optimize("s",on)
void CWeaponRG6::script_register(lua_State *L)
{
	module(L)
		[
			class_<CWeaponRG6, CGameObject>("CWeaponRG6")
			.def(constructor<>())
		];
}
