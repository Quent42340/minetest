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
#ifndef GAMEGLOBALSHADERCONSTANTSETTER_HPP_
#define GAMEGLOBALSHADERCONSTANTSETTER_HPP_

#include <vector>
#include "irrlicht/irrlichttypes.h"
#include "client/shader.h"

class Client;
class Sky;

// Before 1.8 there isn't a "integer interface", only float
#if (IRRLICHT_VERSION_MAJOR == 1 && IRRLICHT_VERSION_MINOR < 8)
using SamplerLayer_t = f32;
#else
using SamplerLayer_t = s32;
#endif

// FIXME: Change class names to GameGlobalSCS and GameGlobalSCSFactory
class GameGlobalShaderConstantSetter : public IShaderConstantSetter {
	public:
		GameGlobalShaderConstantSetter(Sky *sky, bool &force_fog_off, f32 &fog_range, Client &client);
		~GameGlobalShaderConstantSetter();

		void onSettingsChange(const std::string &name);
		void onSetConstants(video::IMaterialRendererServices *services, bool is_highlevel) override;

		void setSky(Sky *sky) { m_sky = sky; }

		static void settingsCallback(const std::string &name, void *userdata);

	private:
		Sky *m_sky;

		Client &m_client;

		bool &m_force_fog_off;
		f32 &m_fog_range;
		bool m_fog_enabled;

		CachedPixelShaderSetting<float> m_fog_distance{"fogDistance"};
		CachedPixelShaderSetting<float, 4> m_sky_bg_color{"skyBgColor"};
		CachedPixelShaderSetting<float, 3> m_minimap_yaw{"yawVec"};
		CachedPixelShaderSetting<float, 3> m_day_light{"dayLight"};

		CachedVertexShaderSetting<float> m_animation_timer_vertex{"animationTimer"};
		CachedPixelShaderSetting<float> m_animation_timer_pixel{"animationTimer"};

		CachedVertexShaderSetting<float, 3> m_eye_position_vertex{"eyePosition"};
		CachedPixelShaderSetting<float, 3> m_eye_position_pixel{"eyePosition"};

		CachedPixelShaderSetting<SamplerLayer_t> m_base_texture{"baseTexture"};
		CachedPixelShaderSetting<SamplerLayer_t> m_normal_texture{"normalTexture"};
		CachedPixelShaderSetting<SamplerLayer_t> m_texture_flags{"textureFlags"};
};

class GameGlobalShaderConstantSetterFactory : public IShaderConstantSetterFactory {
	public:
		GameGlobalShaderConstantSetterFactory(bool &force_fog_off, f32 &fog_range, Client &client);

		void setSky(Sky *sky);

		IShaderConstantSetter* create() override;

	private:
		Sky *m_sky = nullptr;
		bool &m_force_fog_off;
		f32 &m_fog_range;
		Client &m_client;
		std::vector<GameGlobalShaderConstantSetter *> created_nosky;
};

#endif // GAMEGLOBALSHADERCONSTANTSETTER_HPP_
