////////////////////////////////////////////////////////////////////////////
//	Module 		: attachable_item.cpp
//	Created 	: 11.02.2004
//  Modified 	: 11.02.2004
//	Author		: Dmitriy Iassenev
//	Description : Attachable item
////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "physicsshellholder.h"
#include "attachable_item.h"
#include "inventoryowner.h"
#include "inventory.h"
#include "../xrUICore/ui_base.h"
#include "../xrEngine/xr_input.h"
#include "../xrEngine/xr_level_controller.h"

CAttachableItem* CAttachableItem::m_dbgItem = nullptr;

CPhysicsShellHolder &CAttachableItem::object() const
{
	return (item().object());
}

DLL_Pure *CAttachableItem::_construct()
{
	if (m_item) xr_delete(m_item);

	m_item = smart_cast<CInventoryItem*>(this);
	VERIFY(m_item);

	return (&item().object());
}

CAttachableItem::~CAttachableItem		()
{
}

void CAttachableItem::reload			(LPCSTR section)
{
	m_valid							= true;

	if(load_attach_position(section))
		enable						(false);
}

bool CAttachableItem::load_attach_position(LPCSTR section) 
{
	if (!pSettings->line_exist(section,"attach_angle_offset"))
		return false;

	Fvector							angle_offset = pSettings->r_fvector3	(section,"attach_angle_offset");
	Fvector							position_offset	= pSettings->r_fvector3	(section,"attach_position_offset");
	m_offset.setHPB					(VPUSH(angle_offset));
	m_offset.c						= position_offset;
	m_bone_name						= pSettings->r_string	(section,"attach_bone_name");
	return							true;
}

void CAttachableItem::OnH_A_Chield		() 
{
	const CInventoryOwner			*inventory_owner = smart_cast<const CInventoryOwner*>(object().H_Parent());
	if (inventory_owner && inventory_owner->attached(&item()))
		object().setVisible			(true);
}

void CAttachableItem::renderable_Render	()
{
	::Render->set_Transform			(&object().XFORM());
	::Render->add_Visual			(object().Visual());
}

void CAttachableItem::OnH_A_Independent	()
{
	enable							(false);
}

void CAttachableItem::enable(bool value)
{
	CGameObject* pGameObject = smart_cast<CGameObject*>(object().H_Parent());
	CAttachmentOwner *pOwner = smart_cast<CAttachmentOwner*>(pGameObject);

	if (pGameObject && pOwner)
	{
		if (value && !enabled())
		{
			m_enabled = value;
			pOwner->attach(&item());
			object().setVisible(true);
		}
		else if (!value && enabled())
		{
			m_enabled = value;
			pOwner->detach(&item());
			object().setVisible(false);
		}
	}
	else if(!pGameObject)
	{
		m_enabled = value;
	}
}

bool  CAttachableItem::can_be_attached() const
{
	if (!item().m_pInventory)
		return (false);

	if (!item().m_pInventory->IsBeltUseful())
		return (true);

	if (item().m_ItemCurrPlace.type != eItemPlaceBelt)
		return (false);

	return     (true);
}

void CAttachableItem::afterAttach		()
{
	VERIFY							(m_valid);
	object().processing_activate	();
}

void CAttachableItem::afterDetach		()
{
	VERIFY							(m_valid);
	object().processing_deactivate	();
}

float ATT_ITEM_MOVE_CURR = 0.01f;
float ATT_ITEM_ROT_CURR = 0.1f;

float ATT_ITEM_MOVE_STEP = 0.001f;
float ATT_ITEM_ROT_STEP = 0.01f;

void attach_adjust_mode_keyb(u8 dik)
{
	if(!CAttachableItem::m_dbgItem)	return;

	bool b_move		= !!(pInput->iGetAsyncKeyState(VK_SHIFT));
	bool b_rot		= !!(pInput->iGetAsyncKeyState(VK_LMENU));
	
	int axis		= -1;
	if(pInput->iGetAsyncKeyState(VK_Z))
		axis = 0;
	else
	if(pInput->iGetAsyncKeyState(VK_X))
		axis = 1;
	if(pInput->iGetAsyncKeyState(VK_C))
		axis = 2;

	if(!b_move && !b_rot)	return;

	switch(dik)
	{
	case VK_LEFT:
		{
			if(b_move)
				CAttachableItem::mov(axis, ATT_ITEM_MOVE_CURR);
			else
				CAttachableItem::rot(axis, ATT_ITEM_ROT_CURR);
		}break;
	case VK_RIGHT:
		{
			if(b_move)
				CAttachableItem::mov(axis, -ATT_ITEM_MOVE_CURR);
			else
				CAttachableItem::rot(axis, -ATT_ITEM_ROT_CURR);
		}break;
	case VK_PRIOR:
		{
			if(b_move)
				ATT_ITEM_MOVE_CURR +=ATT_ITEM_MOVE_STEP;
			else
				ATT_ITEM_ROT_CURR +=ATT_ITEM_ROT_STEP;
		}break;
	case VK_NEXT:
		{
			if(b_move)
				ATT_ITEM_MOVE_CURR -=ATT_ITEM_MOVE_STEP;
			else
				ATT_ITEM_ROT_CURR -=ATT_ITEM_ROT_STEP;
		}break;
	};
}

void attach_draw_adjust_mode()
{
	if(!CAttachableItem::m_dbgItem)	return;

	string1024 _text;

	CGameFont* F		= UI().Font().pFontDI;
	F->SetAligment		(CGameFont::alCenter);
	F->OutSetI			(0.f,-0.8f);
	F->SetColor			(0xffffffff);
	xr_sprintf(_text, "Adjusting attachable item [%s]", CAttachableItem::m_dbgItem->object().cNameSect().c_str());
	F->OutNext			(_text);
	xr_sprintf(_text, "move step  [%3.3f] rotate step  [%3.3f]", ATT_ITEM_MOVE_CURR, ATT_ITEM_ROT_CURR);
	F->OutNext			(_text);

	F->OutNext			("HOLD LShift to move. ALT to rotate");
	F->OutNext			("HOLD [Z]-x axis [X]-y axis [C]-z axis");

	F->OutNext			("RIGHT-LEFT - move. PgUP-PgDOWN - step");
	F->OutSkip			();

	Fvector _pos = CAttachableItem::get_pos_offset();
	xr_sprintf(_text, "attach_position_offset IS [%3.3f][%3.3f][%3.3f]", _pos.x, _pos.y, _pos.z);
	F->OutNext			(_text);

	Fvector _ang = CAttachableItem::get_angle_offset();
	xr_sprintf(_text, "attach_angle_offset IS [%3.3f][%3.3f][%3.3f]", _ang.x, _ang.y, _ang.z);
	F->OutNext			(_text);
}