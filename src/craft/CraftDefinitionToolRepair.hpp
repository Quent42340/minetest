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
#ifndef CRAFTDEFINITIONTOOLREPAIR_HPP_
#define CRAFTDEFINITIONTOOLREPAIR_HPP_

#include "craft/ICraftDefinition.hpp"

/*
	Tool repair crafting definition
	Supported crafting method: CRAFT_METHOD_NORMAL.
	Put two damaged tools into the crafting grid, get one tool back.
	There should only be one crafting definition of this type.
*/
class CraftDefinitionToolRepair: public ICraftDefinition {
	public:
		CraftDefinitionToolRepair() = delete;
		CraftDefinitionToolRepair(float additional_wear_) : additional_wear(additional_wear_) {}
		virtual ~CraftDefinitionToolRepair() = default;

		std::string getName() const override { return "toolrepair"; }

		bool check(const CraftInput &input, IGameDef *gamedef) const override;
		CraftOutput getOutput(const CraftInput &input, IGameDef *gamedef) const override;
		CraftInput getInput(const CraftOutput &output, IGameDef *gamedef) const override;
		void decrementInput(CraftInput &input,
				std::vector<ItemStack> &output_replacements, IGameDef *gamedef) const override;

		CraftHashType getHashType() const override { return CRAFT_HASH_TYPE_COUNT; }
		u64 getHash(CraftHashType type) const override { return 2; }

		void initHash(IGameDef *gamedef) override {}

		std::string dump() const override;

	private:
		// This is a constant that is added to the wear of the result.
		// May be positive or negative, allowed range [-1,1].
		// 1 = new tool is completely broken
		// 0 = simply add remaining uses of both input tools
		// -1 = new tool is completely pristine
		float additional_wear = 0.0f;
};

#endif // CRAFTDEFINITIONTOOLREPAIR_HPP_
