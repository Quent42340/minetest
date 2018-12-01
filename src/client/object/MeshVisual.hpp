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
#ifndef MESHVISUAL_HPP_
#define MESHVISUAL_HPP_

#include <IAnimatedMeshSceneNode.h>

#include "client/object/IGenericCAOVisual.hpp"

class MeshVisual : public IGenericCAOVisual {
	public:
		void init(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type,
				const ObjectProperties &prop, Client *client, u8 last_light, bool is_player) override;

		void updateTexture(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type,
				const ObjectProperties &prop, const std::string &mod) override;

		void setColor(video::SColor color) override;

		void removeSceneNode() override;

		scene::ISceneNode *node() override { return m_animated_meshnode; }

	private:
		scene::IAnimatedMeshSceneNode *m_animated_meshnode = nullptr;
};

#endif // MESHVISUAL_HPP_
