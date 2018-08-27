#include "stdafx.h"

#include "UIInventoryUtilities.h"

#include "uicharacterinfo.h"
#include "../actor.h"
#include "../level.h"
#include "../../xrServerEntities/character_info.h"
#include "../string_table.h"
#include "../relation_registry.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"

#include "uistatic.h"
#include "UIScrollView.h"

#include "../alife_simulator.h"
#include "../ai_space.h"
#include "../alife_object_registry.h"
#include "../xrServer.h"
#include "../../xrServerEntities/xrServer_Objects_ALife_Monsters.h"
#include "../../FrayBuildConfig.hpp"

using namespace InventoryUtilities;

CSE_ALifeTraderAbstract* ch_info_get_from_id (u16 id)
{
	if( ai().get_alife() && ai().get_game_graph() )
	{
		return	smart_cast<CSE_ALifeTraderAbstract*>(ai().alife().objects().object(id));
	}else{
		return	smart_cast<CSE_ALifeTraderAbstract*>(Level().Server->game->get_entity_from_eid(id));
	}
}

CUICharacterInfo::CUICharacterInfo()
	: m_ownerID(u16(-1)),
	pUIBio(NULL)
{
    std::memset(m_icons,0,sizeof(m_icons));
	m_bForceUpdate		= false;
	m_texture_name		= NULL;
}

CUICharacterInfo::~CUICharacterInfo()
{}

void CUICharacterInfo::InitCharacterInfo(Fvector2 pos, Fvector2 size, CUIXml* xml_doc)
{
	inherited::SetWndPos(pos);
	inherited::SetWndSize(size);

	Init_IconInfoItem(*xml_doc, "icon", eIcon);
	Init_IconInfoItem(*xml_doc, "icon_over", eIconOver);

	VERIFY(m_icons[eIcon]);

	m_deadbody_color = color_argb(205, 255, 160, 160);
	if (xml_doc->NavigateToNode("icon:deadbody", 0))
	{
		m_deadbody_color = CUIXmlInit::GetColor(*xml_doc, "icon:deadbody", 0, m_deadbody_color);
	}

	// ----------------------------
	Init_StrInfoItem( *xml_doc, "name_caption",      eNameCaption       );
	Init_StrInfoItem( *xml_doc, "name_static",       eName              );

	Init_StrInfoItem( *xml_doc, "rank_caption",      eRankCaption       );
	Init_StrInfoItem( *xml_doc, "rank_static",       eRank              );
	
	Init_StrInfoItem( *xml_doc, "community_caption", eCommunityCaption  );
	Init_StrInfoItem( *xml_doc, "community_static",  eCommunity         );

	Init_StrInfoItem( *xml_doc, "reputation_caption",eReputationCaption );
	Init_StrInfoItem( *xml_doc, "reputation_static", eReputation        );

	Init_StrInfoItem( *xml_doc, "relation_caption",  eRelationCaption   );
	Init_StrInfoItem( *xml_doc, "relation_static",   eRelation          );

	if (xml_doc->NavigateToNode("biography_list", 0))
	{
		pUIBio = xr_new<CUIScrollView>();
		pUIBio->SetAutoDelete(true);
		CUIXmlInit::InitScrollView( *xml_doc, "biography_list", 0, pUIBio );
		AttachChild(pUIBio);
	}
}

void CUICharacterInfo::Init_StrInfoItem( CUIXml& xml_doc, LPCSTR item_str, UIItemType type )
{
	if ( xml_doc.NavigateToNode( item_str, 0 ) )
	{
		CUIStatic*	pItem = m_icons[type] = xr_new<CUIStatic>();
		CUIXmlInit::InitStatic( xml_doc, item_str, 0, pItem );
		AttachChild( pItem );
		pItem->SetAutoDelete( true );
	}
}

void CUICharacterInfo::Init_IconInfoItem( CUIXml& xml_doc, LPCSTR item_str, UIItemType type )
{
	if ( xml_doc.NavigateToNode( item_str, 0 ) )
	{
		CUIStatic*	pItem = m_icons[type] = xr_new<CUIStatic>();
		CUIXmlInit::InitStatic( xml_doc, item_str, 0, pItem );
		
		pItem->Show( true );
		pItem->Enable( true );
		AttachChild( pItem );
		pItem->SetAutoDelete( true );
	}
}

void CUICharacterInfo::InitCharacterInfo(Fvector2 pos, Fvector2 size, LPCSTR xml_name)
{
	CUIXml						uiXml;
	uiXml.Load					(CONFIG_PATH, UI_PATH, xml_name);
	InitCharacterInfo			(pos, size,&uiXml);
}

void CUICharacterInfo::InitCharacterInfo(CUIXml* xml_doc, LPCSTR node_str)
{
	Fvector2 pos, size;
	XML_NODE* stored_root		= xml_doc->GetLocalRoot();
	XML_NODE* ch_node			= xml_doc->NavigateToNode(node_str,0);
	xml_doc->SetLocalRoot		(ch_node);
	pos.x						= xml_doc->ReadAttribFlt(ch_node, "x");
	pos.y						= xml_doc->ReadAttribFlt(ch_node, "y");
	size.x						= xml_doc->ReadAttribFlt(ch_node, "width");
	size.y						= xml_doc->ReadAttribFlt(ch_node, "height");
	InitCharacterInfo			(pos, size, xml_doc);
	xml_doc->SetLocalRoot		(stored_root);
}

void CUICharacterInfo::InitMonsterCharacter(shared_str monster_tex_name)
{
	if (m_icons[eIcon])
	{
		m_icons[eIcon]->InitTexture(monster_tex_name.c_str());
		m_icons[eIcon]->SetStretchTexture(true);
		m_icons[eIcon]->SetTextureColor(m_deadbody_color); // (color_argb(255, 255, 160, 160));
		m_icons[eIcon]->Show(true);
	}
}

void CUICharacterInfo::InitCharacter(u16 id)
{
	m_ownerID = id;

	CSE_ALifeTraderAbstract*	T = ch_info_get_from_id(m_ownerID);

	CCharacterInfo				chInfo;
	chInfo.Init(T);

	if (m_icons[eName]) { m_icons[eName]->TextItemControl()->SetTextST(T->m_character_name.c_str()); }
	if (m_icons[eRank]) { m_icons[eRank]->TextItemControl()->SetTextST(GetRankAsText(chInfo.Rank().value())); }
	if (m_icons[eCommunity]) { m_icons[eCommunity]->TextItemControl()->SetTextST(chInfo.Community().id().c_str()); }
	if (m_icons[eReputation]) { m_icons[eReputation]->TextItemControl()->SetTextST(GetReputationAsText(chInfo.Reputation().value())); }

	// Bio
	if (pUIBio && pUIBio->IsEnabled())
	{
		pUIBio->Clear();
		if (chInfo.Bio().size())
		{
			CUITextWnd* pItem = xr_new<CUITextWnd>();
			pItem->SetWidth(pUIBio->GetDesiredChildWidth());
			pItem->SetText(chInfo.Bio().c_str());
			pItem->AdjustHeightToText();
			pUIBio->AddWindow(pItem, true);
		}
	}

	shared_str const& comm_id = chInfo.Community().id();
	LPCSTR   community0 = comm_id.c_str();
	string64 community1;
	xr_strcpy(community1, sizeof(community1), community0);
	xr_strcat(community1, sizeof(community1), "_icon");

	string64 community2;
	xr_strcpy(community2, sizeof(community2), community0);
	xr_strcat(community2, sizeof(community2), "_wide");

	m_bForceUpdate = true;
	for (int i = eIcon; i < eMaxCaption; ++i)
	{
		if (m_icons[i])
		{
			m_icons[i]->Show(true);
		}
	}

	m_texture_name = chInfo.IconName();

	if (m_icons[eIcon]) 
	{ 
		m_icons[eIcon]->InitTexture(m_texture_name.c_str()); 
		m_icons[eIcon]->SetTextureColor(color_argb(205, 160, 160, 160));
	}
}

void  CUICharacterInfo::SetRelation( ALife::ERelationType relation, CHARACTER_GOODWILL goodwill )
{
	if ( !m_icons[eRelation] || !m_icons[eRelationCaption] )
	{
		return;
	}
	m_icons[eRelation]->TextItemControl()->SetTextColor( GetRelationColor(  relation ) );
	m_icons[eRelation]->TextItemControl()->SetTextST(    GetGoodwillAsText( goodwill ) );
}


//////////////////////////////////////////////////////////////////////////

void CUICharacterInfo::ResetAllStrings()
{
	if(m_icons[eName])			m_icons[eName]->TextItemControl()->SetText			("");
	if(m_icons[eRank])			m_icons[eRank]->TextItemControl()->SetText			("");
	if(m_icons[eCommunity])		m_icons[eCommunity]->TextItemControl()->SetText		("");
	if(m_icons[eReputation])	m_icons[eReputation]->TextItemControl()->SetText	("");
	if(m_icons[eRelation])		m_icons[eRelation]->TextItemControl()->SetText		("");
}

void CUICharacterInfo::UpdateRelation()
{
	if ( !m_icons[eRelation] || !m_icons[eRelationCaption] )
	{
		return;
	}

	if ( Actor()->ID() == m_ownerID || !hasOwner() )
	{
		m_icons[eRelationCaption]->Show(false);
		m_icons[eRelation]->Show(false);
	}
	else
	{
		m_icons[eRelationCaption]->Show(true);
		m_icons[eRelation]->Show(true);

		CSE_ALifeTraderAbstract* T = ch_info_get_from_id	(m_ownerID);
		CSE_ALifeTraderAbstract* TA = ch_info_get_from_id	(Actor()->ID());

		SetRelation( RELATION_REGISTRY().GetRelationType( T, TA ), RELATION_REGISTRY().GetAttitude( T, TA ) );
	}
}

namespace detail
{ // helper function implemented in file alife_simulator.cpp
	bool object_exists_in_alife_registry (u32 id);
} // namespace detail

void CUICharacterInfo::Update()
{
	inherited::Update();

	if ( hasOwner() && ( m_bForceUpdate ||(Device.dwFrame%50 == 0) )  )
	{
		m_bForceUpdate = false;

		CSE_ALifeTraderAbstract* T = detail::object_exists_in_alife_registry(m_ownerID) ?
									 ch_info_get_from_id(m_ownerID) : NULL;
		if (NULL==T){
			m_ownerID = u16(-1);
			return;
		}
		else
		{
			UpdateRelation();
		}

		if ( m_icons[eIcon] )
		{
			CSE_ALifeCreatureAbstract* pCreature = smart_cast<CSE_ALifeCreatureAbstract*>(T);
			if (pCreature && !pCreature->g_Alive())
			{
				m_icons[eIcon]->SetTextureColor(m_deadbody_color);
			}
		}
	}
}

void CUICharacterInfo::ClearInfo()
{
	ResetAllStrings	();
	
	for ( int i = eIcon; i < eMaxCaption; ++i )
	{
		if ( m_icons[i] )
		{
			m_icons[i]->Show( false );
		}
	}
}

// ------- static ---------
bool CUICharacterInfo::get_actor_community( shared_str* our, shared_str* enemy )
{
	VERIFY( our && enemy );
	our->_set( NULL );
	enemy->_set( NULL );
	shared_str const& actor_team = Actor()->CharacterInfo().Community().id();

	LPCSTR vs_teams  = pSettings->r_string( "actor_communities", actor_team.c_str() );
	if ( _GetItemCount( vs_teams ) != 2 )
	{
		return false;
	}
	u32   size_temp   = (xr_strlen(vs_teams) + 1) * sizeof(char);
	PSTR  our_fract   = (PSTR)_alloca( size_temp );
	PSTR  enemy_fract = (PSTR)_alloca( size_temp );
	_GetItem( vs_teams, 0, our_fract, size_temp );
	_GetItem( vs_teams, 1, enemy_fract, size_temp );

	if ( xr_strlen(our_fract) == 0 || xr_strlen(enemy_fract) == 0 )
	{
		return false;
	}
	our->_set( our_fract );
	enemy->_set( enemy_fract );
	return true;
}

bool CUICharacterInfo::ignore_community(shared_str const& check_community)
{
	LPCSTR comm_section_str = "ignore_icons_communities";
	VERIFY2(pSettings->section_exist(comm_section_str), make_string("Section [%s] does not exist !", comm_section_str));

	CInifile::Sect& faction_section = pSettings->r_section(comm_section_str);

	for (CInifile::Item itm : faction_section.Data)
	{
		if (check_community == itm.first)
		{
			return true;
		}
	}
	return false;
}
#include <luabind/luabind.hpp>

#include "ai_space.h"
#include "script_engine.h"
void CUICharacterInfo::SetActorIcon()
{
	shared_str icon_name = "ui_inGame2_Hero";
	luabind::functor<LPCSTR> functor;
	if (ai().script_engine().functor("oxy_callbacks.get_actor_icon", functor))
		icon_name = functor();

	m_texture_name = icon_name;
	if (m_icons[eIcon])
		m_icons[eIcon]->InitTexture(m_texture_name.c_str());
}