#include "stdafx.h"
#include "Explosive.h"
#include "xrGame\ExplosiveItem.h"

XRay::Explosive::Explosive(GameObject^ pObject)
{
	pGameObject = smart_cast<CExplosiveItem*>((CGameObject*)(pObject->GetNativeObject().ToPointer()));
}

void XRay::Explosive::Explode()
{
	if (pGameObject)
		pGameObject->Explode();
}
