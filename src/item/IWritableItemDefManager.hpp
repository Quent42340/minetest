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
#ifndef IWRITABLEITEMDEFMANAGER_HPP_
#define IWRITABLEITEMDEFMANAGER_HPP_

#include "item/IItemDefManager.hpp"

class IGameDef;

class IWritableItemDefManager : public IItemDefManager {
	public:
		IWritableItemDefManager() = default;

		virtual ~IWritableItemDefManager() = default;

		// Get item definition
		virtual const ItemDefinition& get(const std::string &name) const = 0;

		// Get alias definition
		virtual const std::string &getAlias(const std::string &name) const = 0;

		// Get set of all defined item names and aliases
		virtual void getAll(std::set<std::string> &result) const = 0;

		// Check if item is known
		virtual bool isKnown(const std::string &name) const = 0;

#ifndef SERVER
		// Get item inventory texture
		virtual video::ITexture* getInventoryTexture(const std::string &name, Client *client) const = 0;

		// Get item wield mesh
		virtual ItemMesh* getWieldMesh(const std::string &name, Client *client) const = 0;
#endif

		// Remove all registered item and node definitions and aliases
		// Then re-add the builtin item definitions
		virtual void clear() = 0;

		// Register item definition
		virtual void registerItem(const ItemDefinition &def) = 0;
		virtual void unregisterItem(const std::string &name) = 0;

		// Set an alias so that items named <name> will load as <convert_to>.
		// Alias is not set if <name> has already been defined.
		// Alias will be removed if <name> is defined at a later point of time.
		virtual void registerAlias(const std::string &name, const std::string &convert_to) = 0;

		virtual void serialize(std::ostream &os, u16 protocol_version) = 0;
		virtual void deSerialize(std::istream &is) = 0;

		// Do stuff asked by threads that can only be done in the main thread
		virtual void processQueue(IGameDef *gamedef) = 0;
};

#endif // IWRITABLEITEMDEFMANAGER_HPP_
