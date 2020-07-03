#pragma once

enum WeaponActionFlags
{
    CMD_START = 1<<0,
    CMD_STOP  = 1<<1,
    CMD_IN    = 1<<2,
    CMD_OUT   = 1<<3
};

enum{
	NO_ACTIVE_SLOT	= 0,
	KNIFE_SLOT		= 1,//btn1			was (0)			!!!
	INV_SLOT_2,		//btn2 PISTOL_SLOT	was (1)
	INV_SLOT_3,		//btn3 RIFLE_SLOT	was (2)
	GRENADE_SLOT,	//btn4 GRENADE_SLOT	was (3)
	BINOCULAR_SLOT,	//btn5 BINOCULAR_SLOT
	BOLT_SLOT,		//btn6 BOLT_SLOT
	OUTFIT_SLOT,	// outfit
	PDA_SLOT,		// pda
	DETECTOR_SLOT,	// detector
	TORCH_SLOT,		// torch
	ARTEFACT_SLOT,	// artefact
	HELMET_SLOT,	// helmet
	RUCK_SLOT,		//backpack
	LAST_SLOT		= RUCK_SLOT
};

#define RUCK_HEIGHT			280
#define RUCK_WIDTH			7

class CInventoryItem;
class CInventory;

using PIItem = CInventoryItem*;
using TIItemContainer = xr_vector<PIItem>;


enum eItemPlace
{			
	eItemPlaceUndefined = 0,
	eItemPlaceSlot,
	eItemPlaceBelt,
	eItemPlaceRuck
};

struct SInvItemPlace
{
	union{
		struct{
			u16 type				: 4;
			u16 slot_id				: 6;
			u16 base_slot_id		: 6;
		};
		u16	value;
	};
};

extern u16 INV_STATE_BLOCK_ALL;

struct II_BriefInfo
{
	shared_str		name;
	shared_str		icon;
	shared_str		cur_ammo;
	shared_str		fmj_ammo;
	shared_str		ap_ammo;
	shared_str		fire_mode;

	shared_str		grenade;

	II_BriefInfo() { clear(); }
	
	IC void clear()
	{
		name		= "";
		icon		= "";
		cur_ammo	= "";
		fmj_ammo	= "";
		ap_ammo		= "";
		fire_mode	= "";
		grenade		= "";
	}
};
