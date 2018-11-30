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

#include "craft/CraftDefinitionCooking.hpp"
#include "server/game/gamedef.h"

bool CraftDefinitionCooking::check(const CraftInput &input, IGameDef *gamedef) const
{
	if (input.method != CRAFT_METHOD_COOKING)
		return false;

	// Filter empty items out of input
	std::vector<std::string> input_filtered;
	for (const auto &item : input.items) {
		const std::string &name = item.name;
		if (!name.empty())
			input_filtered.push_back(name);
	}

	// If there is a wrong number of items in input, no match
	if (input_filtered.size() != 1) {
		/*dstream<<"Number of input items ("<<input_filtered.size()
				<<") does not match recipe size (1) "
				<<"of cooking recipe with output="<<output<<std::endl;*/
		return false;
	}

	// Check the single input item
	return CraftUtils::inputItemMatchesRecipe(input_filtered[0], recipe, gamedef->idef());
}

CraftOutput CraftDefinitionCooking::getOutput(const CraftInput &input, IGameDef *gamedef) const
{
	return CraftOutput(output, cooktime);
}

CraftInput CraftDefinitionCooking::getInput(const CraftOutput &output, IGameDef *gamedef) const
{
	std::vector<std::string> rec;
	rec.push_back(recipe);
	return CraftInput(CRAFT_METHOD_COOKING, cooktime, CraftUtils::craftGetItems(rec, gamedef));
}

void CraftDefinitionCooking::decrementInput(CraftInput &input, std::vector<ItemStack> &output_replacements,
	IGameDef *gamedef) const
{
	CraftUtils::craftDecrementOrReplaceInput(input, output_replacements, replacements, gamedef);
}

CraftHashType CraftDefinitionCooking::getHashType() const
{
	if (CraftUtils::isGroupRecipeStr(recipe_name))
		return CRAFT_HASH_TYPE_COUNT;

	return CRAFT_HASH_TYPE_ITEM_NAMES;
}

u64 CraftDefinitionCooking::getHash(CraftHashType type) const
{
	if (type == CRAFT_HASH_TYPE_ITEM_NAMES) {
		return CraftUtils::getHashForString(recipe_name);
	}

	if (type == CRAFT_HASH_TYPE_COUNT) {
		return 1;
	}

	// illegal hash type for this CraftDefinition (pre-condition)
	assert(false);
	return 0;
}

void CraftDefinitionCooking::initHash(IGameDef *gamedef)
{
	if (hash_inited)
		return;
	hash_inited = true;
	recipe_name = CraftUtils::craftGetItemName(recipe, gamedef);
}

std::string CraftDefinitionCooking::dump() const
{
	std::ostringstream os(std::ios::binary);
	os << "(cooking, output=\"" << output
		<< "\", recipe=\"" << recipe
		<< "\", cooktime=" << cooktime << ")"
		<< ", replacements=" << replacements.dump() << ")";
	return os.str();
}

