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
#include "client/object/UprightSpriteVisual.hpp"
#include "client/renderingengine.h"
#include "client/tile.h"
#include "common/world/object_properties.h"
#include "core/constants.h"
#include "core/settings.h"

void UprightSpriteVisual::init(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type,
		const ObjectProperties &prop, Client *client, u8 last_light, bool is_player)
{
	if (m_meshnode) removeSceneNode();

	scene::SMesh *mesh = new scene::SMesh();
	double dx = BS * prop.visual_size.X / 2;
	double dy = BS * prop.visual_size.Y / 2;
	u8 li = last_light;
	video::SColor c(255, li, li, li);

	{ // Front
		scene::IMeshBuffer *buf = new scene::SMeshBuffer();
		video::S3DVertex vertices[4] = {
			video::S3DVertex(-dx, -dy, 0, 0,0,0, c, 1,1),
			video::S3DVertex( dx, -dy, 0, 0,0,0, c, 0,1),
			video::S3DVertex( dx,  dy, 0, 0,0,0, c, 0,0),
			video::S3DVertex(-dx,  dy, 0, 0,0,0, c, 1,0),
		};

		if (is_player) {
			// Move minimal Y position to 0 (feet position)
			for (video::S3DVertex &vertex : vertices)
				vertex.Pos.Y += dy;
		}
		u16 indices[] = {0,1,2,2,3,0};
		buf->append(vertices, 4, indices, 6);
		// Set material
		buf->getMaterial().setFlag(video::EMF_LIGHTING, false);
		buf->getMaterial().setFlag(video::EMF_BILINEAR_FILTER, false);
		buf->getMaterial().setFlag(video::EMF_FOG_ENABLE, true);
		buf->getMaterial().MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
		// Add to mesh
		mesh->addMeshBuffer(buf);
		buf->drop();
	}
	{ // Back
		scene::IMeshBuffer *buf = new scene::SMeshBuffer();
		video::S3DVertex vertices[4] = {
			video::S3DVertex( dx,-dy, 0, 0,0,0, c, 1,1),
			video::S3DVertex(-dx,-dy, 0, 0,0,0, c, 0,1),
			video::S3DVertex(-dx, dy, 0, 0,0,0, c, 0,0),
			video::S3DVertex( dx, dy, 0, 0,0,0, c, 1,0),
		};
		if (is_player) {
			// Move minimal Y position to 0 (feet position)
			for (video::S3DVertex &vertex : vertices)
				vertex.Pos.Y += dy;
		}
		u16 indices[] = {0,1,2,2,3,0};
		buf->append(vertices, 4, indices, 6);
		// Set material
		buf->getMaterial().setFlag(video::EMF_LIGHTING, false);
		buf->getMaterial().setFlag(video::EMF_BILINEAR_FILTER, false);
		buf->getMaterial().setFlag(video::EMF_FOG_ENABLE, true);
		buf->getMaterial().MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
		// Add to mesh
		mesh->addMeshBuffer(buf);
		buf->drop();
	}
	m_meshnode = RenderingEngine::get_scene_manager()->addMeshSceneNode(mesh, nullptr);
	m_meshnode->grab();
	mesh->drop();
	// Set it to use the materials of the meshbuffers directly.
	// This is needed for changing the texture in the future
	m_meshnode->setReadOnlyMaterials(true);
}

void UprightSpriteVisual::updateTexture(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type,
		const ObjectProperties &prop, const std::string &mod)
{
	if (!m_meshnode) return;

	bool use_trilinear_filter = g_settings->getBool("trilinear_filter");
	bool use_bilinear_filter = g_settings->getBool("bilinear_filter");
	bool use_anisotropic_filter = g_settings->getBool("anisotropic_filter");

	scene::IMesh *mesh = m_meshnode->getMesh();
	{
		std::string tname = "unknown_object.png";
		if (!prop.textures.empty())
			tname = prop.textures[0];
		tname += mod;

		scene::IMeshBuffer *buf = mesh->getMeshBuffer(0);
		buf->getMaterial().setTexture(0, tsrc->getTextureForMesh(tname));

		// This allows setting per-material colors. However, until a real lighting
		// system is added, the code below will have no effect. Once MineTest
		// has directional lighting, it should work automatically.
		if(!prop.colors.empty()) {
			buf->getMaterial().AmbientColor = prop.colors[0];
			buf->getMaterial().DiffuseColor = prop.colors[0];
			buf->getMaterial().SpecularColor = prop.colors[0];
		}

		buf->getMaterial().setFlag(video::EMF_TRILINEAR_FILTER, use_trilinear_filter);
		buf->getMaterial().setFlag(video::EMF_BILINEAR_FILTER, use_bilinear_filter);
		buf->getMaterial().setFlag(video::EMF_ANISOTROPIC_FILTER, use_anisotropic_filter);
	}
	{
		std::string tname = "unknown_object.png";
		if (prop.textures.size() >= 2)
			tname = prop.textures[1];
		else if (!prop.textures.empty())
			tname = prop.textures[0];
		tname += mod;

		scene::IMeshBuffer *buf = mesh->getMeshBuffer(1);
		buf->getMaterial().setTexture(0,
				tsrc->getTextureForMesh(tname));

		// This allows setting per-material colors. However, until a real lighting
		// system is added, the code below will have no effect. Once MineTest
		// has directional lighting, it should work automatically.
		if (prop.colors.size() >= 2) {
			buf->getMaterial().AmbientColor = prop.colors[1];
			buf->getMaterial().DiffuseColor = prop.colors[1];
			buf->getMaterial().SpecularColor = prop.colors[1];
			setMeshColor(mesh, prop.colors[1]);
		} else if (!prop.colors.empty()) {
			buf->getMaterial().AmbientColor = prop.colors[0];
			buf->getMaterial().DiffuseColor = prop.colors[0];
			buf->getMaterial().SpecularColor = prop.colors[0];
			setMeshColor(mesh, prop.colors[0]);
		}

		buf->getMaterial().setFlag(video::EMF_TRILINEAR_FILTER, use_trilinear_filter);
		buf->getMaterial().setFlag(video::EMF_BILINEAR_FILTER, use_bilinear_filter);
		buf->getMaterial().setFlag(video::EMF_ANISOTROPIC_FILTER, use_anisotropic_filter);
	}
}

void UprightSpriteVisual::setColor(video::SColor color) {
	if (!m_meshnode) return;

	setMeshColor(m_meshnode->getMesh(), color);
}

void UprightSpriteVisual::removeSceneNode() {
	if (!m_meshnode) return;

	m_meshnode->remove();
	m_meshnode->drop();
	m_meshnode = nullptr;
}

