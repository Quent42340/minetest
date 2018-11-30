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
#include "craft/CraftDefinitionShaped.hpp"
#include "server/game/gamedef.h"

bool CraftDefinitionShaped::check(const CraftInput &input, IGameDef *gamedef) const
{
	if (input.method != CRAFT_METHOD_NORMAL)
		return false;

	// Get input item matrix
	std::vector<std::string> inp_names = CraftUtils::craftGetItemNames(input.items, gamedef);
	unsigned int inp_width = input.width;
	if (inp_width == 0)
		return false;
	while (inp_names.size() % inp_width != 0)
		inp_names.emplace_back("");

	// Get input bounds
	unsigned int inp_min_x = 0, inp_max_x = 0, inp_min_y = 0, inp_max_y = 0;
	if (!CraftUtils::craftGetBounds(inp_names, inp_width, inp_min_x, inp_max_x,
			inp_min_y, inp_max_y))
		return false;  // it was empty

	std::vector<std::string> rec_names;
	if (hash_inited)
		rec_names = recipe_names;
	else
		rec_names = CraftUtils::craftGetItemNames(recipe, gamedef);

	// Get recipe item matrix
	unsigned int rec_width = width;
	if (rec_width == 0)
		return false;
	while (rec_names.size() % rec_width != 0)
		rec_names.emplace_back("");

	// Get recipe bounds
	unsigned int rec_min_x=0, rec_max_x=0, rec_min_y=0, rec_max_y=0;
	if (!CraftUtils::craftGetBounds(rec_names, rec_width, rec_min_x, rec_max_x,
			rec_min_y, rec_max_y))
		return false;  // it was empty

	// Different sizes?
	if (inp_max_x - inp_min_x != rec_max_x - rec_min_x ||
			inp_max_y - inp_min_y != rec_max_y - rec_min_y)
		return false;

	// Verify that all item names in the bounding box are equal
	unsigned int w = inp_max_x - inp_min_x + 1;
	unsigned int h = inp_max_y - inp_min_y + 1;

	for (unsigned int y=0; y < h; y++) {
		unsigned int inp_y = (inp_min_y + y) * inp_width;
		unsigned int rec_y = (rec_min_y + y) * rec_width;

		for (unsigned int x=0; x < w; x++) {
			unsigned int inp_x = inp_min_x + x;
			unsigned int rec_x = rec_min_x + x;

			if (!CraftUtils::inputItemMatchesRecipe(
					inp_names[inp_y + inp_x],
					rec_names[rec_y + rec_x], gamedef->idef())) {
				return false;
			}
		}
	}

	return true;
}

CraftOutput CraftDefinitionShaped::getOutput(const CraftInput &input, IGameDef *gamedef) const
{
	return CraftOutput(output, 0);
}

CraftInput CraftDefinitionShaped::getInput(const CraftOutput &output, IGameDef *gamedef) const
{
	return CraftInput(CRAFT_METHOD_NORMAL, width, CraftUtils::craftGetItems(recipe, gamedef));
}

void CraftDefinitionShaped::decrementInput(CraftInput &input, std::vector<ItemStack> &output_replacements,
	 IGameDef *gamedef) const
{
	CraftUtils::craftDecrementOrReplaceInput(input, output_replacements, replacements, gamedef);
}

CraftHashType CraftDefinitionShaped::getHashType() const
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

u64 CraftDefinitionShaped::getHash(CraftHashType type) const
{
	assert(hash_inited); // Pre-condition
	assert((type == CRAFT_HASH_TYPE_ITEM_NAMES)
		|| (type == CRAFT_HASH_TYPE_COUNT)); // Pre-condition

	std::vector<std::string> rec_names = recipe_names;
	std::sort(rec_names.begin(), rec_names.end());
	return CraftUtils::getHashForGrid(type, rec_names);
}

void CraftDefinitionShaped::initHash(IGameDef *gamedef)
{
	if (hash_inited)
		return;
	hash_inited = true;
	recipe_names = CraftUtils::craftGetItemNames(recipe, gamedef);
}

std::string CraftDefinitionShaped::dump() const
{
	std::ostringstream os(std::ios::binary);
	os << "(shaped, output=\"" << output
		<< "\", recipe=" << CraftUtils::craftDumpMatrix(recipe, width)
		<< ", replacements=" << replacements.dump() << ")";
	return os.str();
}

