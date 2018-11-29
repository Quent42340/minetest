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
#include "client/minimap.h"
#include "client/sky.h"

#include "client/game/GameGlobalShaderConstantSetter.h"

GameGlobalShaderConstantSetter::GameGlobalShaderConstantSetter(Sky *sky, bool &force_fog_off, f32 &fog_range, Client &client) :
	m_sky(sky),
	m_client(client),
	m_force_fog_off(force_fog_off),
	m_fog_range(fog_range)
{
	g_settings->registerChangedCallback("enable_fog", settingsCallback, this);
	m_fog_enabled = g_settings->getBool("enable_fog");
}

GameGlobalShaderConstantSetter::~GameGlobalShaderConstantSetter() {
	g_settings->deregisterChangedCallback("enable_fog", settingsCallback, this);
}

void GameGlobalShaderConstantSetter::onSettingsChange(const std::string &name) {
	if (name == "enable_fog")
		m_fog_enabled = g_settings->getBool("enable_fog");
}

void GameGlobalShaderConstantSetter::onSetConstants(video::IMaterialRendererServices *services, bool is_highlevel) {
	if (!is_highlevel)
		return;

	// Background color
	video::SColor bgcolor = m_sky->getBgColor();
	video::SColorf bgcolorf(bgcolor);
	float bgcolorfa[4] = {
		bgcolorf.r,
		bgcolorf.g,
		bgcolorf.b,
		bgcolorf.a,
	};
	m_sky_bg_color.set(bgcolorfa, services);

	// Fog distance
	float fog_distance = 10000 * BS;

	if (m_fog_enabled && !m_force_fog_off)
		fog_distance = m_fog_range;

	m_fog_distance.set(&fog_distance, services);

	u32 daynight_ratio = (float)m_client.getEnv().getDayNightRatio();
	video::SColorf sunlight;
	get_sunlight_color(&sunlight, daynight_ratio);
	float dnc[3] = {
		sunlight.r,
		sunlight.g,
		sunlight.b
	};
	m_day_light.set(dnc, services);

	u32 animation_timer = porting::getTimeMs() % 100000;
	float animation_timer_f = (float)animation_timer / 100000.f;
	m_animation_timer_vertex.set(&animation_timer_f, services);
	m_animation_timer_pixel.set(&animation_timer_f, services);

	float eye_position_array[3];
	v3f epos = m_client.getEnv().getLocalPlayer()->getEyePosition();

#if (IRRLICHT_VERSION_MAJOR == 1 && IRRLICHT_VERSION_MINOR < 8)
	eye_position_array[0] = epos.X;
	eye_position_array[1] = epos.Y;
	eye_position_array[2] = epos.Z;
#else
	epos.getAs3Values(eye_position_array);
#endif

	m_eye_position_pixel.set(eye_position_array, services);
	m_eye_position_vertex.set(eye_position_array, services);

	if (m_client.getMinimap()) {
		float minimap_yaw_array[3];
		v3f minimap_yaw = m_client.getMinimap()->getYawVec();

#if (IRRLICHT_VERSION_MAJOR == 1 && IRRLICHT_VERSION_MINOR < 8)
		minimap_yaw_array[0] = minimap_yaw.X;
		minimap_yaw_array[1] = minimap_yaw.Y;
		minimap_yaw_array[2] = minimap_yaw.Z;
#else
		minimap_yaw.getAs3Values(minimap_yaw_array);
#endif

		m_minimap_yaw.set(minimap_yaw_array, services);
	}

	SamplerLayer_t base_tex = 0,
	               normal_tex = 1,
	               flags_tex = 2;

	m_base_texture.set(&base_tex, services);
	m_normal_texture.set(&normal_tex, services);
	m_texture_flags.set(&flags_tex, services);
}

void GameGlobalShaderConstantSetter::settingsCallback(const std::string &name, void *userdata) {
	reinterpret_cast<GameGlobalShaderConstantSetter*>(userdata)->onSettingsChange(name);
}

GameGlobalShaderConstantSetterFactory::GameGlobalShaderConstantSetterFactory(bool &force_fog_off, f32 &fog_range, Client &client) :
	m_force_fog_off(force_fog_off),
	m_fog_range(fog_range),
	m_client(client)
{}

void GameGlobalShaderConstantSetterFactory::setSky(Sky *sky) {
	m_sky = sky;

	for (GameGlobalShaderConstantSetter *ggscs : created_nosky) {
		ggscs->setSky(m_sky);
	}

	created_nosky.clear();
}

IShaderConstantSetter* GameGlobalShaderConstantSetterFactory::create() {
	// FIXME: Use smart ptr
	auto *scs = new GameGlobalShaderConstantSetter(m_sky, m_force_fog_off, m_fog_range, m_client);
	if (!m_sky)
		created_nosky.push_back(scs);
	return scs;
}

