#include "stdafx.h"
#include "flesh.h"


#include "luabind/luabind.hpp"
using namespace luabind;

#pragma optimize("s",on)
void CAI_Flesh::script_register(lua_State *L)
{
	module(L)
	[
		class_<CAI_Flesh,CGameObject>("CAI_Flesh")
			.def(constructor<>())
	];
}
