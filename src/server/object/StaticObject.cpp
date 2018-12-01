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

#include "server/object/StaticObject.hpp"
#include "server/serverobject.h"
#include "util/serialize.h"

StaticObject::StaticObject(const ServerActiveObject *s_obj, const v3f &pos_)
	: type(s_obj->getType()), pos(pos_)
{
	s_obj->getStaticData(&data);
}

void StaticObject::serialize(std::ostream &os)
{
	// type
	writeU8(os, type);
	// pos
	writeV3F1000(os, pos);
	// data
	os<<serializeString(data);
}

void StaticObject::deSerialize(std::istream &is, u8 version)
{
	// type
	type = readU8(is);
	// pos
	pos = readV3F1000(is);
	// data
	data = deSerializeString(is);
}

