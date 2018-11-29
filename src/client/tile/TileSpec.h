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
#ifndef TILESPEC_H_
#define TILESPEC_H_

#include "irrlichttypes.h"
#include "client/tile/TileLayer.h"

#define MAX_TILE_LAYERS 2

/*!
 * Defines a face of a node. May have up to two layers.
 */
struct TileSpec
{
	TileSpec() = default;

	/*!
	 * Returns true if this tile can be merged with the other tile.
	 */
	bool isTileable(const TileSpec &other) const {
		for (int layer = 0; layer < MAX_TILE_LAYERS; layer++) {
			if (layers[layer] != other.layers[layer])
				return false;
			if (!layers[layer].isTileable())
				return false;
		}
		return rotation == 0
			&& rotation == other.rotation
			&& emissive_light == other.emissive_light;
	}

	//! If true, the tile rotation is ignored.
	bool world_aligned = false;
	//! Tile rotation.
	u8 rotation = 0;
	//! This much light does the tile emit.
	u8 emissive_light = 0;
	//! The first is base texture, the second is overlay.
	TileLayer layers[MAX_TILE_LAYERS];
};

#endif // TILESPEC_H_
