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

#include "client/mesh.h"
#include "client/object/CubeVisual.hpp"
#include "client/RenderingEngine.hpp"
#include "client/tile.h"
#include "common/world/object_properties.h"
#include "core/log.h"
#include "core/settings.h"

void CubeVisual::init(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type,
		const ObjectProperties &prop, Client *client, u8 last_light, bool is_player)
{
	infostream << "GenericCAO::addToScene(): cube" << std::endl;

	scene::IMesh *mesh = createCubeMesh(v3f(BS,BS,BS));
	m_meshnode = RenderingEngine::get_scene_manager()->addMeshSceneNode(mesh, NULL);
	m_meshnode->grab();
	mesh->drop();

	m_meshnode->setScale(v3f(prop.visual_size.X,
				prop.visual_size.Y,
				prop.visual_size.X));

	u8 li = last_light;
	setMeshColor(m_meshnode->getMesh(), video::SColor(255,li,li,li));

	m_meshnode->setMaterialFlag(video::EMF_LIGHTING, false);
	m_meshnode->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
	m_meshnode->setMaterialType(material_type);
	m_meshnode->setMaterialFlag(video::EMF_FOG_ENABLE, true);
}

void CubeVisual::updateTexture(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type,
		const ObjectProperties &prop, const std::string &mod)
{
	for (u32 i = 0; i < 6; ++i)
	{
		std::string texturestring = "unknown_node.png";
		if(prop.textures.size() > i)
			texturestring = prop.textures[i];
		texturestring += mod;


		// Set material flags and texture
		video::SMaterial& material = m_meshnode->getMaterial(i);
		material.MaterialType = material_type;
		material.setFlag(video::EMF_LIGHTING, false);
		material.setFlag(video::EMF_BILINEAR_FILTER, false);
		material.setTexture(0, tsrc->getTextureForMesh(texturestring));
		material.getTextureMatrix(0).makeIdentity();

		// This allows setting per-material colors. However, until a real lighting
		// system is added, the code below will have no effect. Once MineTest
		// has directional lighting, it should work automatically.
		if(prop.colors.size() > i)
		{
			m_meshnode->getMaterial(i).AmbientColor = prop.colors[i];
			m_meshnode->getMaterial(i).DiffuseColor = prop.colors[i];
			m_meshnode->getMaterial(i).SpecularColor = prop.colors[i];
		}

		bool use_trilinear_filter = g_settings->getBool("trilinear_filter");
		bool use_bilinear_filter = g_settings->getBool("bilinear_filter");
		bool use_anisotropic_filter = g_settings->getBool("anisotropic_filter");

		m_meshnode->getMaterial(i).setFlag(video::EMF_TRILINEAR_FILTER, use_trilinear_filter);
		m_meshnode->getMaterial(i).setFlag(video::EMF_BILINEAR_FILTER, use_bilinear_filter);
		m_meshnode->getMaterial(i).setFlag(video::EMF_ANISOTROPIC_FILTER, use_anisotropic_filter);
	}
}

void CubeVisual::setColor(video::SColor color) {
	if (!m_meshnode) return;

	setMeshColor(m_meshnode->getMesh(), color);
}

void CubeVisual::removeSceneNode() {
	if (!m_meshnode) return;

	m_meshnode->remove();
	m_meshnode->drop();
	m_meshnode = nullptr;
}

