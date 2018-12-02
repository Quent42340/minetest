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

#include "client/object/SpriteVisual.hpp"
#include "client/RenderingEngine.hpp"
#include "client/tile.h"
#include "core/log.h"
#include "core/settings.h"
#include "common/world/object_properties.h"

#include "client/object/GenericCAOAnimation.hpp"

void SpriteVisual::init(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type,
		const ObjectProperties &prop, Client *client, u8 last_light, bool)
{
	if (m_spritenode) removeSceneNode();

	infostream << "GenericCAO::addToScene(): single_sprite" << std::endl;

	m_spritenode = RenderingEngine::get_scene_manager()->addBillboardSceneNode(
			nullptr, v2f(1, 1), v3f(0,0,0), -1);
	m_spritenode->grab();
	m_spritenode->setMaterialTexture(0, tsrc->getTextureForMesh("unknown_node.png"));
	m_spritenode->setMaterialFlag(video::EMF_LIGHTING, false);
	m_spritenode->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
	m_spritenode->setMaterialType(material_type);
	m_spritenode->setMaterialFlag(video::EMF_FOG_ENABLE, true);
	m_spritenode->setColor(video::SColor(255, last_light, last_light, last_light));
	m_spritenode->setSize(prop.visual_size * BS);

	{
		const float txs = 1.0 / 1;
		const float tys = 1.0 / 1;
		GenericCAOAnimation::setBillboardTextureMatrix(m_spritenode,
				txs, tys, 0, 0);
	}
}

void SpriteVisual::updateTexture(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type,
		const ObjectProperties &prop, const std::string &mod)
{
	if (!m_spritenode) return;

	std::string texturestring = "unknown_node.png";
	if (!prop.textures.empty())
		texturestring = prop.textures[0];
	texturestring += mod;

	m_spritenode->getMaterial(0).MaterialType = material_type;
	m_spritenode->setMaterialTexture(0, tsrc->getTextureForMesh(texturestring));

	// This allows setting per-material colors. However, until a real lighting
	// system is added, the code below will have no effect. Once MineTest
	// has directional lighting, it should work automatically.
	if (!prop.colors.empty()) {
		m_spritenode->getMaterial(0).AmbientColor = prop.colors[0];
		m_spritenode->getMaterial(0).DiffuseColor = prop.colors[0];
		m_spritenode->getMaterial(0).SpecularColor = prop.colors[0];
	}

	bool use_trilinear_filter = g_settings->getBool("trilinear_filter");
	bool use_bilinear_filter = g_settings->getBool("bilinear_filter");
	bool use_anisotropic_filter = g_settings->getBool("anisotropic_filter");

	m_spritenode->getMaterial(0).setFlag(video::EMF_TRILINEAR_FILTER, use_trilinear_filter);
	m_spritenode->getMaterial(0).setFlag(video::EMF_BILINEAR_FILTER, use_bilinear_filter);
	m_spritenode->getMaterial(0).setFlag(video::EMF_ANISOTROPIC_FILTER, use_anisotropic_filter);
}

void SpriteVisual::setColor(video::SColor color) {
	if (!m_spritenode) return;

	m_spritenode->setColor(color);
}

void SpriteVisual::removeSceneNode() {
	if (!m_spritenode) return;

	m_spritenode->remove();
	m_spritenode->drop();
	m_spritenode = nullptr;
}

