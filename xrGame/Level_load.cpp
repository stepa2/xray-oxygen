#include "stdafx.h"
#include "LevelGameDef.h"
#include "ai_space.h"
#include "../xrParticles/psystem.h"
#include "../xrParticles/ParticlesObject.h"
#include "script_process.h"
#include "script_engine.h"
#include "script_engine_space.h"
#include "level.h"

#include "../xrEngine/x_ray.h"
#include "../xrEngine/gamemtllib.h"
#include "../xrphysics/PhysicsCommon.h"
#include "level_sounds.h"
#include "GamePersistent.h"

BOOL CLevel::Load_GameSpecific_Before()
{
	// AI space
	g_pGamePersistent->SetLoadStageTitle	("st_loading_ai_objects");
	g_pGamePersistent->LoadTitle();
	string_path fn_game;

	if (!ai().get_alife() && ai().is_game_graph_presented() && FS.exist(fn_game, "$level$", "level.game"))
	{
		IReader *stream = FS.r_open(fn_game);
		ai().patrol_path_storage_raw(*stream);
		FS.r_close(stream);
	}

	return(TRUE);
}

#include "../xrEngine/Rain.h"
BOOL CLevel::Load_GameSpecific_After()
{
	R_ASSERT(m_StaticParticles.empty());
	// loading static particles
	string_path		fn_game;
	if (FS.exist(fn_game, "$level$", "level.ps_static"))
	{
		IReader *F = FS.r_open(fn_game);
		u32				chunk = 0;
		string256		ref_name;
		Fmatrix			transform;
		Fvector			zero_vel = { 0.f,0.f,0.f };
		u32 ver = 0;
		for (IReader *OBJ = F->open_chunk_iterator(chunk); OBJ; OBJ = F->open_chunk_iterator(chunk, OBJ))
		{
			if (chunk == 0)
			{
				if (OBJ->length() == sizeof(u32))
				{
					ver = OBJ->r_u32();
					continue;
				}
			}
			u16 gametype_usage = 0;
			if (ver > 0)
			{
				gametype_usage = OBJ->r_u16();
			}
			OBJ->r_stringZ(ref_name, sizeof(ref_name));
			OBJ->r(&transform, sizeof(Fmatrix)); transform.c.y += 0.01f;


			if ((g_pGamePersistent->m_game_params.m_e_game_type & EGameIDs(gametype_usage)) || (ver == 0))
			{
				CParticlesObject* pStaticParticles = CParticlesObject::Create(ref_name, FALSE, false);
				pStaticParticles->UpdateParent(transform, zero_vel);
				pStaticParticles->Play(false);
				m_StaticParticles.push_back(pStaticParticles);
			}
		}
		FS.r_close(F);
	}

	// loading static sounds
	VERIFY(m_level_sound_manager);
	m_level_sound_manager->Load();

	// loading sound environment
	if (FS.exist(fn_game, "$level$", "level.snd_env")) 
	{
		IReader *F = FS.r_open(fn_game);
		::Sound->set_geometry_env(F);
		FS.r_close(F);
	}
	// loading SOM
	if (FS.exist(fn_game, "$level$", "level.som")) 
	{
		IReader *F = FS.r_open(fn_game);
		::Sound->set_geometry_som(F);
		FS.r_close(F);
	}

	// loading random (around player) sounds
	if (pSettings->section_exist("sounds_random")) 
	{
		CInifile::Sect& S = pSettings->r_section("sounds_random");
		Sounds_Random.reserve(S.Data.size());
		for (CInifile::Item I : S.Data)
		{
			Sounds_Random.emplace_back();
			Sound->create(Sounds_Random.back(), I.first.c_str(), st_Effect, sg_SourceType);
		}
		Sounds_Random_dwNextTime = Device.TimerAsync() + 50000;
		Sounds_Random_Enabled = FALSE;
	}

	CEffectRain* pRain = Environment().eff_Rain;
	if (pRain)
		pRain->InvalidateState();

	// loading scripts
	ai().script_engine().remove_script_process(ScriptEngine::eScriptProcessorLevel);

	if (pLevel->section_exist("level_scripts") && pLevel->line_exist("level_scripts", "script"))
		ai().script_engine().add_script_process(ScriptEngine::eScriptProcessorLevel, xr_new<CScriptProcess>("level", pLevel->r_string("level_scripts", "script")));
	else
		ai().script_engine().add_script_process(ScriptEngine::eScriptProcessorLevel, xr_new<CScriptProcess>("level", ""));

	if (game)
		Environment().SetGameTime(GetEnvironmentGameDayTimeSec(), game->GetEnvironmentGameTimeFactor());

	return TRUE;
}

struct translation_pair 
{
	u32			m_id;
	u16			m_index;

	IC			translation_pair(u32 id, u16 index)
	{
		m_id = id;
		m_index = index;
	}

	IC	bool	operator==	(const u16 &id) const
	{
		return	(m_id == id);
	}

	IC	bool	operator<	(const translation_pair &pair) const
	{
		return	(m_id < pair.m_id);
	}

	IC	bool	operator<	(const u16 &id) const
	{
		return	(m_id < id);
	}
};

void CLevel::Load_GameSpecific_CFORM	( CDB::TRI* tris, u32 count )
{
	using ID_INDEX_PAIRS = xr_vector<translation_pair>;
	ID_INDEX_PAIRS						translator;
	translator.reserve					(GMLib.CountMaterial());
	u16									default_id = (u16)GMLib.GetMaterialIdx("default");
	translator.emplace_back				(u32(-1),default_id);

	u16									index = 0, static_mtl_count = 1;
	int max_ID							= 0;
	int max_static_ID					= 0;
	for (GameMtlIt I=GMLib.FirstMaterial(); GMLib.LastMaterial()!=I; ++I, ++index) {
		if (!(*I)->Flags.test(SGameMtl::flDynamic)) {
			++static_mtl_count;
			translator.emplace_back		((*I)->GetID(),index);
			if ((*I)->GetID()>max_static_ID)	max_static_ID	= (*I)->GetID(); 
		}
		if ((*I)->GetID()>max_ID)				max_ID			= (*I)->GetID(); 
	}
	// Msg("* Material remapping ID: [Max:%d, StaticMax:%d]",max_ID,max_static_ID);
	VERIFY(max_static_ID<0xFFFF);
	
	if (static_mtl_count < 128) {
		CDB::TRI						*I = tris;
		CDB::TRI						*E = tris + count;
		for ( ; I != E; ++I) {
			ID_INDEX_PAIRS::iterator	i = std::find(translator.begin(),translator.end(),(u16)(*I).material);
			if (i != translator.end()) {
				(*I).material			= (*i).m_index;
				SGameMtl* mtl			= GMLib.GetMaterialByIdx	((*i).m_index);
				(*I).suppress_shadows	= mtl->Flags.is(SGameMtl::flSuppressShadows);
				(*I).suppress_wm		= mtl->Flags.is(SGameMtl::flSuppressWallmarks);
				continue;
			}

			Debug.fatal					(DEBUG_INFO,"Game material '%d' not found",(*I).material);
		}
		return;
	}

	std::sort							(translator.begin(),translator.end());
	{
		CDB::TRI						*I = tris;
		CDB::TRI						*E = tris + count;
		for ( ; I != E; ++I) {
			ID_INDEX_PAIRS::iterator	i = std::lower_bound(translator.begin(),translator.end(),(u16)(*I).material);
			if ((i != translator.end()) && ((*i).m_id == (*I).material)) {
				(*I).material			= (*i).m_index;
				SGameMtl* mtl			= GMLib.GetMaterialByIdx	((*i).m_index);
				(*I).suppress_shadows	= mtl->Flags.is(SGameMtl::flSuppressShadows);
				(*I).suppress_wm		= mtl->Flags.is(SGameMtl::flSuppressWallmarks);
				continue;
			}
			Debug.fatal					(DEBUG_INFO,"Game material '%d' not found",(*I).material);
		}
	}
}
