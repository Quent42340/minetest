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
#ifndef MAPEDITEVENT_HPP_
#define MAPEDITEVENT_HPP_

#include <set>

#include "core/constants.h"
#include "common/map/MapNode.hpp"
#include "common/world/voxel.h"

#define MAPTYPE_BASE 0
#define MAPTYPE_SERVER 1
#define MAPTYPE_CLIENT 2

enum MapEditEventType{
	// Node added (changed from air or something else to something)
	MEET_ADDNODE,
	// Node removed (changed to air)
	MEET_REMOVENODE,
	// Node swapped (changed without metadata change)
	MEET_SWAPNODE,
	// Node metadata of block changed (not knowing which node exactly)
	// p stores block coordinate
	MEET_BLOCK_NODE_METADATA_CHANGED,
	// Anything else (modified_blocks are set unsent)
	MEET_OTHER
};

struct MapEditEvent
{
	MapEditEventType type = MEET_OTHER;
	v3s16 p;
	MapNode n = CONTENT_AIR;
	std::set<v3s16> modified_blocks;

	MapEditEvent() = default;

	MapEditEvent *clone()
	{
		MapEditEvent *event = new MapEditEvent();
		event->type = type;
		event->p = p;
		event->n = n;
		event->modified_blocks = modified_blocks;
		return event;
	}

	VoxelArea getArea()
	{
		switch(type) {
			case MEET_ADDNODE:
				return VoxelArea(p);
			case MEET_REMOVENODE:
				return VoxelArea(p);
			case MEET_SWAPNODE:
				return VoxelArea(p);
			case MEET_BLOCK_NODE_METADATA_CHANGED:
			{
				v3s16 np1 = p*MAP_BLOCKSIZE;
				v3s16 np2 = np1 + v3s16(1,1,1)*MAP_BLOCKSIZE - v3s16(1,1,1);
				return VoxelArea(np1, np2);
			}
			case MEET_OTHER:
			{
				VoxelArea a;
				for (v3s16 p : modified_blocks) {
					v3s16 np1 = p*MAP_BLOCKSIZE;
					v3s16 np2 = np1 + v3s16(1,1,1)*MAP_BLOCKSIZE - v3s16(1,1,1);
					a.addPoint(np1);
					a.addPoint(np2);
				}
				return a;
			}
		}
		return VoxelArea();
	}
};

class MapEventReceiver {
	public:
		// event shall be deleted by caller after the call.
		virtual void onMapEditEvent(MapEditEvent *event) = 0;
};

#endif // MAPEDITEVENT_HPP_
