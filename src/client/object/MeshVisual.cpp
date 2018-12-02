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
#include "client/client.h"
#include "client/object/MeshVisual.hpp"
#include "client/RenderingEngine.hpp"
#include "client/tile.h"
#include "common/world/object_properties.h"
#include "core/log.h"
#include "core/settings.h"

void MeshVisual::init(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type,
		const ObjectProperties &prop, Client *client, u8 last_light, bool is_player)
{
	if (m_animated_meshnode) removeSceneNode();

	infostream << "GenericCAO::addToScene(): mesh" << std::endl;

	scene::IAnimatedMesh *mesh = client->getMesh(prop.mesh, true);

	if (mesh)
	{
		m_animated_meshnode = RenderingEngine::get_scene_manager()->
			addAnimatedMeshSceneNode(mesh, nullptr);
		m_animated_meshnode->grab();

		mesh->drop(); // The scene node took hold of it

		m_animated_meshnode->animateJoints(); // Needed for some animations
		m_animated_meshnode->setScale(v3f(prop.visual_size.X,
					prop.visual_size.Y,
					prop.visual_size.X));

		u8 li = last_light;
		// set vertex colors to ensure alpha is set
		setMeshColor(m_animated_meshnode->getMesh(), video::SColor(255,li,li,li));
		setAnimatedMeshColor(m_animated_meshnode, video::SColor(255,li,li,li));

		m_animated_meshnode->setMaterialFlag(video::EMF_LIGHTING, true);
		m_animated_meshnode->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
		m_animated_meshnode->setMaterialType(material_type);
		m_animated_meshnode->setMaterialFlag(video::EMF_FOG_ENABLE, true);
		m_animated_meshnode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, prop.backface_culling);
	}
	else
		errorstream << "GenericCAO::addToScene(): Could not load mesh " << prop.mesh << std::endl;
}

void MeshVisual::updateTexture(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type,
		const ObjectProperties &prop, const std::string &mod)
{
	if (!m_animated_meshnode) return;

	for (u32 i = 0; i < prop.textures.size() && i < m_animated_meshnode->getMaterialCount(); ++i)
	{
		std::string texturestring = prop.textures[i];
		if (texturestring.empty())
			continue; // Empty texture string means don't modify that material
		texturestring += mod;

		video::ITexture* texture = tsrc->getTextureForMesh(texturestring);
		if (!texture) {
			errorstream<<"GenericCAO::updateTextures(): Could not load texture "<<texturestring<<std::endl;
			continue;
		}

		// Set material flags and texture
		video::SMaterial& material = m_animated_meshnode->getMaterial(i);
		material.MaterialType = material_type;
		material.TextureLayer[0].Texture = texture;
		material.setFlag(video::EMF_LIGHTING, true);
		material.setFlag(video::EMF_BILINEAR_FILTER, false);
		material.setFlag(video::EMF_BACK_FACE_CULLING, prop.backface_culling);

		bool use_trilinear_filter = g_settings->getBool("trilinear_filter");
		bool use_bilinear_filter = g_settings->getBool("bilinear_filter");
		bool use_anisotropic_filter = g_settings->getBool("anisotropic_filter");

		// don't filter low-res textures, makes them look blurry
		// player models have a res of 64
		const core::dimension2d<u32> &size = texture->getOriginalSize();
		const u32 res = std::min(size.Height, size.Width);
		use_trilinear_filter &= res > 64;
		use_bilinear_filter &= res > 64;

		m_animated_meshnode->getMaterial(i).setFlag(video::EMF_TRILINEAR_FILTER,   use_trilinear_filter);
		m_animated_meshnode->getMaterial(i).setFlag(video::EMF_BILINEAR_FILTER,    use_bilinear_filter);
		m_animated_meshnode->getMaterial(i).setFlag(video::EMF_ANISOTROPIC_FILTER, use_anisotropic_filter);
	}

	for (u32 i = 0; i < prop.colors.size() && i < m_animated_meshnode->getMaterialCount(); ++i)
	{
		// This allows setting per-material colors. However, until a real lighting
		// system is added, the code below will have no effect. Once MineTest
		// has directional lighting, it should work automatically.
		m_animated_meshnode->getMaterial(i).AmbientColor = prop.colors[i];
		m_animated_meshnode->getMaterial(i).DiffuseColor = prop.colors[i];
		m_animated_meshnode->getMaterial(i).SpecularColor = prop.colors[i];
	}
}

void MeshVisual::setColor(video::SColor color) {
	if (!m_animated_meshnode) return;

	setAnimatedMeshColor(m_animated_meshnode, color);
}

void MeshVisual::removeSceneNode() {
	if (!m_animated_meshnode) return;

	m_animated_meshnode->remove();
	m_animated_meshnode->drop();
	m_animated_meshnode = nullptr;
}

