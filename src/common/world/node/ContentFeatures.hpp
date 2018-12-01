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
#ifndef CONTENTFEATURES_HPP_
#define CONTENTFEATURES_HPP_

#include "client/tile.h"
#include "common/SimpleSoundSpec.hpp"
#include "common/inventory/itemgroup.h"
#include "common/map/MapNode.hpp"
#include "common/world/node/NodeBox.hpp"
#include "common/world/node/NodeDrawType.hpp"
#include "common/world/node/TileDef.hpp"

#define CF_SPECIAL_COUNT 6

class Client;
class IShaderSource;
class ITextureSource;
class TextureSettings;

struct ContentFeatures
{
	/*
		Cached stuff
	 */
#ifndef SERVER
	// 0     1     2     3     4     5
	// up    down  right left  back  front
	TileSpec tiles[6];
	// Special tiles
	// - Currently used for flowing liquids
	TileSpec special_tiles[CF_SPECIAL_COUNT];
	u8 solidness; // Used when choosing which face is drawn
	u8 visual_solidness; // When solidness=0, this tells how it looks like
	bool backface_culling;
#endif

	// Server-side cached callback existence for fast skipping
	bool has_on_construct;
	bool has_on_destruct;
	bool has_after_destruct;

	/*
		Actual data
	 */

	// --- GENERAL PROPERTIES ---

	std::string name; // "" = undefined node
	ItemGroupList groups; // Same as in itemdef
	// Type of MapNode::param1
	ContentParamType param_type;
	// Type of MapNode::param2
	ContentParamType2 param_type_2;

	// --- VISUAL PROPERTIES ---

	enum NodeDrawType drawtype;
	std::string mesh;
#ifndef SERVER
	scene::IMesh *mesh_ptr[24];
	video::SColor minimap_color;
#endif
	float visual_scale; // Misc. scale parameter
	TileDef tiledef[6];
	// These will be drawn over the base tiles.
	TileDef tiledef_overlay[6];
	TileDef tiledef_special[CF_SPECIAL_COUNT]; // eg. flowing liquid
	// If 255, the node is opaque.
	// Otherwise it uses texture alpha.
	u8 alpha;
	// The color of the node.
	video::SColor color;
	std::string palette_name;
	std::vector<video::SColor> *palette;
	// Used for waving leaves/plants
	u8 waving;
	// for NDT_CONNECTED pairing
	u8 connect_sides;
	std::vector<std::string> connects_to;
	std::vector<content_t> connects_to_ids;
	// Post effect color, drawn when the camera is inside the node.
	video::SColor post_effect_color;
	// Flowing liquid or snow, value = default level
	u8 leveled;

	// --- LIGHTING-RELATED ---

	bool light_propagates;
	bool sunlight_propagates;
	// Amount of light the node emits
	u8 light_source;

	// --- MAP GENERATION ---

	// True for all ground-like things like stone and mud, false for eg. trees
	bool is_ground_content;

	// --- INTERACTION PROPERTIES ---

	// This is used for collision detection.
	// Also for general solidness queries.
	bool walkable;
	// Player can point to these
	bool pointable;
	// Player can dig these
	bool diggable;
	// Player can climb these
	bool climbable;
	// Player can build on these
	bool buildable_to;
	// Player cannot build to these (placement prediction disabled)
	bool rightclickable;
	u32 damage_per_second;
	// client dig prediction
	std::string node_dig_prediction;

	// --- LIQUID PROPERTIES ---

	// Whether the node is non-liquid, source liquid or flowing liquid
	enum LiquidType liquid_type;
	// If the content is liquid, this is the flowing version of the liquid.
	std::string liquid_alternative_flowing;
	// If the content is liquid, this is the source version of the liquid.
	std::string liquid_alternative_source;
	// Viscosity for fluid flow, ranging from 1 to 7, with
	// 1 giving almost instantaneous propagation and 7 being
	// the slowest possible
	u8 liquid_viscosity;
	// Is liquid renewable (new liquid source will be created between 2 existing)
	bool liquid_renewable;
	// Number of flowing liquids surrounding source
	u8 liquid_range;
	u8 drowning;
	// Liquids flow into and replace node
	bool floodable;

	// --- NODEBOXES ---

	NodeBox node_box;
	NodeBox selection_box;
	NodeBox collision_box;

	// --- SOUND PROPERTIES ---

	SimpleSoundSpec sound_footstep;
	SimpleSoundSpec sound_dig;
	SimpleSoundSpec sound_dug;

	// --- LEGACY ---

	// Compatibility with old maps
	// Set to true if paramtype used to be 'facedir_simple'
	bool legacy_facedir_simple;
	// Set to true if wall_mounted used to be set to true
	bool legacy_wallmounted;

	/*
		Methods
	*/

	ContentFeatures();
	~ContentFeatures() = default;
	void reset();
	void serialize(std::ostream &os, u16 protocol_version) const;
	void deSerialize(std::istream &is);
	/*!
	 * Since vertex alpha is no longer supported, this method
	 * adds opacity directly to the texture pixels.
	 *
	 * \param tiles array of the tile definitions.
	 * \param length length of tiles
	 */
	void correctAlpha(TileDef *tiles, int length);

	/*
		Some handy methods
	*/
	bool isLiquid() const{
		return (liquid_type != LIQUID_NONE);
	}
	bool sameLiquid(const ContentFeatures &f) const{
		if(!isLiquid() || !f.isLiquid()) return false;
		return (liquid_alternative_flowing == f.liquid_alternative_flowing);
	}

	int getGroup(const std::string &group) const
	{
		return itemgroup_get(groups, group);
	}

#ifndef SERVER
	void updateTextures(ITextureSource *tsrc, IShaderSource *shdsrc,
		scene::IMeshManipulator *meshmanip, Client *client, const TextureSettings &tsettings);
#endif
};

#endif // CONTENTFEATURES_HPP_
