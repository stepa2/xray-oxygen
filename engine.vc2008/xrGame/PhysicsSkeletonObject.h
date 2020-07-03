#ifndef PHYSICS_SKELETON_OBJECT_H
#define PHYSICS_SKELETON_OBJECT_H
#include "physicsshellholder.h"
#include "PHSkeleton.h"


class CSE_ALifePHSkeletonObject;
class CPhysicsSkeletonObject : 
	public CPhysicsShellHolder,
	public CPHSkeleton

{
using inherited = CPhysicsShellHolder;

public:
	CPhysicsSkeletonObject();
	virtual ~CPhysicsSkeletonObject();


	virtual BOOL					net_Spawn			( CSE_Abstract* DC)																	;
	virtual void					net_Destroy			()																				;
	virtual void					Load				(LPCSTR section)																;
	virtual void					UpdateCL			( )																				;// Called each frame, so no need for dt
	virtual void					shedule_Update		(u32 dt)																		;	//
	virtual void					net_Save			(NET_Packet& P)																	;
	virtual	BOOL					net_SaveRelevant	()																				;
	virtual BOOL					UsedAI_Locations	()																				;
protected:
	virtual CPhysicsShellHolder		*PPhysicsShellHolder()													{return PhysicsShellHolder();}
	virtual CPHSkeleton				*PHSkeleton			()																	{return this;}
	virtual void					SpawnInitPhysics	(CSE_Abstract	*D)																;
	virtual void					PHObjectPositionUpdate()																			;
	virtual	void					CreatePhysicsShell	(CSE_Abstract	*e)																;
};



#endif