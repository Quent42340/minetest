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
#include "CCraftDefManager.hpp"
#include "log.h"

CCraftDefManager::CCraftDefManager()
{
	m_craft_defs.resize(craft_hash_type_max + 1);
}

CCraftDefManager::~CCraftDefManager()
{
	clear();
}

bool CCraftDefManager::getCraftResult(CraftInput &input, CraftOutput &output,
		std::vector<ItemStack> &output_replacement, bool decrementInput,
		IGameDef *gamedef) const
{
	output.item = "";
	output.time = 0;

	// If all input items are empty, abort.
	bool all_empty = true;
	for (const auto &item : input.items) {
		if (!item.empty()) {
			all_empty = false;
			break;
		}
	}
	if (all_empty)
		return false;

	std::vector<std::string> input_names;
	input_names = CraftUtils::craftGetItemNames(input.items, gamedef);
	std::sort(input_names.begin(), input_names.end());

	// Try hash types with increasing collision rate, and return if found.
	for (int type = 0; type <= craft_hash_type_max; type++) {
		u64 hash = CraftUtils::getHashForGrid((CraftHashType) type, input_names);

		/*errorstream << "Checking type " << type << " with hash " << hash << std::endl;*/

		// We'd like to do "const [...] hash_collisions = m_craft_defs[type][hash];"
		// but that doesn't compile for some reason. This does.
		auto col_iter = (m_craft_defs[type]).find(hash);

		if (col_iter == (m_craft_defs[type]).end())
			continue;

		const std::vector<ICraftDefinition*> &hash_collisions = col_iter->second;
		// Walk crafting definitions from back to front, so that later
		// definitions can override earlier ones.
		for (std::vector<ICraftDefinition*>::size_type
				i = hash_collisions.size(); i > 0; i--) {
			ICraftDefinition *def = hash_collisions[i - 1];

			/*errorstream << "Checking " << input.dump() << std::endl
			  << " against " << def->dump() << std::endl;*/

			if (def->check(input, gamedef)) {
				// Check if the crafted node/item exists
				CraftOutput out = def->getOutput(input, gamedef);
				ItemStack is;
				is.deSerialize(out.item, gamedef->idef());
				if (!is.isKnown(gamedef->idef())) {
					infostream << "trying to craft non-existent "
						<< out.item << ", ignoring recipe" << std::endl;
					continue;
				}

				// Get output, then decrement input (if requested)
				output = out;

				if (decrementInput)
					def->decrementInput(input, output_replacement, gamedef);
				/*errorstream << "Check RETURNS TRUE" << std::endl;*/
				return true;
			}
		}
	}
	return false;
}

std::vector<ICraftDefinition*> CCraftDefManager::getCraftRecipes(CraftOutput &output,
		IGameDef *gamedef, unsigned limit) const
{
	std::vector<ICraftDefinition*> recipes;

	auto vec_iter = m_output_craft_definitions.find(output.item);

	if (vec_iter == m_output_craft_definitions.end())
		return recipes;

	const std::vector<ICraftDefinition*> &vec = vec_iter->second;

	recipes.reserve(limit ? MYMIN(limit, vec.size()) : vec.size());

	for (std::vector<ICraftDefinition*>::size_type i = vec.size();
			i > 0; i--) {
		ICraftDefinition *def = vec[i - 1];
		if (limit && recipes.size() >= limit)
			break;
		recipes.push_back(def);
	}

	return recipes;
}

bool CCraftDefManager::clearCraftRecipesByOutput(const CraftOutput &output, IGameDef *gamedef)
{
	auto vec_iter = m_output_craft_definitions.find(output.item);

	if (vec_iter == m_output_craft_definitions.end())
		return false;

	std::vector<ICraftDefinition*> &vec = vec_iter->second;
	for (auto def : vec) {
		// Recipes are not yet hashed at this point
		std::vector<ICraftDefinition*> &unhashed_inputs_vec = m_craft_defs[(int) CRAFT_HASH_TYPE_UNHASHED][0];
		std::vector<ICraftDefinition*> new_vec_by_input;
		/* We will preallocate necessary memory addresses, so we don't need to reallocate them later.
		   This would save us some performance. */
		new_vec_by_input.reserve(unhashed_inputs_vec.size());
		for (auto &i2 : unhashed_inputs_vec) {
			if (def != i2) {
				new_vec_by_input.push_back(i2);
			}
		}
		m_craft_defs[(int) CRAFT_HASH_TYPE_UNHASHED][0].swap(new_vec_by_input);
	}
	m_output_craft_definitions.erase(output.item);
	return true;
}

bool CCraftDefManager::clearCraftRecipesByInput(CraftMethod craft_method, unsigned int craft_grid_width,
		const std::vector<std::string> &recipe, IGameDef *gamedef)
{
	bool all_empty = true;
	for (const auto &i : recipe) {
		if (!i.empty()) {
			all_empty = false;
			break;
		}
	}
	if (all_empty)
		return false;

	CraftInput input(craft_method, craft_grid_width, CraftUtils::craftGetItems(recipe, gamedef));
	// Recipes are not yet hashed at this point
	std::vector<ICraftDefinition*> &unhashed_inputs_vec = m_craft_defs[(int) CRAFT_HASH_TYPE_UNHASHED][0];
	std::vector<ICraftDefinition*> new_vec_by_input;
	bool got_hit = false;
	for (std::vector<ICraftDefinition*>::size_type
			i = unhashed_inputs_vec.size(); i > 0; i--) {
		ICraftDefinition *def = unhashed_inputs_vec[i - 1];
		/* If the input doesn't match the recipe definition, this recipe definition later
		   will be added back in source map. */
		if (!def->check(input, gamedef)) {
			new_vec_by_input.push_back(def);
			continue;
		}
		CraftOutput output = def->getOutput(input, gamedef);
		got_hit = true;
		auto vec_iter = m_output_craft_definitions.find(output.item);
		if (vec_iter == m_output_craft_definitions.end())
			continue;
		std::vector<ICraftDefinition*> &vec = vec_iter->second;
		std::vector<ICraftDefinition*> new_vec_by_output;
		/* We will preallocate necessary memory addresses, so we don't need
		   to reallocate them later. This would save us some performance. */
		new_vec_by_output.reserve(vec.size());
		for (auto &vec_i : vec) {
			/* If pointers from map by input and output are not same,
			   we will add 'ICraftDefinition*' to a new vector. */
			if (def != vec_i) {
				/* Adding dereferenced iterator value (which are
				   'ICraftDefinition' reference) to a new vector. */
				new_vec_by_output.push_back(vec_i);
			}
		}
		// Swaps assigned to current key value with new vector for output map.
		m_output_craft_definitions[output.item].swap(new_vec_by_output);
	}
	if (got_hit)
		// Swaps value with new vector for input map.
		m_craft_defs[(int) CRAFT_HASH_TYPE_UNHASHED][0].swap(new_vec_by_input);

	return got_hit;
}

std::string CCraftDefManager::dump() const
{
	std::ostringstream os(std::ios::binary);
	os << "Crafting definitions:\n";
	for (int type = 0; type <= craft_hash_type_max; ++type) {
		for (auto it = m_craft_defs[type].begin();
				it != m_craft_defs[type].end(); ++it) {
			for (std::vector<ICraftDefinition*>::size_type i = 0;
					i < it->second.size(); i++) {
				os << "type " << type
					<< " hash " << it->first
					<< " def " << it->second[i]->dump()
					<< "\n";
			}
		}
	}
	return os.str();
}

void CCraftDefManager::registerCraft(ICraftDefinition *def, IGameDef *gamedef)
{
	verbosestream << "registerCraft: registering craft definition: "
		<< def->dump() << std::endl;
	m_craft_defs[(int) CRAFT_HASH_TYPE_UNHASHED][0].push_back(def);

	CraftInput input;
	std::string output_name = CraftUtils::craftGetItemName(
			def->getOutput(input, gamedef).item, gamedef);
	m_output_craft_definitions[output_name].push_back(def);
}

void CCraftDefManager::clear()
{
	for (int type = 0; type <= craft_hash_type_max; ++type) {
		for (auto &it : m_craft_defs[type]) {
			for (auto &iit : it.second) {
				delete iit;
			}
			it.second.clear();
		}
		m_craft_defs[type].clear();
	}
	m_output_craft_definitions.clear();
}

void CCraftDefManager::initHashes(IGameDef *gamedef)
{
	// Move the CraftDefs from the unhashed layer into layers higher up.
	std::vector<ICraftDefinition *> &unhashed =
		m_craft_defs[(int) CRAFT_HASH_TYPE_UNHASHED][0];
	for (auto def : unhashed) {
		// Initialize and get the definition's hash
		def->initHash(gamedef);
		CraftHashType type = def->getHashType();
		u64 hash = def->getHash(type);

		// Enter the definition
		m_craft_defs[type][hash].push_back(def);
	}
	unhashed.clear();
}

