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

#include "common/world/EmergeThread.hpp"
#include "common/map/MapBlock.hpp"
#include "server/server.h"
#include "scripting_server.h"
#include "core/Profiler.hpp"

EmergeThread::EmergeThread(Server *server, int ethreadid) :
	enable_mapgen_debug_info(false),
	id(ethreadid),
	m_server(server),
	m_map(NULL),
	m_emerge(NULL),
	m_mapgen(NULL)
{
	m_name = "Emerge-" + itos(ethreadid);
}


void EmergeThread::signal()
{
	m_queue_event.signal();
}


bool EmergeThread::pushBlock(const v3s16 &pos)
{
	m_block_queue.push(pos);
	return true;
}


void EmergeThread::cancelPendingItems()
{
	MutexAutoLock queuelock(m_emerge->m_queue_mutex);

	while (!m_block_queue.empty()) {
		BlockEmergeData bedata;
		v3s16 pos;

		pos = m_block_queue.front();
		m_block_queue.pop();

		m_emerge->popBlockEmergeData(pos, &bedata);

		runCompletionCallbacks(pos, EMERGE_CANCELLED, bedata.callbacks);
	}
}


void EmergeThread::runCompletionCallbacks(const v3s16 &pos, EmergeAction action,
	const EmergeCallbackList &callbacks)
{
	for (size_t i = 0; i != callbacks.size(); i++) {
		EmergeCompletionCallback callback;
		void *param;

		callback = callbacks[i].first;
		param    = callbacks[i].second;

		callback(pos, action, param);
	}
}


bool EmergeThread::popBlockEmerge(v3s16 *pos, BlockEmergeData *bedata)
{
	MutexAutoLock queuelock(m_emerge->m_queue_mutex);

	if (m_block_queue.empty())
		return false;

	*pos = m_block_queue.front();
	m_block_queue.pop();

	m_emerge->popBlockEmergeData(*pos, bedata);

	return true;
}


EmergeAction EmergeThread::getBlockOrStartGen(
	const v3s16 &pos, bool allow_gen, MapBlock **block, BlockMakeData *bmdata)
{
	MutexAutoLock envlock(m_server->m_env_mutex);

	// 1). Attempt to fetch block from memory
	*block = m_map->getBlockNoCreateNoEx(pos);
	if (*block && !(*block)->isDummy()) {
		if ((*block)->isGenerated())
			return EMERGE_FROM_MEMORY;
	} else {
		// 2). Attempt to load block from disk if it was not in the memory
		*block = m_map->loadBlock(pos);
		if (*block && (*block)->isGenerated())
			return EMERGE_FROM_DISK;
	}

	// 3). Attempt to start generation
	if (allow_gen && m_map->initBlockMake(pos, bmdata))
		return EMERGE_GENERATED;

	// All attempts failed; cancel this block emerge
	return EMERGE_CANCELLED;
}


MapBlock *EmergeThread::finishGen(v3s16 pos, BlockMakeData *bmdata,
	std::map<v3s16, MapBlock *> *modified_blocks)
{
	MutexAutoLock envlock(m_server->m_env_mutex);
	ScopeProfiler sp(g_profiler,
		"EmergeThread: after Mapgen::makeChunk", SPT_AVG);

	/*
		Perform post-processing on blocks (invalidate lighting, queue liquid
		transforms, etc.) to finish block make
	*/
	m_map->finishBlockMake(bmdata, modified_blocks);

	MapBlock *block = m_map->getBlockNoCreateNoEx(pos);
	if (!block) {
		errorstream << "EmergeThread::finishGen: Couldn't grab block we "
			"just generated: " << PP(pos) << std::endl;
		return NULL;
	}

	v3s16 minp = bmdata->blockpos_min * MAP_BLOCKSIZE;
	v3s16 maxp = bmdata->blockpos_max * MAP_BLOCKSIZE +
				 v3s16(1,1,1) * (MAP_BLOCKSIZE - 1);

	// Ignore map edit events, they will not need to be sent
	// to anybody because the block hasn't been sent to anybody
	MapEditEventAreaIgnorer ign(
		&m_server->m_ignore_map_edit_events_area,
		VoxelArea(minp, maxp));

	/*
		Run Lua on_generated callbacks
	*/
	try {
		m_server->getScriptIface()->environment_OnGenerated(
			minp, maxp, m_mapgen->blockseed);
	} catch (LuaError &e) {
		m_server->setAsyncFatalError("Lua: finishGen" + std::string(e.what()));
	}

	/*
		Clear generate notifier events
	*/
	Mapgen *mg = m_emerge->getCurrentMapgen();
	mg->gennotify.clearEvents();

	EMERGE_DBG_OUT("ended up with: " << analyze_block(block));

	/*
		Activate the block
	*/
	m_server->m_env->activateBlock(block, 0);

	return block;
}


void *EmergeThread::run()
{
	BEGIN_DEBUG_EXCEPTION_HANDLER

	v3s16 pos;

	m_map    = (ServerMap *)&(m_server->m_env->getMap());
	m_emerge = m_server->m_emerge;
	m_mapgen = m_emerge->m_mapgens[id];
	enable_mapgen_debug_info = m_emerge->enable_mapgen_debug_info;

	try {
	while (!stopRequested()) {
		std::map<v3s16, MapBlock *> modified_blocks;
		BlockEmergeData bedata;
		BlockMakeData bmdata;
		EmergeAction action;
		MapBlock *block;

		if (!popBlockEmerge(&pos, &bedata)) {
			m_queue_event.wait();
			continue;
		}

		if (blockpos_over_max_limit(pos))
			continue;

		bool allow_gen = bedata.flags & BLOCK_EMERGE_ALLOW_GEN;
		EMERGE_DBG_OUT("pos=" PP(pos) " allow_gen=" << allow_gen);

		action = getBlockOrStartGen(pos, allow_gen, &block, &bmdata);
		if (action == EMERGE_GENERATED) {
			{
				ScopeProfiler sp(g_profiler,
					"EmergeThread: Mapgen::makeChunk", SPT_AVG);
				TimeTaker t("mapgen::make_block()");

				m_mapgen->makeChunk(&bmdata);

				if (!enable_mapgen_debug_info)
					t.stop(true); // Hide output
			}

			block = finishGen(pos, &bmdata, &modified_blocks);
		}

		runCompletionCallbacks(pos, action, bedata.callbacks);

		if (block)
			modified_blocks[pos] = block;

		if (!modified_blocks.empty())
			m_server->SetBlocksNotSent(modified_blocks);
	}
	} catch (VersionMismatchException &e) {
		std::ostringstream err;
		err << "World data version mismatch in MapBlock " << PP(pos) << std::endl
			<< "----" << std::endl
			<< "\"" << e.what() << "\"" << std::endl
			<< "See debug.txt." << std::endl
			<< "World probably saved by a newer version of " PROJECT_NAME_C "."
			<< std::endl;
		m_server->setAsyncFatalError(err.str());
	} catch (SerializationError &e) {
		std::ostringstream err;
		err << "Invalid data in MapBlock " << PP(pos) << std::endl
			<< "----" << std::endl
			<< "\"" << e.what() << "\"" << std::endl
			<< "See debug.txt." << std::endl
			<< "You can ignore this using [ignore_world_load_errors = true]."
			<< std::endl;
		m_server->setAsyncFatalError(err.str());
	}

	END_DEBUG_EXCEPTION_HANDLER
	return NULL;
}

