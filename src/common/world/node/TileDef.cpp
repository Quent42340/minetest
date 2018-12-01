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

#include "util/serialize.h"
#include "common/world/node/TileDef.hpp"

#define TILE_FLAG_BACKFACE_CULLING	(1 << 0)
#define TILE_FLAG_TILEABLE_HORIZONTAL	(1 << 1)
#define TILE_FLAG_TILEABLE_VERTICAL	(1 << 2)
#define TILE_FLAG_HAS_COLOR	(1 << 3)
#define TILE_FLAG_HAS_SCALE	(1 << 4)
#define TILE_FLAG_HAS_ALIGN_STYLE	(1 << 5)

void TileDef::serialize(std::ostream &os, u16 protocol_version) const
{
	// protocol_version >= 36
	u8 version = 6;
	writeU8(os, version);

	os << serializeString(name);
	animation.serialize(os, version);
	bool has_scale = scale > 0;
	u16 flags = 0;
	if (backface_culling)
		flags |= TILE_FLAG_BACKFACE_CULLING;
	if (tileable_horizontal)
		flags |= TILE_FLAG_TILEABLE_HORIZONTAL;
	if (tileable_vertical)
		flags |= TILE_FLAG_TILEABLE_VERTICAL;
	if (has_color)
		flags |= TILE_FLAG_HAS_COLOR;
	if (has_scale)
		flags |= TILE_FLAG_HAS_SCALE;
	if (align_style != ALIGN_STYLE_NODE)
		flags |= TILE_FLAG_HAS_ALIGN_STYLE;
	writeU16(os, flags);
	if (has_color) {
		writeU8(os, color.getRed());
		writeU8(os, color.getGreen());
		writeU8(os, color.getBlue());
	}
	if (has_scale)
		writeU8(os, scale);
	if (align_style != ALIGN_STYLE_NODE)
		writeU8(os, align_style);
}

void TileDef::deSerialize(std::istream &is, u8 contentfeatures_version,
	NodeDrawType drawtype)
{
	int version = readU8(is);
	if (version < 6)
		throw SerializationError("unsupported TileDef version");
	name = deSerializeString(is);
	animation.deSerialize(is, version);
	u16 flags = readU16(is);
	backface_culling = flags & TILE_FLAG_BACKFACE_CULLING;
	tileable_horizontal = flags & TILE_FLAG_TILEABLE_HORIZONTAL;
	tileable_vertical = flags & TILE_FLAG_TILEABLE_VERTICAL;
	has_color = flags & TILE_FLAG_HAS_COLOR;
	bool has_scale = flags & TILE_FLAG_HAS_SCALE;
	bool has_align_style = flags & TILE_FLAG_HAS_ALIGN_STYLE;
	if (has_color) {
		color.setRed(readU8(is));
		color.setGreen(readU8(is));
		color.setBlue(readU8(is));
	}
	scale = has_scale ? readU8(is) : 0;
	if (has_align_style)
		align_style = static_cast<AlignStyle>(readU8(is));
	else
		align_style = ALIGN_STYLE_NODE;
}

