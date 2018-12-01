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

#include <algorithm>

#include <IAnimatedMeshSceneNode.h>
#include <IBillboardSceneNode.h>

#include "client/client.h"
#include "client/ClientEnvironment.h"
#include "client/content_cso.h"
#include "client/localplayer.h"
#include "client/mesh.h"
#include "client/nodeentity.h"
#include "client/object/GenericCAO.hpp"
#include "client/renderingengine.h"
#include "client/sound.h"
#include "client/wieldmesh.h"
#include "core/constants.h"
#include "core/log.h"
#include "common/genericobject.h"
#include "util/serialize.h"
#include "common/world/collision.h"
#include "common/world/node/NodeDefManager.hpp"

/*
	Other stuff
*/

/*
	GenericCAO
*/

// Prototype
GenericCAO proto_GenericCAO(nullptr, nullptr);

GenericCAO::GenericCAO(Client *client, ClientEnvironment *env) : ClientActiveObject(0, client, env)
{
	if (client == nullptr) {
		ClientActiveObject::registerType(getType(), create);
	} else {
		m_client = client;
	}
}

GenericCAO::~GenericCAO()
{
	removeFromScene(true);
}

bool GenericCAO::getCollisionBox(aabb3f *toset) const
{
	if (m_prop.physical)
	{
		//update collision box
		toset->MinEdge = m_prop.collisionbox.MinEdge * BS;
		toset->MaxEdge = m_prop.collisionbox.MaxEdge * BS;

		toset->MinEdge += m_position;
		toset->MaxEdge += m_position;

		return true;
	}

	return false;
}

bool GenericCAO::collideWithObjects() const
{
	return m_prop.collideWithObjects;
}

void GenericCAO::initialize(const std::string &data)
{
	infostream<<"GenericCAO: Got init data"<<std::endl;
	processInitData(data);

	if (m_is_player) {
		// Check if it's the current player
		LocalPlayer *player = m_env->getLocalPlayer();
		if (player && strcmp(player->getName(), m_name.c_str()) == 0) {
			m_is_local_player = true;
			m_is_visible = false;
			player->setCAO(this);
		}
	}
}

void GenericCAO::processInitData(const std::string &data)
{
	std::istringstream is(data, std::ios::binary);
	int num_messages = 0;
	// version
	u8 version = readU8(is);
	// check version
	if (version == 1) { // In PROTOCOL_VERSION 14
		m_name = deSerializeString(is);
		m_is_player = readU8(is);
		m_id = readU16(is);
		m_position = readV3F1000(is);
		m_rotation = readV3F1000(is);
		m_hp = readS16(is);
		num_messages = readU8(is);
	} else {
		errorstream<<"GenericCAO: Unsupported init data version"
				<<std::endl;
		return;
	}

	for (int i = 0; i < num_messages; i++) {
		std::string message = deSerializeLongString(is);
		processMessage(message);
	}

	m_rotation = wrapDegrees_0_360_v3f(m_rotation);
	pos_translator.init(m_position);
	rot_translator.init(m_rotation);
	updateNodePos();
}

bool GenericCAO::getSelectionBox(aabb3f *toset) const
{
	if (!m_prop.is_visible || !m_is_visible || m_is_local_player
			|| !m_prop.pointable) {
		return false;
	}
	*toset = m_selection_box;
	return true;
}

v3f GenericCAO::getPosition()
{
	if (getParent() != nullptr) {
		scene::ISceneNode *node = getSceneNode();
		if (node)
			return node->getAbsolutePosition();

		return m_position;
	}
	return pos_translator.val_current;
}

bool GenericCAO::isImmortal()
{
	return itemgroup_get(getGroups(), "immortal");
}

scene::ISceneNode* GenericCAO::getSceneNode()
{
	if (m_cubeVisual.node()) {
		return m_cubeVisual.node();
	}

	if (m_meshVisual.node()) {
		return m_meshVisual.node();
	}

	if (m_uprightSpriteVisual.node()) {
		return m_uprightSpriteVisual.node();
	}

	if (m_spriteVisual.node()) {
		return m_spriteVisual.node();
	}

	if (m_wield_meshnode) {
		return m_wield_meshnode;
	}

	return nullptr;
}

scene::IAnimatedMeshSceneNode* GenericCAO::getAnimatedMeshSceneNode()
{
	return (scene::IAnimatedMeshSceneNode *)m_meshVisual.node();
}

void GenericCAO::setChildrenVisible(bool toset)
{
	for (u16 cao_id : m_children) {
		GenericCAO *obj = m_env->getGenericCAO(cao_id);
		if (obj) {
			obj->setVisible(toset);
		}
	}
}

void GenericCAO::setAttachments()
{
	updateAttachments();
}

ClientActiveObject* GenericCAO::getParent() const
{
	ClientActiveObject *obj = nullptr;

	u16 attached_id = m_env->attachement_parent_ids[getId()];

	if ((attached_id != 0) &&
			(attached_id != getId())) {
		obj = m_env->getActiveObject(attached_id);
	}
	return obj;
}

void GenericCAO::removeFromScene(bool permanent)
{
	// Should be true when removing the object permanently and false when refreshing (eg: updating visuals)
	if((m_env != nullptr) && (permanent))
	{
		for (u16 ci : m_children) {
			if (m_env->attachement_parent_ids[ci] == getId()) {
				m_env->attachement_parent_ids[ci] = 0;
			}
		}
		m_children.clear();

		m_env->attachement_parent_ids[getId()] = 0;

		LocalPlayer* player = m_env->getLocalPlayer();
		if (this == player->parent) {
			player->parent = nullptr;
			player->isAttached = false;
		}
	}

	m_cubeVisual.removeSceneNode();
	m_meshVisual.removeSceneNode();
	m_spriteVisual.removeSceneNode();
	m_uprightSpriteVisual.removeSceneNode();

	if (m_wield_meshnode) {
		m_wield_meshnode->remove();
		m_wield_meshnode->drop();
		m_wield_meshnode = nullptr;
	}

	if (m_nametag) {
		m_client->getCamera()->removeNametag(m_nametag);
		m_nametag = nullptr;
	}
}

void GenericCAO::addToScene(ITextureSource *tsrc)
{
	m_smgr = RenderingEngine::get_scene_manager();

	if (getSceneNode() != nullptr) {
		return;
	}

	m_visuals_expired = false;

	if (!m_prop.is_visible) {
		return;
	}

	video::E_MATERIAL_TYPE material_type = (m_prop.use_texture_alpha) ?
		video::EMT_TRANSPARENT_ALPHA_CHANNEL : video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;

	if (m_prop.visual == "sprite") {
		m_spriteVisual.init(tsrc, material_type, m_prop, m_client, m_last_light, m_is_player);
	}
	else if (m_prop.visual == "upright_sprite") {
		m_uprightSpriteVisual.init(tsrc, material_type, m_prop, m_client, m_last_light, m_is_player);
	}
	else if(m_prop.visual == "cube") {
		m_cubeVisual.init(tsrc, material_type, m_prop, m_client, m_last_light, m_is_player);
	}
	else if(m_prop.visual == "mesh") {
		m_meshVisual.init(tsrc, material_type, m_prop, m_client, m_last_light, m_is_player);
	}
	else if (m_prop.visual == "wielditem" || m_prop.visual == "dynamicnode") {
		initWielditemVisual(tsrc, material_type);
	}
	else {
		infostream<<"GenericCAO::addToScene(): \""<<m_prop.visual
				<<"\" not supported"<<std::endl;
	}

	/* don't update while punch texture modifier is active */
	if (m_reset_textures_timer < 0)
		updateTextures(m_current_texture_modifier);

	scene::ISceneNode *node = getSceneNode();
	if (node && !m_prop.nametag.empty() && !m_is_local_player) {
		// Add nametag
		v3f pos;
		pos.Y = m_prop.selectionbox.MaxEdge.Y + 0.3f;
		m_nametag = m_client->getCamera()->addNametag(node,
			m_prop.nametag, m_prop.nametag_color,
			pos);
	}

	updateNodePos();
	m_animation.update((scene::IAnimatedMeshSceneNode *)m_meshVisual.node());
	updateBonePosition();
	updateAttachments();
}

void GenericCAO::updateLight(u8 light_at_pos)
{
	// Don't update light of attached one
	if (getParent() != nullptr) {
		return;
	}

	updateLightNoCheck(light_at_pos);

	// Update light of all children
	for (u16 i : m_children) {
		ClientActiveObject *obj = m_env->getActiveObject(i);
		if (obj) {
			obj->updateLightNoCheck(light_at_pos);
		}
	}
}

void GenericCAO::updateLightNoCheck(u8 light_at_pos)
{
	if (m_glow < 0)
		return;

	u8 li = decode_light(light_at_pos + m_glow);
	if (li != m_last_light)	{
		m_last_light = li;
		video::SColor color(255, li, li, li);

		m_cubeVisual.setColor(color);
		m_uprightSpriteVisual.setColor(color);
		m_meshVisual.setColor(color);
		m_wield_meshnode->setColor(color);
		m_spriteVisual.setColor(color);
	}
}

v3s16 GenericCAO::getLightPosition()
{
	if (m_is_player)
		return floatToInt(m_position + v3f(0, 0.5 * BS, 0), BS);

	return floatToInt(m_position, BS);
}

void GenericCAO::updateNodePos()
{
	if (getParent() != nullptr)
		return;

	scene::ISceneNode *node = getSceneNode();

	if (node) {
		v3s16 camera_offset = m_env->getCameraOffset();
		node->setPosition(pos_translator.val_current - intToFloat(camera_offset, BS));
		if (node != m_spriteVisual.node()) { // rotate if not a sprite
			v3f rot = m_is_local_player ? -m_rotation : -rot_translator.val_current;
			node->setRotation(rot);
		}
	}
}

void GenericCAO::step(float dtime, ClientEnvironment *env)
{
	// Handle model of local player instantly to prevent lags
	if (m_is_local_player) {
		LocalPlayer *player = m_env->getLocalPlayer();
		if (m_is_visible) {
			m_position = player->getPosition();
			m_rotation.Y = wrapDegrees_0_360(player->getYaw());
			m_velocity = v3f(0, 0, 0);
			m_acceleration = v3f(0, 0, 0);
			pos_translator.val_current = m_position;
			rot_translator.val_current = m_rotation;

			m_animation.animatePlayer(m_client, player, (scene::IAnimatedMeshSceneNode *)m_meshVisual.node());
		}
	}

	if (m_visuals_expired && m_smgr) {
		m_visuals_expired = false;

		// Attachments, part 1: All attached objects must be unparented first,
		// or Irrlicht causes a segmentation fault
		for (auto ci = m_children.begin(); ci != m_children.end();) {
			if (m_env->attachement_parent_ids[*ci] != getId()) {
				ci = m_children.erase(ci);
				continue;
			}
			ClientActiveObject *obj = m_env->getActiveObject(*ci);
			if (obj) {
				scene::ISceneNode *child_node = obj->getSceneNode();
				if (child_node)
					child_node->setParent(m_smgr->getRootSceneNode());
			}
			++ci;
		}

		removeFromScene(false);
		addToScene(m_client->tsrc());

		// Attachments, part 2: Now that the parent has been refreshed, put its attachments back
		for (u16 cao_id : m_children) {
			// Get the object of the child
			ClientActiveObject *obj = m_env->getActiveObject(cao_id);
			if (obj)
				obj->setAttachments();
		}
	}

	// Make sure m_is_visible is always applied
	scene::ISceneNode *node = getSceneNode();
	if (node)
		node->setVisible(m_is_visible);

	if (getParent() != nullptr) // Attachments should be glued to their parent by Irrlicht
	{
		// Set these for later
		m_position = getPosition();
		m_velocity = v3f(0,0,0);
		m_acceleration = v3f(0,0,0);
		pos_translator.val_current = m_position;

		if(m_is_local_player) // Update local player attachment position
		{
			LocalPlayer *player = m_env->getLocalPlayer();
			player->overridePosition = getParent()->getPosition();
			m_env->getLocalPlayer()->parent = getParent();
		}
	} else {
		rot_translator.translate(dtime);
		v3f lastpos = pos_translator.val_current;

		if(m_prop.physical)
		{
			aabb3f box = m_prop.collisionbox;
			box.MinEdge *= BS;
			box.MaxEdge *= BS;
			collisionMoveResult moveresult;
			f32 pos_max_d = BS * 0.125; // Distance per iteration
			v3f p_pos = m_position;
			v3f p_velocity = m_velocity;
			moveresult = collisionMoveSimple(env,env->getGameDef(),
					pos_max_d, box, m_prop.stepheight, dtime,
					&p_pos, &p_velocity, m_acceleration,
					this, m_prop.collideWithObjects);
			// Apply results
			m_position = p_pos;
			m_velocity = p_velocity;

			bool is_end_position = moveresult.collides;
			pos_translator.update(m_position, is_end_position, dtime);
			pos_translator.translate(dtime);
			updateNodePos();
		} else {
			m_position += dtime * m_velocity + 0.5 * dtime * dtime * m_acceleration;
			m_velocity += dtime * m_acceleration;
			pos_translator.update(m_position, pos_translator.aim_is_end,
					pos_translator.anim_time);
			pos_translator.translate(dtime);
			updateNodePos();
		}

		float moved = lastpos.getDistanceFrom(pos_translator.val_current);
		m_step_distance_counter += moved;
		if (m_step_distance_counter > 1.5f * BS) {
			m_step_distance_counter = 0.0f;
			if (!m_is_local_player && m_prop.makes_footstep_sound) {
				const NodeDefManager *ndef = m_client->ndef();
				v3s16 p = floatToInt(getPosition() +
					v3f(0.0f, (m_prop.collisionbox.MinEdge.Y - 0.5f) * BS, 0.0f), BS);
				MapNode n = m_env->getMap().getNodeNoEx(p);
				SimpleSoundSpec spec = ndef->get(n).sound_footstep;
				// Reduce footstep gain, as non-local-player footsteps are
				// somehow louder.
				spec.gain *= 0.6f;
				m_client->sound()->playSoundAt(spec, false, getPosition());
			}
		}
	}

	m_animation.updateFrame(dtime);

	m_animation.updateTexturePos(dynamic_cast<scene::IBillboardSceneNode *>(m_spriteVisual.node()), m_rotation);

	if(m_reset_textures_timer >= 0)
	{
		m_reset_textures_timer -= dtime;
		if(m_reset_textures_timer <= 0) {
			m_reset_textures_timer = -1;
			updateTextures(m_previous_texture_modifier);
		}
	}
	if (!getParent() && std::fabs(m_prop.automatic_rotate) > 0.001) {
		m_rotation.Y += dtime * m_prop.automatic_rotate * 180 / M_PI;
		rot_translator.val_current = m_rotation;
		updateNodePos();
	}

	if (!getParent() && m_prop.automatic_face_movement_dir &&
			(fabs(m_velocity.Z) > 0.001 || fabs(m_velocity.X) > 0.001)) {

		float target_yaw = atan2(m_velocity.Z, m_velocity.X) * 180 / M_PI
				+ m_prop.automatic_face_movement_dir_offset;
		float max_rotation_delta =
				dtime * m_prop.automatic_face_movement_max_rotation_per_sec;

		wrappedApproachShortest(m_rotation.Y, target_yaw, max_rotation_delta, 360.f);
		rot_translator.val_current = m_rotation;

		updateNodePos();
	}
}

void GenericCAO::updateTextures(std::string mod)
{
	ITextureSource *tsrc = m_client->tsrc();

	m_previous_texture_modifier = m_current_texture_modifier;
	m_current_texture_modifier = mod;
	m_glow = m_prop.glow;

	video::E_MATERIAL_TYPE material_type = (m_prop.use_texture_alpha) ?
		video::EMT_TRANSPARENT_ALPHA_CHANNEL : video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;

	if (m_prop.visual == "sprite")
		m_spriteVisual.updateTexture(tsrc, material_type, m_prop, mod);
	else if (m_prop.visual == "mesh")
		m_meshVisual.updateTexture(tsrc, material_type, m_prop, mod);
	else if(m_prop.visual == "cube")
		m_cubeVisual.updateTexture(tsrc, material_type, m_prop, mod);
	else if (m_prop.visual == "upright_sprite")
		m_uprightSpriteVisual.updateTexture(tsrc, material_type, m_prop, mod);
}

void GenericCAO::updateBonePosition()
{
	if (m_bone_position.empty() || !m_meshVisual.node())
		return;

	scene::IAnimatedMeshSceneNode *animated_meshnode = (scene::IAnimatedMeshSceneNode *)m_meshVisual.node();
	animated_meshnode->setJointMode(irr::scene::EJUOR_CONTROL); // To write positions to the mesh on render
	for(std::unordered_map<std::string, core::vector2d<v3f>>::const_iterator
			ii = m_bone_position.begin(); ii != m_bone_position.end(); ++ii) {
		std::string bone_name = (*ii).first;
		v3f bone_pos = (*ii).second.X;
		v3f bone_rot = (*ii).second.Y;
		irr::scene::IBoneSceneNode* bone = animated_meshnode->getJointNode(bone_name.c_str());
		if(bone)
		{
			bone->setPosition(bone_pos);
			bone->setRotation(bone_rot);
		}
	}
}

void GenericCAO::updateAttachments()
{

	if (!getParent()) { // Detach or don't attach
		scene::ISceneNode *node = getSceneNode();
		if (node) {
			v3f old_position = node->getAbsolutePosition();
			v3f old_rotation = node->getRotation();
			node->setParent(m_smgr->getRootSceneNode());
			node->setPosition(old_position);
			node->setRotation(old_rotation);
			node->updateAbsolutePosition();
		}
		if (m_is_local_player) {
			LocalPlayer *player = m_env->getLocalPlayer();
			player->isAttached = false;
		}
	}
	else // Attach
	{
		scene::ISceneNode *my_node = getSceneNode();

		scene::ISceneNode *parent_node = getParent()->getSceneNode();
		scene::IAnimatedMeshSceneNode *parent_animated_mesh_node =
				getParent()->getAnimatedMeshSceneNode();
		if (parent_animated_mesh_node && !m_attachment_bone.empty()) {
			parent_node = parent_animated_mesh_node->getJointNode(m_attachment_bone.c_str());
		}

		if (my_node && parent_node) {
			my_node->setParent(parent_node);
			my_node->setPosition(m_attachment_position);
			my_node->setRotation(m_attachment_rotation);
			my_node->updateAbsolutePosition();
		}
		if (m_is_local_player) {
			LocalPlayer *player = m_env->getLocalPlayer();
			player->isAttached = true;
		}
	}
}

void GenericCAO::processMessage(const std::string &data)
{
	//infostream<<"GenericCAO: Got message"<<std::endl;
	std::istringstream is(data, std::ios::binary);
	// command
	u8 cmd = readU8(is);
	if (cmd == GENERIC_CMD_SET_PROPERTIES) {
		m_prop = gob_read_set_properties(is);

		m_selection_box = m_prop.selectionbox;
		m_selection_box.MinEdge *= BS;
		m_selection_box.MaxEdge *= BS;

		m_animation.initTiles(m_prop);

		if (m_is_local_player) {
			LocalPlayer *player = m_env->getLocalPlayer();
			player->makes_footstep_sound = m_prop.makes_footstep_sound;
			aabb3f collision_box = m_prop.collisionbox;
			collision_box.MinEdge *= BS;
			collision_box.MaxEdge *= BS;
			player->setCollisionbox(collision_box);
			player->setEyeHeight(m_prop.eye_height);
			player->setZoomFOV(m_prop.zoom_fov);
		}

		if ((m_is_player && !m_is_local_player) && m_prop.nametag.empty())
			m_prop.nametag = m_name;

		expireVisuals();
	} else if (cmd == GENERIC_CMD_UPDATE_POSITION) {
		// Not sent by the server if this object is an attachment.
		// We might however get here if the server notices the object being detached before the client.
		m_position = readV3F1000(is);
		m_velocity = readV3F1000(is);
		m_acceleration = readV3F1000(is);

		if (std::fabs(m_prop.automatic_rotate) < 0.001f)
			m_rotation = readV3F1000(is);
		else
			readV3F1000(is);

		m_rotation = wrapDegrees_0_360_v3f(m_rotation);
		bool do_interpolate = readU8(is);
		bool is_end_position = readU8(is);
		float update_interval = readF1000(is);

		// Place us a bit higher if we're physical, to not sink into
		// the ground due to sucky collision detection...
		if(m_prop.physical)
			m_position += v3f(0,0.002,0);

		if(getParent() != nullptr) // Just in case
			return;

		if(do_interpolate)
		{
			if(!m_prop.physical)
				pos_translator.update(m_position, is_end_position, update_interval);
		} else {
			pos_translator.init(m_position);
		}
		rot_translator.update(m_rotation, false, update_interval);
		updateNodePos();
	} else if (cmd == GENERIC_CMD_SET_TEXTURE_MOD) {
		std::string mod = deSerializeString(is);

		// immediatly reset a engine issued texture modifier if a mod sends a different one
		if (m_reset_textures_timer > 0) {
			m_reset_textures_timer = -1;
			updateTextures(m_previous_texture_modifier);
		}
		updateTextures(mod);
	} else if (cmd == GENERIC_CMD_SET_SPRITE) {
		v2s16 p = readV2S16(is);
		int num_frames = readU16(is);
		float framelength = readF1000(is);
		bool select_horiz_by_yawpitch = readU8(is);

		m_animation.updateTiles(p, num_frames, framelength, select_horiz_by_yawpitch);
		m_animation.updateTexturePos(dynamic_cast<scene::IBillboardSceneNode *>(m_spriteVisual.node()), m_rotation);
	}
	else if (cmd == GENERIC_CMD_SET_PHYSICS_OVERRIDE) {
		float override_speed = readF1000(is);
		float override_jump = readF1000(is);
		float override_gravity = readF1000(is);
		// these are sent inverted so we get true when the server sends nothing
		bool sneak = !readU8(is);
		bool sneak_glitch = !readU8(is);
		bool new_move = !readU8(is);


		if(m_is_local_player)
		{
			LocalPlayer *player = m_env->getLocalPlayer();
			player->physics_override_speed = override_speed;
			player->physics_override_jump = override_jump;
			player->physics_override_gravity = override_gravity;
			player->physics_override_sneak = sneak;
			player->physics_override_sneak_glitch = sneak_glitch;
			player->physics_override_new_move = new_move;
		}
	} else if (cmd == GENERIC_CMD_SET_ANIMATION) {
		m_animation.updateData(is, m_env, m_is_local_player, (scene::IAnimatedMeshSceneNode *)m_meshVisual.node());
	} else if (cmd == GENERIC_CMD_SET_ANIMATION_SPEED) {
		m_animation.updateSpeed(readF1000(is), (scene::IAnimatedMeshSceneNode *)m_meshVisual.node());
	} else if (cmd == GENERIC_CMD_SET_BONE_POSITION) {
		std::string bone = deSerializeString(is);
		v3f position = readV3F1000(is);
		v3f rotation = readV3F1000(is);
		m_bone_position[bone] = core::vector2d<v3f>(position, rotation);

		updateBonePosition();
	} else if (cmd == GENERIC_CMD_ATTACH_TO) {
		u16 parent_id = readS16(is);
		u16 &old_parent_id = m_env->attachement_parent_ids[getId()];
		if (parent_id != old_parent_id) {
			if (GenericCAO *old_parent = m_env->getGenericCAO(old_parent_id)) {
				old_parent->m_children.erase(std::remove(
					m_children.begin(), m_children.end(),
					getId()), m_children.end());
			}
			if (GenericCAO *new_parent = m_env->getGenericCAO(parent_id))
				new_parent->m_children.push_back(getId());

			old_parent_id = parent_id;
		}

		m_attachment_bone = deSerializeString(is);
		m_attachment_position = readV3F1000(is);
		m_attachment_rotation = readV3F1000(is);

		// localplayer itself can't be attached to localplayer
		if (!m_is_local_player) {
			m_attached_to_local = getParent() != nullptr && getParent()->isLocalPlayer();
			// Objects attached to the local player should be hidden by default
			m_is_visible = !m_attached_to_local;
		}

		updateAttachments();
	} else if (cmd == GENERIC_CMD_PUNCHED) {
		/*s16 damage =*/ readS16(is);
		s16 result_hp = readS16(is);

		// Use this instead of the send damage to not interfere with prediction
		s16 damage = m_hp - result_hp;

		m_hp = result_hp;

		if (damage > 0)
		{
			if (m_hp <= 0)
			{
				// TODO: Execute defined fast response
				// As there is no definition, make a smoke puff
				ClientSimpleObject *simple = createSmokePuff(
						m_smgr, m_env, m_position,
						m_prop.visual_size * BS);
				m_env->addSimpleObject(simple);
			} else if (m_reset_textures_timer < 0) {
				// TODO: Execute defined fast response
				// Flashing shall suffice as there is no definition
				m_reset_textures_timer = 0.05;
				if(damage >= 2)
					m_reset_textures_timer += 0.05 * damage;
				updateTextures(m_current_texture_modifier + "^[brighten");
			}
		}
	} else if (cmd == GENERIC_CMD_UPDATE_ARMOR_GROUPS) {
		m_armor_groups.clear();
		int armor_groups_size = readU16(is);
		for(int i=0; i<armor_groups_size; i++)
		{
			std::string name = deSerializeString(is);
			int rating = readS16(is);
			m_armor_groups[name] = rating;
		}
	} else if (cmd == GENERIC_CMD_UPDATE_NAMETAG_ATTRIBUTES) {
		// Deprecated, for backwards compatibility only.
		readU8(is); // version
		m_prop.nametag_color = readARGB8(is);
		if (m_nametag != nullptr) {
			m_nametag->nametag_color = m_prop.nametag_color;
			v3f pos;
			pos.Y = m_prop.collisionbox.MaxEdge.Y + 0.3f;
			m_nametag->nametag_pos = pos;
		}
	} else if (cmd == GENERIC_CMD_SPAWN_INFANT) {
		u16 child_id = readU16(is);
		u8 type = readU8(is);

		if (GenericCAO *childobj = m_env->getGenericCAO(child_id)) {
			childobj->processInitData(deSerializeLongString(is));
		} else {
			m_env->addActiveObject(child_id, type, deSerializeLongString(is));
		}
	} else {
		warningstream << FUNCTION_NAME
			<< ": unknown command or outdated client \""
			<< +cmd << "\"" << std::endl;
	}
}

/* \pre punchitem != nullptr
 */
bool GenericCAO::directReportPunch(v3f dir, const ItemStack *punchitem,
		float time_from_last_punch)
{
	assert(punchitem);	// pre-condition
	const ToolCapabilities *toolcap =
			&punchitem->getToolCapabilities(m_client->idef());
	PunchDamageResult result = getPunchDamage(
			m_armor_groups,
			toolcap,
			punchitem,
			time_from_last_punch);

	if(result.did_punch && result.damage != 0)
	{
		if(result.damage < m_hp)
		{
			m_hp -= result.damage;
		} else {
			m_hp = 0;
			// TODO: Execute defined fast response
			// As there is no definition, make a smoke puff
			ClientSimpleObject *simple = createSmokePuff(
					m_smgr, m_env, m_position,
					m_prop.visual_size * BS);
			m_env->addSimpleObject(simple);
		}
		// TODO: Execute defined fast response
		// Flashing shall suffice as there is no definition
		if (m_reset_textures_timer < 0) {
			m_reset_textures_timer = 0.05;
			if (result.damage >= 2)
				m_reset_textures_timer += 0.05 * result.damage;
			updateTextures(m_current_texture_modifier + "^[brighten");
		}
	}

	return false;
}

std::string GenericCAO::debugInfoText()
{
	std::ostringstream os(std::ios::binary);
	os<<"GenericCAO hp="<<m_hp<<"\n";
	os<<"armor={";
	for(ItemGroupList::const_iterator i = m_armor_groups.begin();
			i != m_armor_groups.end(); ++i)
	{
		os<<i->first<<"="<<i->second<<", ";
	}
	os<<"}";
	return os.str();
}

void GenericCAO::initWielditemVisual(ITextureSource *tsrc, video::E_MATERIAL_TYPE material_type)
{
	ItemStack item;
	infostream << "GenericCAO::addToScene(): wielditem" << std::endl;
	if (m_prop.wield_item.empty()) {
		// Old format, only textures are specified.
		infostream << "textures: " << m_prop.textures.size() << std::endl;
		if (!m_prop.textures.empty()) {
			infostream << "textures[0]: " << m_prop.textures[0]
				<< std::endl;
			IItemDefManager *idef = m_client->idef();
			item = ItemStack(m_prop.textures[0], 1, 0, idef);
		}
	} else {
		infostream << "serialized form: " << m_prop.wield_item << std::endl;
		item.deSerialize(m_prop.wield_item, m_client->idef());
	}

	if (m_prop.visual == "wielditem") {
		m_wield_meshnode = new WieldMeshSceneNode(
				RenderingEngine::get_scene_manager(), -1);
	}
	else if (m_prop.visual == "dynamicnode") {
		m_wield_meshnode = new NodeEntitySceneNode(
				RenderingEngine::get_scene_manager(), -1);
	}

	m_wield_meshnode->setItem(item, m_client);

	m_wield_meshnode->setScale(
			v3f(m_prop.visual_size.X / 2, m_prop.visual_size.Y / 2,
				m_prop.visual_size.X / 2));
	u8 li = m_last_light;
	m_wield_meshnode->setColor(video::SColor(255, li, li, li));
}

