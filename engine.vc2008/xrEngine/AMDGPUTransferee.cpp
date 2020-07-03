//////////////////////////////////////////
// Desc:   GPU Info
// Model:  AMD
// Author: ForserX
//////////////////////////////////////////
// Oxygen Engine (2016-2019)
//////////////////////////////////////////

#include "stdafx.h"
#include "AMDGPUTransferee.h"

CAMDReader AMDData;
#define AMDVENDORID 1002

bool CAMDReader::bAMDSupportADL = false;
void* CAMDReader::lpBuffer = nullptr;
static HINSTANCE hDLL;
static HINSTANCE hDLL_AGS;

CAMDReader::CAMDReader() : activity({ 0 }), AdapterID(-1), AdapterAGSInfo(0)
{
	hDLL = LoadLibraryA("atiadlxx.dll");
	hDLL_AGS = LoadLibraryA("amd_ags_x64.dll");

	if (!bAMDSupportADL && hDLL)
	{
		bAMDSupportADL = true;

		Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL_Main_Control_Create");
		GetAdapter_NumberOfAdapters = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL_Adapter_NumberOfAdapters_Get");
		GetAdapter_AdapterInfo = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL_Adapter_AdapterInfo_Get");
		GetAdapter_Active = (ADL_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL_Adapter_Active_Get");
		GetOverdrive5_CurrentActivity = (ADL_OVERDRIVE5_CURRENTACTIVITY_GET)GetProcAddress(hDLL, "ADL_Overdrive5_CurrentActivity_Get");
		Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL_Main_Control_Destroy");
		GetTemperatureGPU = (ADL_OVERDRIVE5_TEMPERATURE_GET)GetProcAddress(hDLL, "ADL_Overdrive5_Temperature_Get");
		Main_Control_Create(MemoryAllocator, 1);
		InitDeviceInfo();

		activity.iSize = sizeof(ADLPMActivity);
	}

	if (bAMDSupportADL && hDLL_AGS)
	{
		GetAGSCrossfireGPUCount = (AGS_GPU_COUNT_GET)GetProcAddress(hDLL_AGS, "agsGetCrossfireGPUCount");
		AGSCrossfireDeinit = (AGS_DEINIT)GetProcAddress(hDLL_AGS, "agsDeInit");
		AGSCrossfireInit = (AGS_INIT)GetProcAddress(hDLL_AGS, "agsInit");

		// 5.2 and later
		if (!GetAGSCrossfireGPUCount)
		{
			GetAGSCrossfireGPUCountExt = (AGS_DX11EXT)GetProcAddress(hDLL_AGS, "agsDriverExtensionsDX11_CreateDevice");
			AGSCrossfireGPUExtDestroy = (AGS_DX11EXTDestroy)GetProcAddress(hDLL_AGS, "agsDriverExtensionsDX11_DestroyDevice");
		}

		MakeGPUCount();
	}
}

CAMDReader::~CAMDReader()
{
	if (bAMDSupportADL)
	{
		Main_Control_Destroy();
		MemoryDeallocator();
		FreeLibrary(hDLL);
		FreeLibrary(hDLL_AGS);
	}
}

void CAMDReader::InitDeviceInfo()
{
	LPAdapterInfo lpAdapterInfo = nullptr;
	GetAdapter_NumberOfAdapters(&AdapterADLInfo);

	lpAdapterInfo = new AdapterInfo[AdapterADLInfo]();
	RtlZeroMemory(lpAdapterInfo, sizeof(AdapterInfo) * AdapterADLInfo);

	// Get the AdapterInfo structure for all adapters in the system
	GetAdapter_AdapterInfo(lpAdapterInfo, sizeof(AdapterInfo) * AdapterADLInfo);

	for (u32 i = 0; i < (u32)AdapterADLInfo; i++)
	{
		int adapterActive = 0;
		AdapterInfo adapterInfo = lpAdapterInfo[i];
		GetAdapter_Active(adapterInfo.iAdapterIndex, &adapterActive);
		if (adapterActive && adapterInfo.iVendorID == AMDVENDORID)
		{
			AdapterID = adapterInfo.iAdapterIndex;
			break;
		}
	}
}

void CAMDReader::MakeGPUCount()
{
	AGSContext* ags = nullptr;
	AGSGPUInfo gpuInfo = {};
	AGSConfiguration* config = nullptr;
	AGSReturnCode status = AGSCrossfireInit(&ags, config, &gpuInfo);

	if (status != AGS_SUCCESS)
	{
		Msg("! AGS: Initialization failed (%d)", status);
		return;
	}

	if (GetAGSCrossfireGPUCount)
	{
		// FX: Old style for Win7 and lazy users
		// But, it's just a beautiful 
		status = GetAGSCrossfireGPUCount(ags, &AdapterAGSInfo);
		AdapterAGSInfo = AdapterAGSInfo ? AdapterAGSInfo : AdapterADLInfo / 2;
		Msg("[AGS] Used old ags driver...");
	}
	else
	{
		AGSDX11DeviceCreationParams creationParams;
		RtlZeroMemory(&creationParams, sizeof(AGSDX11DeviceCreationParams));
		creationParams.SDKVersion = 7; // Skip debug output errors. crossfireGPUCount need only
		creationParams.FeatureLevels = 45312; // 11.1

		AGSDX11ExtensionParams extensionParams = {};
		// FX: Enable AFR without requiring a driver profile
		extensionParams.crossfireMode = AGS_CROSSFIRE_MODE_EXPLICIT_AFR;
		extensionParams.uavSlot = 7;
		AGSDX11ReturnedParams returnedParams = {};

		GetAGSCrossfireGPUCountExt(ags, &creationParams, &extensionParams, &returnedParams);
		AdapterAGSInfo = returnedParams.crossfireGPUCount ? returnedParams.crossfireGPUCount : AdapterADLInfo / 2;

		AGSCrossfireGPUExtDestroy(ags, nullptr, nullptr, nullptr, nullptr);
	}

	if (status != AGS_SUCCESS) Msg("[AGS] Error! Unable to get CrossFire GPU count (%d)", status);
	else					   Msg("[AGS] CrossFire GPU count: %d", AdapterAGSInfo);

	AGSCrossfireDeinit(ags);
}

u32 CAMDReader::GetPercentActive()
{
	GetOverdrive5_CurrentActivity(AdapterID, &activity);
	return activity.iActivityPercent;
}

u32 CAMDReader::GetTemperature()
{
	ADLTemperature adlTemperature = { 0 };
	adlTemperature.iSize = sizeof(ADLTemperature);
	GetTemperatureGPU(AdapterID, 0, &adlTemperature);
	return u32(adlTemperature.iTemperature);
}

u32 CAMDReader::GetGPUCount()
{
	u32 uCount = hDLL_AGS ? u32(AdapterAGSInfo) : u32(AdapterADLInfo / 2);
	return uCount ? uCount : 1;
}
