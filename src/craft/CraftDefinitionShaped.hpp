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
#ifndef CRAFTDEFINITIONSHAPED_HPP_
#define CRAFTDEFINITIONSHAPED_HPP_

#include "craft/ICraftDefinition.hpp"

/*
	A plain-jane (shaped) crafting definition

	Supported crafting method: CRAFT_METHOD_NORMAL.
	Requires the input items to be arranged exactly like in the recipe.
*/
class CraftDefinitionShaped: public ICraftDefinition {
	public:
		CraftDefinitionShaped() = delete;
		CraftDefinitionShaped(const std::string &output_,
		                      unsigned int width_,
				              const std::vector<std::string> &recipe_,
				              const CraftReplacements &replacements_):
			output(output_),
			width(width_),
			recipe(recipe_),
			replacements(replacements_)
		{}

		virtual ~CraftDefinitionShaped() = default;

		std::string getName() const override { return "shaped"; }

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
		// Output itemstring
		std::string output = "";

		// Width of recipe
		unsigned int width = 1;

		// Recipe matrix (itemstrings)
		std::vector<std::string> recipe;

		// Recipe matrix (item names)
		std::vector<std::string> recipe_names;

		// bool indicating if initHash has been called already
		bool hash_inited = false;

		// Replacement items for decrementInput()
		CraftReplacements replacements;
};

#endif // CRAFTDEFINITIONSHAPED_HPP_
