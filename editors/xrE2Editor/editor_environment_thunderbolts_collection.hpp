////////////////////////////////////////////////////////////////////////////
//	Module 		: editor_environment_thunderbolts_collection.hpp
//	Created 	: 10.01.2008
//  Modified 	: 10.01.2008
//	Author		: Dmitriy Iassenev
//	Description : editor environment thunderbolts collection identifier class
////////////////////////////////////////////////////////////////////////////
#pragma once

#ifdef INGAME_EDITOR
#include "../include/editor/property_holder.hpp"
#include "property_collection_forward.hpp"
#include "ThunderboltCollection.h"

namespace editor {

class property_holder_collection;

namespace environment {
namespace thunderbolts {

class manager;
class thunderbolt_id;

class collection :
	public CThunderboltCollection,
	public editor::property_holder_holder {
public:
							collection		(manager const& manager, shared_str const& id);
                            collection(const collection&) = delete;
                            collection& operator= (const collection&) = delete;
	virtual					~collection		();
			void			load			(CInifile& config);
			void			save			(CInifile& config);
			void			fill			(editor::property_holder_collection* collection);
	inline	LPCSTR			id				() const { return section.c_str(); }


private:
			LPCSTR		id_getter	() const;
			void		id_setter	(LPCSTR value);
private:
	typedef editor::property_holder			property_holder_type;

public:
	virtual	property_holder_type* object	();

private:
	typedef xr_vector<thunderbolt_id*>						container_type;
	typedef property_collection<container_type, collection>	collection_type;

private:
	container_type			m_ids;
	collection_type*		m_collection;
	property_holder_type*	m_property_holder;

public:
	manager const&			m_manager;
}; // class collection
} // namespace thunderbolts
} // namespace environment
} // namespace editor

#endif // #ifdef INGAME_EDITOR