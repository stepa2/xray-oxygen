#include "stdafx.h"
#include "UIActorMenu.h"
#include "../inventory.h"
#include "../inventoryOwner.h"
#include "UIInventoryUtilities.h"
#include "UIItemInfo.h"
#include "../Level.h"
#include "UICellItemFactory.h"
#include "UIDragDropListEx.h"
#include "UIDragDropReferenceList.h"
#include "UICellCustomItems.h"
#include "UIItemInfo.h"
#include "../xrUICore/UIFrameLineWnd.h"
#include "../xrUICore/UIPropertiesBox.h"
#include "../xrUICore/UIListBoxItem.h"
#include "UIMainIngameWnd.h"
#include "UIGame.h"
#include "eatable_item_object.h"
#include "../items/FoodItem.h"
#include "../items/Silencer.h"
#include "../items/Scope.h"
#include "../items/GrenadeLauncher.h"
#include "../items/Artefact.h"
#include "../items/WeaponMagazined.h"
#include "../items/CustomOutfit.h"
#include "../items/Helmet.h"
#include "../eatable_item.h"
#include "../../xrUICore/UICursor.h"
#include "../player_hud.h"
#include "../CustomDetector.h"
#include "../PDA.h"

#include "../actor_defs.h"

#include "../../FrayBuildConfig.hpp"


void move_item_from_to(u16 from_id, u16 to_id, u16 what_id);

void CUIActorMenu::InitInventoryMode()
{
	m_pInventoryBagList->Show			(true);
	m_pInventoryBeltList->Show			(true);
	m_pInventoryOutfitList->Show		(true);
	m_pInventoryHelmetList->Show		(true);
	m_pInventoryDetectorList->Show		(true);
	m_pInventoryPistolList->Show		(true);
	m_pInventoryAutomaticList->Show		(true);

    if (g_extraFeatures.is(GAME_EXTRA_RUCK))
    {
        m_pInventoryRuckList->Show(true);
    }

    m_pInventoryKnifeList->Show         (true);
    m_pInventoryBinocularList->Show     (true);
    m_pInventoryTorchList->Show         (true);
	m_pQuickSlot->Show					(true);
	m_pTrashList->Show					(true);
	m_RightDelimiter->Show				(false);

	InitInventoryContents				(m_pInventoryBagList);
}

void CUIActorMenu::DeInitInventoryMode()
{
	m_pTrashList->Show				(false);
}

void CUIActorMenu::SendEvent_ActivateSlot(u16 slot, u16 recipient)
{
	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ACTIVATE_SLOT, recipient);
	P.w_u16							(slot);
	CGameObject::u_EventSend		(P);
}

void CUIActorMenu::SendEvent_Item2Slot(PIItem pItem, u16 recipient, u16 slot_id)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2SLOT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	P.w_u16							(slot_id);
	CGameObject::u_EventSend		(P);

	PlaySnd							(eItemToSlot);
};

void CUIActorMenu::SendEvent_Item2Belt(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2BELT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);

	PlaySnd							(eItemToBelt);
};

void CUIActorMenu::SendEvent_Item2Ruck(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2RUCK, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);

	PlaySnd							(eItemToRuck);
};

void CUIActorMenu::SendEvent_Item_Eat(PIItem pItem, u16 recipient)
{
	//if(pItem->parent_id()!=recipient)
		//move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM_EAT, recipient);
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);
};

void CUIActorMenu::SendEvent_Item_Drop(PIItem pItem, u16 recipient)
{
	R_ASSERT(pItem->parent_id() == recipient);
	
	NET_Packet					P;
	pItem->object().u_EventGen	(P,GE_OWNERSHIP_REJECT,pItem->parent_id());
	P.w_u16						(pItem->object().ID());
	pItem->object().u_EventSend	(P);
	PlaySnd						(eDropItem);
}

void CUIActorMenu::DropAllCurrentItem()
{
	if ( CurrentIItem() && !CurrentIItem()->IsQuestItem() )
	{
		CUICellItem*	itm;
		u32 const cnt = CurrentItem()->ChildsCount();
		for( u32 i = 0; i < cnt; ++i )
		{
			itm  = CurrentItem()->PopChild(nullptr);
			PIItem			iitm = (PIItem)itm->m_pData;			
			SendEvent_Item_Drop( iitm, m_pActorInvOwner->object_id() );			
		}

		SendEvent_Item_Drop( CurrentIItem(), m_pActorInvOwner->object_id() );
		itm->Show(false);
	}
	SetCurrentItem								(nullptr);
}

bool CUIActorMenu::DropAllItemsFromRuck( bool quest_force )
{
	if ( !IsShown() || !m_pInventoryBagList || m_currMenuMode != mmInventory )
	{
		return false;
	}

	u32 const ci_count = m_pInventoryBagList->ItemsCount();
	for ( u32 i = 0; i < ci_count; ++i )
	{
		CUICellItem* ci = m_pInventoryBagList->GetItemIdx( i );
		VERIFY( ci );
		PIItem item = (PIItem)ci->m_pData;
		VERIFY( item );

		if ( !quest_force && item->IsQuestItem() )
		{
			continue;
		}

		u32 const cnt = ci->ChildsCount();
		for( u32 j = 0; j < cnt; ++j )
		{
			CUICellItem*	child_ci   = ci->PopChild(nullptr);
			PIItem			child_item = (PIItem)child_ci->m_pData;
			SendEvent_Item_Drop( child_item, m_pActorInvOwner->object_id() );
		}
		SendEvent_Item_Drop( item, m_pActorInvOwner->object_id() );
	}

	SetCurrentItem( nullptr );
	return true;
}

bool FindItemInList(CUIDragDropListEx* lst, PIItem pItem, CUICellItem*& ci_res)
{
	u32 count = lst ? lst->ItemsCount() : 0;
	for (u32 i=0; i<count; ++i)
	{
		CUICellItem* pCellItm = lst->GetItemIdx(i);
		if (!pCellItm) continue;

		for (u32 j = 0; j < pCellItm->ChildsCount(); ++j)
		{
			CUIInventoryCellItem* ici = smart_cast<CUIInventoryCellItem*>(pCellItm->Child(j));
			if (ici->object() == pItem)
			{
				ci_res = ici;
				return true;
			}
		}

		CUIInventoryCellItem* pInvCellItem = smart_cast<CUIInventoryCellItem*>(pCellItm);
		if(pInvCellItem && pInvCellItem->object()==pItem)
		{
			ci_res = pCellItm;
			return true;
		}
	}
	return false;
}

bool RemoveItemFromList(CUIDragDropListEx* lst, PIItem pItem)
{// fixme
	CUICellItem*	ci	= nullptr;
	if(FindItemInList(lst, pItem, ci))
	{
		R_ASSERT		(ci);

		CUICellItem* dying_cell = lst->RemoveItem	(ci, false);
		xr_delete(dying_cell);

		return			true;
	}else
		return			false;
}

void CUIActorMenu::OnInventoryAction(PIItem pItem, u16 action_type)
{
	CUIDragDropListEx* all_lists[] =
	{
		m_pInventoryBeltList,
		m_pInventoryPistolList,
		m_pInventoryAutomaticList,
		m_pInventoryKnifeList,
		m_pInventoryBinocularList,
		m_pInventoryTorchList,
		m_pInventoryRuckList,
		m_pInventoryOutfitList,
		m_pInventoryHelmetList,
		m_pInventoryDetectorList,
		m_pInventoryBagList,
		m_pTradeActorBagList,
		m_pTradeActorList,
		nullptr
	};

	switch (action_type)
	{
		case GE_TRADE_BUY :
		case GE_OWNERSHIP_TAKE :
			{
				u32 i			= 0;
				bool b_already	= false;

				CUIDragDropListEx* lst_to_add		= nullptr;
				SInvItemPlace pl						= pItem->m_ItemCurrPlace;
				if ( pItem->BaseSlot() == GRENADE_SLOT )
				{
					pl.type		= eItemPlaceRuck;
					pl.slot_id	= GRENADE_SLOT;
				}

				bool isDeadBodySearch = GetMenuMode() == mmDeadBodyOrContainerSearch;
				if (!isDeadBodySearch && pl.type == eItemPlaceSlot)
					lst_to_add = GetSlotList(pl.slot_id);
				else if (!isDeadBodySearch && pl.type == eItemPlaceBelt)
					lst_to_add = GetListByType(iActorBelt);
				else if (pItem->parent_id() == m_pActorInvOwner->object_id())
					lst_to_add = GetListByType(iActorBag);
				else
					lst_to_add = GetListByType(iDeadBodyBag);

				while (all_lists[i])
				{
					CUIDragDropListEx*	curr = all_lists[i];
					CUICellItem*		ci = nullptr;

					if (FindItemInList(curr, pItem, ci))
					{
						if (lst_to_add != curr)
						{
							RemoveItemFromList(curr, pItem);
						}
						else
						{
							b_already = true;
						}
						//break;
					}
					++i;
				}

				CUICellItem* ci = nullptr;
				if(isDeadBodySearch && FindItemInList(lst_to_add, pItem, ci))
					break;

				if ( !b_already )
				{
					if ( lst_to_add )
					{
						CUICellItem* itm	= create_cell_item(pItem);
						lst_to_add->SetItem	(itm);
					}
				}
				if(m_pActorInvOwner)
					m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
			}break;
		case GE_TRADE_SELL :
		case GE_OWNERSHIP_REJECT :
			{
				if(CUIDragDropListEx::m_drag_item)
				{
					CUIInventoryCellItem* ici = smart_cast<CUIInventoryCellItem*>(CUIDragDropListEx::m_drag_item->ParentItem());
					R_ASSERT(ici);
					if(ici->object()==pItem)
					{
						CUIDragDropListEx*	_drag_owner		= ici->OwnerList();
						_drag_owner->DestroyDragItem		();
					}
				}

				u32 i = 0;
				while(all_lists[i])
				{
					CUIDragDropListEx* curr = all_lists[i];
					if(RemoveItemFromList(curr, pItem))
					{
#ifndef MASTER_GOLD
						Msg("all ok. item [%d] removed from list", pItem->object_id());
#endif // #ifndef MASTER_GOLD
						break;
					}
					++i;
				}
				if(m_pActorInvOwner)
					m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
			}break;
	}
	UpdateItemsPlace();
	UpdateConditionProgressBars();
}
void CUIActorMenu::AttachAddon(PIItem item_to_upgrade)
{
	PlaySnd										(eAttachAddon);
	R_ASSERT									(item_to_upgrade);
	item_to_upgrade->Attach						(CurrentIItem(), true);

	SetCurrentItem								(nullptr);
}

void CUIActorMenu::DetachAddon(LPCSTR addon_name, PIItem itm)
{
	PlaySnd										(eDetachAddon);

	if(!itm)
		CurrentIItem()->Detach					(addon_name, true);
	else
		itm->Detach								(addon_name, true);
}

void CUIActorMenu::InitCellForSlot( u16 slot_idx )
{
	VERIFY( KNIFE_SLOT <= slot_idx && slot_idx <= LAST_SLOT );
	PIItem item	= m_pActorInvOwner->inventory().ItemFromSlot(slot_idx);
	if ( !item )
	{
		return;
	}

	CUIDragDropListEx* curr_list	= GetSlotList( slot_idx );
	CUICellItem* cell_item			= create_cell_item( item );
	curr_list->SetItem( cell_item );
	if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
		ColorizeItem( cell_item, !CanMoveToPartner( item ) );
}

void CUIActorMenu::InitInventoryContents(CUIDragDropListEx* pBagList)
{
	ClearAllLists				();
	m_pMouseCapturer			= nullptr;
	m_UIPropertiesBox->Hide		();
	SetCurrentItem				(nullptr);

	CUIDragDropListEx*			curr_list = nullptr;
	//Slots
	InitCellForSlot				(INV_SLOT_2);
	InitCellForSlot				(INV_SLOT_3);

    if (g_extraFeatures.is(GAME_EXTRA_RUCK))
    {
	    InitCellForSlot(RUCK_SLOT);
    }

    InitCellForSlot             (KNIFE_SLOT);
    InitCellForSlot             (BINOCULAR_SLOT);
    InitCellForSlot             (TORCH_SLOT);
	InitCellForSlot				(OUTFIT_SLOT);
	InitCellForSlot				(DETECTOR_SLOT);
	InitCellForSlot				(GRENADE_SLOT);
	InitCellForSlot				(HELMET_SLOT);

	curr_list					= m_pInventoryBeltList;
	TIItemContainer::iterator itb = m_pActorInvOwner->inventory().m_belt.begin();
	TIItemContainer::iterator ite = m_pActorInvOwner->inventory().m_belt.end();
	for ( ; itb != ite; ++itb )
	{
		CUICellItem* itm		= create_cell_item(*itb);
		curr_list->SetItem		(itm);
		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
			ColorizeItem( itm, !CanMoveToPartner( *itb ) );
	}

	TIItemContainer				ruck_list;
	ruck_list					= m_pActorInvOwner->inventory().m_ruck;
	std::sort					( ruck_list.begin(), ruck_list.end(), InventoryUtilities::GreaterRoomInRuck );

	curr_list					= pBagList;

	itb = ruck_list.begin();
	ite = ruck_list.end();
	for ( ; itb != ite; ++itb )
	{
		CUICellItem* itm = create_cell_item( *itb );
		curr_list->SetItem(itm);
		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
			ColorizeItem( itm, !CanMoveToPartner( *itb ) );
	}
	m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
}

bool CUIActorMenu::TryActiveSlot(CUICellItem* itm)
{
	PIItem	iitem	= (PIItem)itm->m_pData;
	u16 slot		= iitem->BaseSlot();

	if ( slot == GRENADE_SLOT )
	{
		PIItem	prev_iitem = m_pActorInvOwner->inventory().ItemFromSlot(slot);
		if ( prev_iitem && (prev_iitem->object().cNameSect() != iitem->object().cNameSect()) )
		{
			SendEvent_Item2Ruck( prev_iitem, m_pActorInvOwner->object_id() );
			SendEvent_Item2Slot( iitem, m_pActorInvOwner->object_id(), slot );
		}
		SendEvent_ActivateSlot( slot, m_pActorInvOwner->object_id() );
		return true;
	}
	if ( slot == DETECTOR_SLOT )
	{

	}
	return false;
}

bool CUIActorMenu::ToSlot(CUICellItem* itm, bool force_place, u16 slot_id)
{
	CUIDragDropListEx*	old_owner			= itm->OwnerList();
	PIItem	iitem							= (PIItem)itm->m_pData;

	bool b_own_item							= (iitem->parent_id()==m_pActorInvOwner->object_id());
	if (slot_id==HELMET_SLOT)
	{
		CCustomOutfit* pOutfit = m_pActorInvOwner->GetOutfit();
		if(pOutfit && !pOutfit->bIsHelmetAvaliable)
			return false;
	}

	if(m_pActorInvOwner->inventory().CanPutInSlot(iitem, slot_id))
	{
		CUIDragDropListEx* new_owner		= GetSlotList(slot_id);

		if ( slot_id == GRENADE_SLOT || !new_owner )
		{
			return true; //fake, sorry (((
		}

		if(slot_id==OUTFIT_SLOT)
		{
			CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(iitem);
			if(pOutfit && !pOutfit->bIsHelmetAvaliable)
			{
				CUIDragDropListEx* helmet_list		= GetSlotList(HELMET_SLOT);
				if(helmet_list->ItemsCount()==1)
				{
					CUICellItem* helmet_cell		= helmet_list->GetItemIdx(0);
					ToBag(helmet_cell, false);
				}
			}
		}


		bool result							= (!b_own_item) || m_pActorInvOwner->inventory().Slot(slot_id, iitem);
		VERIFY								(result);

		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );

		new_owner->SetItem					(i);

		SendEvent_Item2Slot					(iitem, m_pActorInvOwner->object_id(), slot_id);

		SendEvent_ActivateSlot				(slot_id, m_pActorInvOwner->object_id());

		//ColorizeItem						( itm, false );
		if ( slot_id == OUTFIT_SLOT )
		{
			MoveArtefactsToBag();
		}
		return								true;
	}
	else
	{ // in case slot is busy
		if ( !force_place || slot_id == NO_ACTIVE_SLOT ) 
			return false;

		if ( m_pActorInvOwner->inventory().SlotIsPersistent(slot_id) && slot_id != DETECTOR_SLOT  )
			return false;

		if ( slot_id == INV_SLOT_2 && m_pActorInvOwner->inventory().CanPutInSlot(iitem, INV_SLOT_3))
			return ToSlot(itm, force_place, INV_SLOT_3);

		if ( slot_id == INV_SLOT_3 && m_pActorInvOwner->inventory().CanPutInSlot(iitem, INV_SLOT_2))
			return ToSlot(itm, force_place, INV_SLOT_2);

		PIItem	_iitem						= m_pActorInvOwner->inventory().ItemFromSlot(slot_id);
		CUIDragDropListEx* slot_list		= GetSlotList(slot_id);
		VERIFY								(slot_list->ItemsCount()==1);

		CUICellItem* slot_cell				= slot_list->GetItemIdx(0);
		VERIFY								(slot_cell && ((PIItem)slot_cell->m_pData)==_iitem);

		bool result							= ToBag(slot_cell, false);
		VERIFY								(result);

		result								= ToSlot(itm, false, slot_id);
		if(b_own_item && result && slot_id==DETECTOR_SLOT)
		{
			CCustomDetector* det			= smart_cast<CCustomDetector*>(iitem);
			det->ToggleDetector				(g_player_hud->attached_item(0)!=nullptr);
		}

		return result;
	}
}

#include "../xrEngine/xr_input.h"
bool CUIActorMenu::ToBag(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= (PIItem)itm->m_pData;

	bool b_own_item						= (iitem->parent_id()==m_pActorInvOwner->object_id());

	bool b_already						= m_pActorInvOwner->inventory().InRuck(iitem);

	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= nullptr;
	if(b_use_cursor_pos)
	{
			new_owner					= CUIDragDropListEx::m_drag_item->BackList();
			VERIFY						(GetListType(new_owner)==iActorBag);
	}else
			new_owner					= GetListByType(iActorBag);

	if(m_pActorInvOwner->inventory().CanPutInRuck(iitem) || (b_already && (new_owner!=old_owner)) )
	{
		bool result							= b_already || (!b_own_item || m_pActorInvOwner->inventory().Ruck(iitem) );
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		if(!i)
			return false;

		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);

		if(!b_already || !b_own_item)
			SendEvent_Item2Ruck					(iitem, m_pActorInvOwner->object_id());

		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
		{
			ColorizeItem( itm, !CanMoveToPartner( iitem ) );
		}
	if ((i != itm) && !!pInput->iGetAsyncKeyState(VK_CONTROL)) return ToBag(itm, (old_owner == new_owner));
		return true;
	}
	return false;
}

bool CUIActorMenu::ToBelt(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem = (PIItem)itm->m_pData;
	bool b_own_item = (iitem->parent_id() == m_pActorInvOwner->object_id());

	if (m_pActorInvOwner->inventory().CanPutInBelt(iitem))
	{
		CUIDragDropListEx*	old_owner = itm->OwnerList();
		CUIDragDropListEx*	new_owner = nullptr;

		if (b_use_cursor_pos)
		{
			new_owner = CUIDragDropListEx::m_drag_item->BackList();
			VERIFY(new_owner == m_pInventoryBeltList);
		}
		else	new_owner = m_pInventoryBeltList;

		bool result = (!b_own_item) || m_pActorInvOwner->inventory().Belt(iitem);
		VERIFY(result);
		CUICellItem* i = old_owner->RemoveItem(itm, (old_owner == new_owner));

		if (b_use_cursor_pos)
			new_owner->SetItem(i, old_owner->GetDragItemPosition());
		else
			new_owner->SetItem(i);

		if (!b_own_item)
			SendEvent_Item2Belt(iitem, m_pActorInvOwner->object_id());

		//ColorizeItem						(itm, false);
		return true;
	}

	// in case belt slot is busy
	if (!iitem->Belt() || !m_pActorInvOwner->inventory().BeltWidth())
		return false;

	CUIDragDropListEx* belt_list = nullptr;
	if (b_use_cursor_pos)
		belt_list = CUIDragDropListEx::m_drag_item->BackList();
	else return false;

	Ivector2 belt_cell_pos = belt_list->PickCell(GetUICursor().GetCursorPosition());
	if (belt_cell_pos.x == -1 && belt_cell_pos.y == -1)
		return false;

	CUICellItem* slot_cell = belt_list->GetCellAt(belt_cell_pos).m_item;

	bool result = ToBag(slot_cell, false);
	VERIFY(result);

	result = ToBelt(itm, b_use_cursor_pos);
	return result;
}

CUIDragDropListEx* CUIActorMenu::GetSlotList(u16 slot_idx)
{
	if (slot_idx == NO_ACTIVE_SLOT)
		return nullptr;

	switch ( slot_idx )
	{
		case INV_SLOT_2:				return m_pInventoryPistolList;
		case INV_SLOT_3:				return m_pInventoryAutomaticList;
		case RUCK_SLOT:					return g_extraFeatures.is(GAME_EXTRA_RUCK) ? m_pInventoryRuckList : nullptr;
		case KNIFE_SLOT:				return m_pInventoryKnifeList;
		case BINOCULAR_SLOT:			return m_pInventoryBinocularList; 
		case TORCH_SLOT:				return m_pInventoryTorchList;
		case OUTFIT_SLOT:				return m_pInventoryOutfitList;
		case HELMET_SLOT:				return m_pInventoryHelmetList;
		case DETECTOR_SLOT:				return m_pInventoryDetectorList;
		case GRENADE_SLOT: /* fake */	return (m_currMenuMode == mmTrade) ? m_pTradeActorBagList : m_pInventoryBagList;
        default:						return nullptr;
	};
}

#include "UIActorStateInfo.h"
bool CUIActorMenu::TryUseFoodItem(CUICellItem* cell_itm)
{
	if (!cell_itm)	
		return false;

	PIItem item = dynamic_cast<CFoodItem*>((PIItem)cell_itm->m_pData);

	if (!item || !item->Useful())
		return false;

	u16 ActorInventoryID = m_pActorInvOwner->object_id();
	
	// Send event to Actor fell
	SendEvent_Item_Eat(item, ActorInventoryID);
	PlaySnd(eItemUse);
	SetCurrentItem(nullptr);

	cell_itm->OwnerList()->RemoveItem(cell_itm, false);

	return true;
}


bool CUIActorMenu::ToQuickSlot(CUICellItem* itm)
{
	PIItem iitem = (PIItem)itm->m_pData;
	CEatableItemObject* eat_item = smart_cast<CEatableItemObject*>(iitem);
	if(!eat_item)
		return false;

	u8 slot_idx = u8(m_pQuickSlot->PickCell(GetUICursor().GetCursorPosition()).x);
	if(slot_idx==255)
		return false;

	CObject*	pObj			= smart_cast<CObject*>		(iitem);
	shared_str	section_name	= pObj->cNameSect();	
	
	Ivector2 iWH = iitem->GetInvGridRect().rb;
	bool CanSwitchToFastSlot = READ_IF_EXISTS(pSettings, r_bool, section_name, "can_switch_to_fast_slot", true);
	
	if (!CanSwitchToFastSlot || iWH.x > 1 || iWH.y > 1)
		return false;	
	
	m_pQuickSlot->SetItem(create_cell_item(iitem), GetUICursor().GetCursorPosition());
	xr_strcpy(ACTOR_DEFS::g_quick_use_slots[slot_idx], iitem->m_section_id.c_str());
	return true;
}


bool CUIActorMenu::OnItemDropped(PIItem itm, CUIDragDropListEx* new_owner, CUIDragDropListEx* old_owner)
{
	CUICellItem* _citem	= (new_owner->ItemsCount()==1) ? new_owner->GetItemIdx(0) : nullptr;
	PIItem _iitem       = _citem ? (PIItem)_citem->m_pData : nullptr;

	if (!_iitem || !_iitem->CanAttach(itm)) return	false;
	if (old_owner != m_pInventoryBagList)	return	false;

	AttachAddon(_iitem);

	switch (m_currMenuMode)
	{
	case mmInventory: RemoveItemFromList(m_pInventoryBagList, itm); break;
	case mmDeadBodyOrContainerSearch: RemoveItemFromList(m_pDeadBodyBagList, itm); break;
	}
	return true;
}

void CUIActorMenu::TryHidePropertiesBox()
{
	if (m_UIPropertiesBox->IsShown())
		m_UIPropertiesBox->Hide();
}

void CUIActorMenu::ActivatePropertiesBox()
{
	TryHidePropertiesBox();
	if (!(m_currMenuMode == mmInventory || m_currMenuMode == mmDeadBodyOrContainerSearch || m_currMenuMode == mmUpgrade))
		return;

	PIItem item = CurrentIItem();
	if (!item)
		return;

	CUICellItem* cell_item = CurrentItem();
	m_UIPropertiesBox->RemoveAll();
	bool b_show = false;

	if (m_currMenuMode == mmInventory || m_currMenuMode == mmDeadBodyOrContainerSearch)
	{
		PropertiesBoxForSlots(item, b_show);
		PropertiesBoxForWeapon(cell_item, item, b_show);
		PropertiesBoxForAddon(item, b_show);
		PropertiesBoxForUsing(item, b_show);
		PropertiesBoxForPlaying(item, b_show);

		if (m_currMenuMode == mmInventory)
			PropertiesBoxForDrop(cell_item, item, b_show);
	}
	else if (m_currMenuMode == mmUpgrade)
	{
		PropertiesBoxForRepair(item, b_show);
	}

	if (b_show)
	{
		m_UIPropertiesBox->AutoUpdateSize();

		Fvector2 cursor_pos;
		Frect vis_rect;

		GetAbsoluteRect(vis_rect);
		cursor_pos = GetUICursor().GetCursorPosition();
		cursor_pos.sub(vis_rect.lt);
		m_UIPropertiesBox->Show(vis_rect, cursor_pos);
		PlaySnd(eProperties);
	}
}

void CUIActorMenu::PropertiesBoxForSlots(PIItem item, bool& b_show)
{
	CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(item);
	CHelmet* pHelmet = smart_cast<CHelmet*>		(item);
	CInventory&  inv = m_pActorInvOwner->inventory();
	CUICellItem*	itm = CurrentItem();
	PIItem	iitem = (PIItem)itm->m_pData;

	// Флаг-признак для невлючения пункта контекстного меню: Dreess Outfit, если костюм уже надет
	bool bAlreadyDressed = false;
	u16 cur_slot = item->BaseSlot();

	// Rietmon: A choice is made where to move the weapon
	if (!pOutfit && !pHelmet)
	{
		if (inv.CanPutInSlot(item, INV_SLOT_2) && iitem->BaseSlot() != DETECTOR_SLOT)
		{
			m_UIPropertiesBox->AddItem("st_move_to_slot_2", nullptr, INVENTORY_TO_SLOT2_ACTION);
			b_show = true;
		}

		if (inv.CanPutInSlot(item, INV_SLOT_3) && iitem->BaseSlot() != DETECTOR_SLOT)
		{
			m_UIPropertiesBox->AddItem("st_move_to_slot_3", nullptr, INVENTORY_TO_SLOT3_ACTION);
			b_show = true;
		}

		if (iitem->BaseSlot() == KNIFE_SLOT && inv.CanPutInSlot(item, KNIFE_SLOT))
		{
			m_UIPropertiesBox->AddItem("st_move_to_slot_knife", nullptr, INVENTORY_TO_SLOT_ACTION);
			b_show = true;
		}

		if (iitem->BaseSlot() == BINOCULAR_SLOT && inv.CanPutInSlot(item, BINOCULAR_SLOT))
		{
			m_UIPropertiesBox->AddItem("st_move_to_slot_binoc", nullptr, INVENTORY_TO_SLOT_ACTION);
			b_show = true;
		}

		if (iitem->BaseSlot() == DETECTOR_SLOT && inv.CanPutInSlot(item, DETECTOR_SLOT))
		{
			m_UIPropertiesBox->AddItem("st_move_to_slot_detect", nullptr, INVENTORY_TO_SLOT_ACTION);
			b_show = true;
		}
	}
	if (item->Belt() &&
		inv.CanPutInBelt(item))
	{
		m_UIPropertiesBox->AddItem("st_move_on_belt", nullptr, INVENTORY_TO_BELT_ACTION);
		b_show = true;
	}

	if (item->Ruck() &&
		inv.CanPutInRuck(item) &&
		(cur_slot == NO_ACTIVE_SLOT || !inv.SlotIsPersistent(cur_slot)))
	{
		if (!pOutfit)
		{
			if (!pHelmet)
				m_UIPropertiesBox->AddItem("st_move_to_bag", nullptr, INVENTORY_TO_BAG_ACTION);
			else
				m_UIPropertiesBox->AddItem("st_undress_helmet", nullptr, INVENTORY_TO_BAG_ACTION);
		}
		else
			m_UIPropertiesBox->AddItem("st_undress_outfit", nullptr, INVENTORY_TO_BAG_ACTION);

		bAlreadyDressed = true;
		b_show = true;
	}
	if (pOutfit && !bAlreadyDressed)
	{
		m_UIPropertiesBox->AddItem("st_dress_outfit", nullptr, INVENTORY_TO_SLOT_ACTION);
		b_show = true;
	}

	CCustomOutfit* outfit_in_slot = m_pActorInvOwner->GetOutfit();
	if (pHelmet && !bAlreadyDressed && (!outfit_in_slot || outfit_in_slot->bIsHelmetAvaliable))
	{
		m_UIPropertiesBox->AddItem("st_dress_helmet", nullptr, INVENTORY_TO_SLOT_ACTION);
		b_show = true;
	}
}

void CUIActorMenu::PropertiesBoxForWeapon(CUICellItem* cell_item, PIItem item, bool& b_show)
{
	//отсоединение аддонов от вещи
	CWeapon*	pWeapon = smart_cast<CWeapon*>(item);
	if (!pWeapon)
		return;

	if (pWeapon->GrenadeLauncherAttachable() && pWeapon->IsGrenadeLauncherAttached())
	{
		m_UIPropertiesBox->AddItem("st_detach_gl", nullptr, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON);
		b_show = true;
	}
	if (pWeapon->ScopeAttachable() && pWeapon->IsScopeAttached())
	{
		m_UIPropertiesBox->AddItem("st_detach_scope", nullptr, INVENTORY_DETACH_SCOPE_ADDON);
		b_show = true;
	}
	if (pWeapon->SilencerAttachable() && pWeapon->IsSilencerAttached())
	{
		m_UIPropertiesBox->AddItem("st_detach_silencer", nullptr, INVENTORY_DETACH_SILENCER_ADDON);
		b_show = true;
	}
	if (smart_cast<CWeaponMagazined*>(pWeapon))
	{
		bool b = (pWeapon->GetAmmoElapsed() != 0);
		if (!b)
		{
			for (u32 i = 0; i < cell_item->ChildsCount(); ++i)
			{
				CWeaponMagazined* weap_mag = smart_cast<CWeaponMagazined*>((CWeapon*)cell_item->Child(i)->m_pData);
				if (weap_mag && weap_mag->GetAmmoElapsed())
				{
					b = true;
					break; // for
				}
			}
		}
		if (b)
		{
			m_UIPropertiesBox->AddItem("st_unload_magazine", nullptr, INVENTORY_UNLOAD_MAGAZINE);
			b_show = true;
		}
	}
}

#include "../../xrEngine/string_table.h"
void CUIActorMenu::PropertiesBoxForAddon(PIItem item, bool& b_show)
{
	//присоединение аддонов к активному слоту (2 или 3)

	CScope*				pScope = smart_cast<CScope*>			(item);
	CSilencer*			pSilencer = smart_cast<CSilencer*>		(item);
	CGrenadeLauncher*	pGrenadeLauncher = smart_cast<CGrenadeLauncher*>	(item);
	CInventory*			inv = &m_pActorInvOwner->inventory();

	PIItem	item_in_slot_2 = inv->ItemFromSlot(INV_SLOT_2);
	PIItem	item_in_slot_3 = inv->ItemFromSlot(INV_SLOT_3);

	if (!item_in_slot_2 && !item_in_slot_3)	return;

	if (pScope)
	{
		if (item_in_slot_2 && item_in_slot_2->CanAttach(pScope))
		{
			shared_str str = CStringTable().translate("st_attach_scope_to_pistol");
			str.printf("%s %s", str.c_str(), item_in_slot_2->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_slot_2, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		if (item_in_slot_3 && item_in_slot_3->CanAttach(pScope))
		{
			shared_str str = CStringTable().translate("st_attach_scope_to_pistol");
			str.printf("%s %s", str.c_str(), item_in_slot_3->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_slot_3, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		return;
	}

	if (pSilencer)
	{
		if (item_in_slot_2 && item_in_slot_2->CanAttach(pSilencer))
		{
			shared_str str = CStringTable().translate("st_attach_silencer_to_pistol");
			str.printf("%s %s", str.c_str(), item_in_slot_2->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_slot_2, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		if (item_in_slot_3 && item_in_slot_3->CanAttach(pSilencer))
		{
			shared_str str = CStringTable().translate("st_attach_silencer_to_pistol");
			str.printf("%s %s", str.c_str(), item_in_slot_3->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_slot_3, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		return;
	}

	if (pGrenadeLauncher)
	{
		if (item_in_slot_2 && item_in_slot_2->CanAttach(pGrenadeLauncher))
		{
			shared_str str = CStringTable().translate("st_attach_gl_to_rifle");
			str.printf("%s %s", str.c_str(), item_in_slot_2->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_slot_2, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		if (item_in_slot_3 && item_in_slot_3->CanAttach(pGrenadeLauncher))
		{
			shared_str str = CStringTable().translate("st_attach_gl_to_rifle");
			str.printf("%s %s", str.c_str(), item_in_slot_3->m_name.c_str());
			m_UIPropertiesBox->AddItem(str.c_str(), (void*)item_in_slot_3, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
	}
}

void CUIActorMenu::PropertiesBoxForUsing(PIItem item, bool& b_show)
{
	if (dynamic_cast<CFoodItem*>(item))
	{
		const char* act_str = READ_IF_EXISTS(pSettings, r_string, item->object().cNameSect().c_str(), "st_use_action_name", "st_use");

		m_UIPropertiesBox->AddItem(act_str, (void*)item, INVENTORY_EAT_ACTION);
		b_show = true;
	}
	else b_show = false;
}

void CUIActorMenu::PropertiesBoxForPlaying(PIItem item, bool& b_show)
{
	CPda* pPda = smart_cast<CPda*>(item);
	if(!pPda || !pPda->CanPlayScriptFunction())
		return;

	LPCSTR act_str = "st_play";
	m_UIPropertiesBox->AddItem(act_str,  nullptr, INVENTORY_PLAY_ACTION);
	b_show = true;
}

void CUIActorMenu::PropertiesBoxForDrop( CUICellItem* cell_item, PIItem item, bool& b_show )
{
	if ( !item->IsQuestItem() )
	{
		m_UIPropertiesBox->AddItem( "st_drop", nullptr, INVENTORY_DROP_ACTION );
		b_show = true;

		if ( cell_item->ChildsCount() )
		{
			m_UIPropertiesBox->AddItem( "st_drop_all", (void*)33, INVENTORY_DROP_ACTION );
		}
	}
}

void CUIActorMenu::PropertiesBoxForRepair( PIItem item, bool& b_show )
{
	CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>( item );
	CWeapon*       pWeapon = smart_cast<CWeapon*>( item );
	CHelmet*       pHelmet = smart_cast<CHelmet*>( item );

	if ( (pOutfit || pWeapon || pHelmet) && item->GetCondition() < 0.99f )
	{
		m_UIPropertiesBox->AddItem( "ui_inv_repair", nullptr, INVENTORY_REPAIR );
		b_show = true;
	}
}

void CUIActorMenu::ProcessPropertiesBoxClicked( CUIWindow* w, void* d )
{
	PIItem			item		= CurrentIItem();
	CUICellItem*	cell_item	= CurrentItem();
	if ( !m_UIPropertiesBox->GetClickedItem() || !item || !cell_item || !cell_item->OwnerList() )
	{
		return;
	}
	CWeapon* pWeapon = smart_cast<CWeapon*>(item);

	switch ( m_UIPropertiesBox->GetClickedItem()->GetTAG() )
	{
	case INVENTORY_TO_SLOT2_ACTION:	if (pWeapon) ToSlot(cell_item, true, INV_SLOT_2);		break;
	case INVENTORY_TO_SLOT3_ACTION:	if (pWeapon) ToSlot(cell_item, true, INV_SLOT_3);		break;
	case INVENTORY_TO_SLOT_ACTION:	ToSlot( cell_item, true, item->BaseSlot() );		break;
	case INVENTORY_RELOAD_MAGAZINE: if (pWeapon) pWeapon->Action(kWPN_RELOAD, CMD_START); break;
	case INVENTORY_TO_BELT_ACTION:	ToBelt( cell_item, false );		break;
	case INVENTORY_TO_BAG_ACTION:	ToBag ( cell_item, false );		break;
	case INVENTORY_EAT_ACTION:		TryUseFoodItem( cell_item ); 	break;
	case INVENTORY_REPAIR:			TryRepairItem(this, nullptr); return;

	case INVENTORY_DROP_ACTION:
		{
			if (!item->m_pInventory)
				return;
			void* d = m_UIPropertiesBox->GetClickedItem()->GetData();
			if (d == (void*)33)
			{
				DropAllCurrentItem();
			}
			else
			{
				SendEvent_Item_Drop(item, m_pActorInvOwner->object_id());
			}

			cell_item->OwnerList()->RemoveItem(cell_item, false);
		break;
		}
	case INVENTORY_ATTACH_ADDON:
		{
			// temporary storing because of AttachAddon is setting curiitem to NULL
			PIItem item = CurrentIItem();
			AttachAddon((PIItem)(m_UIPropertiesBox->GetClickedItem()->GetData()));
			
			switch(m_currMenuMode)
			{
				case mmInventory: RemoveItemFromList(m_pInventoryBagList, item); break;
				case mmDeadBodyOrContainerSearch: RemoveItemFromList(m_pDeadBodyBagList, item); break;
			}

			break;
		}
	case INVENTORY_DETACH_SCOPE_ADDON:
		if (pWeapon)
		{
			DetachAddon(pWeapon->GetScopeName().c_str() );
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*	child_itm	= cell_item->Child(i);
				PIItem			child_iitm	= (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>( child_iitm );
				if ( child_iitm && wpn )
				{
					DetachAddon(wpn->GetScopeName().c_str(), child_iitm);
				}
			}
		}
		break;
	case INVENTORY_DETACH_SILENCER_ADDON:
		if (pWeapon)
		{
			DetachAddon(pWeapon->GetSilencerName().c_str() );
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*	child_itm	= cell_item->Child(i);
				PIItem			child_iitm	= (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>( child_iitm );
				if ( child_iitm && wpn )
				{
					DetachAddon(wpn->GetSilencerName().c_str(), child_iitm);
				}
			}
		}
		break;
	case INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON:
		if (pWeapon)
		{
			DetachAddon(pWeapon->GetGrenadeLauncherName().c_str() );
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*	child_itm	= cell_item->Child(i);
				PIItem			child_iitm	= (PIItem)(child_itm->m_pData);
				CWeapon* wpn = smart_cast<CWeapon*>( child_iitm );
				if ( child_iitm && wpn )
				{
					DetachAddon(wpn->GetGrenadeLauncherName().c_str(), child_iitm);
				}
			}
		}
		break;
	case INVENTORY_UNLOAD_MAGAZINE:
		{
			CWeaponMagazined* weap_mag = smart_cast<CWeaponMagazined*>( (CWeapon*)cell_item->m_pData );
			if ( !weap_mag )
			{
				break;
			}
			weap_mag->UnloadMagazine();
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*		child_itm		= cell_item->Child(i);
				CWeaponMagazined*	child_weap_mag	= smart_cast<CWeaponMagazined*>( (CWeapon*)child_itm->m_pData );
				if ( child_weap_mag )
				{
					child_weap_mag->UnloadMagazine();
				}
			}
			break;
		}
	case INVENTORY_PLAY_ACTION:
		{
			CPda* pPda = smart_cast<CPda*>(item);
			if(pPda)
				pPda->PlayScriptFunction();
			break;
		}
	}//switch

	SetCurrentItem( nullptr );
	UpdateItemsPlace();
	UpdateConditionProgressBars();
}

void CUIActorMenu::UpdateOutfit()
{
	for (u8 i = 0; i < e_af_count; ++i)
	{
		m_belt_list_over[i]->SetVisible(true);
	}

	const u32 af_count = m_pActorInvOwner->inventory().BeltWidth();
	VERIFY(0 <= af_count && af_count <= 5);

	VERIFY(m_pInventoryBeltList);
	CCustomOutfit* outfit = m_pActorInvOwner->GetOutfit();

	if (!outfit)
	{
		MoveArtefactsToBag();
		m_HelmetOver->Show(false);
		return;
	}
	m_HelmetOver->Show(!outfit->bIsHelmetAvaliable);
	
	Ivector2 afc;
    if (g_extraFeatures.is(GAME_EXTRA_VERTICAL_BELTS))
    {
        afc.x = 1;
        afc.y = af_count;
    }
    else
    {
        afc.x = af_count;
        afc.y = 1;
    }

	m_pInventoryBeltList->SetCellsCapacity(afc);

	for (u8 i = 0; i < af_count; ++i)
	{
		m_belt_list_over[i]->SetVisible(false);
	}

}

void CUIActorMenu::MoveArtefactsToBag()
{
	while (m_pInventoryBeltList->ItemsCount())
	{
		CUICellItem* ci = m_pInventoryBeltList->GetItemIdx(0);
		VERIFY(ci && ci->m_pData);
		ToBag(ci, false);
	}
	m_pInventoryBeltList->ClearAll(true);
}