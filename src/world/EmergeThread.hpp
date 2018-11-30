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
#ifndef EMERGETHREAD_HPP_
#define EMERGETHREAD_HPP_

#include "world/EmergeManager.hpp"
#include "map/ServerMap.hpp"
#include "threading/event.h"
#include "util/thread.h"

class EmergeThread : public Thread {
	public:
		EmergeThread(Server *server, int ethreadid);
		~EmergeThread() = default;

		void *run();
		void signal();

		// Requires queue mutex held
		bool pushBlock(const v3s16 &pos);

		void cancelPendingItems();

		static void runCompletionCallbacks(
				const v3s16 &pos, EmergeAction action,
				const EmergeCallbackList &callbacks);

		bool enable_mapgen_debug_info;
		int id;

	private:
		bool popBlockEmerge(v3s16 *pos, BlockEmergeData *bedata);

		EmergeAction getBlockOrStartGen(const v3s16 &pos, bool allow_gen, MapBlock **block, BlockMakeData *data);
		MapBlock *finishGen(v3s16 pos, BlockMakeData *bmdata, std::map<v3s16, MapBlock *> *modified_blocks);

		Server *m_server;
		ServerMap *m_map;
		EmergeManager *m_emerge;
		Mapgen *m_mapgen;

		Event m_queue_event;
		std::queue<v3s16> m_block_queue;

		friend class EmergeManager;
};

#endif // EMERGETHREAD_HPP_
