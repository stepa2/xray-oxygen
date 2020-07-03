﻿// Stats.h: interface for the CStats class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

class ENGINE_API CGameFont;

#include "../Include/xrRender/FactoryPtr.h"
#include "../Include/xrRender/StatsRender.h"

DECLARE_MESSAGE(Stats);

class ENGINE_API CStatsPhysics
{
public:
	CStatTimer	ph_collision;		// collision
	CStatTimer	ph_core;			// integrate
	CStatTimer	Physics;			// movement+collision
};

class ENGINE_API CStats: 
	public pureRender,
	public CStatsPhysics
{
public:
	CGameFont*	pFont;

    float       fRawFrameDeltaTime; // Basically is a Device.fTimeDelta, but not freezed in main menu, and more precise
	float		fFPS,fRFPS,fTPS, fLastDisplayedFPS;			// FPS, RenderFPS, TPS, FPS that changed every 25 frame
	float		fMem_calls			;
	u32			dwMem_calls			;
	u32			dwSND_Played,dwSND_Allocated;	// Play/Alloc

	CStatTimer	EngineTOTAL;			// 

	// Giperion Engine Stat timers
	CStatTimer  Engine_RenderFrame;		 // void CRender::OnFrame()
	CStatTimer  Engine_ApplicationFrame; // void CApplication::OnFrame()
	CStatTimer  Engine_InputFrame;		 // void CInput::OnFrame()
	CStatTimer  Engine_MenuFrame;		 // void CMainMenu::OnFrame()
	CStatTimer  Engine_PersistanceFrame; // void CGamePersistent::OnFrame()
	CStatTimer  Engine_PersistanceFrame_Begin; // void CGamePersistent::OnFrame() - Beginning
	CStatTimer  Engine_PersistanceFrame_EnvAndSpectre; // void CGamePersistent::OnFrame() - Environment & Spectre
	CStatTimer  Engine_PersistanceFrame_ParticlePlay; // void CGamePersistent::OnFrame() - Playing particles
	CStatTimer  Engine_PersistanceFrame_ParticleDestroy; // void CGamePersistent::OnFrame() - Destroying inactive particles
	CStatTimer  Engine_PersistanceFrame_Scheduler; // void CGamePersistent::OnFrame() - Engine.Sheduler.Update();
	CStatTimer  Engine_PersistanceFrame_WeatherAndDOF; // void CGamePersistent::OnFrame() - WeathersUpdate() and UpdateDof()

	u32			UpdateClient_updated;	//
	u32			UpdateClient_crows;		//
	u32			Particles_starting;	// starting
	u32			Particles_active;	// active
	u32			Particles_destroy;	// destroying
	CStatTimer	AI_Think;			// thinking
	CStatTimer	AI_Range;			// query: range
	CStatTimer	AI_Path;			// query: path
	CStatTimer	AI_Node;			// query: node
	CStatTimer	AI_Vis;				// visibility detection - total
	CStatTimer	AI_Vis_Query;		// visibility detection - portal traversal and frustum culling
	CStatTimer	AI_Vis_RayTests;	// visibility detection - ray casting

	CStatTimer	RenderTOTAL;		// 
	CStatTimer	RenderTOTAL_Real;	
	CStatTimer	Render_CRenderRender_render_main;			// portal traversal, frustum culling, entities "renderable_Render"
	CStatTimer	RenderCALC_HOM;		// HOM rendering
	CStatTimer	Animation;			// skeleton calculation
	CStatTimer	RenderDUMP;			// actual primitive rendering
	CStatTimer	RenderDUMP_Wait;	// ...waiting something back (queries results, etc.)
	CStatTimer	Render_CRenderRender_WaitForFrame;	// ...frame-limit sync
	CStatTimer	RenderDUMP_RT;		// ...render-targets
	CStatTimer	RenderDUMP_SKIN;	// ...skinning
	CStatTimer	RenderDUMP_HUD;		// ...hud rendering
	CStatTimer	RenderDUMP_Glows;	// ...glows vis-testing,sorting,render
	CStatTimer	RenderDUMP_Lights;	// ...d-lights building/rendering
	CStatTimer	RenderDUMP_WM;		// ...wallmark sorting, rendering
	u32			RenderDUMP_WMS_Count;// ...number of static wallmark
	u32			RenderDUMP_WMD_Count;// ...number of dynamic wallmark
	u32			RenderDUMP_WMT_Count;// ...number of wallmark tri
	CStatTimer	RenderDUMP_DT_VIS;	// ...details visibility detection
	CStatTimer	RenderDUMP_DT_Render;// ...details rendering
	CStatTimer	RenderDUMP_DT_Cache;// ...details slot cache access
	u32			RenderDUMP_DT_Count;// ...number of DT-elements
	CStatTimer	RenderDUMP_Pcalc;	// ...projectors	building
	CStatTimer	RenderDUMP_Scalc;	// ...shadows		building
	CStatTimer	RenderDUMP_Srender;	// ...shadows		render

	// Giperion Render specific timers
	CStatTimer  Render_CRenderRender;				// void CRender::Render()
	CStatTimer  Render_CRenderRender_MainMenu;		// void CRender::render_menu()
	CStatTimer  Render_CRenderRender_ScenePrepare;	// scene preparing: create view matrix, clear render targets and depth stencil
	CStatTimer  Render_CRenderRender_DeferPart0;	// main pass - albedo, normal
	CStatTimer  Render_CRenderRender_LightVisibility; // lights visibility and GI (?)
	CStatTimer  Render_CRenderRender_ItemUIWallmarks;// Actor item (weapon, detector, bolt etc.) and wallmarks
	CStatTimer  Render_CRenderRender_FlushOcclusion;// strange thing. The special lights, that need a different occlusion testing (we already has one occlusion testing in LightVisibility)
	CStatTimer  Render_CRenderRender_MSAA_Rain;		// MSAA and rain effect
	CStatTimer  Render_CRenderRender_Sun;			// The Fucking Sun
	CStatTimer  Render_CRenderRender_LightRender;	// Lights and self-illuminate objects
	CStatTimer  Render_CRenderRender_Combine;		// Post process, forward rendering (wtf is this, btw?), and tons of shit
	CStatTimer  Render_Combine_Begin;				// SSAO
	CStatTimer  Render_Combine_Sky;					// SSAO
	CStatTimer  Render_Combine_Cloud;				// SSAO
	CStatTimer  Render_Combine_Combine1;			// SSAO
	CStatTimer  Render_Combine_ForwardRendering;	// SSAO
	CStatTimer  Render_End;		// presenter

	CStatTimer	Sound;				// total time taken by sound subsystem (accurate only in single-threaded mode)
	CStatTimer	Input;				// total time taken by input subsystem (accurate only in single-threaded mode)
	CStatTimer	clRAY;				// total: ray-testing
	CStatTimer	clBOX;				// total: box query
	CStatTimer	clFRUSTUM;			// total: frustum query
	
	CStatTimer	TEST0;				// debug counter
	CStatTimer	TEST1;				// debug counter
	CStatTimer	TEST2;				// debug counter
	CStatTimer	TEST3;				// debug counter

	u32 AMDGPULoad;
	u32 NVGPULoad;
	float AvailableMem;
	float AvailablePageFileMem;
	float PageFileMemUsedByApp;
	float PhysMemoryUsedPercent;

	double cpuBefore;
	double cpuLoad = cpuBefore;
#ifdef DEBUG
	LPVOID MinAppAddress;
    LPVOID MaxAppAddress;
#endif
public:

	void			Show			(void);
	virtual void 	OnRender		();
	void			OnDeviceCreate	(void);
	void			OnDeviceDestroy	(void);
public:
	xr_vector		<shared_str>	errors;
	xrCriticalSection errorsGuard;
	CRegistrator	<pureStats>		seqStats;
public:
					CStats			();
					~CStats			();

	IC CGameFont*	Font			(){return pFont;}

private:
	FactoryPtr<IStatsRender>	m_pRender;
};

enum{
	st_sound			= (1<<0),
	st_sound_min_dist	= (1<<1),
	st_sound_max_dist	= (1<<2),
	st_sound_ai_dist	= (1<<3),
	st_sound_info_name	= (1<<4),
	st_sound_info_object= (1<<5),
};

extern Flags32 g_stats_flags;
