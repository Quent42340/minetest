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

#include "client/game/NodeDugEvent.h"
#include "client/game/SoundMaker.h"

void SoundMaker::playPlayerStep() {
	if (m_player_step_timer <= 0 && m_player_step_sound.exists()) {
		m_player_step_timer = 0.03;
		if (makes_footstep_sound)
			m_sound->playSound(m_player_step_sound, false);
	}
}

void SoundMaker::registerReceiver(MtEventManager *mgr) {
	mgr->reg(MtEvent::VIEW_BOBBING_STEP, SoundMaker::viewBobbingStep, this);
	mgr->reg(MtEvent::PLAYER_REGAIN_GROUND, SoundMaker::playerRegainGround, this);
	mgr->reg(MtEvent::PLAYER_JUMP, SoundMaker::playerJump, this);
	mgr->reg(MtEvent::CAMERA_PUNCH_LEFT, SoundMaker::cameraPunchLeft, this);
	mgr->reg(MtEvent::CAMERA_PUNCH_RIGHT, SoundMaker::cameraPunchRight, this);
	mgr->reg(MtEvent::NODE_DUG, SoundMaker::nodeDug, this);
	mgr->reg(MtEvent::PLAYER_DAMAGE, SoundMaker::playerDamage, this);
	mgr->reg(MtEvent::PLAYER_FALLING_DAMAGE, SoundMaker::playerFallingDamage, this);
}

void SoundMaker::step(float dtime) {
	m_player_step_timer -= dtime;
}

void SoundMaker::viewBobbingStep(MtEvent *e, void *data) {
	SoundMaker *sm = (SoundMaker *)data;
	sm->playPlayerStep();
}

void SoundMaker::playerRegainGround(MtEvent *e, void *data) {
	SoundMaker *sm = (SoundMaker *)data;
	sm->playPlayerStep();
}

void SoundMaker::playerJump(MtEvent *e, void *data) {
	// FIXME: Useless
	//SoundMaker *sm = (SoundMaker*)data;
}

void SoundMaker::cameraPunchLeft(MtEvent *e, void *data) {
	SoundMaker *sm = (SoundMaker *)data;
	sm->m_sound->playSound(sm->m_player_leftpunch_sound, false);
}

void SoundMaker::cameraPunchRight(MtEvent *e, void *data) {
	SoundMaker *sm = (SoundMaker *)data;
	sm->m_sound->playSound(sm->m_player_rightpunch_sound, false);
}

void SoundMaker::nodeDug(MtEvent *e, void *data) {
	// FIXME: Use an union
	SoundMaker *sm = (SoundMaker *)data;
	NodeDugEvent *nde = (NodeDugEvent *)e;
	sm->m_sound->playSound(sm->m_ndef->get(nde->n).sound_dug, false);
}

void SoundMaker::playerDamage(MtEvent *e, void *data) {
	SoundMaker *sm = (SoundMaker *)data;
	sm->m_sound->playSound(SimpleSoundSpec("player_damage", 0.5), false);
}

void SoundMaker::playerFallingDamage(MtEvent *e, void *data) {
	SoundMaker *sm = (SoundMaker *)data;
	sm->m_sound->playSound(SimpleSoundSpec("player_falling_damage", 0.5), false);
}

