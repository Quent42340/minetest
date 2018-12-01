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

#include "common/item/CItemDefManager.hpp"
#include "common/inventory/tool.h"
#include "util/serialize.h"

#ifndef SERVER
#include "client/client.h"
#endif

CItemDefManager::CItemDefManager()
{
#ifndef SERVER
		m_main_thread = std::this_thread::get_id();
#endif
		clear();
}

CItemDefManager::~CItemDefManager()
{
#ifndef SERVER
	const std::vector<ClientCached*> &values = m_clientcached.getValues();
	for (ClientCached *cc : values) {
		if (cc->wield_mesh.mesh)
			cc->wield_mesh.mesh->drop();
		delete cc;
	}

#endif
	for (auto &item_definition : m_item_definitions) {
		delete item_definition.second;
	}

	m_item_definitions.clear();
}

const ItemDefinition& CItemDefManager::get(const std::string &name_) const
{
	// Convert name according to possible alias
	std::string name = getAlias(name_);
	// Get the definition
	std::map<std::string, ItemDefinition*>::const_iterator i;
	i = m_item_definitions.find(name);
	if(i == m_item_definitions.end())
		i = m_item_definitions.find("unknown");
	assert(i != m_item_definitions.end());
	return *(i->second);
}

const std::string &CItemDefManager::getAlias(const std::string &name) const
{
	StringMap::const_iterator it = m_aliases.find(name);
	if (it != m_aliases.end())
		return it->second;
	return name;
}

void CItemDefManager::getAll(std::set<std::string> &result) const
{
	result.clear();
	for (const auto &item_definition : m_item_definitions) {
		result.insert(item_definition.first);
	}

	for (const auto &alias : m_aliases) {
		result.insert(alias.first);
	}
}

bool CItemDefManager::isKnown(const std::string &name_) const
{
	// Convert name according to possible alias
	std::string name = getAlias(name_);
	// Get the definition
	std::map<std::string, ItemDefinition*>::const_iterator i;
	return m_item_definitions.find(name) != m_item_definitions.end();
}

#ifndef SERVER
CItemDefManager::ClientCached* CItemDefManager::createClientCachedDirect(const std::string &name, Client *client) const
{
	infostream<<"Lazily creating item texture and mesh for \""
		<<name<<"\""<<std::endl;

	// This is not thread-safe
	sanity_check(std::this_thread::get_id() == m_main_thread);

	// Skip if already in cache
	ClientCached *cc = NULL;
	m_clientcached.get(name, &cc);
	if(cc)
		return cc;

	ITextureSource *tsrc = client->getTextureSource();
	const ItemDefinition &def = get(name);

	// Create new ClientCached
	cc = new ClientCached();

	// Create an inventory texture
	cc->inventory_texture = NULL;
	if (!def.inventory_image.empty())
		cc->inventory_texture = tsrc->getTexture(def.inventory_image);

	ItemStack item = ItemStack();
	item.name = def.name;

	getItemMesh(client, item, &(cc->wield_mesh));

	cc->palette = tsrc->getPalette(def.palette_image);

	// Put in cache
	m_clientcached.set(name, cc);

	return cc;
}

CItemDefManager::ClientCached* CItemDefManager::getClientCached(const std::string &name, Client *client) const
{
	ClientCached *cc = NULL;
	m_clientcached.get(name, &cc);
	if (cc)
		return cc;

	if (std::this_thread::get_id() == m_main_thread) {
		return createClientCachedDirect(name, client);
	}

	// We're gonna ask the result to be put into here
	static ResultQueue<std::string, ClientCached*, u8, u8> result_queue;

	// Throw a request in
	m_get_clientcached_queue.add(name, 0, 0, &result_queue);
	try {
		while(true) {
			// Wait result for a second
			GetResult<std::string, ClientCached*, u8, u8>
				result = result_queue.pop_front(1000);

			if (result.key == name) {
				return result.item;
			}
		}
	} catch(ItemNotFoundException &e) {
		errorstream << "Waiting for clientcached " << name
			<< " timed out." << std::endl;
		return &m_dummy_clientcached;
	}
}

// Get item inventory texture
video::ITexture* CItemDefManager::getInventoryTexture(const std::string &name, Client *client) const
{
	ClientCached *cc = getClientCached(name, client);
	if(!cc)
		return NULL;
	return cc->inventory_texture;
}

// Get item wield mesh
ItemMesh* CItemDefManager::getWieldMesh(const std::string &name, Client *client) const
{
	ClientCached *cc = getClientCached(name, client);
	if(!cc)
		return NULL;
	return &(cc->wield_mesh);
}

// Get item palette
Palette* CItemDefManager::getPalette(const std::string &name, Client *client) const
{
	ClientCached *cc = getClientCached(name, client);
	if(!cc)
		return NULL;
	return cc->palette;
}


video::SColor CItemDefManager::getItemstackColor(const ItemStack &stack, Client *client) const
{
	// Look for direct color definition
	const std::string &colorstring = stack.metadata.getString("color", 0);
	video::SColor directcolor;
	if (!colorstring.empty() && parseColorString(colorstring, directcolor, true))
		return directcolor;
	// See if there is a palette
	Palette *palette = getPalette(stack.name, client);
	const std::string &index = stack.metadata.getString("palette_index", 0);
	if (palette && !index.empty())
		return (*palette)[mystoi(index, 0, 255)];
	// Fallback color
	return get(stack.name).color;
}
#endif

void CItemDefManager::clear()
{
	for(std::map<std::string, ItemDefinition*>::const_iterator
			i = m_item_definitions.begin();
			i != m_item_definitions.end(); ++i)
	{
		delete i->second;
	}
	m_item_definitions.clear();
	m_aliases.clear();

	// Add the four builtin items:
	//   "" is the hand
	//   "unknown" is returned whenever an undefined item
	//     is accessed (is also the unknown node)
	//   "air" is the air node
	//   "ignore" is the ignore node

	ItemDefinition* hand_def = new ItemDefinition;
	hand_def->name = "";
	hand_def->wield_image = "wieldhand.png";
	hand_def->tool_capabilities = new ToolCapabilities;
	m_item_definitions.insert(std::make_pair("", hand_def));

	ItemDefinition* unknown_def = new ItemDefinition;
	unknown_def->type = ITEM_NODE;
	unknown_def->name = "unknown";
	m_item_definitions.insert(std::make_pair("unknown", unknown_def));

	ItemDefinition* air_def = new ItemDefinition;
	air_def->type = ITEM_NODE;
	air_def->name = "air";
	m_item_definitions.insert(std::make_pair("air", air_def));

	ItemDefinition* ignore_def = new ItemDefinition;
	ignore_def->type = ITEM_NODE;
	ignore_def->name = "ignore";
	m_item_definitions.insert(std::make_pair("ignore", ignore_def));
}

void CItemDefManager::registerItem(const ItemDefinition &def)
{
	verbosestream<<"ItemDefManager: registering \""<<def.name<<"\""<<std::endl;
	// Ensure that the "" item (the hand) always has ToolCapabilities
	if (def.name.empty())
		FATAL_ERROR_IF(!def.tool_capabilities, "Hand does not have ToolCapabilities");

	if(m_item_definitions.count(def.name) == 0)
		m_item_definitions[def.name] = new ItemDefinition(def);
	else
		*(m_item_definitions[def.name]) = def;

	// Remove conflicting alias if it exists
	bool alias_removed = (m_aliases.erase(def.name) != 0);
	if(alias_removed)
		infostream<<"ItemDefManager: erased alias "<<def.name
			<<" because item was defined"<<std::endl;
}

void CItemDefManager::unregisterItem(const std::string &name)
{
	verbosestream<<"ItemDefManager: unregistering \""<<name<<"\""<<std::endl;

	delete m_item_definitions[name];
	m_item_definitions.erase(name);
}

void CItemDefManager::registerAlias(const std::string &name, const std::string &convert_to)
{
	if (m_item_definitions.find(name) == m_item_definitions.end()) {
		verbosestream<<"ItemDefManager: setting alias "<<name
			<<" -> "<<convert_to<<std::endl;
		m_aliases[name] = convert_to;
	}
}

void CItemDefManager::serialize(std::ostream &os, u16 protocol_version)
{
	writeU8(os, 0); // version
	u16 count = m_item_definitions.size();
	writeU16(os, count);

	for (std::map<std::string, ItemDefinition *>::const_iterator
			it = m_item_definitions.begin();
			it != m_item_definitions.end(); ++it) {
		ItemDefinition *def = it->second;
		// Serialize ItemDefinition and write wrapped in a string
		std::ostringstream tmp_os(std::ios::binary);
		def->serialize(tmp_os, protocol_version);
		os << serializeString(tmp_os.str());
	}

	writeU16(os, m_aliases.size());

	for (StringMap::const_iterator
			it = m_aliases.begin();
			it != m_aliases.end(); ++it) {
		os << serializeString(it->first);
		os << serializeString(it->second);
	}
}

void CItemDefManager::deSerialize(std::istream &is)
{
	// Clear everything
	clear();
	// Deserialize
	int version = readU8(is);
	if(version != 0)
		throw SerializationError("unsupported ItemDefManager version");
	u16 count = readU16(is);
	for(u16 i=0; i<count; i++)
	{
		// Deserialize a string and grab an ItemDefinition from it
		std::istringstream tmp_is(deSerializeString(is), std::ios::binary);
		ItemDefinition def;
		def.deSerialize(tmp_is);
		// Register
		registerItem(def);
	}
	u16 num_aliases = readU16(is);
	for(u16 i=0; i<num_aliases; i++)
	{
		std::string name = deSerializeString(is);
		std::string convert_to = deSerializeString(is);
		registerAlias(name, convert_to);
	}
}

void CItemDefManager::processQueue(IGameDef *gamedef)
{
#ifndef SERVER
	//NOTE this is only thread safe for ONE consumer thread!
	while(!m_get_clientcached_queue.empty())
	{
		GetRequest<std::string, ClientCached*, u8, u8>
			request = m_get_clientcached_queue.pop();

		m_get_clientcached_queue.pushResult(request,
				createClientCachedDirect(request.key, (Client *)gamedef));
	}
#endif
}

// FIXME: Use static or virtual function instead of this
IWritableItemDefManager* createItemDefManager()
{
	return new CItemDefManager();
}

