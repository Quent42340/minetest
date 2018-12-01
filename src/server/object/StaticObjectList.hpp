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
#ifndef STATICOBJECTLIST_HPP_
#define STATICOBJECTLIST_HPP_

#include <map>
#include <vector>

#include "server/object/StaticObject.hpp"

class StaticObjectList {
	public:
		/*
			Inserts an object to the container.
			Id must be unique (active) or 0 (stored).
		*/
		void insert(u16 id, const StaticObject &obj);
		void remove(u16 id);

		void serialize(std::ostream &os);
		void deSerialize(std::istream &is);

		/*
			NOTE: When an object is transformed to active, it is removed
			from m_stored and inserted to m_active.
			The caller directly manipulates these containers.
		*/
		std::vector<StaticObject> m_stored;
		std::map<u16, StaticObject> m_active;
};

#endif // STATICOBJECTLIST_HPP_
