#pragma once

#include "rocketlauncher.h"
#include "weaponShotgun.h"
#include "../xrScripts/export/script_export_space.h"

class CWeaponRG6 :  public CRocketLauncher,
					public CWeaponShotgun
{
	typedef CRocketLauncher		inheritedRL;
	typedef CWeaponShotgun		inheritedSG;
	
public:
	virtual			~CWeaponRG6				();
	virtual BOOL	net_Spawn				(CSE_Abstract* DC);
	virtual void	Load					(LPCSTR section);
	virtual void	OnEvent					(NET_Packet& P, u16 type);
protected:
	virtual u8		AddCartridge			(u8 cnt);
	virtual	void	FireTrace				(const Fvector& P, const Fvector& D);
	virtual void	LaunchGrenade			(const Fvector& P, const Fvector& D);

			bool install_upgrade_impl(LPCSTR section, bool test) override;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CWeaponRG6)
#undef script_type_list
#define script_type_list save_type_list(CWeaponRG6)
