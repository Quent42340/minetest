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
#ifndef CCRAFTDEFMANAGER_HPP_
#define CCRAFTDEFMANAGER_HPP_

#include "ICraftDefManager.hpp"

/*
	Craft definition manager
*/

class CCraftDefManager: public IWritableCraftDefManager {
	public:
		CCraftDefManager();
		virtual ~CCraftDefManager();

		bool getCraftResult(CraftInput &input, CraftOutput &output,
				std::vector<ItemStack> &output_replacement, bool decrementInput,
				IGameDef *gamedef) const override;

		std::vector<ICraftDefinition*> getCraftRecipes(CraftOutput &output,
				IGameDef *gamedef, unsigned limit=0) const override;

		bool clearCraftRecipesByOutput(const CraftOutput &output, IGameDef *gamedef) override;

		bool clearCraftRecipesByInput(CraftMethod craft_method, unsigned int craft_grid_width,
				const std::vector<std::string> &recipe, IGameDef *gamedef) override;

		std::string dump() const override;

		void registerCraft(ICraftDefinition *def, IGameDef *gamedef) override;
		void clear() override;
		void initHashes(IGameDef *gamedef) override;

	private:
		// FIXME: Test this
		// TODO: change both maps to unordered_map when c++11 can be used
		std::vector<std::map<u64, std::vector<ICraftDefinition*> > > m_craft_defs;
		std::map<std::string, std::vector<ICraftDefinition*> > m_output_craft_definitions;
};

inline IWritableCraftDefManager* createCraftDefManager()
{
	return new CCraftDefManager();
}

#endif // CCRAFTDEFMANAGER_HPP_
