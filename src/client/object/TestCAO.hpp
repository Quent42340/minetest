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
#ifndef TESTCAO_HPP_
#define TESTCAO_HPP_

#include "client/clientobject.h"

class TestCAO : public ClientActiveObject {
	public:
		TestCAO(Client *client, ClientEnvironment *env);
		virtual ~TestCAO() = default;

		ActiveObjectType getType() const { return ACTIVEOBJECT_TYPE_TEST; }

		void addToScene(ITextureSource *tsrc);
		void removeFromScene(bool permanent);
		void updateLight(u8 light_at_pos);
		v3s16 getLightPosition();
		void updateNodePos();

		void step(float dtime, ClientEnvironment *env);

		void processMessage(const std::string &data);

		bool getCollisionBox(aabb3f *toset) const { return false; }

		static ClientActiveObject* create(Client *client, ClientEnvironment *env);

	private:
		scene::IMeshSceneNode *m_node;
		v3f m_position;
};

#endif // TESTCAO_HPP_
