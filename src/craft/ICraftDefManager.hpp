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
#ifndef ICRAFTDEFMANAGER_HPP_
#define ICRAFTDEFMANAGER_HPP_

#include "gamedef.h"
#include "ICraftDefinition.hpp"

/*
	Crafting definition manager
*/
class ICraftDefManager {
	public:
		ICraftDefManager() = default;
		virtual ~ICraftDefManager() = default;

		// The main crafting function
		virtual bool getCraftResult(CraftInput             &input,
		                            CraftOutput            &output,
		                            std::vector<ItemStack> &output_replacements,
		                            bool                   decrementInput,
		                            IGameDef               *gamedef) const = 0;

		virtual std::vector<ICraftDefinition*> getCraftRecipes(CraftOutput &output,
		                                                       IGameDef *gamedef,
		                                                       unsigned limit = 0) const = 0;

		// Print crafting recipes for debugging
		virtual std::string dump() const = 0;
};

class IWritableCraftDefManager : public ICraftDefManager {
	public:
		IWritableCraftDefManager() = default;
		virtual ~IWritableCraftDefManager() = default;

		virtual bool clearCraftRecipesByOutput(const CraftOutput &output, IGameDef *gamedef) = 0;

		virtual bool clearCraftRecipesByInput(CraftMethod craft_method,
		                                      unsigned int craft_grid_width,
		                                      const std::vector<std::string> &recipe,
		                                      IGameDef *gamedef) = 0;

		// Add a crafting definition.
		// After calling this, the pointer belongs to the manager.
		virtual void registerCraft(ICraftDefinition *def, IGameDef *gamedef) = 0;

		// Delete all crafting definitions
		virtual void clear() = 0;

		// To be called after all mods are loaded, so that we catch all aliases
		virtual void initHashes(IGameDef *gamedef) = 0;
};

#endif // ICRAFTDEFMANAGER_HPP_
