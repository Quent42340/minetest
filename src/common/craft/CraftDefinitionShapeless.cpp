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

#include <algorithm>
#include "common/craft/CraftDefinitionShapeless.hpp"
#include "server/game/gamedef.h"

bool CraftDefinitionShapeless::check(const CraftInput &input, IGameDef *gamedef) const
{
	if (input.method != CRAFT_METHOD_NORMAL)
		return false;

	// Filter empty items out of input
	std::vector<std::string> input_filtered;
	for (const auto &item : input.items) {
		if (!item.name.empty())
			input_filtered.push_back(item.name);
	}

	// If there is a wrong number of items in input, no match
	if (input_filtered.size() != recipe.size()) {
		/*dstream<<"Number of input items ("<<input_filtered.size()
				<<") does not match recipe size ("<<recipe.size()<<") "
				<<"of recipe with output="<<output<<std::endl;*/
		return false;
	}

	std::vector<std::string> recipe_copy;
	if (hash_inited)
		recipe_copy = recipe_names;
	else {
		recipe_copy = CraftUtils::craftGetItemNames(recipe, gamedef);
		std::sort(recipe_copy.begin(), recipe_copy.end());
	}

	// Try with all permutations of the recipe,
	// start from the lexicographically first permutation (=sorted),
	// recipe_names is pre-sorted
	do {
		// If all items match, the recipe matches
		bool all_match = true;
		//dstream<<"Testing recipe (output="<<output<<"):";
		for (size_t i=0; i<recipe.size(); i++) {
			//dstream<<" ("<<input_filtered[i]<<" == "<<recipe_copy[i]<<")";
			if (!CraftUtils::inputItemMatchesRecipe(input_filtered[i], recipe_copy[i], gamedef->idef())) {
				all_match = false;
				break;
			}
		}
		//dstream<<" -> match="<<all_match<<std::endl;
		if (all_match)
			return true;
	} while (std::next_permutation(recipe_copy.begin(), recipe_copy.end()));

	return false;
}

CraftOutput CraftDefinitionShapeless::getOutput(const CraftInput &input, IGameDef *gamedef) const
{
	return CraftOutput(output, 0);
}

CraftInput CraftDefinitionShapeless::getInput(const CraftOutput &output, IGameDef *gamedef) const
{
	return CraftInput(CRAFT_METHOD_NORMAL, 0, CraftUtils::craftGetItems(recipe, gamedef));
}

void CraftDefinitionShapeless::decrementInput(CraftInput &input, std::vector<ItemStack> &output_replacements,
	IGameDef *gamedef) const
{
	CraftUtils::craftDecrementOrReplaceInput(input, output_replacements, replacements, gamedef);
}

CraftHashType CraftDefinitionShapeless::getHashType() const
{
	assert(hash_inited); // Pre-condition
	bool has_group = false;
	for (const auto &recipe_name : recipe_names) {
		if (CraftUtils::isGroupRecipeStr(recipe_name)) {
			has_group = true;
			break;
		}
	}
	if (has_group)
		return CRAFT_HASH_TYPE_COUNT;

	return CRAFT_HASH_TYPE_ITEM_NAMES;
}

u64 CraftDefinitionShapeless::getHash(CraftHashType type) const
{
	assert(hash_inited); // Pre-condition
	assert(type == CRAFT_HASH_TYPE_ITEM_NAMES
		|| type == CRAFT_HASH_TYPE_COUNT); // Pre-condition
	return CraftUtils::getHashForGrid(type, recipe_names);
}

void CraftDefinitionShapeless::initHash(IGameDef *gamedef)
{
	if (hash_inited)
		return;
	hash_inited = true;
	recipe_names = CraftUtils::craftGetItemNames(recipe, gamedef);
	std::sort(recipe_names.begin(), recipe_names.end());
}

std::string CraftDefinitionShapeless::dump() const
{
	std::ostringstream os(std::ios::binary);
	os << "(shapeless, output=\"" << output
		<< "\", recipe=" << CraftUtils::craftDumpMatrix(recipe, recipe.size())
		<< ", replacements=" << replacements.dump() << ")";
	return os.str();
}

