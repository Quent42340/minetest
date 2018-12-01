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
#ifndef CRAFTUTILS_HPP_
#define CRAFTUTILS_HPP_

#include <vector>
#include "common/inventory/inventory.h"
#include "util/numeric.h"

/*
	Crafting methods.

	The crafting method depends on the inventory list
	that the crafting input comes from.
*/
enum CraftMethod
{
	// Crafting grid
	CRAFT_METHOD_NORMAL,
	// Cooking something in a furnace
	CRAFT_METHOD_COOKING,
	// Using something as fuel for a furnace
	CRAFT_METHOD_FUEL,
};

/*
	The type a hash can be. The earlier a type is mentioned in this enum,
	the earlier it is tried at crafting, and the less likely is a collision.
	Changing order causes changes in behaviour, so know what you do.
 */
enum CraftHashType
{
	// Hashes the normalized names of the recipe's elements.
	// Only recipes without group usage can be found here,
	// because groups can't be guessed efficiently.
	CRAFT_HASH_TYPE_ITEM_NAMES,

	// Counts the non-empty slots.
	CRAFT_HASH_TYPE_COUNT,

	// This layer both spares an extra variable, and helps to retain (albeit rarely used) functionality. Maps to 0.
	// Before hashes are "initialized", all hashes reside here, after initialisation, none are.
	CRAFT_HASH_TYPE_UNHASHED

};

const int craft_hash_type_max = (int) CRAFT_HASH_TYPE_UNHASHED;

/*
	Input: The contents of the crafting slots, arranged in matrix form
*/
struct CraftInput
{
	CraftMethod method = CRAFT_METHOD_NORMAL;
	unsigned int width = 0;
	std::vector<ItemStack> items;

	CraftInput() = default;

	CraftInput(CraftMethod method_, unsigned int width_,
			const std::vector<ItemStack> &items_):
		method(method_), width(width_), items(items_)
	{}

	std::string dump() const;
};

/*
	Output: Result of crafting operation
*/
struct CraftOutput
{
	// Used for normal crafting and cooking, itemstring
	std::string item = "";
	// Used for cooking (cook time) and fuel (burn time), seconds
	float time = 0.0f;

	CraftOutput() = default;

	CraftOutput(const std::string &item_, float time_):
		item(item_), time(time_)
	{}
	std::string dump() const;
};

/*
	A list of replacements. A replacement indicates that a specific
	input item should not be deleted (when crafting) but replaced with
	a different item. Each replacements is a pair (itemstring to remove,
	itemstring to replace with)

	Example: If ("bucket:bucket_water", "bucket:bucket_empty") is a
	replacement pair, the crafting input slot that contained a water
	bucket will contain an empty bucket after crafting.

	Note: replacements only work correctly when stack_max of the item
	to be replaced is 1. It is up to the mod writer to ensure this.
*/
struct CraftReplacements
{
	// List of replacements
	std::vector<std::pair<std::string, std::string> > pairs;

	CraftReplacements() = default;
	CraftReplacements(const std::vector<std::pair<std::string, std::string> > &pairs_):
		pairs(pairs_)
	{}
	std::string dump() const;
};

namespace CraftUtils {
	inline bool isGroupRecipeStr(const std::string &rec_name)
	{
		return str_starts_with(rec_name, std::string("group:"));
	}

	inline u64 getHashForString(const std::string &recipe_str)
	{
		/*errorstream << "Hashing craft string  \"" << recipe_str << '"';*/
		return murmur_hash_64_ua(recipe_str.data(), recipe_str.length(), 0xdeadbeef);
	}

	u64 getHashForGrid(CraftHashType type, const std::vector<std::string> &grid_names);

	// Check if input matches recipe
	// Takes recipe groups into account
	bool inputItemMatchesRecipe(const std::string &inp_name,
			const std::string &rec_name, IItemDefManager *idef);

	// Deserialize an itemstring then return the name of the item
	std::string craftGetItemName(const std::string &itemstring, IGameDef *gamedef);

	// (mapcar craftGetItemName itemstrings)
	std::vector<std::string> craftGetItemNames(const std::vector<std::string> &itemstrings, IGameDef *gamedef);

	// Get name of each item, and return them as a new list.
	std::vector<std::string> craftGetItemNames(const std::vector<ItemStack> &items, IGameDef *gamedef);

	// Convert a list of item names, to ItemStacks.
	std::vector<ItemStack> craftGetItems(const std::vector<std::string> &items, IGameDef *gamedef);

	// Compute bounding rectangle given a matrix of items
	// Returns false if every item is ""
	bool craftGetBounds(const std::vector<std::string> &items, unsigned int width,
			unsigned int &min_x, unsigned int &max_x,
			unsigned int &min_y, unsigned int &max_y);

	// Removes 1 from each item stack
	void craftDecrementInput(CraftInput &input, IGameDef *gamedef);

	// Removes 1 from each item stack with replacement support
	// Example: if replacements contains the pair ("bucket:bucket_water", "bucket:bucket_empty"),
	//   a water bucket will not be removed but replaced by an empty bucket.
	void craftDecrementOrReplaceInput(CraftInput &input,
			std::vector<ItemStack> &output_replacements,
			const CraftReplacements &replacements,
			IGameDef *gamedef);

	// Dump an itemstring matrix
	std::string craftDumpMatrix(const std::vector<std::string> &items, unsigned int width);
	std::string craftDumpMatrix(const std::vector<ItemStack> &items, unsigned int width);
}

#endif // CRAFTUTILS_HPP_
