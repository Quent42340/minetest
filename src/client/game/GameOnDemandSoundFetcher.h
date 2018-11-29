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
#ifndef GAMEONDEMANDSOUNDFETCHER_HPP_
#define GAMEONDEMANDSOUNDFETCHER_HPP_

#if USE_SOUND
	#include "client/sound_openal.h"
#else
	#include "client/sound.h"
#endif

// Locally stored sounds don't need to be preloaded because of this
class GameOnDemandSoundFetcher: public OnDemandSoundFetcher {
	public:
		void fetchSounds(const std::string &name, std::set<std::string> &dst_paths, std::set<std::string> &dst_datas) override;

	private:
		void paths_insert(std::set<std::string> &dst_paths, const std::string &base, const std::string &name);

		std::set<std::string> m_fetched;
};

#endif // GAMEONDEMANDSOUNDFETCHER_HPP_
