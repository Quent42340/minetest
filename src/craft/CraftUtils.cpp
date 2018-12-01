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

#include "craft/CraftUtils.hpp"
#include "irrlicht/irrlichttypes.h"
#include "core/log.h"
#include <sstream>
#include <set>
#include <algorithm>
#include "server/game/gamedef.h"
#include "inventory/inventory.h"
#include "util/serialize.h"
#include "util/string.h"
#include "util/numeric.h"
#include "util/strfnd.h"
#include "core/Exception.hpp"

/*
	CraftInput
*/

std::string CraftInput::dump() const
{
	std::ostringstream os(std::ios::binary);
	os << "(method=" << ((int)method) << ", items="
		<< CraftUtils::craftDumpMatrix(items, width) << ")";
	return os.str();
}

/*
	CraftOutput
*/

std::string CraftOutput::dump() const
{
	std::ostringstream os(std::ios::binary);
	os << "(item=\"" << item << "\", time=" << time << ")";
	return os.str();
}

/*
	CraftReplacements
*/

std::string CraftReplacements::dump() const
{
	std::ostringstream os(std::ios::binary);
	os<<"{";
	const char *sep = "";
	for (const auto &repl_p : pairs) {
		os << sep
			<< '"' << (repl_p.first)
			<< "\"=>\"" << (repl_p.second) << '"';
		sep = ",";
	}
	os << "}";
	return os.str();
}

u64 CraftUtils::getHashForGrid(CraftHashType type, const std::vector<std::string> &grid_names)
{
	switch (type) {
		case CRAFT_HASH_TYPE_ITEM_NAMES: {
			std::ostringstream os;
			bool is_first = true;
			for (const std::string &grid_name : grid_names) {
				if (!grid_name.empty()) {
					os << (is_first ? "" : "\n") << grid_name;
					is_first = false;
				}
			}
			return getHashForString(os.str());
		} case CRAFT_HASH_TYPE_COUNT: {
			u64 cnt = 0;
			for (const std::string &grid_name : grid_names)
				if (!grid_name.empty())
					cnt++;
			return cnt;
		} case CRAFT_HASH_TYPE_UNHASHED:
			return 0;
	}
	// invalid CraftHashType
	assert(false);
	return 0;
}

bool CraftUtils::inputItemMatchesRecipe(const std::string &inp_name,
		const std::string &rec_name, IItemDefManager *idef)
{
	// Exact name
	if (inp_name == rec_name)
		return true;

	// Group
	if (isGroupRecipeStr(rec_name) && idef->isKnown(inp_name)) {
		const struct ItemDefinition &def = idef->get(inp_name);
		Strfnd f(rec_name.substr(6));
		bool all_groups_match = true;
		do {
			std::string check_group = f.next(",");
			if (itemgroup_get(def.groups, check_group) == 0) {
				all_groups_match = false;
				break;
			}
		} while (!f.at_end());
		if (all_groups_match)
			return true;
	}

	// Didn't match
	return false;
}

std::string CraftUtils::craftGetItemName(const std::string &itemstring, IGameDef *gamedef)
{
	ItemStack item;
	item.deSerialize(itemstring, gamedef->idef());
	return item.name;
}

std::vector<std::string> CraftUtils::craftGetItemNames(const std::vector<std::string> &itemstrings, IGameDef *gamedef)
{
	std::vector<std::string> result;
	for (const auto &itemstring : itemstrings) {
		result.push_back(craftGetItemName(itemstring, gamedef));
	}
	return result;
}

std::vector<std::string> CraftUtils::craftGetItemNames(const std::vector<ItemStack> &items, IGameDef *gamedef)
{
	std::vector<std::string> result;
	for (const auto &item : items) {
		result.push_back(item.name);
	}
	return result;
}

std::vector<ItemStack> CraftUtils::craftGetItems(const std::vector<std::string> &items, IGameDef *gamedef)
{
	std::vector<ItemStack> result;
	for (const auto &item : items) {
		result.emplace_back(std::string(item), (u16)1,
			(u16)0, gamedef->getItemDefManager());
	}
	return result;
}

// Compute bounding rectangle given a matrix of items
// Returns false if every item is ""
bool CraftUtils::craftGetBounds(const std::vector<std::string> &items, unsigned int width,
		unsigned int &min_x, unsigned int &max_x,
		unsigned int &min_y, unsigned int &max_y)
{
	bool success = false;
	unsigned int x = 0;
	unsigned int y = 0;
	for (const std::string &item : items) {
		// Is this an actual item?
		if (!item.empty()) {
			if (!success) {
				// This is the first nonempty item
				min_x = max_x = x;
				min_y = max_y = y;
				success = true;
			} else {
				if (x < min_x) min_x = x;
				if (x > max_x) max_x = x;
				if (y < min_y) min_y = y;
				if (y > max_y) max_y = y;
			}
		}

		// Step coordinate
		x++;
		if (x == width) {
			x = 0;
			y++;
		}
	}
	return success;
}

void CraftUtils::craftDecrementInput(CraftInput &input, IGameDef *gamedef)
{
	for (auto &item : input.items) {
		if (item.count != 0)
			item.remove(1);
	}
}

void CraftUtils::craftDecrementOrReplaceInput(CraftInput &input,
		std::vector<ItemStack> &output_replacements,
		const CraftReplacements &replacements,
		IGameDef *gamedef)
{
	if (replacements.pairs.empty()) {
		craftDecrementInput(input, gamedef);
		return;
	}

	// Make a copy of the replacements pair list
	std::vector<std::pair<std::string, std::string> > pairs = replacements.pairs;

	for (auto &item : input.items) {
		// Find an appropriate replacement
		bool found_replacement = false;
		for (auto j = pairs.begin(); j != pairs.end(); ++j) {
			if (inputItemMatchesRecipe(item.name, j->first, gamedef->idef())) {
				if (item.count == 1) {
					item.deSerialize(j->second, gamedef->idef());
					found_replacement = true;
					pairs.erase(j);
					break;
				}

				ItemStack rep;
				rep.deSerialize(j->second, gamedef->idef());
				item.remove(1);
				found_replacement = true;
				output_replacements.push_back(rep);
				break;

			}
		}
		// No replacement was found, simply decrement count by one
		if (!found_replacement && item.count > 0)
			item.remove(1);
	}
}

// Dump an item matrix
std::string CraftUtils::craftDumpMatrix(const std::vector<ItemStack> &items, unsigned int width)
{
	std::ostringstream os(std::ios::binary);
	os << "{ ";
	unsigned int x = 0;
	for (std::vector<ItemStack>::size_type i = 0;
			i < items.size(); i++, x++) {
		if (x == width) {
			os << "; ";
			x = 0;
		} else if (x != 0) {
			os << ",";
		}
		os << '"' << (items[i].getItemString()) << '"';
	}
	os << " }";
	return os.str();
}

// Dump an itemstring matrix
std::string CraftUtils::craftDumpMatrix(const std::vector<std::string> &items, unsigned int width)
{
	std::ostringstream os(std::ios::binary);
	os << "{ ";
	unsigned int x = 0;
	for(std::vector<std::string>::size_type i = 0;
			i < items.size(); i++, x++) {
		if (x == width) {
			os << "; ";
			x = 0;
		} else if (x != 0) {
			os << ",";
		}
		os << '"' << items[i] << '"';
	}
	os << " }";
	return os.str();
}

