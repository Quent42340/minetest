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
#ifndef CITEMDEFMANAGER_HPP_
#define CITEMDEFMANAGER_HPP_

#include <map>
#include <thread>

#include "client/wieldmesh.h"
#include "item/IWritableItemDefManager.hpp"
#include "util/string.h"
#include "util/thread.h"

// SUGG: Support chains of aliases?
class CItemDefManager: public IWritableItemDefManager {
#ifndef SERVER
	struct ClientCached
	{
		video::ITexture *inventory_texture = nullptr;
		ItemMesh wield_mesh;
		Palette *palette = nullptr;
	};
#endif

	public:
		CItemDefManager();
		virtual ~CItemDefManager();

		const ItemDefinition& get(const std::string &name_) const override;
		const std::string &getAlias(const std::string &name) const override;
		void getAll(std::set<std::string> &result) const override;

		bool isKnown(const std::string &name_) const override;

#ifndef SERVER
	public:
		ClientCached* createClientCachedDirect(const std::string &name, Client *client) const;
		ClientCached* getClientCached(const std::string &name, Client *client) const;

		// Get item inventory texture
		video::ITexture* getInventoryTexture(const std::string &name, Client *client) const override;

		// Get item wield mesh
		ItemMesh* getWieldMesh(const std::string &name, Client *client) const override;

		// Get item palette
		Palette* getPalette(const std::string &name, Client *client) const override;

		video::SColor getItemstackColor(const ItemStack &stack, Client *client) const override;
#endif

		void clear() override;

		void registerItem(const ItemDefinition &def) override;
		void unregisterItem(const std::string &name) override;

		void registerAlias(const std::string &name, const std::string &convert_to) override;

		void serialize(std::ostream &os, u16 protocol_version) override;
		void deSerialize(std::istream &is) override;

		void processQueue(IGameDef *gamedef) override;

	private:
		// Key is name
		std::map<std::string, ItemDefinition*> m_item_definitions;

		// Aliases
		StringMap m_aliases;

#ifndef SERVER
		// The id of the thread that is allowed to use irrlicht directly
		std::thread::id m_main_thread;

		// A reference to this can be returned when nothing is found, to avoid NULLs
		mutable ClientCached m_dummy_clientcached;

		// Cached textures and meshes
		mutable MutexedMap<std::string, ClientCached*> m_clientcached;

		// Queued clientcached fetches (to be processed by the main thread)
		mutable RequestQueue<std::string, ClientCached*, u8, u8> m_get_clientcached_queue;
#endif
};

IWritableItemDefManager* createItemDefManager();

#endif // CITEMDEFMANAGER_HPP_
