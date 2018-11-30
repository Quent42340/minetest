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
#ifndef IITEMDEFMANAGER_HPP_
#define IITEMDEFMANAGER_HPP_

#include "ItemDefinition.hpp"

class IItemDefManager {
	public:
		IItemDefManager() = default;
		virtual ~IItemDefManager() = default;

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

		// Get item palette
		virtual Palette* getPalette(const std::string &name, Client *client) const = 0;

		// Returns the base color of an item stack: the color of all
		// tiles that do not define their own color.
		virtual video::SColor getItemstackColor(const ItemStack &stack, Client *client) const = 0;
#endif

		virtual void serialize(std::ostream &os, u16 protocol_version)=0;
};

#endif // IITEMDEFMANAGER_HPP_
