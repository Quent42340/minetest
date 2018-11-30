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
#ifndef TILELAYER_H_
#define TILELAYER_H_

#include <memory>
#include <vector>

#include "core/constants.h"
#include "irrlicht/irrlichttypes_extrabloated.h"

enum MaterialType{
	TILE_MATERIAL_BASIC,
	TILE_MATERIAL_ALPHA,
	TILE_MATERIAL_LIQUID_TRANSPARENT,
	TILE_MATERIAL_LIQUID_OPAQUE,
	TILE_MATERIAL_WAVING_LEAVES,
	TILE_MATERIAL_WAVING_PLANTS,
	TILE_MATERIAL_OPAQUE
};

// Material flags
// Should backface culling be enabled?
#define MATERIAL_FLAG_BACKFACE_CULLING 0x01
// Should a crack be drawn?
#define MATERIAL_FLAG_CRACK 0x02
// Should the crack be drawn on transparent pixels (unset) or not (set)?
// Ignored if MATERIAL_FLAG_CRACK is not set.
#define MATERIAL_FLAG_CRACK_OVERLAY 0x04
#define MATERIAL_FLAG_ANIMATION 0x08
//#define MATERIAL_FLAG_HIGHLIGHTED 0x10
#define MATERIAL_FLAG_TILEABLE_HORIZONTAL 0x20
#define MATERIAL_FLAG_TILEABLE_VERTICAL 0x40

/*
	This fully defines the looks of a tile.
	The SMaterial of a tile is constructed according to this.
*/
struct FrameSpec
{
	FrameSpec() = default;

	u32 texture_id = 0;
	video::ITexture *texture = nullptr;
	video::ITexture *normal_texture = nullptr;
	video::ITexture *flags_texture = nullptr;
};

//! Defines a layer of a tile.
struct TileLayer
{
	TileLayer() = default;

	/*!
	 * Two layers are equal if they can be merged.
	 */
	bool operator==(const TileLayer &other) const
	{
		return
			texture_id == other.texture_id &&
			material_type == other.material_type &&
			material_flags == other.material_flags &&
			color == other.color &&
			scale == other.scale;
	}

	/*!
	 * Two tiles are not equal if they must have different vertices.
	 */
	bool operator!=(const TileLayer &other) const
	{
		return !(*this == other);
	}

	// Sets everything else except the texture in the material
	void applyMaterialOptions(video::SMaterial &material) const
	{
		switch (material_type) {
		case TILE_MATERIAL_OPAQUE:
		case TILE_MATERIAL_LIQUID_OPAQUE:
			material.MaterialType = video::EMT_SOLID;
			break;
		case TILE_MATERIAL_BASIC:
		case TILE_MATERIAL_WAVING_LEAVES:
		case TILE_MATERIAL_WAVING_PLANTS:
			material.MaterialTypeParam = 0.5;
			material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
			break;
		case TILE_MATERIAL_ALPHA:
		case TILE_MATERIAL_LIQUID_TRANSPARENT:
			material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
			break;
		default:
			break;
		}
		material.BackfaceCulling = (material_flags & MATERIAL_FLAG_BACKFACE_CULLING) != 0;
		if (!(material_flags & MATERIAL_FLAG_TILEABLE_HORIZONTAL)) {
			material.TextureLayer[0].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
		}
		if (!(material_flags & MATERIAL_FLAG_TILEABLE_VERTICAL)) {
			material.TextureLayer[0].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
		}
	}

	void applyMaterialOptionsWithShaders(video::SMaterial &material) const
	{
		material.BackfaceCulling = (material_flags & MATERIAL_FLAG_BACKFACE_CULLING) != 0;
		if (!(material_flags & MATERIAL_FLAG_TILEABLE_HORIZONTAL)) {
			material.TextureLayer[0].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
			material.TextureLayer[1].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
		}
		if (!(material_flags & MATERIAL_FLAG_TILEABLE_VERTICAL)) {
			material.TextureLayer[0].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
			material.TextureLayer[1].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
		}
	}

	bool isTileable() const
	{
		return (material_flags & MATERIAL_FLAG_TILEABLE_HORIZONTAL)
			&& (material_flags & MATERIAL_FLAG_TILEABLE_VERTICAL);
	}

	// Ordered for size, please do not reorder

	video::ITexture *texture = nullptr;
	video::ITexture *normal_texture = nullptr;
	video::ITexture *flags_texture = nullptr;

	u32 shader_id = 0;

	u32 texture_id = 0;

	u16 animation_frame_length_ms = 0;
	u8 animation_frame_count = 1;

	u8 material_type = TILE_MATERIAL_BASIC;
	u8 material_flags =
		//0 // <- DEBUG, Use the one below
		MATERIAL_FLAG_BACKFACE_CULLING |
		MATERIAL_FLAG_TILEABLE_HORIZONTAL|
		MATERIAL_FLAG_TILEABLE_VERTICAL;

	//! If true, the tile has its own color.
	bool has_color = false;

	std::shared_ptr<std::vector<FrameSpec>> frames = nullptr;

	/*!
	 * The color of the tile, or if the tile does not own
	 * a color then the color of the node owning this tile.
	 */
	video::SColor color;

	u8 scale;
};

#endif // TILELAYER_H_
