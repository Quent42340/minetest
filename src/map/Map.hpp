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
#ifndef MAP_HPP_
#define MAP_HPP_

#include <iostream>
#include <sstream>
#include <set>
#include <map>
#include <list>

#include "irrlicht/irrlichttypes_bloated.h"
#include "map/MapNode.hpp"
#include "core/constants.h"
#include "world/voxel.h"
#include "common/modifiedstate.h"
#include "util/container.h"
#include "world/nodetimer.h"
#include "map/MapSettingsManager.hpp"
#include "debug/Debug.hpp"
#include "map/MapEditEvent.hpp"

class Settings;
class MapDatabase;
class ClientMap;
class MapSector;
class ServerMapSector;
class MapBlock;
class NodeMetadata;
class IGameDef;
class IRollbackManager;
class EmergeManager;
class ServerEnvironment;
struct BlockMakeData;

class Map /*: public NodeContainer*/ {
	public:
		Map(std::ostream &dout, IGameDef *gamedef);
		virtual ~Map();
		DISABLE_CLASS_COPY(Map);

		virtual s32 mapType() const { return MAPTYPE_BASE; }

		/*
		   Drop (client) or delete (server) the map.
		   */
		virtual void drop() { delete this; }

		void addEventReceiver(MapEventReceiver *event_receiver);
		void removeEventReceiver(MapEventReceiver *event_receiver);
		// event shall be deleted by caller after the call.
		void dispatchEvent(MapEditEvent *event);

		// On failure returns NULL
		MapSector * getSectorNoGenerateNoExNoLock(v2s16 p2d);
		// Same as the above (there exists no lock anymore)
		MapSector * getSectorNoGenerateNoEx(v2s16 p2d);
		// On failure throws InvalidPositionException
		MapSector * getSectorNoGenerate(v2s16 p2d);
		// Gets an existing sector or creates an empty one
		//MapSector * getSectorCreate(v2s16 p2d);

		/*
		   This is overloaded by ClientMap and ServerMap to allow
		   their differing fetch methods.
		   */
		virtual MapSector * emergeSector(v2s16 p){ return NULL; }

		// Returns InvalidPositionException if not found
		MapBlock * getBlockNoCreate(v3s16 p);
		// Returns NULL if not found
		MapBlock * getBlockNoCreateNoEx(v3s16 p);

		/* Server overrides */
		virtual MapBlock * emergeBlock(v3s16 p, bool create_blank=true)
		{ return getBlockNoCreateNoEx(p); }

		inline const NodeDefManager * getNodeDefManager() { return m_nodedef; }

		// Returns InvalidPositionException if not found
		bool isNodeUnderground(v3s16 p);

		bool isValidPosition(v3s16 p);

		// throws InvalidPositionException if not found
		void setNode(v3s16 p, MapNode & n);

		// Returns a CONTENT_IGNORE node if not found
		// If is_valid_position is not NULL then this will be set to true if the
		// position is valid, otherwise false
		MapNode getNodeNoEx(v3s16 p, bool *is_valid_position = NULL);

		/*
		   These handle lighting but not faces.
		   */
		void addNodeAndUpdate(v3s16 p, MapNode n,
				std::map<v3s16, MapBlock*> &modified_blocks,
				bool remove_metadata = true);
		void removeNodeAndUpdate(v3s16 p,
				std::map<v3s16, MapBlock*> &modified_blocks);

		/*
		   Wrappers for the latter ones.
		   These emit events.
		   Return true if succeeded, false if not.
		   */
		bool addNodeWithEvent(v3s16 p, MapNode n, bool remove_metadata = true);
		bool removeNodeWithEvent(v3s16 p);

		// Call these before and after saving of many blocks
		virtual void beginSave() {}
		virtual void endSave() {}

		virtual void save(ModifiedState save_level) { FATAL_ERROR("FIXME"); }

		// Server implements these.
		// Client leaves them as no-op.
		virtual bool saveBlock(MapBlock *block) { return false; }
		virtual bool deleteBlock(v3s16 blockpos) { return false; }

		/*
		   Updates usage timers and unloads unused blocks and sectors.
		   Saves modified blocks before unloading on MAPTYPE_SERVER.
		   */
		void timerUpdate(float dtime, float unload_timeout, u32 max_loaded_blocks,
				std::vector<v3s16> *unloaded_blocks=NULL);

		/*
		   Unloads all blocks with a zero refCount().
		   Saves modified blocks before unloading on MAPTYPE_SERVER.
		   */
		void unloadUnreferencedBlocks(std::vector<v3s16> *unloaded_blocks=NULL);

		// Deletes sectors and their blocks from memory
		// Takes cache into account
		// If deleted sector is in sector cache, clears cache
		void deleteSectors(std::vector<v2s16> &list);

		// For debug printing. Prints "Map: ", "ServerMap: " or "ClientMap: "
		virtual void PrintInfo(std::ostream &out);

		void transformLiquids(std::map<v3s16, MapBlock*> & modified_blocks,
				ServerEnvironment *env);

		/*
		   Node metadata
		   These are basically coordinate wrappers to MapBlock
		   */

		std::vector<v3s16> findNodesWithMetadata(v3s16 p1, v3s16 p2);
		NodeMetadata *getNodeMetadata(v3s16 p);

		/**
		 * Sets metadata for a node.
		 * This method sets the metadata for a given node.
		 * On success, it returns @c true and the object pointed to
		 * by @p meta is then managed by the system and should
		 * not be deleted by the caller.
		 *
		 * In case of failure, the method returns @c false and the
		 * caller is still responsible for deleting the object!
		 *
		 * @param p node coordinates
		 * @param meta pointer to @c NodeMetadata object
		 * @return @c true on success, false on failure
		 */
		bool setNodeMetadata(v3s16 p, NodeMetadata *meta);
		void removeNodeMetadata(v3s16 p);

		/*
		   Node Timers
		   These are basically coordinate wrappers to MapBlock
		   */

		NodeTimer getNodeTimer(v3s16 p);
		void setNodeTimer(const NodeTimer &t);
		void removeNodeTimer(v3s16 p);

		/*
		   Misc.
		   */
		std::map<v2s16, MapSector*> *getSectorsPtr(){return &m_sectors;}

		/*
		   Variables
		   */

		void transforming_liquid_add(v3s16 p);

		bool isBlockOccluded(MapBlock *block, v3s16 cam_pos_nodes);
	protected:
		friend class LuaVoxelManip;

		std::ostream &m_dout; // A bit deprecated, could be removed

		IGameDef *m_gamedef;

		std::set<MapEventReceiver*> m_event_receivers;

		std::map<v2s16, MapSector*> m_sectors;

		// Be sure to set this to NULL when the cached sector is deleted
		MapSector *m_sector_cache = nullptr;
		v2s16 m_sector_cache_p;

		// Queued transforming water nodes
		UniqueQueue<v3s16> m_transforming_liquid;

		// This stores the properties of the nodes on the map.
		const NodeDefManager *m_nodedef;

		bool isOccluded(v3s16 p0, v3s16 p1, float step, float stepfac,
				float start_off, float end_off, u32 needed_count);

	private:
		f32 m_transforming_liquid_loop_count_multiplier = 1.0f;
		u32 m_unprocessed_count = 0;
		u64 m_inc_trending_up_start_time = 0; // milliseconds
		bool m_queue_size_timer_started = false;
};

#endif // MAP_HPP_
