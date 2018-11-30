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
#ifndef SOUNDMAKER_H_
#define SOUNDMAKER_H_

#include "world/nodedef.h"

#if USE_SOUND
	#include "client/sound_openal.h"
#else
	#include "client/sound.h"
#endif

class MtEvent;
class MtEventManager;

class SoundMaker {
	public:
		SoundMaker(ISoundManager *sound, const NodeDefManager *ndef)
			: m_sound(sound), m_ndef(ndef) {}

		void playPlayerStep();

		void registerReceiver(MtEventManager *mgr);

		void step(float dtime);

		static void viewBobbingStep(MtEvent *e, void *data);
		static void playerRegainGround(MtEvent *e, void *data);
		static void playerJump(MtEvent *e, void *data);
		static void cameraPunchLeft(MtEvent *e, void *data);
		static void cameraPunchRight(MtEvent *e, void *data);
		static void nodeDug(MtEvent *e, void *data);
		static void playerDamage(MtEvent *e, void *data);
		static void playerFallingDamage(MtEvent *e, void *data);

		bool makes_footstep_sound = true;
		float m_player_step_timer = 0;

		SimpleSoundSpec m_player_step_sound;
		SimpleSoundSpec m_player_leftpunch_sound;
		SimpleSoundSpec m_player_rightpunch_sound;

	private:
		ISoundManager *m_sound;
		const NodeDefManager *m_ndef;
};

#endif // SOUNDMAKER_H_
