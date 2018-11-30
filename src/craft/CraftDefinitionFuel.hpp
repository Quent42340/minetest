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
#ifndef CRAFTDEFINITIONFUEL_HPP_
#define CRAFTDEFINITIONFUEL_HPP_

#include "craft/ICraftDefinition.hpp"

/*
	A fuel (for furnace) definition
	Supported crafting method: CRAFT_METHOD_FUEL.
*/
class CraftDefinitionFuel: public ICraftDefinition {
	public:
		CraftDefinitionFuel() = delete;
		CraftDefinitionFuel(const std::string &recipe_,
		                    float burntime_,
		                    const CraftReplacements &replacements_):
			recipe(recipe_),
			burntime(burntime_),
			replacements(replacements_)
		{}

		virtual ~CraftDefinitionFuel() = default;

		std::string getName() const override { return "fuel"; }

		bool check(const CraftInput &input, IGameDef *gamedef) const override;
		CraftOutput getOutput(const CraftInput &input, IGameDef *gamedef) const override;
		CraftInput getInput(const CraftOutput &output, IGameDef *gamedef) const override;
		void decrementInput(CraftInput &input,
				std::vector<ItemStack> &output_replacements, IGameDef *gamedef) const override;

		CraftHashType getHashType() const override;
		u64 getHash(CraftHashType type) const override;

		void initHash(IGameDef *gamedef) override;

		std::string dump() const override;

	private:
		// Recipe itemstring
		std::string recipe;

		// Recipe item name
		std::string recipe_name;

		// bool indicating if initHash has been called already
		bool hash_inited = false;

		// Time in seconds
		float burntime;

		// Replacement items for decrementInput()
		CraftReplacements replacements;
};

#endif // CRAFTDEFINITIONFUEL_HPP_
