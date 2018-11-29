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

#include "filesys.h"
#include "porting.h"

#include "client/game/GameOnDemandSoundFetcher.h"

void GameOnDemandSoundFetcher::fetchSounds(const std::string &name, std::set<std::string> &dst_paths, std::set<std::string> &dst_datas) {
	if (m_fetched.count(name))
		return;

	m_fetched.insert(name);

	paths_insert(dst_paths, porting::path_share, name);
	paths_insert(dst_paths, porting::path_user,  name);
}

void GameOnDemandSoundFetcher::paths_insert(std::set<std::string> &dst_paths, const std::string &base, const std::string &name) {
	dst_paths.insert(base + DIR_DELIM + "sounds" + DIR_DELIM + name + ".ogg");
	dst_paths.insert(base + DIR_DELIM + "sounds" + DIR_DELIM + name + ".0.ogg");
	dst_paths.insert(base + DIR_DELIM + "sounds" + DIR_DELIM + name + ".1.ogg");
	dst_paths.insert(base + DIR_DELIM + "sounds" + DIR_DELIM + name + ".2.ogg");
	dst_paths.insert(base + DIR_DELIM + "sounds" + DIR_DELIM + name + ".3.ogg");
	dst_paths.insert(base + DIR_DELIM + "sounds" + DIR_DELIM + name + ".4.ogg");
	dst_paths.insert(base + DIR_DELIM + "sounds" + DIR_DELIM + name + ".5.ogg");
	dst_paths.insert(base + DIR_DELIM + "sounds" + DIR_DELIM + name + ".6.ogg");
	dst_paths.insert(base + DIR_DELIM + "sounds" + DIR_DELIM + name + ".7.ogg");
	dst_paths.insert(base + DIR_DELIM + "sounds" + DIR_DELIM + name + ".8.ogg");
	dst_paths.insert(base + DIR_DELIM + "sounds" + DIR_DELIM + name + ".9.ogg");
}

