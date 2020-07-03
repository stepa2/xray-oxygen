////////////////////////////////////////////////////////////////////////////
//	Module 		: UIInvUpgradeInfo.h
//	Created 	: 21.11.2007
//  Modified 	: 13.03.2009
//	Author		: Evgeniy Sokolov, Prishchepa Sergey
//	Description : inventory upgrade UI info window class
////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../xrUICore/UIWindow.h"
#include "../xrUICore/xrUIXmlParser.h"


namespace inventory { namespace upgrade {
	class Upgrade;
} } // namespace upgrade, inventory

class CUITextWnd;
class CUIFrameWindow;
class UIInvUpgPropertiesWnd;
class CInventoryItem;

class UIInvUpgradeInfo: public CUIWindow
{
private:
	typedef CUIWindow                    inherited;
	typedef inventory::upgrade::Upgrade  Upgrade_type;

public:
						UIInvUpgradeInfo();
	virtual				~UIInvUpgradeInfo();

			void		init_from_xml( LPCSTR xml_name );
			bool		init_upgrade( Upgrade_type* upgr, CInventoryItem* inv_item );
			bool		is_upgrade() { return (m_upgrade != NULL); }
	IC Upgrade_type const*	get_upgrade() const { return m_upgrade; }

	virtual void		Draw();

protected:
	Upgrade_type*		m_upgrade;
	CUIFrameWindow*		m_background;
	
	UIInvUpgPropertiesWnd*	m_properties_wnd;

	CUITextWnd*			m_name;
	CUITextWnd*			m_cost;
	CUITextWnd*			m_desc;
	CUITextWnd*			m_prereq;
	
}; // class UIInvUpgradeInfo