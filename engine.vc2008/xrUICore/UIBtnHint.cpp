#include "stdafx.h"
#include "UIBtnHint.h"
#include "UIFrameWindow.h"
#include "UIStatic.h"
#include "UIXmlInit.h"

UI_API CUIButtonHint* g_btnHint = nullptr;
UI_API CUIButtonHint* g_statHint = nullptr;

CUIButtonHint::CUIButtonHint() :m_ownerWnd(nullptr), m_enabledOnFrame(false)
{
	CUIXmlInit xml_init;
	CXml uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, "pda.xml");
	xml_init.InitFrameWindow(uiXml, "button_hint", 0, this);

	m_text = xr_new<CUITextWnd>();
	m_text->SetAutoDelete(true);
	AttachChild(m_text);
	xml_init.InitTextWnd(uiXml, "button_hint:description", 0, m_text);
}

CUIButtonHint::~CUIButtonHint	()
{
}

void CUIButtonHint::OnRender	()
{
	if(m_enabledOnFrame)
	{
		m_text->Update		();
		SetTextureColor		(color_rgba(255,255,255,color_get_A(m_text->GetTextColor())));
		Draw				();
		m_enabledOnFrame	= false;
	}
}

void CUIButtonHint::SetHintText	(CUIWindow* w, LPCSTR text)
{
	m_ownerWnd					= w;
	m_text->SetTextST			(text);

	m_text->AdjustHeightToText	();

	Fvector2					new_size;
	new_size.x					= GetWndSize().x;
	new_size.y					= m_text->GetWndSize().y+20.0f;

	SetWndSize					(new_size);

	m_text->ResetColorAnimation	();
}
