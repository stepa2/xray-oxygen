#pragma once
#include "UIStatic.h"
#include "../xrScripts/export/script_export_space.h"

class CUI3tButton;

class UI_API CUIMessageBox: public CUIStatic
{
private:
	typedef CUIStatic inherited;
public:
				CUIMessageBox		();
	virtual		~CUIMessageBox		();

	//разновидности MessageBox
	typedef enum {		
		MESSAGEBOX_OK, 
		MESSAGEBOX_INFO,
		MESSAGEBOX_YES_NO, 
		MESSAGEBOX_YES_NO_CANCEL,
		MESSAGEBOX_QUIT_WINDOWS, 
		MESSAGEBOX_QUIT_GAME,
		MESSAGEBOX_YES_NO_COPY
	} E_MESSAGEBOX_STYLE;

	virtual void InitMessageBox		(LPCSTR xml_template);
			void Clear				();
	virtual void SetText			(LPCSTR str);
	virtual LPCSTR GetText			();

IC	E_MESSAGEBOX_STYLE GetBoxStyle() { return m_eMessageBoxStyle; };

	virtual bool OnMouseAction			(float x, float y, EUIMessages mouse_action);
	virtual void SendMessageToWnd		(CUIWindow *pWnd, s16 msg, void *pData);

	void		OnYesOk				();
protected:
	xr_string	m_ret_val;
	CUI3tButton* m_UIButtonYesOk;
	CUI3tButton* m_UIButtonNo;
	CUI3tButton* m_UIButtonCancel;
	CUI3tButton* m_UIButtonCopy;

	CUIStatic*	m_UIStaticPicture;
	CUITextWnd*	m_UIStaticText;

	E_MESSAGEBOX_STYLE m_eMessageBoxStyle;
	DECLARE_SCRIPT_REGISTER_FUNCTION
};
