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
#ifndef MMVMANIP_HPP_
#define MMVMANIP_HPP_

#include <map>

#include "world/voxel.h"

#define VMANIP_BLOCK_DATA_INEXIST     1
#define VMANIP_BLOCK_CONTAINS_CIGNORE 2

class MapBlock;

class MMVManip : public VoxelManipulator {
	public:
		MMVManip(Map *map);
		virtual ~MMVManip() = default;

		void clear() override
		{
			VoxelManipulator::clear();
			m_loaded_blocks.clear();
		}

		void initialEmerge(v3s16 blockpos_min, v3s16 blockpos_max, bool load_if_inexistent = true);

		// This is much faster with big chunks of generated data
		void blitBackAll(std::map<v3s16, MapBlock*> *modified_blocks, bool overwrite_generated = true);

		bool m_is_dirty = false;

	protected:
		Map *m_map;
		/*
		   key = blockpos
		   value = flags describing the block
		   */
		std::map<v3s16, u8> m_loaded_blocks;
};

#endif // MMVMANIP_HPP_
