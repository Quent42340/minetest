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
#ifndef ICRAFTDEFINITION_HPP_
#define ICRAFTDEFINITION_HPP_

#include <string>
#include "common/craft/CraftUtils.hpp"

/*
	Crafting definition base class
*/
class ICraftDefinition {
	public:
		ICraftDefinition() = default;
		virtual ~ICraftDefinition() = default;

		// Returns type of crafting definition
		virtual std::string getName() const = 0;

		// Checks whether the recipe is applicable
		virtual bool check(const CraftInput &input, IGameDef *gamedef) const = 0;

		// Returns the output structure, meaning depends on crafting method
		// The implementation can assume that check(input) returns true
		virtual CraftOutput getOutput(const CraftInput &input, IGameDef *gamedef) const = 0;

		// the inverse of the above
		virtual CraftInput getInput(const CraftOutput &output, IGameDef *gamedef) const = 0;

		// Decreases count of every input item
		virtual void decrementInput(CraftInput &input,
				std::vector<ItemStack> &output_replacements, IGameDef *gamedef) const = 0;

		virtual CraftHashType getHashType() const = 0;
		virtual u64 getHash(CraftHashType type) const = 0;

		// to be called after all mods are loaded, so that we catch all aliases
		virtual void initHash(IGameDef *gamedef) = 0;

		virtual std::string dump() const = 0;
};

#endif // ICRAFTDEFINITION_HPP_
