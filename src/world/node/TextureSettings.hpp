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

#ifndef TEXTURESETTINGS_HPP_
#define TEXTURESETTINGS_HPP_

#include "irrlicht/irrlichttypes.h"

enum LeavesStyle {
	LEAVES_FANCY,
	LEAVES_SIMPLE,
	LEAVES_OPAQUE,
};

enum AutoScale : u8 {
	AUTOSCALE_DISABLE,
	AUTOSCALE_ENABLE,
	AUTOSCALE_FORCE,
};

enum WorldAlignMode : u8 {
	WORLDALIGN_DISABLE,
	WORLDALIGN_ENABLE,
	WORLDALIGN_FORCE,
	WORLDALIGN_FORCE_NODEBOX,
};

class TextureSettings {
	public:
		TextureSettings() = default;

		void readSettings();

		LeavesStyle leaves_style;
		WorldAlignMode world_aligned_mode;
		AutoScale autoscale_mode;

		int node_texture_size;

		bool opaque_water;
		bool connected_glass;
		bool use_normal_texture;
		bool enable_mesh_cache;
		bool enable_minimap;
};

#endif // TEXTURESETTINGS_HPP_
