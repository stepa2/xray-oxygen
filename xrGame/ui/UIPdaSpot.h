#pragma once
#include "../xrUICore/UIDialogWnd.h"
#include "../xrUICore/UIWndCallback.h"

class CUIXml;
class CUI3tButton;
class CUIStatic;
class CUIEditBox;
class UIHint;

class CUIPdaSpot : public CUIDialogWnd, public CUIWndCallback
{
	typedef CUIDialogWnd base_class;

	CUIStatic* m_background;
	CUIEditBox* m_editBox;
	CUI3tButton* m_btn_ok;
	CUI3tButton* m_btn_cancel;

	bool m_mainWnd;
	LPCSTR m_levelName;
	Fvector m_position;
	u16 m_spotID;
	shared_str m_spotType;

public:
	CUIPdaSpot();
	~CUIPdaSpot();

	void Init(u16 spot_id, LPCSTR level_name, Fvector pos, bool main_wnd);
	void InitControls();

	void  OnAdd(CUIWindow* w, void* d);
	void  OnApply(CUIWindow* w, void* d);
	void  OnExit(CUIWindow* w, void* d);
	void Exit();
	virtual bool OnKeyboardAction(u8 dik, EUIMessages keyboard_action);
	virtual void SendMessageToWnd(CUIWindow* pWnd, s16 msg, void* pData = NULL);
};