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
#ifndef STATICOBJECT_HPP_
#define STATICOBJECT_HPP_

#include <string>

#include "irrlicht/irrlichttypes_bloated.h"

class ServerActiveObject;

struct StaticObject
{
	u8 type = 0;
	v3f pos;
	std::string data;

	StaticObject() = default;
	StaticObject(const ServerActiveObject *s_obj, const v3f &pos_);

	void serialize(std::ostream &os);
	void deSerialize(std::istream &is, u8 version);
};

#endif // STATICOBJECT_HPP_
