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

#include "craft/CraftDefinitionToolRepair.hpp"
#include "server/game/gamedef.h"

static ItemStack craftToolRepair(
		const ItemStack &item1,
		const ItemStack &item2,
		float additional_wear,
		IGameDef *gamedef)
{
	IItemDefManager *idef = gamedef->idef();
	if (item1.count != 1 || item2.count != 1 || item1.name != item2.name
			|| idef->get(item1.name).type != ITEM_TOOL
			|| itemgroup_get(idef->get(item1.name).groups, "disable_repair") == 1) {
		// Failure
		return ItemStack();
	}

	s32 item1_uses = 65536 - (u32) item1.wear;
	s32 item2_uses = 65536 - (u32) item2.wear;
	s32 new_uses = item1_uses + item2_uses;
	s32 new_wear = 65536 - new_uses + floor(additional_wear * 65536 + 0.5);
	if (new_wear >= 65536)
		return ItemStack();
	if (new_wear < 0)
		new_wear = 0;

	ItemStack repaired = item1;
	repaired.wear = new_wear;
	return repaired;
}

bool CraftDefinitionToolRepair::check(const CraftInput &input, IGameDef *gamedef) const
{
	if (input.method != CRAFT_METHOD_NORMAL)
		return false;

	ItemStack item1;
	ItemStack item2;
	for (const auto &item : input.items) {
		if (!item.empty()) {
			if (item1.empty())
				item1 = item;
			else if (item2.empty())
				item2 = item;
			else
				return false;
		}
	}
	ItemStack repaired = craftToolRepair(item1, item2, additional_wear, gamedef);
	return !repaired.empty();
}

CraftOutput CraftDefinitionToolRepair::getOutput(const CraftInput &input, IGameDef *gamedef) const
{
	ItemStack item1;
	ItemStack item2;
	for (const auto &item : input.items) {
		if (!item.empty()) {
			if (item1.empty())
				item1 = item;
			else if (item2.empty())
				item2 = item;
		}
	}
	ItemStack repaired = craftToolRepair(item1, item2, additional_wear, gamedef);
	return CraftOutput(repaired.getItemString(), 0);
}

CraftInput CraftDefinitionToolRepair::getInput(const CraftOutput &output, IGameDef *gamedef) const
{
	std::vector<ItemStack> stack;
	stack.emplace_back();
	return CraftInput(CRAFT_METHOD_COOKING, additional_wear, stack);
}

void CraftDefinitionToolRepair::decrementInput(CraftInput &input, std::vector<ItemStack> &output_replacements,
	IGameDef *gamedef) const
{
	CraftUtils::craftDecrementInput(input, gamedef);
}

std::string CraftDefinitionToolRepair::dump() const
{
	std::ostringstream os(std::ios::binary);
	os << "(toolrepair, additional_wear=" << additional_wear << ")";
	return os.str();
}

