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
#ifndef NODEDRAWTYPE_HPP_
#define NODEDRAWTYPE_HPP_

#include "irrlicht/irrlichttypes.h"

enum ContentParamType
{
	CPT_NONE,
	CPT_LIGHT,
};

enum ContentParamType2
{
	CPT2_NONE,
	// Need 8-bit param2
	CPT2_FULL,
	// Flowing liquid properties
	CPT2_FLOWINGLIQUID,
	// Direction for chests and furnaces and such
	CPT2_FACEDIR,
	// Direction for signs, torches and such
	CPT2_WALLMOUNTED,
	// Block level like FLOWINGLIQUID
	CPT2_LEVELED,
	// 2D rotation for things like plants
	CPT2_DEGROTATE,
	// Mesh options for plants
	CPT2_MESHOPTIONS,
	// Index for palette
	CPT2_COLOR,
	// 3 bits of palette index, then facedir
	CPT2_COLORED_FACEDIR,
	// 5 bits of palette index, then wallmounted
	CPT2_COLORED_WALLMOUNTED,
	// Glasslike framed drawtype internal liquid level, param2 values 0 to 63
	CPT2_GLASSLIKE_LIQUID_LEVEL,
};

enum LiquidType
{
	LIQUID_NONE,
	LIQUID_FLOWING,
	LIQUID_SOURCE,
};

// Mesh options for NDT_PLANTLIKE with CPT2_MESHOPTIONS
static const u8 MO_MASK_STYLE          = 0x07;
static const u8 MO_BIT_RANDOM_OFFSET   = 0x08;
static const u8 MO_BIT_SCALE_SQRT2     = 0x10;
static const u8 MO_BIT_RANDOM_OFFSET_Y = 0x20;

enum PlantlikeStyle {
	PLANT_STYLE_CROSS,
	PLANT_STYLE_CROSS2,
	PLANT_STYLE_STAR,
	PLANT_STYLE_HASH,
	PLANT_STYLE_HASH2,
};

enum NodeDrawType
{
	// A basic solid block
	NDT_NORMAL,
	// Nothing is drawn
	NDT_AIRLIKE,
	// Do not draw face towards same kind of flowing/source liquid
	NDT_LIQUID,
	// A very special kind of thing
	NDT_FLOWINGLIQUID,
	// Glass-like, don't draw faces towards other glass
	NDT_GLASSLIKE,
	// Leaves-like, draw all faces no matter what
	NDT_ALLFACES,
	// Enabled -> ndt_allfaces, disabled -> ndt_normal
	NDT_ALLFACES_OPTIONAL,
	// Single plane perpendicular to a surface
	NDT_TORCHLIKE,
	// Single plane parallel to a surface
	NDT_SIGNLIKE,
	// 2 vertical planes in a 'X' shape diagonal to XZ axes.
	// paramtype2 = "meshoptions" allows various forms, sizes and
	// vertical and horizontal random offsets.
	NDT_PLANTLIKE,
	// Fenceposts that connect to neighbouring fenceposts with horizontal bars
	NDT_FENCELIKE,
	// Selects appropriate junction texture to connect like rails to
	// neighbouring raillikes.
	NDT_RAILLIKE,
	// Custom Lua-definable structure of multiple cuboids
	NDT_NODEBOX,
	// Glass-like, draw connected frames and all visible faces.
	// param2 > 0 defines 64 levels of internal liquid
	// Uses 3 textures, one for frames, second for faces,
	// optional third is a 'special tile' for the liquid.
	NDT_GLASSLIKE_FRAMED,
	// Draw faces slightly rotated and only on neighbouring nodes
	NDT_FIRELIKE,
	// Enabled -> ndt_glasslike_framed, disabled -> ndt_glasslike
	NDT_GLASSLIKE_FRAMED_OPTIONAL,
	// Uses static meshes
	NDT_MESH,
	// Combined plantlike-on-solid
	NDT_PLANTLIKE_ROOTED,
};

#endif // NODEDRAWTYPE_HPP_
