////////////////////////////////////////////////////////////////////////////
//	Module 		: cover_point_script.cpp
//	Created 	: 24.03.2004
//  Modified 	: 24.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Cover point class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cover_point.h"

#include "luabind/luabind.hpp"
using namespace luabind;

bool CCoverPoint__is_smart_cover	(CCoverPoint const* cover)
{
	return		(cover->m_is_smart_cover);
}

#pragma optimize("gyts",on)
void CCoverPoint::script_register(lua_State *L)
{
	module(L)
	[
		class_<CCoverPoint>("cover_point")
			.def("position",			&CCoverPoint::position)
			.def("level_vertex_id",		&CCoverPoint::level_vertex_id)
			.def("is_smart_cover",		&CCoverPoint__is_smart_cover)
	];
}
