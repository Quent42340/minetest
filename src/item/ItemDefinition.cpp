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

#include "item/ItemDefinition.hpp"

#include "nodedef.h"
#include "tool.h"
#include "inventory.h"

#ifndef SERVER
#include "client/mapblock_mesh.h"
#include "client/mesh.h"
#include "client/wieldmesh.h"
#include "client/tile.h"
#include "client/client.h"
#endif

#include "core/log.h"
#include "core/settings.h"
#include "util/serialize.h"
#include "util/container.h"
#include "util/thread.h"
#include <map>
#include <set>

#ifdef __ANDROID__
#include <GLES/gl.h>
#endif

ItemDefinition::ItemDefinition()
{
	resetInitial();
}

ItemDefinition::ItemDefinition(const ItemDefinition &def)
{
	resetInitial();
	*this = def;
}

ItemDefinition& ItemDefinition::operator=(const ItemDefinition &def)
{
	if(this == &def)
		return *this;

	reset();

	type = def.type;
	name = def.name;
	description = def.description;
	inventory_image = def.inventory_image;
	inventory_overlay = def.inventory_overlay;
	wield_image = def.wield_image;
	wield_overlay = def.wield_overlay;
	wield_scale = def.wield_scale;
	stack_max = def.stack_max;
	usable = def.usable;
	liquids_pointable = def.liquids_pointable;
	if(def.tool_capabilities)
	{
		tool_capabilities = new ToolCapabilities(
				*def.tool_capabilities);
	}
	groups = def.groups;
	node_placement_prediction = def.node_placement_prediction;
	sound_place = def.sound_place;
	sound_place_failed = def.sound_place_failed;
	range = def.range;
	palette_image = def.palette_image;
	color = def.color;
	return *this;
}

ItemDefinition::~ItemDefinition()
{
	reset();
}

void ItemDefinition::resetInitial()
{
	// Initialize pointers to NULL so reset() does not delete undefined pointers
	tool_capabilities = NULL;
	reset();
}

void ItemDefinition::reset()
{
	type = ITEM_NONE;
	name = "";
	description = "";
	inventory_image = "";
	inventory_overlay = "";
	wield_image = "";
	wield_overlay = "";
	palette_image = "";
	color = video::SColor(0xFFFFFFFF);
	wield_scale = v3f(1.0, 1.0, 1.0);
	stack_max = 99;
	usable = false;
	liquids_pointable = false;
	delete tool_capabilities;
	tool_capabilities = NULL;
	groups.clear();
	sound_place = SimpleSoundSpec();
	sound_place_failed = SimpleSoundSpec();
	range = -1;

	node_placement_prediction = "";
}

void ItemDefinition::serialize(std::ostream &os, u16 protocol_version) const
{
	// protocol_version >= 36
	u8 version = 5;
	writeU8(os, version);
	writeU8(os, type);
	os << serializeString(name);
	os << serializeString(description);
	os << serializeString(inventory_image);
	os << serializeString(wield_image);
	writeV3F1000(os, wield_scale);
	writeS16(os, stack_max);
	writeU8(os, usable);
	writeU8(os, liquids_pointable);
	std::string tool_capabilities_s;
	if(tool_capabilities){
		std::ostringstream tmp_os(std::ios::binary);
		tool_capabilities->serialize(tmp_os, protocol_version);
		tool_capabilities_s = tmp_os.str();
	}
	os << serializeString(tool_capabilities_s);
	writeU16(os, groups.size());
	for (const auto &group : groups) {
		os << serializeString(group.first);
		writeS16(os, group.second);
	}
	os << serializeString(node_placement_prediction);
	os << serializeString(sound_place.name);
	writeF1000(os, sound_place.gain);
	writeF1000(os, range);
	os << serializeString(sound_place_failed.name);
	writeF1000(os, sound_place_failed.gain);
	os << serializeString(palette_image);
	writeARGB8(os, color);

	writeF1000(os, sound_place.pitch);
	writeF1000(os, sound_place_failed.pitch);
	os << serializeString(inventory_overlay);
	os << serializeString(wield_overlay);
}

void ItemDefinition::deSerialize(std::istream &is)
{
	// Reset everything
	reset();

	// Deserialize
	int version = readU8(is);
	if (version < 5)
		throw SerializationError("unsupported ItemDefinition version");

	type = (enum ItemType)readU8(is);
	name = deSerializeString(is);
	description = deSerializeString(is);
	inventory_image = deSerializeString(is);
	wield_image = deSerializeString(is);
	wield_scale = readV3F1000(is);
	stack_max = readS16(is);
	usable = readU8(is);
	liquids_pointable = readU8(is);
	std::string tool_capabilities_s = deSerializeString(is);
	if(!tool_capabilities_s.empty())
	{
		std::istringstream tmp_is(tool_capabilities_s, std::ios::binary);
		tool_capabilities = new ToolCapabilities;
		tool_capabilities->deSerialize(tmp_is);
	}
	groups.clear();
	u32 groups_size = readU16(is);
	for(u32 i=0; i<groups_size; i++){
		std::string name = deSerializeString(is);
		int value = readS16(is);
		groups[name] = value;
	}

	node_placement_prediction = deSerializeString(is);
	//deserializeSimpleSoundSpec(sound_place, is);
	sound_place.name = deSerializeString(is);
	sound_place.gain = readF1000(is);
	range = readF1000(is);

	sound_place_failed.name = deSerializeString(is);
	sound_place_failed.gain = readF1000(is);
	palette_image = deSerializeString(is);
	color = readARGB8(is);

	sound_place.pitch = readF1000(is);
	sound_place_failed.pitch = readF1000(is);
	inventory_overlay = deSerializeString(is);
	wield_overlay = deSerializeString(is);

	// If you add anything here, insert it primarily inside the try-catch
	// block to not need to increase the version.
	//try {
	//} catch(SerializationError &e) {};
}

