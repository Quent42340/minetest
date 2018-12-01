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
#ifndef TILEDEF_HPP_
#define TILEDEF_HPP_

#include <istream>

#include "common/TileAnimationParams.hpp"
#include "irrlicht/irrlichttypes_extrabloated.h"
#include "common/world/node/NodeDrawType.hpp"

enum AlignStyle : u8 {
	ALIGN_STYLE_NODE,
	ALIGN_STYLE_WORLD,
	ALIGN_STYLE_USER_DEFINED,
};

/*
	Stand-alone definition of a TileSpec (basically a server-side TileSpec)
*/

struct TileDef {
	TileDef() { animation.type = TAT_NONE; }

	void serialize(std::ostream &os, u16 protocol_version) const;
	void deSerialize(std::istream &is, u8 contentfeatures_version, NodeDrawType drawtype);

	std::string name = "";
	bool backface_culling = true; // Takes effect only in special cases
	bool tileable_horizontal = true;
	bool tileable_vertical = true;
	//! If true, the tile has its own color.
	bool has_color = false;
	//! The color of the tile.
	video::SColor color = video::SColor(0xFFFFFFFF);
	AlignStyle align_style = ALIGN_STYLE_NODE;
	u8 scale = 0;

	struct TileAnimationParams animation;
};

#endif // TILEDEF_HPP_
