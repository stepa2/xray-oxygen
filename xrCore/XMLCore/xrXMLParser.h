#pragma once
#define CONFIG_PATH "$game_config$"
#define UI_PATH	 "ui"

#include "tinyxml.h"

using XML_NODE		= tinyxml2::XMLNode;
using XML_ATTRIBUTE = tinyxml2::XMLAttribute;

class XRCORE_API	CXml  
{
	void 				Load					(const char* path_alias, const char* xml_filename);
public:
	string_path			m_xml_file_name;
						CXml					();
	virtual				~CXml					();
	void				ClearInternal			();

	void 				Load					(const char* path_alias, const char* path, const char*  xml_filename);


	//чтение элементов
	const char* 		Read					(const char* path, int index,  const char*   default_str_val);
	const char* 		Read					(XML_NODE* start_node, const char* path, int index,  const char*   default_str_val );
	const char* 		Read					(XML_NODE* node,  const char*   default_str_val);

	int   				ReadInt					(const char* path, int index,  int default_int_val);
	int   				ReadInt					(XML_NODE* start_node,  const char* path, int index,  int default_int_val);
	int   				ReadInt					(XML_NODE* node,  int default_int_val);

	float   			ReadFlt					(const char* path, int index,  float default_flt_val);
	float   			ReadFlt					(XML_NODE* start_node,  const char* path, int index,  float default_flt_val);
	float   			ReadFlt					(XML_NODE* node,  float default_flt_val);


	const char*			ReadAttrib				(const char* path,  int index, const char* attrib, const char* default_str_val = "");
	const char*			ReadAttrib				(XML_NODE* start_node, const char* path,  int index, const char* attrib, const char* default_str_val = "");
	const char*			ReadAttrib				(XML_NODE* node, const char* attrib, const char* default_str_val);

	bool				ReadAttribBool			(const char* path, int index, const char* attrib, bool default_value = false);
	bool				ReadAttribBool			(XML_NODE* start_node, const char* path, int index, const char* attrib, bool default_value = false);
	bool				ReadAttribBool			(XML_NODE* node, const char* attrib, bool default_value = false);

	int					ReadAttribInt			(const char* path, int index, const char* attrib, int default_int_val = 0);
	int					ReadAttribInt			(XML_NODE* start_node, const char* path, int index, const char* attrib, int default_int_val = 0);
	int					ReadAttribInt			(XML_NODE* node, const char* attrib, int default_int_val);

	float   			ReadAttribFlt			(const char* path,	int index,  const char* attrib, float default_flt_val = 0.0f);
	float   			ReadAttribFlt			(XML_NODE* start_node, const char* path, int index,  const char* attrib, float default_flt_val = 0.0f);
	float   			ReadAttribFlt			(XML_NODE* node,	const char* attrib, float default_flt_val = 0.0f);

	XML_NODE*			SearchForAttribute		(const char* path, int index, const char* tag_name, const char* attrib, const char* attrib_value_pattern);
	XML_NODE*			SearchForAttribute		(XML_NODE* start_node, const char* tag_name, const char* attrib, const char* attrib_value_pattern);

	//возвращает количество узлов с заданым именем
	int					GetNodesNum				(const char* path, int index, const char* tag_name);
	int					GetNodesNum				(XML_NODE* node, const char*  tag_name);


	//проверка того, что аттрибуты у тегов уникальны (если не NULL, то уникальность нарушена и возвращается имя повторяющегося атрибута)
	const char*			CheckUniqueAttrib		(XML_NODE* start_node, const char* tag_name, const char* attrib_name);

	//переместиться по XML дереву 
	//путь задается в форме PARENT:CHILD:CHIDLS_CHILD
	//node_index - номер, если узлов с одним именем несколько
	XML_NODE*			NavigateToNode			(const char*  path, int node_index = 0);
	XML_NODE*			NavigateToNode			(XML_NODE* start_node, const char*  path, int node_index = 0);
	XML_NODE*			NavigateToNodeWithAttribute(const char* tag_name, const char* attrib_name, const char* attrib_value);

	void				SetLocalRoot			(XML_NODE* pLocalRoot)		 	{m_pLocalRoot = pLocalRoot;}
	XML_NODE*			GetLocalRoot			()							 	{return m_pLocalRoot;}

	XML_NODE*			GetRoot					() {return m_root;}

protected:
	XML_NODE*			m_root;
	XML_NODE*			m_pLocalRoot;

	//буфферный вектор для проверки уникальность аттрибутов
	xr_vector<shared_str> m_AttribValues;
public:
	virtual shared_str correct_file_name		(const char* path, const char* fn) {return fn;}
private:
						CXml					(const CXml& copy);
	void				operator=				(const CXml& copy);

	using XML_ELEM 								= tinyxml2::XMLElement;
	tinyxml2::XMLDocument								m_Doc;
};
