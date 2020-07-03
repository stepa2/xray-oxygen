#include "stdafx.h"
#include "UIGameTutorial.h"
#include "../xrUICore/UIStatic.h"
#include "../xrUICore/UICursor.h"
#include "../xrUICore/UIXmlInit.h"
#include "object_broker.h"
#include "../../xrEngine/xr_input.h"
#include "../xrEngine/xr_level_controller.h"
#include "../UIGame.h"
#include "../level.h"
#include "UIPdaWnd.h"
#include "UIActorMenu.h"
#include "UITalkWnd.h"
#include "level_changer.h"
#include "../../xrUICore/MainMenu.h"
#include "../../xrServerEntities/script_engine.h"
#include "../ai_space.h"

extern ENGINE_API BOOL bShowPauseString;

CUISequenceSimpleItem::~CUISequenceSimpleItem()
{
	auto _I = m_subitems.begin();
	auto _E = m_subitems.end();

	for (; _I != _E; ++_I)
		_I->Stop();

	m_subitems.clear();
	m_sound.stop();
	delete_data(m_UIWindow);
}

bool CUISequenceSimpleItem::IsPlaying()
{
	if (m_time_start < 0.0f)
		return true;

	return (m_time_start + m_time_length) > (Device.dwTimeContinual / 1000.0f);
}

CUIWindow* find_child_window(CUIWindow* parent, const shared_str& _name)
{
	CUIWindow::WINDOW_LIST& wl = parent->GetChildWndList();
	CUIWindow::WINDOW_LIST_it _I = wl.begin();
	CUIWindow::WINDOW_LIST_it _E = wl.end();

	for (; _I != _E; ++_I)
		if ((*_I)->WindowName() == _name)
			return (*_I);

	return nullptr;
}

void CUISequenceSimpleItem::Load(CUIXml* xml, int idx)
{
	CUISequenceItem::Load(xml, idx);

	XML_NODE* _stored_root = xml->GetLocalRoot();
	xml->SetLocalRoot(xml->NavigateToNode("item", idx));

	LPCSTR m_snd_name = xml->Read("sound", 0, "");

	if (m_snd_name&&m_snd_name[0])
	{
		m_sound.create(m_snd_name, st_Effect, sg_Undefined);
		VERIFY(m_sound._handle() || strstr(Core.Params, "-nosound"));
	}

	m_time_length = xml->ReadFlt("length_sec", 0, 0);
	m_desired_cursor_pos.x = xml->ReadAttribFlt("cursor_pos", 0, "x", 0);
	m_desired_cursor_pos.y = xml->ReadAttribFlt("cursor_pos", 0, "y", 0);
	xr_strcpy(m_pda_section, xml->Read("pda_section", 0, ""));

	LPCSTR str = xml->Read("pause_state", 0, "ignore");
	m_flags.set(etiNeedPauseOn, 0 == _stricmp(str, "on"));
	m_flags.set(etiNeedPauseOff, 0 == _stricmp(str, "off"));
	m_flags.set(etiNeedPauseSound, 0 == _stricmp(str, "on"));

	str = xml->Read("guard_key", 0, NULL);
	m_continue_VK_guard = -1;

	if (str && !_stricmp(str, "any"))
	{
		m_continue_VK_guard = 9999;
		str = nullptr;
	}

	if (str)
	{
		EGameActions cmd = action_name_to_id(str);
		m_continue_VK_guard = get_action_dik(cmd);
	}

	m_flags.set(etiCanBeStopped, (m_continue_VK_guard == -1));

	LPCSTR str_grab_input = xml->Read("grab_input", 0, "on");
	m_flags.set(etiGrabInput, (0 == _stricmp(str_grab_input, "on") || 0 == _stricmp(str_grab_input, "1")));

	int actions_count = xml->GetNodesNum(0, 0, "action");
	m_actions.resize(actions_count);

	for (int idx = 0; idx < actions_count; ++idx)
	{
		SActionItem& itm = m_actions[idx];
		LPCSTR str = xml->ReadAttrib("action", idx, "id");
		itm.m_action = action_name_to_id(str);
		itm.m_bfinalize = !!xml->ReadAttribInt("action", idx, "finalize", FALSE);
		itm.m_functor = xml->Read(xml->GetLocalRoot(), "action", idx, "");
	}

	//ui-components
	m_UIWindow = xr_new<CUIWindow>();
	m_UIWindow->SetAutoDelete(false);
	XML_NODE* _lsr = xml->GetLocalRoot();
	CUIXmlInit xml_init;
	xml_init.InitWindow(*xml, "main_wnd", 0, m_UIWindow);
	xml->SetLocalRoot(_lsr);

	// initialize auto_static
	int cnt = xml->GetNodesNum("main_wnd", 0, "auto_static");
	m_subitems.resize(cnt);
	string64 sname;
	for (int i = 0; i < cnt; ++i)
	{
		XML_NODE* _sr = xml->GetLocalRoot();
		xml->SetLocalRoot(xml->NavigateToNode("main_wnd", 0));

		xr_sprintf(sname, "auto_static_%d", i);

		SSubItem* _si = &m_subitems[i];
		_si->m_start = xml->ReadAttribFlt("auto_static", i, "start_time", 0);
		_si->m_length = xml->ReadAttribFlt("auto_static", i, "length_sec", 0);

		_si->m_visible = false;
		_si->m_wnd = smart_cast<CUIStatic*>(find_child_window(m_UIWindow, sname));
		VERIFY(_si->m_wnd);

		_si->m_wnd->TextItemControl()->SetTextComplexMode(true);
		_si->m_wnd->Show(false);
		_si->m_wnd->SetWidth(_si->m_wnd->GetWidth()*UI().get_current_kx());

		if (UI().is_widescreen())
		{
			XML_NODE* autostatic_node = xml->NavigateToNode("auto_static", i);
			XML_NODE* ws_rect = xml->NavigateToNode(autostatic_node, "widescreen_rect", 0);
			if (ws_rect)
			{
				xml->SetLocalRoot(autostatic_node);

				Fvector2 pos, size;
				pos.x = xml->ReadAttribFlt("widescreen_rect", 0, "x");
				pos.y = xml->ReadAttribFlt("widescreen_rect", 0, "y");
				size.x = xml->ReadAttribFlt("widescreen_rect", 0, "width");
				size.y = xml->ReadAttribFlt("widescreen_rect", 0, "height");
				_si->m_wnd->SetWndPos(pos);
				_si->m_wnd->SetWndSize(size);
			}
		}

		xml->SetLocalRoot(_sr);
	}
	xml->SetLocalRoot(_stored_root);
}

void CUISequenceSimpleItem::SSubItem::Start()
{
	m_wnd->Show(true);
	m_wnd->ResetColorAnimation();
	m_visible = true;
}

void CUISequenceSimpleItem::SSubItem::Stop()
{
	m_wnd->Show(false);
	m_visible = false;
}

void CUISequenceSimpleItem::OnRender()
{
	if (m_time_start < -2.0f)
		m_time_start = -1.0f;
	else
		if (m_time_start < 0.0f)
			m_time_start = float(Device.dwTimeContinual) / 1000.0f;
}

float CUISequenceSimpleItem::current_factor()
{
	if (m_time_start < 0.0f || fis_zero(m_time_length))
		return 0.0f;
	else
		return ((Device.dwTimeContinual / 1000.0f) - m_time_start) / m_time_length;
}

void CUISequenceSimpleItem::Update()
{
	inherited::Update();
	float _start = (m_time_start < 0.0f) ? (float(Device.dwTimeContinual) / 1000.0f) : m_time_start;

	float gt = float(Device.dwTimeContinual) / 1000.0f;
	auto _I = m_subitems.begin();
	auto _E = m_subitems.end();
	for (; _I != _E; ++_I)
	{
		SSubItem& s = *_I;
		bool bPlaying = (gt > (_start + s.m_start - EPS)) && (gt < (_start + s.m_start + s.m_length + EPS));

		if (true == bPlaying && (false == s.m_visible))
			s.Start();
		else
			if ((false == bPlaying) && (true == s.m_visible))
				s.Stop();
	}

	if (g_pGameLevel && (!m_pda_section || 0 == xr_strlen(m_pda_section)))
	{
		if (GameUI()->PdaMenu().IsShown() || GameUI()->ActorMenu().IsShown() || GameUI()->TalkMenu->IsShown() || GameUI()->UIChangeLevelWnd->IsShown() ||
			(MainMenu()->IsActive() && !m_owner->m_flags.test(CUISequencer::etsOverMainMenu)))
				m_UIWindow->Show(false);
			else
				m_UIWindow->Show(true);
	}
	if (m_desired_cursor_pos.x && m_desired_cursor_pos.y)
		GetUICursor().SetUICursorPosition(m_desired_cursor_pos);
}

void CUISequenceSimpleItem::Start()
{
	m_time_start = -3.0f;
	inherited::Start();
	m_flags.set(etiStoredPauseState, Device.Paused());

	if (m_flags.test(etiNeedPauseOn) && !m_flags.test(etiStoredPauseState))
	{
		Device.Pause(true, true, false, "simpleitem_start");
		bShowPauseString = FALSE;
	}

	if (m_flags.test(etiNeedPauseOff) && m_flags.test(etiStoredPauseState))
		Device.Pause(false, true, false, "simpleitem_startPauseState");

	if (m_flags.test(etiNeedPauseSound))
		Device.Pause(true, false, true, "simpleitem_startSound");

	if (m_desired_cursor_pos.x && m_desired_cursor_pos.y)
		GetUICursor().SetUICursorPosition(m_desired_cursor_pos);

	m_owner->MainWnd()->AttachChild(m_UIWindow);

	if (m_sound._handle())
		m_sound.play(nullptr, sm_2D);

	if (g_pGameLevel)
	{
		bool bShowPda = false;

		if (!stricmp(m_pda_section, "pda_tasks"))
		{
			GameUI()->PdaMenu().SetActiveSubdialog("eptTasks");
			bShowPda = true;
		}
		else if (!stricmp(m_pda_section, "pda_ranking"))
		{
			GameUI()->PdaMenu().SetActiveSubdialog("eptRanking");
			bShowPda = true;
		}
		else if (!stricmp(m_pda_section, "pda_logs"))
		{
			GameUI()->PdaMenu().SetActiveSubdialog("eptLogs");
			bShowPda = true;
		}
		else if (!stricmp(m_pda_section, "pda_show_second_task_wnd"))
		{
			GameUI()->PdaMenu().Show_SecondTaskWnd(true);
			bShowPda = true;
		}

		if ((!GameUI()->PdaMenu().IsShown() && bShowPda) || (GameUI()->PdaMenu().IsShown() && !bShowPda))
			GameUI()->PdaMenu().HideDialog();
	}
}

bool CUISequenceSimpleItem::Stop(bool bForce)
{
	if (!m_flags.test(etiCanBeStopped) && !bForce)
		return false;

	if (m_UIWindow->GetParent() == m_owner->MainWnd()) //started??
		m_owner->MainWnd()->DetachChild(m_UIWindow);

	m_sound.stop();

	if (m_flags.test(etiNeedPauseOn) && !m_flags.test(etiStoredPauseState))
		Device.Pause(false, false, false, "simpleitem_stop");

	if (m_flags.test(etiNeedPauseOff) && m_flags.test(etiStoredPauseState))
		Device.Pause(true, true, false, "simpleitem_stopPauseState");

	if (m_flags.test(etiNeedPauseSound))
		Device.Pause(false, true, true, "simpleitem_stopSound");

	if (g_pGameLevel)
	{
		if (GameUI()->PdaMenu().IsShown())
			GameUI()->PdaMenu().HideDialog();
	}
	inherited::Stop();
	return true;
}

#include <luabind/luabind.hpp>
void CUISequenceSimpleItem::OnKeyboardPress(u8 dik)
{
	if (!m_flags.test(etiCanBeStopped))
	{
		VERIFY(m_continue_VK_guard != -1);
		if (m_continue_VK_guard == -1)m_flags.set(etiCanBeStopped, TRUE); //not binded action :(

		if (m_continue_VK_guard == 9999 || dik == m_continue_VK_guard)
			m_flags.set(etiCanBeStopped, TRUE); //match key
	}

	for (u32 idx = 0; idx < m_actions.size(); ++idx)
	{
		SActionItem& itm = m_actions[idx];
		bool b = is_binded(itm.m_action, dik);
		if (b)
		{
			luabind::functor<void>	functor_to_call;
			bool functor_exists = ai().script_engine().functor(itm.m_functor.c_str(), functor_to_call);
			THROW3(functor_exists, "Cannot find script function described in tutorial item ", itm.m_functor.c_str());
			functor_to_call();

			if (itm.m_bfinalize)
			{
				m_flags.set(etiCanBeStopped, TRUE);
				m_stop_lua_functions.clear();
				Stop();
			}
		}
	}
}

void CUISequenceSimpleItem::OnMousePress(int btn)
{
	u8 dik = 0;

	switch (btn)
	{
		case 0:
			dik = VK_LBUTTON;
			break;

		case 1:
			dik = VK_RBUTTON;
			break;

		case 2:
			dik = VK_MBUTTON;
			break;

		default: 
			return;
	}
	OnKeyboardPress(dik);
}