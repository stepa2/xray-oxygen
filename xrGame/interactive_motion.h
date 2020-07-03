#pragma once
#include "../Include/xrRender/KinematicsAnimated.h"
class IPhysicsShellEx;
class interactive_motion
{
	MotionID motion;

protected:

	Flags8			flags;
	IPhysicsShellEx	*shell;
	float			angle;
	enum Flag
	{
		fl_use_death_motion			=1<<4,
		fl_switch_dm_toragdoll		=1<<5,
		
		fl_started					=1<<6, 
		fl_not_played				=1<<7
	};
public:
			interactive_motion	( );
	virtual	~interactive_motion	( );
	void	init				( );
	void	destroy				( );
	void	setup				( LPCSTR m, IPhysicsShellEx *s, float angle );
	void	setup				( const MotionID &m, IPhysicsShellEx *s, float angle );
	
	void	update				( );
inline bool	is_enabled			( )	{ return !!flags.test( fl_use_death_motion ); }

	void	play( );

private:
virtual	void	move_update	( )=0;
virtual	void	collide		( )=0;

protected:
virtual	void	state_end	( );
virtual	void	state_start ( );

private:
//biped hack
		void	shell_setup				(  );

		//BoneCallbackFun bone_instance.set_callback(bctPhysics,
//
protected:
		void	switch_to_free	( );
protected:
static	void	anim_callback		( CBlend *B );


};

IC void destroy( interactive_motion* & im )
{
	if(!im)
		return;
	im->destroy();
	xr_delete( im );
}

void interactive_motion_diagnostic( LPCSTR message, const MotionID &m, IPhysicsShellEx *s );
#ifdef	DEBUG
extern XRPHYSICS_API BOOL		death_anim_debug;
#endif