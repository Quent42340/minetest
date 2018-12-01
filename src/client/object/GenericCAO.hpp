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
#ifndef GENERICCAO_HPP_
#define GENERICCAO_HPP_

#include "core/constants.h"
#include "client/camera.h"
#include "client/clientobject.h"
#include "client/object/GenericCAOAnimation.hpp"
#include "client/object/SmoothTranslator.hpp"
#include "common/world/object_properties.h"

class GenericCAO : public ClientActiveObject {
	public:
		GenericCAO(Client *client, ClientEnvironment *env);
		~GenericCAO();

		void initialize(const std::string &data) override;

		void processInitData(const std::string &data);

		bool collideWithObjects() const override;

		bool getCollisionBox(aabb3f *toset) const override;
		bool getSelectionBox(aabb3f *toset) const override;

		scene::ISceneNode *getSceneNode() override;
		scene::IAnimatedMeshSceneNode *getAnimatedMeshSceneNode() override;

		v3f getPosition() override;
		const v3f &getRotation() { return m_rotation; }

		ActiveObjectType getType() const override { return ACTIVEOBJECT_TYPE_GENERIC; }
		const ItemGroupList &getGroups() const { return m_armor_groups; }

		bool isImmortal();

		f32 getStepHeight() const { return m_prop.stepheight; }

		bool isLocalPlayer() const override { return m_is_local_player; }

		ClientActiveObject *getParent() const override;

		void removeFromScene(bool permanent) override;

		void addToScene(ITextureSource *tsrc) override;

		void updateLight(u8 light_at_pos) override;
		void updateLightNoCheck(u8 light_at_pos) override;
		v3s16 getLightPosition() override;

		void step(float dtime, ClientEnvironment *env) override;

		void processMessage(const std::string &data) override;

		bool directReportPunch(v3f dir, const ItemStack *punchitem = nullptr,
				float time_from_last_punch = 1000000) override;

		std::string debugInfoText() override;
		std::string infoText() override { return m_prop.infotext; }

		void setAttachments() override;

		bool isVisible() const { return m_is_visible; }
		void setVisible(bool toset) { m_is_visible = toset; }

		void setChildrenVisible(bool toset);

		void expireVisuals() { m_visuals_expired = true; }

		// std::string copy is mandatory as mod can be a class member and there is a swap
		// on those class members... do NOT pass by reference
		void updateTextures(std::string mod);
		void updateNodePos();
		void updateBonePosition();
		void updateAttachments();

		static ClientActiveObject* create(Client *client, ClientEnvironment *env)
		{
			return new GenericCAO(client, env);
		}

	private:
		// Only set at initialization
		std::string m_name = "";
		bool m_is_player = false;
		bool m_is_local_player = false;

		// Property-ish things
		ObjectProperties m_prop;

		// Irrlicht
		scene::ISceneManager *m_smgr = nullptr;
		scene::IMeshSceneNode *m_meshnode = nullptr;
		scene::IAnimatedMeshSceneNode *m_animated_meshnode = nullptr;
		scene::IBillboardSceneNode *m_spritenode = nullptr;
		WieldMeshSceneNode *m_wield_meshnode = nullptr;

		aabb3f m_selection_box = aabb3f(-BS/3.,-BS/3.,-BS/3., BS/3.,BS/3.,BS/3.);

		Client *m_client = nullptr;
		Nametag *m_nametag = nullptr;

		v3f m_position = v3f(0.0f, 10.0f * BS, 0);
		v3f m_velocity;
		v3f m_acceleration;
		v3f m_rotation;

		s16 m_hp = 1;

		SmoothTranslator<v3f> pos_translator;
		SmoothTranslatorWrappedv3f rot_translator;

		GenericCAOAnimation m_animation;

		// stores position and rotation for each bone name
		std::unordered_map<std::string, core::vector2d<v3f>> m_bone_position;
		std::string m_attachment_bone = "";
		v3f m_attachment_position;
		v3f m_attachment_rotation;
		bool m_attached_to_local = false;

		ItemGroupList m_armor_groups;

		float m_reset_textures_timer = -1.0f;
		// stores texture modifier before punch update
		std::string m_previous_texture_modifier = "";
		// last applied texture modifier
		std::string m_current_texture_modifier = "";

		bool m_visuals_expired = false;
		float m_step_distance_counter = 0.0f;
		u8 m_last_light = 255;
		bool m_is_visible = false;
		s8 m_glow = 0;

		std::vector<u16> m_children;
};

#endif // GENERICCAO_HPP_
