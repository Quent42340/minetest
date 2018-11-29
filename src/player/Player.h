/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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
#ifndef PLAYER_H_
#define PLAYER_H_

#include <mutex>
#include "inventory.h"
#include "player/PlayerControl.h"
#include "player/PlayerSettings.h"
#include "util/basic_macros.h" // for DISABLE_CLASS_COPY

#define PLAYERNAME_SIZE 20

// FIXME: Use a regex
#define PLAYERNAME_ALLOWED_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_"
#define PLAYERNAME_ALLOWED_CHARS_USER_EXPL "'a' to 'z', 'A' to 'Z', '0' to '9', '-', '_'"

class Environment;
class Map;

struct CollisionInfo;
struct HudElement;

class Player {
	public:
		Player(const char *name, IItemDefManager *idef);
		virtual ~Player() = 0;

		DISABLE_CLASS_COPY(Player);

		virtual void move(f32 dtime, Environment *env, f32 pos_max_d) {}
		virtual void move(f32 dtime, Environment *env, f32 pos_max_d, std::vector<CollisionInfo> *collision_info) {}

		const v3f &getSpeed() const { return m_speed; }
		void setSpeed(const v3f &speed) { m_speed = speed; }

		const char *getName() const { return m_name; }

		u32 getFreeHudID();

		HudElement* getHud(u32 id);
		u32         addHud(HudElement* hud);
		HudElement* removeHud(u32 id);
		void        clearHud();

		const PlayerControl& getPlayerControl() { return control; }
		PlayerSettings &getPlayerSettings() { return m_player_settings; }
		static void settingsChangedCallback(const std::string &name, void *data);

		v3f eye_offset_first;
		v3f eye_offset_third;

		Inventory inventory;

		f32 movement_acceleration_default;
		f32 movement_acceleration_air;
		f32 movement_acceleration_fast;
		f32 movement_speed_walk;
		f32 movement_speed_crouch;
		f32 movement_speed_fast;
		f32 movement_speed_climb;
		f32 movement_speed_jump;
		f32 movement_liquid_fluidity;
		f32 movement_liquid_fluidity_smooth;
		f32 movement_liquid_sink;
		f32 movement_gravity;

		v2s32 local_animations[4];
		float local_animation_speed;

		std::string inventory_formspec;
		std::string formspec_prepend;

		PlayerControl control;

		u32 keyPressed = 0;

		u32 hud_flags;
		s32 hud_hotbar_itemcount;

	protected:
		char m_name[PLAYERNAME_SIZE];
		v3f m_speed;

		std::vector<HudElement *> hud;

	private:
		// Protect some critical areas
		// hud for example can be modified by EmergeThread
		// and ServerThread
		std::mutex m_mutex;
		PlayerSettings m_player_settings;
};

#endif // PLAYER_H_
