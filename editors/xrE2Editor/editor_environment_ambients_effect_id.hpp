////////////////////////////////////////////////////////////////////////////
//	Module 		: editor_environment_ambients_effect_id.hpp
//	Created 	: 04.01.2008
//  Modified 	: 04.01.2008
//	Author		: Dmitriy Iassenev
//	Description : editor environment ambients effect identifier class
////////////////////////////////////////////////////////////////////////////
#pragma once

#ifdef INGAME_EDITOR
#include "../include/editor/property_holder.hpp"

namespace editor {

class property_holder_collection;

namespace environment {
	namespace effects {
		class manager;
	} // namespace effects

namespace ambients {

class effect_id :
	public editor::property_holder_holder {
public:
							effect_id		(effects::manager const& manager, shared_str const& id);
                            effect_id(const effect_id&) = delete;
                            effect_id& operator= (const effect_id&) = delete;
	virtual					~effect_id		();
			void			fill			(editor::property_holder_collection* collection);
	inline	shared_str const& id			() const { return m_id; }

private:
	typedef editor::property_holder			property_holder_type;

public:
	virtual	property_holder_type* object	();

private:
	LPCSTR const*  collection		();
	u32  			collection_size	();

private:
	property_holder_type*	m_property_holder;
	effects::manager const&	m_manager;
	shared_str				m_id;
}; // class effect_id
} // namespace ambients
} // namespace environment
} // namespace editor

#endif // #ifdef INGAME_EDITOR