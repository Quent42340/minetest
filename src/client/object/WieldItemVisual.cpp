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

#include "client/client.h"
#include "client/object/WieldItemVisual.hpp"
#include "client/RenderingEngine.hpp"
#include "core/log.h"
#include "common/inventory/inventory.h"
#include "common/world/object_properties.h"

void WieldItemVisual::init(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type,
		const ObjectProperties &prop, Client *client, u8 last_light, bool is_player)
{
	if (m_wield_meshnode) removeSceneNode();

	ItemStack item;
	infostream << "GenericCAO::addToScene(): wielditem" << std::endl;
	if (prop.wield_item.empty()) {
		// Old format, only textures are specified.
		infostream << "textures: " << prop.textures.size() << std::endl;
		if (!prop.textures.empty()) {
			infostream << "textures[0]: " << prop.textures[0] << std::endl;

			IItemDefManager *idef = client->idef();
			item = ItemStack(prop.textures[0], 1, 0, idef);
		}
	}
	else {
		infostream << "serialized form: " << prop.wield_item << std::endl;
		item.deSerialize(prop.wield_item, client->idef());
	}

	m_wield_meshnode = new WieldMeshSceneNode(RenderingEngine::get_scene_manager(), -1);
	m_wield_meshnode->setItem(item, client);
	m_wield_meshnode->setScale(
			v3f(prop.visual_size.X / 2, prop.visual_size.Y / 2,
				prop.visual_size.X / 2));

	u8 li = last_light;
	m_wield_meshnode->setColor(video::SColor(255, li, li, li));
}

void WieldItemVisual::updateTexture(ITextureSource *, video::E_MATERIAL_TYPE,
		const ObjectProperties &, const std::string &)
{
}

void WieldItemVisual::setColor(video::SColor color)
{
	if (!m_wield_meshnode) return;

	m_wield_meshnode->setColor(color);
}

void WieldItemVisual::removeSceneNode()
{
	if (!m_wield_meshnode) return;

	m_wield_meshnode->remove();
	m_wield_meshnode->drop();
	m_wield_meshnode = nullptr;
}

