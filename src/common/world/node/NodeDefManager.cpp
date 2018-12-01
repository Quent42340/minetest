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

#include <algorithm>
#include "core/log.h"
#include "util/serialize.h"
#include "util/string.h"
#include "common/item/IItemDefManager.hpp"
#include "common/world/node/NodeDefManager.hpp"
#include "common/world/node/NodeResolver.hpp"
#include "common/world/node/TextureSettings.hpp"

#ifndef SERVER
#include "client/client.h"
#include "client/renderingengine.h"
#endif

NodeDefManager::NodeDefManager()
{
	clear();
}

NodeDefManager::~NodeDefManager()
{
#ifndef SERVER
	for (ContentFeatures &f : m_content_features) {
		for (auto &j : f.mesh_ptr) {
			if (j)
				j->drop();
		}
	}
#endif
}

void NodeDefManager::clear()
{
	m_content_features.clear();
	m_name_id_mapping.clear();
	m_name_id_mapping_with_aliases.clear();
	m_group_to_items.clear();
	m_next_id = 0;
	m_selection_box_union.reset(0,0,0);
	m_selection_box_int_union.reset(0,0,0);

	resetNodeResolveState();

	u32 initial_length = 0;
	initial_length = MYMAX(initial_length, CONTENT_UNKNOWN + 1);
	initial_length = MYMAX(initial_length, CONTENT_AIR + 1);
	initial_length = MYMAX(initial_length, CONTENT_IGNORE + 1);
	m_content_features.resize(initial_length);

	// Set CONTENT_UNKNOWN
	{
		ContentFeatures f;
		f.name = "unknown";
		// Insert directly into containers
		content_t c = CONTENT_UNKNOWN;
		m_content_features[c] = f;
		addNameIdMapping(c, f.name);
	}

	// Set CONTENT_AIR
	{
		ContentFeatures f;
		f.name                = "air";
		f.drawtype            = NDT_AIRLIKE;
		f.param_type          = CPT_LIGHT;
		f.light_propagates    = true;
		f.sunlight_propagates = true;
		f.walkable            = false;
		f.pointable           = false;
		f.diggable            = false;
		f.buildable_to        = true;
		f.floodable           = true;
		f.is_ground_content   = true;
		// Insert directly into containers
		content_t c = CONTENT_AIR;
		m_content_features[c] = f;
		addNameIdMapping(c, f.name);
	}

	// Set CONTENT_IGNORE
	{
		ContentFeatures f;
		f.name                = "ignore";
		f.drawtype            = NDT_AIRLIKE;
		f.param_type          = CPT_NONE;
		f.light_propagates    = false;
		f.sunlight_propagates = false;
		f.walkable            = false;
		f.pointable           = false;
		f.diggable            = false;
		f.buildable_to        = true; // A way to remove accidental CONTENT_IGNOREs
		f.is_ground_content   = true;
		// Insert directly into containers
		content_t c = CONTENT_IGNORE;
		m_content_features[c] = f;
		addNameIdMapping(c, f.name);
	}
}

bool NodeDefManager::getId(const std::string &name, content_t &result) const
{
	std::unordered_map<std::string, content_t>::const_iterator
		i = m_name_id_mapping_with_aliases.find(name);
	if(i == m_name_id_mapping_with_aliases.end())
		return false;
	result = i->second;
	return true;
}

content_t NodeDefManager::getId(const std::string &name) const
{
	content_t id = CONTENT_IGNORE;
	getId(name, id);
	return id;
}

bool NodeDefManager::getIds(const std::string &name, std::vector<content_t> &result) const
{
	//TimeTaker t("getIds", NULL, PRECISION_MICRO);
	if (name.substr(0,6) != "group:") {
		content_t id = CONTENT_IGNORE;
		bool exists = getId(name, id);
		if (exists)
			result.push_back(id);
		return exists;
	}
	std::string group = name.substr(6);

	std::unordered_map<std::string, std::vector<content_t>>::const_iterator
		i = m_group_to_items.find(group);
	if (i == m_group_to_items.end())
		return true;

	const std::vector<content_t> &items = i->second;
	result.insert(result.end(), items.begin(), items.end());
	//printf("getIds: %dus\n", t.stop());
	return true;
}

const ContentFeatures& NodeDefManager::get(const std::string &name) const
{
	content_t id = CONTENT_UNKNOWN;
	getId(name, id);
	return get(id);
}

// returns CONTENT_IGNORE if no free ID found
content_t NodeDefManager::allocateId()
{
	for (content_t id = m_next_id;
			id >= m_next_id; // overflow?
			++id) {
		while (id >= m_content_features.size()) {
			m_content_features.emplace_back();
		}
		const ContentFeatures &f = m_content_features[id];
		if (f.name.empty()) {
			m_next_id = id + 1;
			return id;
		}
	}
	// If we arrive here, an overflow occurred in id.
	// That means no ID was found
	return CONTENT_IGNORE;
}

/*!
 * Returns the smallest box that contains all boxes
 * in the vector. Box_union is expanded.
 * @param[in]      boxes     the vector containing the boxes
 * @param[in, out] box_union the union of the arguments
 */
void boxVectorUnion(const std::vector<aabb3f> &boxes, aabb3f *box_union)
{
	for (const aabb3f &box : boxes) {
		box_union->addInternalBox(box);
	}
}

/*!
 * Returns a box that contains the nodebox in every case.
 * The argument node_union is expanded.
 * @param[in]      nodebox  the nodebox to be measured
 * @param[in]      features  used to decide whether the nodebox
 * can be rotated
 * @param[in, out] box_union the union of the arguments
 */
void getNodeBoxUnion(const NodeBox &nodebox, const ContentFeatures &features,
	aabb3f *box_union)
{
	switch(nodebox.type) {
		case NODEBOX_FIXED:
		case NODEBOX_LEVELED: {
			// Raw union
			aabb3f half_processed(0, 0, 0, 0, 0, 0);
			boxVectorUnion(nodebox.fixed, &half_processed);
			// Set leveled boxes to maximal
			if (nodebox.type == NODEBOX_LEVELED) {
				half_processed.MaxEdge.Y = +BS / 2;
			}
			if (features.param_type_2 == CPT2_FACEDIR ||
					features.param_type_2 == CPT2_COLORED_FACEDIR) {
				// Get maximal coordinate
				f32 coords[] = {
					fabsf(half_processed.MinEdge.X),
					fabsf(half_processed.MinEdge.Y),
					fabsf(half_processed.MinEdge.Z),
					fabsf(half_processed.MaxEdge.X),
					fabsf(half_processed.MaxEdge.Y),
					fabsf(half_processed.MaxEdge.Z) };
				f32 max = 0;
				for (float coord : coords) {
					if (max < coord) {
						max = coord;
					}
				}
				// Add the union of all possible rotated boxes
				box_union->addInternalPoint(-max, -max, -max);
				box_union->addInternalPoint(+max, +max, +max);
			} else {
				box_union->addInternalBox(half_processed);
			}
			break;
		}
		case NODEBOX_WALLMOUNTED: {
			// Add fix boxes
			box_union->addInternalBox(nodebox.wall_top);
			box_union->addInternalBox(nodebox.wall_bottom);
			// Find maximal coordinate in the X-Z plane
			f32 coords[] = {
				fabsf(nodebox.wall_side.MinEdge.X),
				fabsf(nodebox.wall_side.MinEdge.Z),
				fabsf(nodebox.wall_side.MaxEdge.X),
				fabsf(nodebox.wall_side.MaxEdge.Z) };
			f32 max = 0;
			for (float coord : coords) {
				if (max < coord) {
					max = coord;
				}
			}
			// Add the union of all possible rotated boxes
			box_union->addInternalPoint(-max, nodebox.wall_side.MinEdge.Y, -max);
			box_union->addInternalPoint(max, nodebox.wall_side.MaxEdge.Y, max);
			break;
		}
		case NODEBOX_CONNECTED: {
			// Add all possible connected boxes
			boxVectorUnion(nodebox.fixed,               box_union);
			boxVectorUnion(nodebox.connect_top,         box_union);
			boxVectorUnion(nodebox.connect_bottom,      box_union);
			boxVectorUnion(nodebox.connect_front,       box_union);
			boxVectorUnion(nodebox.connect_left,        box_union);
			boxVectorUnion(nodebox.connect_back,        box_union);
			boxVectorUnion(nodebox.connect_right,       box_union);
			boxVectorUnion(nodebox.disconnected_top,    box_union);
			boxVectorUnion(nodebox.disconnected_bottom, box_union);
			boxVectorUnion(nodebox.disconnected_front,  box_union);
			boxVectorUnion(nodebox.disconnected_left,   box_union);
			boxVectorUnion(nodebox.disconnected_back,   box_union);
			boxVectorUnion(nodebox.disconnected_right,  box_union);
			boxVectorUnion(nodebox.disconnected,        box_union);
			boxVectorUnion(nodebox.disconnected_sides,  box_union);
			break;
		}
		default: {
			// NODEBOX_REGULAR
			box_union->addInternalPoint(-BS / 2, -BS / 2, -BS / 2);
			box_union->addInternalPoint(+BS / 2, +BS / 2, +BS / 2);
		}
	}
}

inline void NodeDefManager::fixSelectionBoxIntUnion()
{
	m_selection_box_int_union.MinEdge.X = floorf(
		m_selection_box_union.MinEdge.X / BS + 0.5f);
	m_selection_box_int_union.MinEdge.Y = floorf(
		m_selection_box_union.MinEdge.Y / BS + 0.5f);
	m_selection_box_int_union.MinEdge.Z = floorf(
		m_selection_box_union.MinEdge.Z / BS + 0.5f);
	m_selection_box_int_union.MaxEdge.X = ceilf(
		m_selection_box_union.MaxEdge.X / BS - 0.5f);
	m_selection_box_int_union.MaxEdge.Y = ceilf(
		m_selection_box_union.MaxEdge.Y / BS - 0.5f);
	m_selection_box_int_union.MaxEdge.Z = ceilf(
		m_selection_box_union.MaxEdge.Z / BS - 0.5f);
}

// IWritableNodeDefManager
content_t NodeDefManager::set(const std::string &name, const ContentFeatures &def)
{
	// Pre-conditions
	assert(name != "");
	assert(name != "ignore");
	assert(name == def.name);

	content_t id = CONTENT_IGNORE;
	if (!m_name_id_mapping.getId(name, id)) { // ignore aliases
		// Get new id
		id = allocateId();
		if (id == CONTENT_IGNORE) {
			warningstream << "NodeDefManager: Absolute "
				"limit reached" << std::endl;
			return CONTENT_IGNORE;
		}
		assert(id != CONTENT_IGNORE);
		addNameIdMapping(id, name);
	}
	m_content_features[id] = def;
	verbosestream << "NodeDefManager: registering content id \"" << id
		<< "\": name=\"" << def.name << "\""<<std::endl;

	getNodeBoxUnion(def.selection_box, def, &m_selection_box_union);
	fixSelectionBoxIntUnion();
	// Add this content to the list of all groups it belongs to
	// FIXME: This should remove a node from groups it no longer
	// belongs to when a node is re-registered
	for (const auto &group : def.groups) {
		const std::string &group_name = group.first;
		m_group_to_items[group_name].push_back(id);
	}
	return id;
}

content_t NodeDefManager::allocateDummy(const std::string &name)
{
	assert(name != "");	// Pre-condition
	ContentFeatures f;
	f.name = name;
	return set(name, f);
}

void NodeDefManager::removeNode(const std::string &name)
{
	// Pre-condition
	assert(name != "");

	// Erase name from name ID mapping
	content_t id = CONTENT_IGNORE;
	if (m_name_id_mapping.getId(name, id)) {
		m_name_id_mapping.eraseName(name);
		m_name_id_mapping_with_aliases.erase(name);
	}

	// Erase node content from all groups it belongs to
	for (std::unordered_map<std::string, std::vector<content_t>>::iterator iter_groups =
			m_group_to_items.begin(); iter_groups != m_group_to_items.end();) {
		std::vector<content_t> &items = iter_groups->second;
		items.erase(std::remove(items.begin(), items.end(), id), items.end());

		// Check if group is empty
		if (items.empty())
			m_group_to_items.erase(iter_groups++);
		else
			++iter_groups;
	}
}

void NodeDefManager::updateAliases(IItemDefManager *idef)
{
	std::set<std::string> all;
	idef->getAll(all);
	m_name_id_mapping_with_aliases.clear();
	for (const std::string &name : all) {
		const std::string &convert_to = idef->getAlias(name);
		content_t id;
		if (m_name_id_mapping.getId(convert_to, id)) {
			m_name_id_mapping_with_aliases.insert(
				std::make_pair(name, id));
		}
	}
}

void NodeDefManager::applyTextureOverrides(const std::string &override_filepath)
{
	infostream << "NodeDefManager::applyTextureOverrides(): Applying "
		"overrides to textures from " << override_filepath << std::endl;

	std::ifstream infile(override_filepath.c_str());
	std::string line;
	int line_c = 0;
	while (std::getline(infile, line)) {
		line_c++;
		if (trim(line).empty())
			continue;
		std::vector<std::string> splitted = str_split(line, ' ');
		if (splitted.size() != 3) {
			errorstream << override_filepath
				<< ":" << line_c << " Could not apply texture override \""
				<< line << "\": Syntax error" << std::endl;
			continue;
		}

		content_t id;
		if (!getId(splitted[0], id))
			continue; // Ignore unknown node

		ContentFeatures &nodedef = m_content_features[id];

		if (splitted[1] == "top")
			nodedef.tiledef[0].name = splitted[2];
		else if (splitted[1] == "bottom")
			nodedef.tiledef[1].name = splitted[2];
		else if (splitted[1] == "right")
			nodedef.tiledef[2].name = splitted[2];
		else if (splitted[1] == "left")
			nodedef.tiledef[3].name = splitted[2];
		else if (splitted[1] == "back")
			nodedef.tiledef[4].name = splitted[2];
		else if (splitted[1] == "front")
			nodedef.tiledef[5].name = splitted[2];
		else if (splitted[1] == "all" || splitted[1] == "*")
			for (TileDef &i : nodedef.tiledef)
				i.name = splitted[2];
		else if (splitted[1] == "sides")
			for (int i = 2; i < 6; i++)
				nodedef.tiledef[i].name = splitted[2];
		else {
			errorstream << override_filepath
				<< ":" << line_c << " Could not apply texture override \""
				<< line << "\": Unknown node side \""
				<< splitted[1] << "\"" << std::endl;
			continue;
		}
	}
}

void NodeDefManager::updateTextures(IGameDef *gamedef,
	void (*progress_callback)(void *progress_args, u32 progress, u32 max_progress),
	void *progress_callback_args)
{
#ifndef SERVER
	infostream << "NodeDefManager::updateTextures(): Updating "
		"textures in node definitions" << std::endl;

	Client *client = (Client *)gamedef;
	ITextureSource *tsrc = client->tsrc();
	IShaderSource *shdsrc = client->getShaderSource();
	scene::IMeshManipulator *meshmanip =
		RenderingEngine::get_scene_manager()->getMeshManipulator();
	TextureSettings tsettings;
	tsettings.readSettings();

	u32 size = m_content_features.size();

	for (u32 i = 0; i < size; i++) {
		ContentFeatures *f = &(m_content_features[i]);
		f->updateTextures(tsrc, shdsrc, meshmanip, client, tsettings);
		progress_callback(progress_callback_args, i, size);
	}
#endif
}

void NodeDefManager::serialize(std::ostream &os, u16 protocol_version) const
{
	writeU8(os, 1); // version
	u16 count = 0;
	std::ostringstream os2(std::ios::binary);
	for (u32 i = 0; i < m_content_features.size(); i++) {
		if (i == CONTENT_IGNORE || i == CONTENT_AIR
				|| i == CONTENT_UNKNOWN)
			continue;
		const ContentFeatures *f = &m_content_features[i];
		if (f->name.empty())
			continue;
		writeU16(os2, i);
		// Wrap it in a string to allow different lengths without
		// strict version incompatibilities
		std::ostringstream wrapper_os(std::ios::binary);
		f->serialize(wrapper_os, protocol_version);
		os2<<serializeString(wrapper_os.str());

		// must not overflow
		u16 next = count + 1;
		FATAL_ERROR_IF(next < count, "Overflow");
		count++;
	}
	writeU16(os, count);
	os << serializeLongString(os2.str());
}


void NodeDefManager::deSerialize(std::istream &is)
{
	clear();
	int version = readU8(is);
	if (version != 1)
		throw SerializationError("unsupported NodeDefinitionManager version");
	u16 count = readU16(is);
	std::istringstream is2(deSerializeLongString(is), std::ios::binary);
	ContentFeatures f;
	for (u16 n = 0; n < count; n++) {
		u16 i = readU16(is2);

		// Read it from the string wrapper
		std::string wrapper = deSerializeString(is2);
		std::istringstream wrapper_is(wrapper, std::ios::binary);
		f.deSerialize(wrapper_is);

		// Check error conditions
		if (i == CONTENT_IGNORE || i == CONTENT_AIR || i == CONTENT_UNKNOWN) {
			warningstream << "NodeDefManager::deSerialize(): "
				"not changing builtin node " << i << std::endl;
			continue;
		}
		if (f.name.empty()) {
			warningstream << "NodeDefManager::deSerialize(): "
				"received empty name" << std::endl;
			continue;
		}

		// Ignore aliases
		u16 existing_id;
		if (m_name_id_mapping.getId(f.name, existing_id) && i != existing_id) {
			warningstream << "NodeDefManager::deSerialize(): "
				"already defined with different ID: " << f.name << std::endl;
			continue;
		}

		// All is ok, add node definition with the requested ID
		if (i >= m_content_features.size())
			m_content_features.resize((u32)(i) + 1);
		m_content_features[i] = f;
		addNameIdMapping(i, f.name);
		verbosestream << "deserialized " << f.name << std::endl;

		getNodeBoxUnion(f.selection_box, f, &m_selection_box_union);
		fixSelectionBoxIntUnion();
	}
}


void NodeDefManager::addNameIdMapping(content_t i, std::string name)
{
	m_name_id_mapping.set(i, name);
	m_name_id_mapping_with_aliases.insert(std::make_pair(name, i));
}


NodeDefManager *createNodeDefManager()
{
	return new NodeDefManager();
}


void NodeDefManager::pendNodeResolve(NodeResolver *nr) const
{
	nr->m_ndef = this;
	if (m_node_registration_complete)
		nr->nodeResolveInternal();
	else
		m_pending_resolve_callbacks.push_back(nr);
}


bool NodeDefManager::cancelNodeResolveCallback(NodeResolver *nr) const
{
	size_t len = m_pending_resolve_callbacks.size();
	for (size_t i = 0; i != len; i++) {
		if (nr != m_pending_resolve_callbacks[i])
			continue;

		len--;
		m_pending_resolve_callbacks[i] = m_pending_resolve_callbacks[len];
		m_pending_resolve_callbacks.resize(len);
		return true;
	}

	return false;
}


void NodeDefManager::runNodeResolveCallbacks()
{
	for (size_t i = 0; i != m_pending_resolve_callbacks.size(); i++) {
		NodeResolver *nr = m_pending_resolve_callbacks[i];
		nr->nodeResolveInternal();
	}

	m_pending_resolve_callbacks.clear();
}


void NodeDefManager::resetNodeResolveState()
{
	m_node_registration_complete = false;
	m_pending_resolve_callbacks.clear();
}

void NodeDefManager::mapNodeboxConnections()
{
	for (ContentFeatures &f : m_content_features) {
		if (f.drawtype != NDT_NODEBOX || f.node_box.type != NODEBOX_CONNECTED)
			continue;

		for (const std::string &name : f.connects_to) {
			getIds(name, f.connects_to_ids);
		}
	}
}

bool NodeDefManager::nodeboxConnects(MapNode from, MapNode to,
	u8 connect_face) const
{
	const ContentFeatures &f1 = get(from);

	if ((f1.drawtype != NDT_NODEBOX) || (f1.node_box.type != NODEBOX_CONNECTED))
		return false;

	// lookup target in connected set
	if (!CONTAINS(f1.connects_to_ids, to.param0))
		return false;

	const ContentFeatures &f2 = get(to);

	if ((f2.drawtype == NDT_NODEBOX) && (f2.node_box.type == NODEBOX_CONNECTED))
		// ignores actually looking if back connection exists
		return CONTAINS(f2.connects_to_ids, from.param0);

	// does to node declare usable faces?
	if (f2.connect_sides > 0) {
		if ((f2.param_type_2 == CPT2_FACEDIR ||
				f2.param_type_2 == CPT2_COLORED_FACEDIR)
				&& (connect_face >= 4)) {
			static const u8 rot[33 * 4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 4, 32, 16, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, // 4 - back
				8, 4, 32, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, // 8 - right
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 8, 4, 32, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, // 16 - front
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 32, 16, 8, 4 // 32 - left
				};
			return (f2.connect_sides
				& rot[(connect_face * 4) + (to.param2 & 0x1F)]);
		}
		return (f2.connect_sides & connect_face);
	}
	// the target is just a regular node, so connect no matter back connection
	return true;
}

