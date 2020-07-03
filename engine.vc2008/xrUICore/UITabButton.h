#pragma once
#include "UI3tButton.h"

class UI_API CUITabButton : public CUI3tButton 
{
	typedef CUI3tButton inherited;
public:
	shared_str						m_btn_id;

				CUITabButton		();
	virtual		~CUITabButton		();
	
	virtual void SendMessageToWnd		(CUIWindow* pWnd, s16 msg, void* pData = 0);
	virtual bool OnMouseAction			(float x, float y, EUIMessages mouse_action);
	virtual bool OnMouseDown		(int mouse_btn);
};