// Singleton LuaVM 
// ForserX 27.05.2018 for xrOxygen
#include "stdafx.h"
#include <luabind/luabind.hpp>

#include "VMLua.h"
#include "luaopen.hpp"

static void* __cdecl luabind_allocator(luabind::memory_allocation_function_parameter const, void const * const pointer, size_t const size)
{
	if (!size)
	{
		void*	non_const_pointer = const_cast<LPVOID>(pointer);
		xr_free(non_const_pointer);
		return 0;
	}

	if (!pointer)
	{
		return	(Memory.mem_alloc(size));
	}

	void*		non_const_pointer = const_cast<void*>(pointer);
	return		(Memory.mem_realloc(non_const_pointer, size));
}

void setup_luabind_allocator()
{
    luabind::allocator = &luabind_allocator;
    luabind::allocator_parameter = 0;
}

void CVMLua::luabind_onerror(lua_State* lua)
{
    if (lua_isstring(lua, -1))
    {
        const char* luaError = lua_tostring(lua, -1);
        Msg("! LUA ERROR: %s", luaError);
        lua_pop(lua, 1);
    }

    //#HACK: Invoke crash handler manually for now
    if (crashhandler* CrashHanlderFunc = Debug.get_crashhandler())
    {
        CrashHanlderFunc();
    }
}

bool CVMLua::CreateNamespaceTable(LPCSTR caNamespaceName)
{
	lua_pushstring(m_virtual_machine, "_G");
	lua_gettable(m_virtual_machine, LUA_GLOBALSINDEX);
	LPSTR			S2 = xr_strdup(caNamespaceName);
	LPSTR			S = S2;
	for (;;) {
		if (!xr_strlen(S)) {
			lua_pop(m_virtual_machine, 1);
			Msg("the namespace name %s is incorrect!", caNamespaceName);
			xr_free(S2);
			return		(false);
		}
		LPSTR			S1 = strchr(S, '.');
		if (S1)
			*S1 = 0;
		lua_pushstring(m_virtual_machine, S);
		lua_gettable(m_virtual_machine, -2);
		if (lua_isnil(m_virtual_machine, -1)) {
			lua_pop(m_virtual_machine, 1);
			lua_newtable(m_virtual_machine);
			lua_pushstring(m_virtual_machine, S);
			lua_pushvalue(m_virtual_machine, -2);
			lua_settable(m_virtual_machine, -4);
		}
		else
			if (!lua_istable(m_virtual_machine, -1)) {
				xr_free(S2);
				lua_pop(m_virtual_machine, 2);
				Msg("the namespace name %s is already being used by the non-table object!", caNamespaceName);
				return			(false);
			}
		lua_remove(m_virtual_machine, -2);
		if (S1)
			S = ++S1;
		else
			break;
	}
	xr_free(S2);
	return			(true);
}

CVMLua::CVMLua()
{
	setup_luabind_allocator();
	m_virtual_machine = luaL_newstate();
	R_ASSERT2(m_virtual_machine, "Cannot initialize script virtual machine!");

	OpenLib();
}

CVMLua::~CVMLua()
{
	lua_close(m_virtual_machine);
}

void CVMLua::OpenLib()
{
	lopen::openlua(m_virtual_machine);
	// FX to ALL: Add anothres namespace into this function
	luabind::open(m_virtual_machine);
#ifdef LUABIND_NO_EXCEPTIONS
    luabind::set_error_callback(CVMLua::luabind_onerror);
#endif
}

void CVMLua::Add(AddFun pFun)
{
	pFun(m_virtual_machine);
}

bool CVMLua::IsObjectPresent(LPCSTR namespace_name, LPCSTR identifier, int type)
{
	if (xr_strlen(namespace_name) && !GetNamespaceTable(namespace_name))
		return (false);
	return (IsObjectPresent(identifier, type));
}

bool CVMLua::IsObjectPresent(const char* identifier, int type)
{
	lua_pushnil(m_virtual_machine);
	while (lua_next(m_virtual_machine, -2)) {
		if ((lua_type(m_virtual_machine, -1) == type) && !xr_strcmp(identifier, lua_tostring(m_virtual_machine, -2))) 
		{
			lua_pop(m_virtual_machine, 3);
			return (true);
		}
		lua_pop(m_virtual_machine, 1);
	}
	lua_pop(m_virtual_machine, 1);
	return	(false);
}

void CVMLua::ScriptLog(ELuaMessageType tLuaMessageType, const char* caFormat, ...)
{
	char c;

	switch (tLuaMessageType)
	{
	case ELuaMessageType::eLuaMessageTypeError:		c = '!'; break;
	case ELuaMessageType::eLuaMessageTypeMessage:	c = '~'; break;
	default: c = '\0';
	}

	va_list marker;
	string2048 buf;
	ZeroMemory(&buf, sizeof(buf));
	va_start(marker, caFormat);
	int sz = vsnprintf(buf, sizeof(buf) - 1, caFormat, marker);
	if (sz != -1)
	{
		if (c == '\0')
			Log(buf);
		else
			Msg("%c %s", c, buf);
	}
	va_end(marker);
}

void CVMLua::PrintError(lua_State * L, int iErrorCode)
{
	switch (iErrorCode)
	{
	case LUA_ERRRUN:
	{
		ScriptLog(ScriptStorage::eLuaMessageTypeError, "SCRIPT RUNTIME ERROR");
		break;
	}
	case LUA_ERRMEM:
	{
		ScriptLog(ScriptStorage::eLuaMessageTypeError, "SCRIPT ERROR (memory allocation)");
		break;
	}
	case LUA_ERRERR: 
	{
		ScriptLog(ScriptStorage::eLuaMessageTypeError, "SCRIPT ERROR (while running the error handler function)");
		break;
	}
	case LUA_ERRFILE: 
	{
		ScriptLog(ScriptStorage::eLuaMessageTypeError, "SCRIPT ERROR (while running file)");
		break;
	}
	case LUA_ERRSYNTAX: 
	{
		ScriptLog(ScriptStorage::eLuaMessageTypeError, "SCRIPT SYNTAX ERROR");
		break;
	}
	case LUA_YIELD:
	{
		ScriptLog(ScriptStorage::eLuaMessageTypeInfo, "Thread is yielded");
		break;
	}
	default: NODEFAULT;
	}
}

bool CVMLua::PrintOut(lua_State *L, const char* caScriptFileName, int iErrorCode, const char* caErrorText)
{
	if (iErrorCode)
		PrintError(L, iErrorCode);

	raii_guard guard(iErrorCode, caErrorText);

	if (!lua_isstring(L, -1))
		return				(false);

	caErrorText = lua_tostring(L, -1);
	if (!xr_strcmp(caErrorText, "cannot resume dead coroutine"))
	{
		VERIFY2("Please do not return any values from main!!!", caScriptFileName);
	}
	else {
		if (!iErrorCode)
			ScriptLog(ScriptStorage::eLuaMessageTypeInfo, "Output from %s", caScriptFileName);
		ScriptLog(iErrorCode ? ScriptStorage::eLuaMessageTypeError : ScriptStorage::eLuaMessageTypeMessage, "%s", caErrorText);
	}
	return (true);
}

bool CVMLua::GetNamespaceTable(LPCSTR N)
{
	lua_pushstring(m_virtual_machine, "_G");
	lua_gettable(m_virtual_machine, LUA_GLOBALSINDEX);
	string256 S2;	xr_strcpy(S2, N);
	LPSTR S = S2;
	for (;;)
	{
		if (!xr_strlen(S)) return	(false);
		LPSTR S1 = strchr(S, '.');
		if (S1) 	*S1 = 0;
		lua_pushstring(m_virtual_machine, S);
		lua_gettable(m_virtual_machine, -2);
		if (lua_isnil(m_virtual_machine, -1)) {
			lua_pop(m_virtual_machine, 2);
			return		(false);	//	there is no namespace!
		}
		else if (!lua_istable(m_virtual_machine, -1)) 
		{
			lua_pop(m_virtual_machine, 2);
			FATAL(" Error : the namespace name is already being used by the non-table object!\n");
			return (false);
		}
		lua_remove(m_virtual_machine, -2);
		if (S1) S = ++S1;
		else break;
	}
	return	(true);
}

bool CVMLua::LoadFileIntoNamespace(LPCSTR caScriptName, LPCSTR caNamespaceName, bool bCall)
{
	if (!CreateNamespaceTable(caNamespaceName))
		return (false);

	CopyGlobals();
	if (!DoFile(caScriptName, caNamespaceName, bCall))
		return (false);

	SetNamespace();
	return (true);
}

void CVMLua::CopyGlobals()
{
	lua_newtable(m_virtual_machine);
	lua_pushstring(m_virtual_machine, "_G");
	lua_gettable(m_virtual_machine, LUA_GLOBALSINDEX);
	lua_pushnil(m_virtual_machine);

	while (lua_next(m_virtual_machine, -2)) 
	{
		lua_pushvalue(m_virtual_machine, -2);
		lua_pushvalue(m_virtual_machine, -2);
		lua_settable(m_virtual_machine, -6);
		lua_pop(m_virtual_machine, 1);
	}
}

bool CVMLua::LoadBuffer(LPCSTR caBuffer, size_t tSize, LPCSTR caScriptName, LPCSTR caNameSpaceName)
{
	int l_iErrorCode;
	if (caNameSpaceName) 
	{
		string256 insert;
		xr_sprintf(insert, sizeof(insert), "local this = %s\n", caNameSpaceName);
		size_t str_len = xr_strlen(insert);
		LPSTR script = xr_alloc<char>(u32(str_len + tSize));
		xr_strcpy(script, str_len + tSize, insert);
		CopyMemory(script + str_len, caBuffer, u32(tSize));
		l_iErrorCode = luaL_loadbuffer(m_virtual_machine, script, tSize + str_len, caScriptName);
		xr_free(script);
	}
	else l_iErrorCode = luaL_loadbuffer(m_virtual_machine, caBuffer, tSize, caScriptName);

	return !l_iErrorCode;
}

bool CVMLua::DoFile(LPCSTR caScriptName, LPCSTR caNameSpaceName, bool bCall)
{
	string_path l_caLuaFileName;
	IReader *l_tpFileReader = FS.r_open(caScriptName);
	R_ASSERT(l_tpFileReader);
	xr_strconcat( l_caLuaFileName, "@", caScriptName);

	if (!LoadBuffer(static_cast<LPCSTR>(l_tpFileReader->pointer()), (size_t)l_tpFileReader->length(), l_caLuaFileName, caNameSpaceName)) 
	{
		lua_pop(m_virtual_machine, 4);
		FS.r_close(l_tpFileReader);
		return		(false);
	}
	FS.r_close(l_tpFileReader);

	if (bCall) lua_call(m_virtual_machine, 0, 0);
	else lua_insert(m_virtual_machine, -4);

	return (true);
}


void CVMLua::SetNamespace()
{
	lua_pushnil(m_virtual_machine);
	while (lua_next(m_virtual_machine, -2)) 
	{
		lua_pushvalue(m_virtual_machine, -2);
		lua_gettable(m_virtual_machine, -5);
		if (lua_isnil(m_virtual_machine, -1)) 
		{
			lua_pop(m_virtual_machine, 1);
			lua_pushvalue(m_virtual_machine, -2);
			lua_pushvalue(m_virtual_machine, -2);
			lua_pushvalue(m_virtual_machine, -2);
			lua_pushnil(m_virtual_machine);
			lua_settable(m_virtual_machine, -7);
			lua_settable(m_virtual_machine, -7);
		}
		else {
			lua_pop(m_virtual_machine, 1);
			lua_pushvalue(m_virtual_machine, -2);
			lua_gettable(m_virtual_machine, -4);
			if (!lua_equal(m_virtual_machine, -1, -2)) 
			{
				lua_pushvalue(m_virtual_machine, -3);
				lua_pushvalue(m_virtual_machine, -2);
				lua_pushvalue(m_virtual_machine, -2);
				lua_pushvalue(m_virtual_machine, -5);
				lua_settable(m_virtual_machine, -8);
				lua_settable(m_virtual_machine, -8);
			}
			lua_pop(m_virtual_machine, 1);
		}
		lua_pushvalue(m_virtual_machine, -2);
		lua_pushnil(m_virtual_machine);
		lua_settable(m_virtual_machine, -6);
		lua_pop(m_virtual_machine, 1);
	}
	lua_pop(m_virtual_machine, 3);
}
