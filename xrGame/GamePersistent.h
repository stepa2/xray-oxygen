#pragma once

#include "../xrEngine/IGame_Persistent.h"
class CMainMenu;
class CUICursor;
class CParticlesObject;
class CUISequencer;
class ui_core;

/// Extra features, that previously was in FRayBuildConfig.h. Can be enabled by console commands
/// Should be here, to make this global
enum OxygenExtraFeatures : size_t
{
    GAME_EXTRA_RUCK                             = (1 << 0),
    GAME_EXTRA_MONSTER_INVENTORY                = (1 << 1),
    GAME_EXTRA_SPAWN_ANTIFREEZE                 = (1 << 2),
    GAME_EXTRA_WEAPON_AUTORELOAD                = (1 << 3),
    GAME_EXTRA_DYNAMIC_SUN                      = (1 << 4),
    GAME_EXTRA_HOLD_TO_PICKUP                   = (1 << 5),
    GAME_EXTRA_POLTER_SHOW_PARTICLES_ON_DEAD    = (1 << 6),
    GAME_EXTRA_SOC_WND                          = (1 << 7),
    GAME_EXTRA_VERTICAL_BELTS                   = (1 << 8),
	GAME_EXTRA_THIRST							= (1 << 9),
	GAME_EXTRA_NPC_GRENADE_ATTAK_ALL			= (1 << 10),
	GAME_EXTRA_OLD_SCHOOL_MINIMAP				= (1 << 11),
	GAME_EXTRA_LAMP_IMMUNITY_SUPPORT			= (1 << 12),
	GAME_EXTRA_ALWAYS_PICKUP					= (1 << 13),
};
extern Flags32 g_extraFeatures;

class GAME_API CGamePersistent: public IGame_Persistent, public IEventReceiver
{
	// ambient particles
	CParticlesObject*	ambient_particles; 
	u32					ambient_sound_next_time[20]; //max snd channels
	u32					ambient_effect_next_time;
	u32					ambient_effect_stop_time;

	float				ambient_effect_wind_start;
	float				ambient_effect_wind_in_time;
	float				ambient_effect_wind_end;
	float				ambient_effect_wind_out_time;
	bool				ambient_effect_wind_on;

	bool				m_bPickableDOF;
    bool                m_developerMode;

	CUISequencer*		m_intro;
	EVENT				eQuickLoad;
	Fvector				m_dof		[4];	// 0-dest 1-current 2-from 3-original

    // startup options, originaly was in CLevel
    shared_str					m_ServerOptions;
    shared_str					m_ClientOptions;

	xrDelegate<void()>			m_intro_event;

	void 		start_logo_intro		();
	void 		update_logo_intro		();

	void 		game_loaded				();
	void 		update_game_loaded		();

	void 		start_game_intro		();
	void 		update_game_intro		();

	u32					m_frame_counter;
	u32					m_last_stats_frame;

	void				WeathersUpdate			();
	void				UpdateDof				();

public:
	ui_core*			m_pUI_core;
	u32					uTime2Change;

						CGamePersistent			();
	virtual				~CGamePersistent		();
	void				PreStart				(LPCSTR op) override;
	virtual void		Start					(LPCSTR op);
	virtual void		Disconnect				();

	virtual	void		OnAppActivate			();
	virtual void		OnAppDeactivate			();

	virtual void		OnAppStart				();
	virtual void		OnAppEnd				();
	virtual	void		OnGameStart				();
	virtual void		OnGameEnd				();

	virtual void		OnFrame					();
	virtual void		OnEvent					(EVENT E, u64 P1, u64 P2);

	virtual void		UpdateGameType			();

	virtual void		RegisterModel			(IRenderVisual* V);
	virtual	float		MtlTransparent			(u32 mtl_idx);
	virtual	void		Statistics				(CGameFont* F);

	virtual	bool		OnRenderPPUI_query		();
	virtual void		OnRenderPPUI_main		();
	virtual void		OnRenderPPUI_PP			();

	virtual	void		LoadTitle				(bool change_tip = false, shared_str map_name = "");
	void				SetLoadStageTitle	(const char* ls_title = nullptr) override;

			void		SetPickableEffectorDOF	(bool bSet);
			void		SetEffectorDOF			(const Fvector& needed_dof);
			void		RestoreEffectorDOF		();

	virtual void		GetCurrentDof			(Fvector3& dof);
	virtual void		SetBaseDof				(const Fvector3& dof);
	virtual void		OnSectorChanged			(int sector);
	virtual void		OnAssetsChanged			();

    shared_str          GetServerOption() const;
    shared_str          GetClientOption() const;
    void                SetServerOption(const char* str);
    void                SetClientOption(const char* str);

    bool                IsDeveloperMode() const;
};

IC CGamePersistent&		GamePersistent()		{ return *((CGamePersistent*) g_pGamePersistent); }


