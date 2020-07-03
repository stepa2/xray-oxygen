#pragma once
#include "UIStatic.h"
#include "../xrScripts/export/script_export_space.h"

class UI_API CUIButton : public CUIStatic
{
private:
	typedef			CUIStatic				inherited;
public:
					CUIButton				();
	virtual			~CUIButton				()			{};

	virtual bool	OnMouseAction					(float x, float y, EUIMessages mouse_action);
	virtual void	OnClick					();

	//прорисовка окна
	virtual void	DrawTexture				();
	virtual void	DrawText				();

	virtual void	Update					();
	virtual void	Enable					(bool status);
	virtual bool	OnKeyboardAction				(u8 dik, EUIMessages keyboard_action);
	virtual void	OnFocusLost				();

	//состояния в которых находится кнопка
	typedef enum{BUTTON_NORMAL, //кнопка никак не затрагивается
		BUTTON_PUSHED,			//в нажатом сотоянии
		BUTTON_UP				//при удерживаемой кнопки мыши 
	} E_BUTTON_STATE;


	//заново подготовить состояние
    virtual void	Reset					();

	// Установка состояния кнопки: утоплена, не утоплена
	void				SetButtonState			(E_BUTTON_STATE eBtnState)	{ m_eButtonState = eBtnState; }
	E_BUTTON_STATE		GetButtonState			() const					{ return m_eButtonState;}

	// Поведение кнопки как переключателя реализовано пока только в режиме NORMAL_PRESS
	void				SetButtonAsSwitch		(bool bAsSwitch)			{ m_bIsSwitch = bAsSwitch; }

	// Работа с акселератором
	// Код акселератора берется из файла dinput.h, из DirectX SDK.
	// Например: кнопка A - код 0x1E(VK_A)
	void				SetAccelerator			(int iAccel, int idx);
	const int			GetAccelerator			(int idx) const;
	bool				IsAccelerator			(int iAccel) const;

	shared_str			m_hint_text;
protected:
	
	E_BUTTON_STATE		m_eButtonState;
	s16					m_uAccelerator[4];
	bool				m_bIsSwitch;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};