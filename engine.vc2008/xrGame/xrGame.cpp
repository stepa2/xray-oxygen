////////////////////////////////////////////////////////////////////////////
//	Module 		: xrGame.cpp
//	Created 	: 07.01.2001
//  Modified 	: 27.05.2004
//	Author		: Aleksandr Maksimchuk and Oles' Shyshkovtsov
//	Description : Defines the entry point for the DLL application.
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "object_factory.h"
#include "../xrUICore/xrUIXmlParser.h"
#include "..\xrEngine\xr_level_controller.h"
#include "../xrEngine/profiler.h"
#include "../xrEngine/spectre/Spectre.h"
#pragma comment (lib, "xrCore.lib")
#pragma comment (lib,"xrEngine.lib")

extern "C" {
	DLL_API DLL_Pure*	__cdecl xrFactory_Create		(CLASS_ID clsid)
	{
		DLL_Pure			*object = object_factory().client_object(clsid);
#ifdef DEBUG
		if (!object)
			return			(nullptr);
#endif
		object->CLS_ID		= clsid;

		//Invoke Spectre proxy constructor
		object->SpectreObjectId = SpectreEngineClient::CreateProxyObject(object);
		return				(object);
	}

	DLL_API void		__cdecl	xrFactory_Destroy		(DLL_Pure* O)
	{
		xr_delete			(O);
	}
};

void CCC_RegisterCommands	();
void setup_luabind_allocator();
void RegisterExpressionDelegates();
void LoadGameExtraFeatures();

BOOL APIENTRY DllMain(HANDLE hModule, u32 ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH: {
			// register console commands
			CCC_RegisterCommands();

            // Load cfg with extra game features. Should be early as possible - some features requires early initialization
            LoadGameExtraFeatures();

			// keyboard binding
			CCC_RegisterInput	();

            // register expression delegates
            RegisterExpressionDelegates();
			g_profiler			= xr_new<CProfiler>();

			break;
		}

		case DLL_PROCESS_DETACH: {
			break;
		}
	}
    return								(TRUE);
}
