#include "stdafx.h"
#include "snork.h"


#include "luabind/luabind.hpp"
using namespace luabind;

#pragma optimize("s",on)
void CSnork::script_register(lua_State *L)
{
	module(L)
	[
		class_<CSnork,CGameObject>("CSnork")
			.def(constructor<>())
	];
}
