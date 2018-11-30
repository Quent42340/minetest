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
#ifndef ITEMDEFINITION_HPP_
#define ITEMDEFINITION_HPP_

#include "irrlichttypes_extrabloated.h"
#include <string>
#include <iostream>
#include <set>
#include "itemgroup.h"
#include "common/sound.h"
class IGameDef;
class Client;
struct ToolCapabilities;
#ifndef SERVER
#include "client/tile.h"
struct ItemMesh;
struct ItemStack;
#endif

/*
	Base item definition
*/

enum ItemType
{
	ITEM_NONE,
	ITEM_NODE,
	ITEM_CRAFT,
	ITEM_TOOL,
};

struct ItemDefinition
{
	/*
		Basic item properties
	*/
	ItemType type;
	std::string name; // "" = hand
	std::string description; // Shown in tooltip.

	/*
		Visual properties
	*/
	std::string inventory_image; // Optional for nodes, mandatory for tools/craftitems
	std::string inventory_overlay; // Overlay of inventory_image.
	std::string wield_image; // If empty, inventory_image or mesh (only nodes) is used
	std::string wield_overlay; // Overlay of wield_image.
	std::string palette_image; // If specified, the item will be colorized based on this
	video::SColor color; // The fallback color of the node.
	v3f wield_scale;

	/*
		Item stack and interaction properties
	*/
	u16 stack_max;
	bool usable;
	bool liquids_pointable;
	// May be NULL. If non-NULL, deleted by destructor
	ToolCapabilities *tool_capabilities;
	ItemGroupList groups;
	SimpleSoundSpec sound_place;
	SimpleSoundSpec sound_place_failed;
	f32 range;

	// Client shall immediately place this node when player places the item.
	// Server will update the precise end result a moment later.
	// "" = no prediction
	std::string node_placement_prediction;

	ItemDefinition();
	ItemDefinition(const ItemDefinition &def);
	ItemDefinition& operator=(const ItemDefinition &def);
	~ItemDefinition();
	void reset();
	void serialize(std::ostream &os, u16 protocol_version) const;
	void deSerialize(std::istream &is);
private:
	void resetInitial();
};

#endif // ITEMDEFINITION_HPP_
