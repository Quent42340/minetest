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

#include "core/constants.h"
#include "common/world/node/NodeBox.hpp"
#include "util/serialize.h"

void NodeBox::reset()
{
	type = NODEBOX_REGULAR;
	// default is empty
	fixed.clear();
	// default is sign/ladder-like
	wall_top = aabb3f(-BS/2, BS/2-BS/16., -BS/2, BS/2, BS/2, BS/2);
	wall_bottom = aabb3f(-BS/2, -BS/2, -BS/2, BS/2, -BS/2+BS/16., BS/2);
	wall_side = aabb3f(-BS/2, -BS/2, -BS/2, -BS/2+BS/16., BS/2, BS/2);
	// no default for other parts
	connect_top.clear();
	connect_bottom.clear();
	connect_front.clear();
	connect_left.clear();
	connect_back.clear();
	connect_right.clear();
	disconnected_top.clear();
	disconnected_bottom.clear();
	disconnected_front.clear();
	disconnected_left.clear();
	disconnected_back.clear();
	disconnected_right.clear();
	disconnected.clear();
	disconnected_sides.clear();
}

void NodeBox::serialize(std::ostream &os, u16 protocol_version) const
{
	// Protocol >= 36
	int version = 5;
	writeU8(os, version);

	switch (type) {
	case NODEBOX_LEVELED:
	case NODEBOX_FIXED:
		writeU8(os, type);

		writeU16(os, fixed.size());
		for (const aabb3f &nodebox : fixed) {
			writeV3F1000(os, nodebox.MinEdge);
			writeV3F1000(os, nodebox.MaxEdge);
		}
		break;
	case NODEBOX_WALLMOUNTED:
		writeU8(os, type);

		writeV3F1000(os, wall_top.MinEdge);
		writeV3F1000(os, wall_top.MaxEdge);
		writeV3F1000(os, wall_bottom.MinEdge);
		writeV3F1000(os, wall_bottom.MaxEdge);
		writeV3F1000(os, wall_side.MinEdge);
		writeV3F1000(os, wall_side.MaxEdge);
		break;
	case NODEBOX_CONNECTED:
		writeU8(os, type);

#define WRITEBOX(box) \
		writeU16(os, (box).size()); \
		for (const aabb3f &i: (box)) { \
			writeV3F1000(os, i.MinEdge); \
			writeV3F1000(os, i.MaxEdge); \
		};

		WRITEBOX(fixed);
		WRITEBOX(connect_top);
		WRITEBOX(connect_bottom);
		WRITEBOX(connect_front);
		WRITEBOX(connect_left);
		WRITEBOX(connect_back);
		WRITEBOX(connect_right);
		WRITEBOX(disconnected_top);
		WRITEBOX(disconnected_bottom);
		WRITEBOX(disconnected_front);
		WRITEBOX(disconnected_left);
		WRITEBOX(disconnected_back);
		WRITEBOX(disconnected_right);
		WRITEBOX(disconnected);
		WRITEBOX(disconnected_sides);
		break;
	default:
		writeU8(os, type);
		break;
	}
}

void NodeBox::deSerialize(std::istream &is)
{
	int version = readU8(is);
	if (version < 4)
		throw SerializationError("unsupported NodeBox version");

	reset();

	type = (enum NodeBoxType)readU8(is);

	if(type == NODEBOX_FIXED || type == NODEBOX_LEVELED)
	{
		u16 fixed_count = readU16(is);
		while(fixed_count--)
		{
			aabb3f box;
			box.MinEdge = readV3F1000(is);
			box.MaxEdge = readV3F1000(is);
			fixed.push_back(box);
		}
	}
	else if(type == NODEBOX_WALLMOUNTED)
	{
		wall_top.MinEdge = readV3F1000(is);
		wall_top.MaxEdge = readV3F1000(is);
		wall_bottom.MinEdge = readV3F1000(is);
		wall_bottom.MaxEdge = readV3F1000(is);
		wall_side.MinEdge = readV3F1000(is);
		wall_side.MaxEdge = readV3F1000(is);
	}
	else if (type == NODEBOX_CONNECTED)
	{
#define READBOXES(box) { \
		count = readU16(is); \
		(box).reserve(count); \
		while (count--) { \
			v3f min = readV3F1000(is); \
			v3f max = readV3F1000(is); \
			(box).emplace_back(min, max); }; }

		u16 count;

		READBOXES(fixed);
		READBOXES(connect_top);
		READBOXES(connect_bottom);
		READBOXES(connect_front);
		READBOXES(connect_left);
		READBOXES(connect_back);
		READBOXES(connect_right);
		if (version >= 5) {
			READBOXES(disconnected_top);
			READBOXES(disconnected_bottom);
			READBOXES(disconnected_front);
			READBOXES(disconnected_left);
			READBOXES(disconnected_back);
			READBOXES(disconnected_right);
			READBOXES(disconnected);
			READBOXES(disconnected_sides);
		}
	}
}

