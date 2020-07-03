////////////////////////////////////////////////////////////////////////////
//	Module 		: purchase_list.cpp
//	Created 	: 12.01.2006
//  Modified 	: 12.01.2006
//	Author		: Dmitriy Iassenev
//	Description : purchase list class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "purchase_list.h"
#include "inventoryowner.h"
#include "gameobject.h"
#include "ai_object_location.h"
#include "level.h"

static float min_deficit_factor = .3f;

void CPurchaseList::process	(CInifile &ini_file, LPCSTR section, CInventoryOwner &owner)
{
	owner.sell_useless_items();

	m_deficits.clear		();

	const CGameObject		&game_object = smart_cast<const CGameObject &>(owner);
	CInifile::Sect			&S = ini_file.r_section(section);

	for (CInifile::Item itm: S.Data) {
		VERIFY3				(itm.second.size(),"PurchaseList : cannot handle lines in section without values",section);

		string256			temp0, temp1;
		THROW3				(_GetItemCount(itm.second.c_str()) == 2,"Invalid parameters in section",section);
		process				(game_object, itm.first, atoi_17(_GetItem(itm.second.c_str(),0,temp0)), 
			(float)atof(_GetItem(itm.second.c_str(),1,temp1)));
	}
}
#include <random>
void CPurchaseList::process	(const CGameObject &owner, const shared_str &name, const u32 &count, const float &probability)
{
	VERIFY3					(count,"Invalid count for section in the purchase list",*name);
	VERIFY3					(!fis_zero(probability,EPS_S),"Invalid probability for section in the purchase list",*name);

	const Fvector			&position = owner.Position();
	const u32				&level_vertex_id = owner.ai_location().level_vertex_id();
	const ALife::_OBJECT_ID	&id = owner.ID();

	// Balathruin
	/// With proper range, the trader inventory generation is now actually randomised.
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_real_distribution<float> uni(0.f, 1.f);

    u32 j = 0;
	for (u32 i = 0; i<count; ++i)
	{
		if (uni(rng) > probability)
			continue;

		++j;
		Level().spawn_item		(*name,position,level_vertex_id,id,false);
	}
	// End

	DEFICITS::const_iterator	I = m_deficits.find(name);
	VERIFY3						(I == m_deficits.end(),"Duplicate section in the purchase list",*name);
	m_deficits.insert			(
		std::make_pair(
			name,
			(float)count*probability
			/
            std::max((float)j,min_deficit_factor)
		)
	);
}
