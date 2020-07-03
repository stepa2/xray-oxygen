#pragma once
#include "physicsexternalcommon.h"
class IPHStaticGeomShell
{
protected:
	virtual ~IPHStaticGeomShell() = 0 {}
};

class IPhysicsShellHolder;
class IClimableObject;
XRPHYSICS_API	IPHStaticGeomShell	*P_BuildStaticGeomShell(IPhysicsShellHolder* obj, ObjectContactCallbackFun* object_contact_callback);
XRPHYSICS_API	IPHStaticGeomShell	*P_BuildLeaderGeomShell(IClimableObject* obj, ObjectContactCallbackFun* callback, const Fobb &b);
XRPHYSICS_API	void DestroyStaticGeomShell(IPHStaticGeomShell* &p);
