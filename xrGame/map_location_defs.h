#pragma once
#include "alife_abstract_registry.h"

class CMapLocation;
struct GAME_API SLocationKey : public IPureSerializeObject<IReader,IWriter>,public IPureDestroyableObject {
	shared_str		spot_type;
	u16				object_id;
	CMapLocation*	location;
	bool			actual;
	SLocationKey (shared_str s, u16 id):spot_type(s),object_id(id),location(nullptr),actual(true){};
	SLocationKey ():spot_type(nullptr),object_id(0),location(nullptr),actual(true){};

	bool operator < (const SLocationKey& key)const
	{
		if(actual == key.actual)
			return location<key.location;
		else
			return  actual > key.actual;
	} //move non-actual to tail
	
	virtual void save								(IWriter &stream);
	virtual void load								(IReader &stream);
	virtual void destroy							();
};

using Locations = xr_vector<SLocationKey>;

struct CMapLocationRegistry : public CALifeAbstractRegistry<u16, Locations> {
	virtual void save(IWriter &stream);
};
