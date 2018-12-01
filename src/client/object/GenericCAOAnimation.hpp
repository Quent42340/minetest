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
#ifndef GENERICCAOANIMATION_HPP_
#define GENERICCAOANIMATION_HPP_

#include <istream>
#include "irrlicht/irrlichttypes_extrabloated.h"

class Client;
class ClientEnvironment;
class LocalPlayer;
struct ObjectProperties;

class GenericCAOAnimation {
	public:
		void update(scene::IAnimatedMeshSceneNode *animated_meshnode);
		void updateSpeed(float speed, scene::IAnimatedMeshSceneNode *animated_meshnode);

		void updateData(std::istream &is, ClientEnvironment *env, bool is_local_player, scene::IAnimatedMeshSceneNode *animated_meshnode);
		void updateFrame(float dtime);

		void animatePlayer(Client *client, LocalPlayer *player, scene::IAnimatedMeshSceneNode *animated_meshnode);

		void initTiles(const ObjectProperties &prop);
		void updateTiles(const v2s16 &pos, int frame_count, int frame_length, bool select_horiz_by_yawpitch);

		void updateTexturePos(scene::IBillboardSceneNode *spritenode, const v3f &rotation);

		static void setBillboardTextureMatrix(scene::IBillboardSceneNode *bill,
				float txs, float tys, int col, int row);
	private:
		v2f m_tx_size = v2f(1,1);
		v2s16 m_tx_basepos;
		bool m_initial_tx_basepos_set = false;
		bool m_tx_select_horiz_by_yawpitch = false;
		v2s32 m_animation_range;

		float m_animation_speed = 15.0f;
		float m_animation_blend = 0.0f;
		bool m_animation_loop = true;

		int m_anim_frame = 0;
		int m_anim_num_frames = 1;
		float m_anim_framelength = 0.2f;
		float m_anim_timer = 0.0f;
};

#endif // GENERICCAOANIMATION_HPP_
