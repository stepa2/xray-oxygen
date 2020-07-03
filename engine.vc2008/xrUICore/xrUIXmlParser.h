#pragma once
#include "linker.h"
#include "../xrCore/XMLCore/xrXMLParser.h"

class UI_API CUIXml : public CXml
{
	int	m_dbg_id;
public:
	CUIXml();
	virtual	~CUIXml();

	virtual shared_str correct_file_name(LPCSTR path, LPCSTR fn);
};