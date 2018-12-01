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
#ifndef SMOOTHTRANSLATOR_HPP_
#define SMOOTHTRANSLATOR_HPP_

#include "irrlicht/irrlichttypes.h"
#include "irrlicht/irr_v3d.h"

template<typename T>
struct SmoothTranslator
{
	SmoothTranslator() = default;

	void init(T current);

	void update(T new_target, bool is_end_position = false, float update_interval = -1);

	void translate(f32 dtime);

	T val_old;
	T val_current;
	T val_target;
	f32 anim_time = 0;
	f32 anim_time_counter = 0;
	bool aim_is_end = true;

};

#include "client/object/SmoothTranslator.inl"

#endif // SMOOTHTRANSLATOR_HPP_
