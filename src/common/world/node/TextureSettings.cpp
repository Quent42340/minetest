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

#include "core/settings.h"
#include "TextureSettings.hpp"

void TextureSettings::readSettings()
{
	connected_glass                = g_settings->getBool("connected_glass");
	opaque_water                   = g_settings->getBool("opaque_water");
	bool enable_shaders            = g_settings->getBool("enable_shaders");
	bool enable_bumpmapping        = g_settings->getBool("enable_bumpmapping");
	bool enable_parallax_occlusion = g_settings->getBool("enable_parallax_occlusion");
	bool smooth_lighting           = g_settings->getBool("smooth_lighting");
	enable_mesh_cache              = g_settings->getBool("enable_mesh_cache");
	enable_minimap                 = g_settings->getBool("enable_minimap");
	node_texture_size              = g_settings->getU16("texture_min_size");
	std::string leaves_style_str   = g_settings->get("leaves_style");
	std::string world_aligned_mode_str = g_settings->get("world_aligned_mode");
	std::string autoscale_mode_str = g_settings->get("autoscale_mode");

	// Mesh cache is not supported in combination with smooth lighting
	if (smooth_lighting)
		enable_mesh_cache = false;

	use_normal_texture = enable_shaders &&
		(enable_bumpmapping || enable_parallax_occlusion);
	if (leaves_style_str == "fancy") {
		leaves_style = LEAVES_FANCY;
	} else if (leaves_style_str == "simple") {
		leaves_style = LEAVES_SIMPLE;
	} else {
		leaves_style = LEAVES_OPAQUE;
	}

	if (world_aligned_mode_str == "enable")
		world_aligned_mode = WORLDALIGN_ENABLE;
	else if (world_aligned_mode_str == "force_solid")
		world_aligned_mode = WORLDALIGN_FORCE;
	else if (world_aligned_mode_str == "force_nodebox")
		world_aligned_mode = WORLDALIGN_FORCE_NODEBOX;
	else
		world_aligned_mode = WORLDALIGN_DISABLE;

	if (autoscale_mode_str == "enable")
		autoscale_mode = AUTOSCALE_ENABLE;
	else if (autoscale_mode_str == "force")
		autoscale_mode = AUTOSCALE_FORCE;
	else
		autoscale_mode = AUTOSCALE_DISABLE;
}

