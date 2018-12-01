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

#include "util/filesys.h"
#include "common/world/EmergeManager.hpp"
#include "common/map/ServerMap.hpp"
#include "common/map/MapSector.hpp"
#include "core/porting.h"
#include "core/Profiler.hpp"
#include "common/world/reflowscan.h"
#include "server/network/serialization.h"
#include "core/settings.h"
#include "common/algorithm/voxelalgorithms.h"
#include "database/database.h"
#include "database/database-dummy.h"
#include "database/database-sqlite3.h"

#if USE_LEVELDB
#include "database/database-leveldb.h"
#endif
#if USE_REDIS
#include "database/database-redis.h"
#endif
#if USE_POSTGRESQL
#include "database/database-postgresql.h"
#endif

ServerMap::ServerMap(const std::string &savedir, IGameDef *gamedef,
		EmergeManager *emerge):
	Map(dout_server, gamedef),
	settings_mgr(g_settings, savedir + DIR_DELIM + "map_meta.txt"),
	m_emerge(emerge)
{
	verbosestream << FUNCTION_NAME << std::endl;

	// Tell the EmergeManager about our MapSettingsManager
	emerge->map_settings_mgr = &settings_mgr;

	/*
		Try to load map; if not found, create a new one.
	*/

	// Determine which database backend to use
	std::string conf_path = savedir + DIR_DELIM + "world.mt";
	Settings conf;
	bool succeeded = conf.readConfigFile(conf_path.c_str());
	if (!succeeded || !conf.exists("backend")) {
		// fall back to sqlite3
		conf.set("backend", "sqlite3");
	}
	std::string backend = conf.get("backend");
	dbase = createDatabase(backend, savedir, conf);
	if (conf.exists("readonly_backend")) {
		std::string readonly_dir = savedir + DIR_DELIM + "readonly";
		dbase_ro = createDatabase(conf.get("readonly_backend"), readonly_dir, conf);
	}
	if (!conf.updateConfigFile(conf_path.c_str()))
		errorstream << "ServerMap::ServerMap(): Failed to update world.mt!" << std::endl;

	m_savedir = savedir;
	m_map_saving_enabled = false;

	try {
		// If directory exists, check contents and load if possible
		if (fs::PathExists(m_savedir)) {
			// If directory is empty, it is safe to save into it.
			if (fs::GetDirListing(m_savedir).empty()) {
				infostream<<"ServerMap: Empty save directory is valid."
						<<std::endl;
				m_map_saving_enabled = true;
			}
			else
			{

				if (settings_mgr.loadMapMeta()) {
					infostream << "ServerMap: Metadata loaded from "
						<< savedir << std::endl;
				} else {
					infostream << "ServerMap: Metadata could not be loaded "
						"from " << savedir << ", assuming valid save "
						"directory." << std::endl;
				}

				m_map_saving_enabled = true;
				// Map loaded, not creating new one
				return;
			}
		}
		// If directory doesn't exist, it is safe to save to it
		else{
			m_map_saving_enabled = true;
		}
	}
	catch(std::exception &e)
	{
		warningstream<<"ServerMap: Failed to load map from "<<savedir
				<<", exception: "<<e.what()<<std::endl;
		infostream<<"Please remove the map or fix it."<<std::endl;
		warningstream<<"Map saving will be disabled."<<std::endl;
	}
}

ServerMap::~ServerMap()
{
	verbosestream<<FUNCTION_NAME<<std::endl;

	try
	{
		if (m_map_saving_enabled) {
			// Save only changed parts
			save(MOD_STATE_WRITE_AT_UNLOAD);
			infostream << "ServerMap: Saved map to " << m_savedir << std::endl;
		} else {
			infostream << "ServerMap: Map not saved" << std::endl;
		}
	}
	catch(std::exception &e)
	{
		infostream<<"ServerMap: Failed to save map to "<<m_savedir
				<<", exception: "<<e.what()<<std::endl;
	}

	/*
		Close database if it was opened
	*/
	delete dbase;
	if (dbase_ro)
		delete dbase_ro;

#if 0
	/*
		Free all MapChunks
	*/
	core::map<v2s16, MapChunk*>::Iterator i = m_chunks.getIterator();
	for(; i.atEnd() == false; i++)
	{
		MapChunk *chunk = i.getNode()->getValue();
		delete chunk;
	}
#endif
}

MapgenParams *ServerMap::getMapgenParams()
{
	// getMapgenParams() should only ever be called after Server is initialized
	assert(settings_mgr.mapgen_params != NULL);
	return settings_mgr.mapgen_params;
}

u64 ServerMap::getSeed()
{
	return getMapgenParams()->seed;
}

s16 ServerMap::getWaterLevel()
{
	return getMapgenParams()->water_level;
}

bool ServerMap::blockpos_over_mapgen_limit(v3s16 p)
{
	const s16 mapgen_limit_bp = rangelim(
		getMapgenParams()->mapgen_limit, 0, MAX_MAP_GENERATION_LIMIT) /
		MAP_BLOCKSIZE;
	return p.X < -mapgen_limit_bp ||
		p.X >  mapgen_limit_bp ||
		p.Y < -mapgen_limit_bp ||
		p.Y >  mapgen_limit_bp ||
		p.Z < -mapgen_limit_bp ||
		p.Z >  mapgen_limit_bp;
}

bool ServerMap::initBlockMake(v3s16 blockpos, BlockMakeData *data)
{
	s16 csize = getMapgenParams()->chunksize;
	v3s16 bpmin = EmergeManager::getContainingChunk(blockpos, csize);
	v3s16 bpmax = bpmin + v3s16(1, 1, 1) * (csize - 1);

	bool enable_mapgen_debug_info = m_emerge->enable_mapgen_debug_info;
	EMERGE_DBG_OUT("initBlockMake(): " PP(bpmin) " - " PP(bpmax));

	v3s16 extra_borders(1, 1, 1);
	v3s16 full_bpmin = bpmin - extra_borders;
	v3s16 full_bpmax = bpmax + extra_borders;

	// Do nothing if not inside mapgen limits (+-1 because of neighbors)
	if (blockpos_over_mapgen_limit(full_bpmin) ||
			blockpos_over_mapgen_limit(full_bpmax))
		return false;

	data->seed = getSeed();
	data->blockpos_min = bpmin;
	data->blockpos_max = bpmax;
	data->blockpos_requested = blockpos;
	data->nodedef = m_nodedef;

	/*
		Create the whole area of this and the neighboring blocks
	*/
	for (s16 x = full_bpmin.X; x <= full_bpmax.X; x++)
	for (s16 z = full_bpmin.Z; z <= full_bpmax.Z; z++) {
		v2s16 sectorpos(x, z);
		// Sector metadata is loaded from disk if not already loaded.
		MapSector *sector = createSector(sectorpos);
		FATAL_ERROR_IF(sector == NULL, "createSector() failed");

		for (s16 y = full_bpmin.Y; y <= full_bpmax.Y; y++) {
			v3s16 p(x, y, z);

			MapBlock *block = emergeBlock(p, false);
			if (block == NULL) {
				block = createBlock(p);

				// Block gets sunlight if this is true.
				// Refer to the map generator heuristics.
				bool ug = m_emerge->isBlockUnderground(p);
				block->setIsUnderground(ug);
			}
		}
	}

	/*
		Now we have a big empty area.

		Make a ManualMapVoxelManipulator that contains this and the
		neighboring blocks
	*/

	data->vmanip = new MMVManip(this);
	data->vmanip->initialEmerge(full_bpmin, full_bpmax);

	// Note: we may need this again at some point.
#if 0
	// Ensure none of the blocks to be generated were marked as
	// containing CONTENT_IGNORE
	for (s16 z = blockpos_min.Z; z <= blockpos_max.Z; z++) {
		for (s16 y = blockpos_min.Y; y <= blockpos_max.Y; y++) {
			for (s16 x = blockpos_min.X; x <= blockpos_max.X; x++) {
				core::map<v3s16, u8>::Node *n;
				n = data->vmanip->m_loaded_blocks.find(v3s16(x, y, z));
				if (n == NULL)
					continue;
				u8 flags = n->getValue();
				flags &= ~VMANIP_BLOCK_CONTAINS_CIGNORE;
				n->setValue(flags);
			}
		}
	}
#endif

	// Data is ready now.
	return true;
}

void ServerMap::finishBlockMake(BlockMakeData *data,
	std::map<v3s16, MapBlock*> *changed_blocks)
{
	v3s16 bpmin = data->blockpos_min;
	v3s16 bpmax = data->blockpos_max;

	v3s16 extra_borders(1, 1, 1);

	bool enable_mapgen_debug_info = m_emerge->enable_mapgen_debug_info;
	EMERGE_DBG_OUT("finishBlockMake(): " PP(bpmin) " - " PP(bpmax));

	/*
		Blit generated stuff to map
		NOTE: blitBackAll adds nearly everything to changed_blocks
	*/
	data->vmanip->blitBackAll(changed_blocks);

	EMERGE_DBG_OUT("finishBlockMake: changed_blocks.size()="
		<< changed_blocks->size());

	/*
		Copy transforming liquid information
	*/
	while (data->transforming_liquid.size()) {
		m_transforming_liquid.push_back(data->transforming_liquid.front());
		data->transforming_liquid.pop_front();
	}

	for (auto &changed_block : *changed_blocks) {
		MapBlock *block = changed_block.second;
		if (!block)
			continue;
		/*
			Update day/night difference cache of the MapBlocks
		*/
		block->expireDayNightDiff();
		/*
			Set block as modified
		*/
		block->raiseModified(MOD_STATE_WRITE_NEEDED,
			MOD_REASON_EXPIRE_DAYNIGHTDIFF);
	}

	/*
		Set central blocks as generated
	*/
	for (s16 x = bpmin.X; x <= bpmax.X; x++)
	for (s16 z = bpmin.Z; z <= bpmax.Z; z++)
	for (s16 y = bpmin.Y; y <= bpmax.Y; y++) {
		MapBlock *block = getBlockNoCreateNoEx(v3s16(x, y, z));
		if (!block)
			continue;

		block->setGenerated(true);
	}

	/*
		Save changed parts of map
		NOTE: Will be saved later.
	*/
	//save(MOD_STATE_WRITE_AT_UNLOAD);
}

MapSector *ServerMap::createSector(v2s16 p2d)
{
	/*
		Check if it exists already in memory
	*/
	MapSector *sector = getSectorNoGenerateNoEx(p2d);
	if (sector)
		return sector;

	/*
		Do not create over max mapgen limit
	*/
	const s16 max_limit_bp = MAX_MAP_GENERATION_LIMIT / MAP_BLOCKSIZE;
	if (p2d.X < -max_limit_bp ||
			p2d.X >  max_limit_bp ||
			p2d.Y < -max_limit_bp ||
			p2d.Y >  max_limit_bp)
		throw InvalidPositionException("createSector(): pos. over max mapgen limit");

	/*
		Generate blank sector
	*/

	sector = new MapSector(this, p2d, m_gamedef);

	// Sector position on map in nodes
	//v2s16 nodepos2d = p2d * MAP_BLOCKSIZE;

	/*
		Insert to container
	*/
	m_sectors[p2d] = sector;

	return sector;
}

#if 0
/*
	This is a quick-hand function for calling makeBlock().
*/
MapBlock * ServerMap::generateBlock(
		v3s16 p,
		std::map<v3s16, MapBlock*> &modified_blocks
)
{
	bool enable_mapgen_debug_info = g_settings->getBool("enable_mapgen_debug_info");

	TimeTaker timer("generateBlock");

	//MapBlock *block = original_dummy;

	v2s16 p2d(p.X, p.Z);
	v2s16 p2d_nodes = p2d * MAP_BLOCKSIZE;

	/*
		Do not generate over-limit
	*/
	if(blockpos_over_limit(p))
	{
		infostream<<FUNCTION_NAME<<": Block position over limit"<<std::endl;
		throw InvalidPositionException("generateBlock(): pos. over limit");
	}

	/*
		Create block make data
	*/
	BlockMakeData data;
	initBlockMake(&data, p);

	/*
		Generate block
	*/
	{
		TimeTaker t("mapgen::make_block()");
		mapgen->makeChunk(&data);
		//mapgen::make_block(&data);

		if(enable_mapgen_debug_info == false)
			t.stop(true); // Hide output
	}

	/*
		Blit data back on map, update lighting, add mobs and whatever this does
	*/
	finishBlockMake(&data, modified_blocks);

	/*
		Get central block
	*/
	MapBlock *block = getBlockNoCreateNoEx(p);

#if 0
	/*
		Check result
	*/
	if(block)
	{
		bool erroneus_content = false;
		for(s16 z0=0; z0<MAP_BLOCKSIZE; z0++)
		for(s16 y0=0; y0<MAP_BLOCKSIZE; y0++)
		for(s16 x0=0; x0<MAP_BLOCKSIZE; x0++)
		{
			v3s16 p(x0,y0,z0);
			MapNode n = block->getNode(p);
			if(n.getContent() == CONTENT_IGNORE)
			{
				infostream<<"CONTENT_IGNORE at "
						<<"("<<p.X<<","<<p.Y<<","<<p.Z<<")"
						<<std::endl;
				erroneus_content = true;
				assert(0);
			}
		}
		if(erroneus_content)
		{
			assert(0);
		}
	}
#endif

#if 0
	/*
		Generate a completely empty block
	*/
	if(block)
	{
		for(s16 z0=0; z0<MAP_BLOCKSIZE; z0++)
		for(s16 x0=0; x0<MAP_BLOCKSIZE; x0++)
		{
			for(s16 y0=0; y0<MAP_BLOCKSIZE; y0++)
			{
				MapNode n;
				n.setContent(CONTENT_AIR);
				block->setNode(v3s16(x0,y0,z0), n);
			}
		}
	}
#endif

	if(enable_mapgen_debug_info == false)
		timer.stop(true); // Hide output

	return block;
}
#endif

MapBlock * ServerMap::createBlock(v3s16 p)
{
	/*
		Do not create over max mapgen limit
	*/
	if (blockpos_over_max_limit(p))
		throw InvalidPositionException("createBlock(): pos. over max mapgen limit");

	v2s16 p2d(p.X, p.Z);
	s16 block_y = p.Y;
	/*
		This will create or load a sector if not found in memory.
		If block exists on disk, it will be loaded.

		NOTE: On old save formats, this will be slow, as it generates
		      lighting on blocks for them.
	*/
	MapSector *sector;
	try {
		sector = createSector(p2d);
	} catch (InvalidPositionException &e) {
		infostream<<"createBlock: createSector() failed"<<std::endl;
		throw e;
	}

	/*
		Try to get a block from the sector
	*/

	MapBlock *block = sector->getBlockNoCreateNoEx(block_y);
	if (block) {
		if(block->isDummy())
			block->unDummify();
		return block;
	}
	// Create blank
	block = sector->createBlankBlock(block_y);

	return block;
}

MapBlock * ServerMap::emergeBlock(v3s16 p, bool create_blank)
{
	{
		MapBlock *block = getBlockNoCreateNoEx(p);
		if (block && !block->isDummy())
			return block;
	}

	{
		MapBlock *block = loadBlock(p);
		if(block)
			return block;
	}

	if (create_blank) {
		MapSector *sector = createSector(v2s16(p.X, p.Z));
		MapBlock *block = sector->createBlankBlock(p.Y);

		return block;
	}

	return NULL;
}

MapBlock *ServerMap::getBlockOrEmerge(v3s16 p3d)
{
	MapBlock *block = getBlockNoCreateNoEx(p3d);
	if (block == NULL)
		m_emerge->enqueueBlockEmerge(PEER_ID_INEXISTENT, p3d, false);

	return block;
}

// N.B.  This requires no synchronization, since data will not be modified unless
// the VoxelManipulator being updated belongs to the same thread.
void ServerMap::updateVManip(v3s16 pos)
{
	Mapgen *mg = m_emerge->getCurrentMapgen();
	if (!mg)
		return;

	MMVManip *vm = mg->vm;
	if (!vm)
		return;

	if (!vm->m_area.contains(pos))
		return;

	s32 idx = vm->m_area.index(pos);
	vm->m_data[idx] = getNodeNoEx(pos);
	vm->m_flags[idx] &= ~VOXELFLAG_NO_DATA;

	vm->m_is_dirty = true;
}

s16 ServerMap::findGroundLevel(v2s16 p2d)
{
#if 0
	/*
		Uh, just do something random...
	*/
	// Find existing map from top to down
	s16 max=63;
	s16 min=-64;
	v3s16 p(p2d.X, max, p2d.Y);
	for(; p.Y>min; p.Y--)
	{
		MapNode n = getNodeNoEx(p);
		if(n.getContent() != CONTENT_IGNORE)
			break;
	}
	if(p.Y == min)
		goto plan_b;
	// If this node is not air, go to plan b
	if(getNodeNoEx(p).getContent() != CONTENT_AIR)
		goto plan_b;
	// Search existing walkable and return it
	for(; p.Y>min; p.Y--)
	{
		MapNode n = getNodeNoEx(p);
		if(content_walkable(n.d) && n.getContent() != CONTENT_IGNORE)
			return p.Y;
	}

	// Move to plan b
plan_b:
#endif

	/*
		Determine from map generator noise functions
	*/

	s16 level = m_emerge->getGroundLevelAtPoint(p2d);
	return level;

	//double level = base_rock_level_2d(m_seed, p2d) + AVERAGE_MUD_AMOUNT;
	//return (s16)level;
}

bool ServerMap::loadFromFolders() {
	if (!dbase->initialized() &&
			!fs::PathExists(m_savedir + DIR_DELIM + "map.sqlite"))
		return true;
	return false;
}

void ServerMap::createDirs(std::string path)
{
	if (!fs::CreateAllDirs(path)) {
		m_dout<<"ServerMap: Failed to create directory "
				<<"\""<<path<<"\""<<std::endl;
		throw BaseException("ServerMap failed to create directory");
	}
}

std::string ServerMap::getSectorDir(v2s16 pos, int layout)
{
	char cc[9];
	switch(layout)
	{
		case 1:
			porting::mt_snprintf(cc, sizeof(cc), "%.4x%.4x",
				(unsigned int) pos.X & 0xffff,
				(unsigned int) pos.Y & 0xffff);

			return m_savedir + DIR_DELIM + "sectors" + DIR_DELIM + cc;
		case 2:
			porting::mt_snprintf(cc, sizeof(cc), (std::string("%.3x") + DIR_DELIM + "%.3x").c_str(),
				(unsigned int) pos.X & 0xfff,
				(unsigned int) pos.Y & 0xfff);

			return m_savedir + DIR_DELIM + "sectors2" + DIR_DELIM + cc;
		default:
			assert(false);
			return "";
	}
}

v2s16 ServerMap::getSectorPos(const std::string &dirname)
{
	unsigned int x = 0, y = 0;
	int r;
	std::string component;
	fs::RemoveLastPathComponent(dirname, &component, 1);
	if(component.size() == 8)
	{
		// Old layout
		r = sscanf(component.c_str(), "%4x%4x", &x, &y);
	}
	else if(component.size() == 3)
	{
		// New layout
		fs::RemoveLastPathComponent(dirname, &component, 2);
		r = sscanf(component.c_str(), (std::string("%3x") + DIR_DELIM + "%3x").c_str(), &x, &y);
		// Sign-extend the 12 bit values up to 16 bits...
		if(x & 0x800) x |= 0xF000;
		if(y & 0x800) y |= 0xF000;
	}
	else
	{
		r = -1;
	}

	FATAL_ERROR_IF(r != 2, "getSectorPos()");
	v2s16 pos((s16)x, (s16)y);
	return pos;
}

v3s16 ServerMap::getBlockPos(const std::string &sectordir, const std::string &blockfile)
{
	v2s16 p2d = getSectorPos(sectordir);

	if(blockfile.size() != 4){
		throw InvalidFilenameException("Invalid block filename");
	}
	unsigned int y;
	int r = sscanf(blockfile.c_str(), "%4x", &y);
	if(r != 1)
		throw InvalidFilenameException("Invalid block filename");
	return v3s16(p2d.X, y, p2d.Y);
}

std::string ServerMap::getBlockFilename(v3s16 p)
{
	char cc[5];
	porting::mt_snprintf(cc, sizeof(cc), "%.4x", (unsigned int)p.Y&0xffff);
	return cc;
}

void ServerMap::save(ModifiedState save_level)
{
	if (!m_map_saving_enabled) {
		warningstream<<"Not saving map, saving disabled."<<std::endl;
		return;
	}

	if(save_level == MOD_STATE_CLEAN)
		infostream<<"ServerMap: Saving whole map, this can take time."
				<<std::endl;

	if (m_map_metadata_changed || save_level == MOD_STATE_CLEAN) {
		if (settings_mgr.saveMapMeta())
			m_map_metadata_changed = false;
	}

	// Profile modified reasons
	Profiler modprofiler;

	u32 block_count = 0;
	u32 block_count_all = 0; // Number of blocks in memory

	// Don't do anything with sqlite unless something is really saved
	bool save_started = false;

	for (auto &sector_it : m_sectors) {
		MapSector *sector = sector_it.second;

		MapBlockVect blocks;
		sector->getBlocks(blocks);

		for (MapBlock *block : blocks) {
			block_count_all++;

			if(block->getModified() >= (u32)save_level) {
				// Lazy beginSave()
				if(!save_started) {
					beginSave();
					save_started = true;
				}

				modprofiler.add(block->getModifiedReasonString(), 1);

				saveBlock(block);
				block_count++;
			}
		}
	}

	if(save_started)
		endSave();

	/*
		Only print if something happened or saved whole map
	*/
	if(save_level == MOD_STATE_CLEAN
			|| block_count != 0) {
		infostream<<"ServerMap: Written: "
				<<block_count<<" block files"
				<<", "<<block_count_all<<" blocks in memory."
				<<std::endl;
		PrintInfo(infostream); // ServerMap/ClientMap:
		infostream<<"Blocks modified by: "<<std::endl;
		modprofiler.print(infostream);
	}
}

void ServerMap::listAllLoadableBlocks(std::vector<v3s16> &dst)
{
	if (loadFromFolders()) {
		errorstream << "Map::listAllLoadableBlocks(): Result will be missing "
				<< "all blocks that are stored in flat files." << std::endl;
	}
	dbase->listAllLoadableBlocks(dst);
	if (dbase_ro)
		dbase_ro->listAllLoadableBlocks(dst);
}

void ServerMap::listAllLoadedBlocks(std::vector<v3s16> &dst)
{
	for (auto &sector_it : m_sectors) {
		MapSector *sector = sector_it.second;

		MapBlockVect blocks;
		sector->getBlocks(blocks);

		for (MapBlock *block : blocks) {
			v3s16 p = block->getPos();
			dst.push_back(p);
		}
	}
}

MapDatabase *ServerMap::createDatabase(
	const std::string &name,
	const std::string &savedir,
	Settings &conf)
{
	if (name == "sqlite3")
		return new MapDatabaseSQLite3(savedir);
	if (name == "dummy")
		return new Database_Dummy();
	#if USE_LEVELDB
	if (name == "leveldb")
		return new Database_LevelDB(savedir);
	#endif
	#if USE_REDIS
	if (name == "redis")
		return new Database_Redis(conf);
	#endif
	#if USE_POSTGRESQL
	if (name == "postgresql") {
		std::string connect_string;
		conf.getNoEx("pgsql_connection", connect_string);
		return new MapDatabasePostgreSQL(connect_string);
	}
	#endif

	throw BaseException(std::string("Database backend ") + name + " not supported.");
}

void ServerMap::beginSave()
{
	dbase->beginSave();
}

void ServerMap::endSave()
{
	dbase->endSave();
}

bool ServerMap::saveBlock(MapBlock *block)
{
	return saveBlock(block, dbase);
}

bool ServerMap::saveBlock(MapBlock *block, MapDatabase *db)
{
	v3s16 p3d = block->getPos();

	// Dummy blocks are not written
	if (block->isDummy()) {
		warningstream << "saveBlock: Not writing dummy block "
			<< PP(p3d) << std::endl;
		return true;
	}

	// Format used for writing
	u8 version = SER_FMT_VER_HIGHEST_WRITE;

	/*
		[0] u8 serialization version
		[1] data
	*/
	std::ostringstream o(std::ios_base::binary);
	o.write((char*) &version, 1);
	block->serialize(o, version, true);

	bool ret = db->saveBlock(p3d, o.str());
	if (ret) {
		// We just wrote it to the disk so clear modified flag
		block->resetModified();
	}
	return ret;
}

void ServerMap::loadBlock(const std::string &sectordir, const std::string &blockfile,
		MapSector *sector, bool save_after_load)
{
	std::string fullpath = sectordir + DIR_DELIM + blockfile;
	try {
		std::ifstream is(fullpath.c_str(), std::ios_base::binary);
		if (!is.good())
			throw FileNotGoodException("Cannot open block file");

		v3s16 p3d = getBlockPos(sectordir, blockfile);
		v2s16 p2d(p3d.X, p3d.Z);

		assert(sector->getPos() == p2d);

		u8 version = SER_FMT_VER_INVALID;
		is.read((char*)&version, 1);

		if(is.fail())
			throw SerializationError("ServerMap::loadBlock(): Failed"
					" to read MapBlock version");

		/*u32 block_size = MapBlock::serializedLength(version);
		SharedBuffer<u8> data(block_size);
		is.read((char*)*data, block_size);*/

		// This will always return a sector because we're the server
		//MapSector *sector = emergeSector(p2d);

		MapBlock *block = NULL;
		bool created_new = false;
		block = sector->getBlockNoCreateNoEx(p3d.Y);
		if(block == NULL)
		{
			block = sector->createBlankBlockNoInsert(p3d.Y);
			created_new = true;
		}

		// Read basic data
		block->deSerialize(is, version, true);

		// If it's a new block, insert it to the map
		if (created_new) {
			sector->insertBlock(block);
			ReflowScan scanner(this, m_emerge->ndef);
			scanner.scan(block, &m_transforming_liquid);
		}

		/*
			Save blocks loaded in old format in new format
		*/

		if(version < SER_FMT_VER_HIGHEST_WRITE || save_after_load)
		{
			saveBlock(block);

			// Should be in database now, so delete the old file
			fs::RecursiveDelete(fullpath);
		}

		// We just loaded it from the disk, so it's up-to-date.
		block->resetModified();

	}
	catch(SerializationError &e)
	{
		warningstream<<"Invalid block data on disk "
				<<"fullpath="<<fullpath
				<<" (SerializationError). "
				<<"what()="<<e.what()
				<<std::endl;
				// Ignoring. A new one will be generated.
		abort();

		// TODO: Backup file; name is in fullpath.
	}
}

void ServerMap::loadBlock(std::string *blob, v3s16 p3d, MapSector *sector, bool save_after_load)
{
	try {
		std::istringstream is(*blob, std::ios_base::binary);

		u8 version = SER_FMT_VER_INVALID;
		is.read((char*)&version, 1);

		if(is.fail())
			throw SerializationError("ServerMap::loadBlock(): Failed"
					" to read MapBlock version");

		MapBlock *block = NULL;
		bool created_new = false;
		block = sector->getBlockNoCreateNoEx(p3d.Y);
		if(block == NULL)
		{
			block = sector->createBlankBlockNoInsert(p3d.Y);
			created_new = true;
		}

		// Read basic data
		block->deSerialize(is, version, true);

		// If it's a new block, insert it to the map
		if (created_new) {
			sector->insertBlock(block);
			ReflowScan scanner(this, m_emerge->ndef);
			scanner.scan(block, &m_transforming_liquid);
		}

		/*
			Save blocks loaded in old format in new format
		*/

		//if(version < SER_FMT_VER_HIGHEST_READ || save_after_load)
		// Only save if asked to; no need to update version
		if(save_after_load)
			saveBlock(block);

		// We just loaded it from, so it's up-to-date.
		block->resetModified();
	}
	catch(SerializationError &e)
	{
		errorstream<<"Invalid block data in database"
				<<" ("<<p3d.X<<","<<p3d.Y<<","<<p3d.Z<<")"
				<<" (SerializationError): "<<e.what()<<std::endl;

		// TODO: Block should be marked as invalid in memory so that it is
		// not touched but the game can run

		if(g_settings->getBool("ignore_world_load_errors")){
			errorstream<<"Ignoring block load error. Duck and cover! "
					<<"(ignore_world_load_errors)"<<std::endl;
		} else {
			throw SerializationError("Invalid block data in database");
		}
	}
}

MapBlock* ServerMap::loadBlock(v3s16 blockpos)
{
	bool created_new = (getBlockNoCreateNoEx(blockpos) == NULL);

	v2s16 p2d(blockpos.X, blockpos.Z);

	std::string ret;
	dbase->loadBlock(blockpos, &ret);
	if (!ret.empty()) {
		loadBlock(&ret, blockpos, createSector(p2d), false);
	} else if (dbase_ro) {
		dbase_ro->loadBlock(blockpos, &ret);
		if (!ret.empty()) {
			loadBlock(&ret, blockpos, createSector(p2d), false);
		}
	} else {
		// Not found in database, try the files

		// The directory layout we're going to load from.
		//  1 - original sectors/xxxxzzzz/
		//  2 - new sectors2/xxx/zzz/
		//  If we load from anything but the latest structure, we will
		//  immediately save to the new one, and remove the old.
		std::string sectordir1 = getSectorDir(p2d, 1);
		std::string sectordir;
		if (fs::PathExists(sectordir1)) {
			sectordir = sectordir1;
		} else {
			sectordir = getSectorDir(p2d, 2);
		}

		/*
		Make sure sector is loaded
		 */

		MapSector *sector = getSectorNoGenerateNoEx(p2d);

		/*
		Make sure file exists
		 */

		std::string blockfilename = getBlockFilename(blockpos);
		if (!fs::PathExists(sectordir + DIR_DELIM + blockfilename))
			return NULL;

		/*
		Load block and save it to the database
		 */
		loadBlock(sectordir, blockfilename, sector, true);
	}

	MapBlock *block = getBlockNoCreateNoEx(blockpos);
	if (created_new && (block != NULL)) {
		std::map<v3s16, MapBlock*> modified_blocks;
		// Fix lighting if necessary
		voxalgo::update_block_border_lighting(this, block, modified_blocks);
		if (!modified_blocks.empty()) {
			//Modified lighting, send event
			MapEditEvent event;
			event.type = MEET_OTHER;
			std::map<v3s16, MapBlock *>::iterator it;
			for (it = modified_blocks.begin();
					it != modified_blocks.end(); ++it)
				event.modified_blocks.insert(it->first);
			dispatchEvent(&event);
		}
	}
	return block;
}

bool ServerMap::deleteBlock(v3s16 blockpos)
{
	if (!dbase->deleteBlock(blockpos))
		return false;

	MapBlock *block = getBlockNoCreateNoEx(blockpos);
	if (block) {
		v2s16 p2d(blockpos.X, blockpos.Z);
		MapSector *sector = getSectorNoGenerateNoEx(p2d);
		if (!sector)
			return false;
		sector->deleteBlock(block);
	}

	return true;
}

void ServerMap::PrintInfo(std::ostream &out)
{
	out<<"ServerMap: ";
}

bool ServerMap::repairBlockLight(v3s16 blockpos,
	std::map<v3s16, MapBlock *> *modified_blocks)
{
	MapBlock *block = emergeBlock(blockpos, false);
	if (!block || !block->isGenerated())
		return false;
	voxalgo::repair_block_light(this, block, modified_blocks);
	return true;
}

