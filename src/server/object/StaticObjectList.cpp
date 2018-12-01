/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "debug/Debug.hpp"
#include "core/log.h"
#include "server/object/StaticObjectList.hpp"
#include "util/serialize.h"

void StaticObjectList::insert(u16 id, const StaticObject &obj)
{
	if(id == 0)
	{
		m_stored.push_back(obj);
	}
	else
	{
		if(m_active.find(id) != m_active.end())
		{
			dstream<<"ERROR: StaticObjectList::insert(): "
				<<"id already exists"<<std::endl;
			FATAL_ERROR("StaticObjectList::insert()");
		}
		m_active[id] = obj;
	}
}

void StaticObjectList::remove(u16 id)
{
	assert(id != 0); // Pre-condition
	if(m_active.find(id) == m_active.end())
	{
		warningstream<<"StaticObjectList::remove(): id="<<id
			<<" not found"<<std::endl;
		return;
	}
	m_active.erase(id);
}

void StaticObjectList::serialize(std::ostream &os)
{
	// version
	u8 version = 0;
	writeU8(os, version);

	// count
	size_t count = m_stored.size() + m_active.size();
	// Make sure it fits into u16, else it would get truncated and cause e.g.
	// issue #2610 (Invalid block data in database: unsupported NameIdMapping version).
	if (count > U16_MAX) {
		errorstream << "StaticObjectList::serialize(): "
			<< "too many objects (" << count << ") in list, "
			<< "not writing them to disk." << std::endl;
		writeU16(os, 0);  // count = 0
		return;
	}
	writeU16(os, count);

	for (StaticObject &s_obj : m_stored) {
		s_obj.serialize(os);
	}

	for (auto &i : m_active) {
		StaticObject s_obj = i.second;
		s_obj.serialize(os);
	}
}
void StaticObjectList::deSerialize(std::istream &is)
{
	// version
	u8 version = readU8(is);
	// count
	u16 count = readU16(is);
	for(u16 i = 0; i < count; i++) {
		StaticObject s_obj;
		s_obj.deSerialize(is, version);
		m_stored.push_back(s_obj);
	}
}

