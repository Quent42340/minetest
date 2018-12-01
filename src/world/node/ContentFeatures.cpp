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

#include <sstream>

#include "util/serialize.h"
#include "world/node/ContentFeatures.hpp"
#include "world/node/TextureSettings.hpp"

#ifndef SERVER
#include "client/client.h"
#include "client/mesh.h"
#include "client/shader.h"
#endif

/*
	SimpleSoundSpec serialization
*/

static void serializeSimpleSoundSpec(const SimpleSoundSpec &ss, std::ostream &os, u8 version)
{
	os << serializeString(ss.name);
	writeF1000(os, ss.gain);
	writeF1000(os, ss.pitch);
}

static void deSerializeSimpleSoundSpec(SimpleSoundSpec &ss, std::istream &is, u8 version)
{
	ss.name = deSerializeString(is);
	ss.gain = readF1000(is);
	ss.pitch = readF1000(is);
}

/*
	ContentFeatures
*/

ContentFeatures::ContentFeatures()
{
	reset();
}

void ContentFeatures::reset()
{
	/*
		Cached stuff
	*/
#ifndef SERVER
	solidness = 2;
	visual_solidness = 0;
	backface_culling = true;

#endif
	has_on_construct = false;
	has_on_destruct = false;
	has_after_destruct = false;
	/*
		Actual data

		NOTE: Most of this is always overridden by the default values given
		      in builtin.lua
	*/
	name = "";
	groups.clear();
	// Unknown nodes can be dug
	groups["dig_immediate"] = 2;
	drawtype = NDT_NORMAL;
	mesh = "";
#ifndef SERVER
	for (auto &i : mesh_ptr)
		i = NULL;
	minimap_color = video::SColor(0, 0, 0, 0);
#endif
	visual_scale = 1.0;
	for (auto &i : tiledef)
		i = TileDef();
	for (auto &j : tiledef_special)
		j = TileDef();
	alpha = 255;
	post_effect_color = video::SColor(0, 0, 0, 0);
	param_type = CPT_NONE;
	param_type_2 = CPT2_NONE;
	is_ground_content = false;
	light_propagates = false;
	sunlight_propagates = false;
	walkable = true;
	pointable = true;
	diggable = true;
	climbable = false;
	buildable_to = false;
	floodable = false;
	rightclickable = true;
	leveled = 0;
	liquid_type = LIQUID_NONE;
	liquid_alternative_flowing = "";
	liquid_alternative_source = "";
	liquid_viscosity = 0;
	liquid_renewable = true;
	liquid_range = LIQUID_LEVEL_MAX+1;
	drowning = 0;
	light_source = 0;
	damage_per_second = 0;
	node_box = NodeBox();
	selection_box = NodeBox();
	collision_box = NodeBox();
	waving = 0;
	legacy_facedir_simple = false;
	legacy_wallmounted = false;
	sound_footstep = SimpleSoundSpec();
	sound_dig = SimpleSoundSpec("__group");
	sound_dug = SimpleSoundSpec();
	connects_to.clear();
	connects_to_ids.clear();
	connect_sides = 0;
	color = video::SColor(0xFFFFFFFF);
	palette_name = "";
	palette = NULL;
	node_dig_prediction = "air";
}

void ContentFeatures::serialize(std::ostream &os, u16 protocol_version) const
{
	// protocol_version >= 36
	u8 version = 12;
	writeU8(os, version);

	// general
	os << serializeString(name);
	writeU16(os, groups.size());
	for (const auto &group : groups) {
		os << serializeString(group.first);
		writeS16(os, group.second);
	}
	writeU8(os, param_type);
	writeU8(os, param_type_2);

	// visual
	writeU8(os, drawtype);
	os << serializeString(mesh);
	writeF1000(os, visual_scale);
	writeU8(os, 6);
	for (const TileDef &td : tiledef)
		td.serialize(os, protocol_version);
	for (const TileDef &td : tiledef_overlay)
		td.serialize(os, protocol_version);
	writeU8(os, CF_SPECIAL_COUNT);
	for (const TileDef &td : tiledef_special) {
		td.serialize(os, protocol_version);
	}
	writeU8(os, alpha);
	writeU8(os, color.getRed());
	writeU8(os, color.getGreen());
	writeU8(os, color.getBlue());
	os << serializeString(palette_name);
	writeU8(os, waving);
	writeU8(os, connect_sides);
	writeU16(os, connects_to_ids.size());
	for (u16 connects_to_id : connects_to_ids)
		writeU16(os, connects_to_id);
	writeU8(os, post_effect_color.getAlpha());
	writeU8(os, post_effect_color.getRed());
	writeU8(os, post_effect_color.getGreen());
	writeU8(os, post_effect_color.getBlue());
	writeU8(os, leveled);

	// lighting
	writeU8(os, light_propagates);
	writeU8(os, sunlight_propagates);
	writeU8(os, light_source);

	// map generation
	writeU8(os, is_ground_content);

	// interaction
	writeU8(os, walkable);
	writeU8(os, pointable);
	writeU8(os, diggable);
	writeU8(os, climbable);
	writeU8(os, buildable_to);
	writeU8(os, rightclickable);
	writeU32(os, damage_per_second);

	// liquid
	writeU8(os, liquid_type);
	os << serializeString(liquid_alternative_flowing);
	os << serializeString(liquid_alternative_source);
	writeU8(os, liquid_viscosity);
	writeU8(os, liquid_renewable);
	writeU8(os, liquid_range);
	writeU8(os, drowning);
	writeU8(os, floodable);

	// node boxes
	node_box.serialize(os, protocol_version);
	selection_box.serialize(os, protocol_version);
	collision_box.serialize(os, protocol_version);

	// sound
	serializeSimpleSoundSpec(sound_footstep, os, version);
	serializeSimpleSoundSpec(sound_dig, os, version);
	serializeSimpleSoundSpec(sound_dug, os, version);

	// legacy
	writeU8(os, legacy_facedir_simple);
	writeU8(os, legacy_wallmounted);

	os << serializeString(node_dig_prediction);
}

void ContentFeatures::correctAlpha(TileDef *tiles, int length)
{
	// alpha == 0 means that the node is using texture alpha
	if (alpha == 0 || alpha == 255)
		return;

	for (int i = 0; i < length; i++) {
		if (tiles[i].name.empty())
			continue;
		std::stringstream s;
		s << tiles[i].name << "^[noalpha^[opacity:" << ((int)alpha);
		tiles[i].name = s.str();
	}
}

void ContentFeatures::deSerialize(std::istream &is)
{
	// version detection
	int version = readU8(is);
	if (version < 12)
		throw SerializationError("unsupported ContentFeatures version");

	// general
	name = deSerializeString(is);
	groups.clear();
	u32 groups_size = readU16(is);
	for (u32 i = 0; i < groups_size; i++) {
		std::string name = deSerializeString(is);
		int value = readS16(is);
		groups[name] = value;
	}
	param_type = (enum ContentParamType) readU8(is);
	param_type_2 = (enum ContentParamType2) readU8(is);

	// visual
	drawtype = (enum NodeDrawType) readU8(is);
	mesh = deSerializeString(is);
	visual_scale = readF1000(is);
	if (readU8(is) != 6)
		throw SerializationError("unsupported tile count");
	for (TileDef &td : tiledef)
		td.deSerialize(is, version, drawtype);
	for (TileDef &td : tiledef_overlay)
		td.deSerialize(is, version, drawtype);
	if (readU8(is) != CF_SPECIAL_COUNT)
		throw SerializationError("unsupported CF_SPECIAL_COUNT");
	for (TileDef &td : tiledef_special)
		td.deSerialize(is, version, drawtype);
	alpha = readU8(is);
	color.setRed(readU8(is));
	color.setGreen(readU8(is));
	color.setBlue(readU8(is));
	palette_name = deSerializeString(is);
	waving = readU8(is);
	connect_sides = readU8(is);
	u16 connects_to_size = readU16(is);
	connects_to_ids.clear();
	for (u16 i = 0; i < connects_to_size; i++)
		connects_to_ids.push_back(readU16(is));
	post_effect_color.setAlpha(readU8(is));
	post_effect_color.setRed(readU8(is));
	post_effect_color.setGreen(readU8(is));
	post_effect_color.setBlue(readU8(is));
	leveled = readU8(is);

	// lighting-related
	light_propagates = readU8(is);
	sunlight_propagates = readU8(is);
	light_source = readU8(is);
	light_source = MYMIN(light_source, LIGHT_MAX);

	// map generation
	is_ground_content = readU8(is);

	// interaction
	walkable = readU8(is);
	pointable = readU8(is);
	diggable = readU8(is);
	climbable = readU8(is);
	buildable_to = readU8(is);
	rightclickable = readU8(is);
	damage_per_second = readU32(is);

	// liquid
	liquid_type = (enum LiquidType) readU8(is);
	liquid_alternative_flowing = deSerializeString(is);
	liquid_alternative_source = deSerializeString(is);
	liquid_viscosity = readU8(is);
	liquid_renewable = readU8(is);
	liquid_range = readU8(is);
	drowning = readU8(is);
	floodable = readU8(is);

	// node boxes
	node_box.deSerialize(is);
	selection_box.deSerialize(is);
	collision_box.deSerialize(is);

	// sounds
	deSerializeSimpleSoundSpec(sound_footstep, is, version);
	deSerializeSimpleSoundSpec(sound_dig, is, version);
	deSerializeSimpleSoundSpec(sound_dug, is, version);

	// read legacy properties
	legacy_facedir_simple = readU8(is);
	legacy_wallmounted = readU8(is);

	try {
		node_dig_prediction = deSerializeString(is);
	} catch(SerializationError &e) {};
}

#ifndef SERVER
static void fillTileAttribs(ITextureSource *tsrc, TileLayer *layer,
		const TileSpec &tile, const TileDef &tiledef, video::SColor color,
		u8 material_type, u32 shader_id, bool backface_culling,
		const TextureSettings &tsettings)
{
	layer->shader_id     = shader_id;
	layer->texture       = tsrc->getTextureForMesh(tiledef.name, &layer->texture_id);
	layer->material_type = material_type;

	bool has_scale = tiledef.scale > 0;
	if (((tsettings.autoscale_mode == AUTOSCALE_ENABLE) && !has_scale) ||
			(tsettings.autoscale_mode == AUTOSCALE_FORCE)) {
		auto texture_size = layer->texture->getOriginalSize();
		float base_size = tsettings.node_texture_size;
		float size = std::fmin(texture_size.Width, texture_size.Height);
		layer->scale = std::fmax(base_size, size) / base_size;
	} else if (has_scale) {
		layer->scale = tiledef.scale;
	} else {
		layer->scale = 1;
	}
	if (!tile.world_aligned)
		layer->scale = 1;

	// Normal texture and shader flags texture
	if (tsettings.use_normal_texture) {
		layer->normal_texture = tsrc->getNormalTexture(tiledef.name);
	}
	layer->flags_texture = tsrc->getShaderFlagsTexture(layer->normal_texture ? true : false);

	// Material flags
	layer->material_flags = 0;
	if (backface_culling)
		layer->material_flags |= MATERIAL_FLAG_BACKFACE_CULLING;
	if (tiledef.animation.type != TAT_NONE)
		layer->material_flags |= MATERIAL_FLAG_ANIMATION;
	if (tiledef.tileable_horizontal)
		layer->material_flags |= MATERIAL_FLAG_TILEABLE_HORIZONTAL;
	if (tiledef.tileable_vertical)
		layer->material_flags |= MATERIAL_FLAG_TILEABLE_VERTICAL;

	// Color
	layer->has_color = tiledef.has_color;
	if (tiledef.has_color)
		layer->color = tiledef.color;
	else
		layer->color = color;

	// Animation parameters
	int frame_count = 1;
	if (layer->material_flags & MATERIAL_FLAG_ANIMATION) {
		int frame_length_ms;
		tiledef.animation.determineParams(layer->texture->getOriginalSize(),
				&frame_count, &frame_length_ms, NULL);
		layer->animation_frame_count = frame_count;
		layer->animation_frame_length_ms = frame_length_ms;
	}

	if (frame_count == 1) {
		layer->material_flags &= ~MATERIAL_FLAG_ANIMATION;
	} else {
		std::ostringstream os(std::ios::binary);
		if (!layer->frames) {
			layer->frames = std::make_shared<std::vector<FrameSpec>>();
		}
		layer->frames->resize(frame_count);

		for (int i = 0; i < frame_count; i++) {

			FrameSpec frame;

			os.str("");
			os << tiledef.name;
			tiledef.animation.getTextureModifer(os,
					layer->texture->getOriginalSize(), i);

			frame.texture = tsrc->getTextureForMesh(os.str(), &frame.texture_id);
			if (layer->normal_texture)
				frame.normal_texture = tsrc->getNormalTexture(os.str());
			frame.flags_texture = layer->flags_texture;
			(*layer->frames)[i] = frame;
		}
	}
}
#endif

#ifndef SERVER
bool isWorldAligned(AlignStyle style, WorldAlignMode mode, NodeDrawType drawtype)
{
	if (style == ALIGN_STYLE_WORLD)
		return true;
	if (mode == WORLDALIGN_DISABLE)
		return false;
	if (style == ALIGN_STYLE_USER_DEFINED)
		return true;
	if (drawtype == NDT_NORMAL)
		return mode >= WORLDALIGN_FORCE;
	if (drawtype == NDT_NODEBOX)
		return mode >= WORLDALIGN_FORCE_NODEBOX;
	return false;
}

void ContentFeatures::updateTextures(ITextureSource *tsrc, IShaderSource *shdsrc,
	scene::IMeshManipulator *meshmanip, Client *client, const TextureSettings &tsettings)
{
	// minimap pixel color - the average color of a texture
	if (tsettings.enable_minimap && !tiledef[0].name.empty())
		minimap_color = tsrc->getTextureAverageColor(tiledef[0].name);

	// Figure out the actual tiles to use
	TileDef tdef[6];
	for (u32 j = 0; j < 6; j++) {
		tdef[j] = tiledef[j];
		if (tdef[j].name.empty())
			tdef[j].name = "unknown_node.png";
	}
	// also the overlay tiles
	TileDef tdef_overlay[6];
	for (u32 j = 0; j < 6; j++)
		tdef_overlay[j] = tiledef_overlay[j];
	// also the special tiles
	TileDef tdef_spec[6];
	for (u32 j = 0; j < CF_SPECIAL_COUNT; j++)
		tdef_spec[j] = tiledef_special[j];

	bool is_liquid = false;

	u8 material_type = (alpha == 255) ?
		TILE_MATERIAL_BASIC : TILE_MATERIAL_ALPHA;

	switch (drawtype) {
	default:
	case NDT_NORMAL:
		material_type = (alpha == 255) ?
			TILE_MATERIAL_OPAQUE : TILE_MATERIAL_ALPHA;
		solidness = 2;
		break;
	case NDT_AIRLIKE:
		solidness = 0;
		break;
	case NDT_LIQUID:
		assert(liquid_type == LIQUID_SOURCE);
		if (tsettings.opaque_water)
			alpha = 255;
		solidness = 1;
		is_liquid = true;
		break;
	case NDT_FLOWINGLIQUID:
		assert(liquid_type == LIQUID_FLOWING);
		solidness = 0;
		if (tsettings.opaque_water)
			alpha = 255;
		is_liquid = true;
		break;
	case NDT_GLASSLIKE:
		solidness = 0;
		visual_solidness = 1;
		break;
	case NDT_GLASSLIKE_FRAMED:
		solidness = 0;
		visual_solidness = 1;
		break;
	case NDT_GLASSLIKE_FRAMED_OPTIONAL:
		solidness = 0;
		visual_solidness = 1;
		drawtype = tsettings.connected_glass ? NDT_GLASSLIKE_FRAMED : NDT_GLASSLIKE;
		break;
	case NDT_ALLFACES:
		solidness = 0;
		visual_solidness = 1;
		break;
	case NDT_ALLFACES_OPTIONAL:
		if (tsettings.leaves_style == LEAVES_FANCY) {
			drawtype = NDT_ALLFACES;
			solidness = 0;
			visual_solidness = 1;
		} else if (tsettings.leaves_style == LEAVES_SIMPLE) {
			for (u32 j = 0; j < 6; j++) {
				if (!tdef_spec[j].name.empty())
					tdef[j].name = tdef_spec[j].name;
			}
			drawtype = NDT_GLASSLIKE;
			solidness = 0;
			visual_solidness = 1;
		} else {
			drawtype = NDT_NORMAL;
			solidness = 2;
			for (TileDef &td : tdef)
				td.name += std::string("^[noalpha");
		}
		if (waving >= 1)
			material_type = TILE_MATERIAL_WAVING_LEAVES;
		break;
	case NDT_PLANTLIKE:
		solidness = 0;
		if (waving >= 1)
			material_type = TILE_MATERIAL_WAVING_PLANTS;
		break;
	case NDT_FIRELIKE:
		solidness = 0;
		break;
	case NDT_MESH:
	case NDT_NODEBOX:
		solidness = 0;
		if (waving == 1)
			material_type = TILE_MATERIAL_WAVING_PLANTS;
		else if (waving == 2)
			material_type = TILE_MATERIAL_WAVING_LEAVES;
		break;
	case NDT_TORCHLIKE:
	case NDT_SIGNLIKE:
	case NDT_FENCELIKE:
	case NDT_RAILLIKE:
		solidness = 0;
		break;
	case NDT_PLANTLIKE_ROOTED:
		solidness = 2;
		break;
	}

	if (is_liquid) {
		// Vertex alpha is no longer supported, correct if necessary.
		correctAlpha(tdef, 6);
		correctAlpha(tdef_overlay, 6);
		correctAlpha(tdef_spec, CF_SPECIAL_COUNT);
		material_type = (alpha == 255) ?
			TILE_MATERIAL_LIQUID_OPAQUE : TILE_MATERIAL_LIQUID_TRANSPARENT;
	}

	u32 tile_shader = shdsrc->getShader("nodes_shader", material_type, drawtype);

	u8 overlay_material = material_type;
	if (overlay_material == TILE_MATERIAL_OPAQUE)
		overlay_material = TILE_MATERIAL_BASIC;
	else if (overlay_material == TILE_MATERIAL_LIQUID_OPAQUE)
		overlay_material = TILE_MATERIAL_LIQUID_TRANSPARENT;

	u32 overlay_shader = shdsrc->getShader("nodes_shader", overlay_material, drawtype);

	// Tiles (fill in f->tiles[])
	for (u16 j = 0; j < 6; j++) {
		tiles[j].world_aligned = isWorldAligned(tdef[j].align_style,
				tsettings.world_aligned_mode, drawtype);
		fillTileAttribs(tsrc, &tiles[j].layers[0], tiles[j], tdef[j],
				color, material_type, tile_shader,
				tdef[j].backface_culling, tsettings);
		if (!tdef_overlay[j].name.empty())
			fillTileAttribs(tsrc, &tiles[j].layers[1], tiles[j], tdef_overlay[j],
					color, overlay_material, overlay_shader,
					tdef[j].backface_culling, tsettings);
	}

	u8 special_material = material_type;
	if (drawtype == NDT_PLANTLIKE_ROOTED) {
		if (waving == 1)
			special_material = TILE_MATERIAL_WAVING_PLANTS;
		else if (waving == 2)
			special_material = TILE_MATERIAL_WAVING_LEAVES;
	}
	u32 special_shader = shdsrc->getShader("nodes_shader", special_material, drawtype);

	// Special tiles (fill in f->special_tiles[])
	for (u16 j = 0; j < CF_SPECIAL_COUNT; j++)
		fillTileAttribs(tsrc, &special_tiles[j].layers[0], special_tiles[j], tdef_spec[j],
				color, special_material, special_shader,
				tdef_spec[j].backface_culling, tsettings);

	if (param_type_2 == CPT2_COLOR ||
			param_type_2 == CPT2_COLORED_FACEDIR ||
			param_type_2 == CPT2_COLORED_WALLMOUNTED)
		palette = tsrc->getPalette(palette_name);

	if (drawtype == NDT_MESH && !mesh.empty()) {
		// Meshnode drawtype
		// Read the mesh and apply scale
		mesh_ptr[0] = client->getMesh(mesh);
		if (mesh_ptr[0]){
			v3f scale = v3f(1.0, 1.0, 1.0) * BS * visual_scale;
			scaleMesh(mesh_ptr[0], scale);
			recalculateBoundingBox(mesh_ptr[0]);
			meshmanip->recalculateNormals(mesh_ptr[0], true, false);
		}
	}

	//Cache 6dfacedir and wallmounted rotated clones of meshes
	if (tsettings.enable_mesh_cache && mesh_ptr[0] &&
			(param_type_2 == CPT2_FACEDIR
			|| param_type_2 == CPT2_COLORED_FACEDIR)) {
		for (u16 j = 1; j < 24; j++) {
			mesh_ptr[j] = cloneMesh(mesh_ptr[0]);
			rotateMeshBy6dFacedir(mesh_ptr[j], j);
			recalculateBoundingBox(mesh_ptr[j]);
			meshmanip->recalculateNormals(mesh_ptr[j], true, false);
		}
	} else if (tsettings.enable_mesh_cache && mesh_ptr[0]
			&& (param_type_2 == CPT2_WALLMOUNTED ||
			param_type_2 == CPT2_COLORED_WALLMOUNTED)) {
		static const u8 wm_to_6d[6] = { 20, 0, 16 + 1, 12 + 3, 8, 4 + 2 };
		for (u16 j = 1; j < 6; j++) {
			mesh_ptr[j] = cloneMesh(mesh_ptr[0]);
			rotateMeshBy6dFacedir(mesh_ptr[j], wm_to_6d[j]);
			recalculateBoundingBox(mesh_ptr[j]);
			meshmanip->recalculateNormals(mesh_ptr[j], true, false);
		}
		rotateMeshBy6dFacedir(mesh_ptr[0], wm_to_6d[0]);
		recalculateBoundingBox(mesh_ptr[0]);
		meshmanip->recalculateNormals(mesh_ptr[0], true, false);
	}
}
#endif

