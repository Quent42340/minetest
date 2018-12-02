/*
Minetest
Copyright (C) 2010-2014 sapier <sapier at gmx dot net>

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
#ifndef FONTENGINE_HPP_
#define FONTENGINE_HPP_

#include <map>
#include <vector>
#include "util/basic_macros.h"
#include <IGUIFont.h>
#include <IGUISkin.h>
#include <IGUIEnvironment.h>
#include "core/settings.h"

#define FONT_SIZE_UNSPECIFIED 0xFFFFFFFF

enum FontMode {
	FM_Standard = 0,
	FM_Mono,
	FM_Fallback,
	FM_Simple,
	FM_SimpleMono,
	FM_MaxMode,
	FM_Unspecified
};

class FontEngine {
	public:
		FontEngine(Settings *main_settings, gui::IGUIEnvironment *env);
		~FontEngine();

		irr::gui::IGUIFont *getFont(unsigned int font_size = FONT_SIZE_UNSPECIFIED,
				FontMode mode = FM_Unspecified);

		// Get text height for a specific font
		unsigned int getTextHeight(unsigned int font_size = FONT_SIZE_UNSPECIFIED,
				FontMode mode = FM_Unspecified);

		// Get text width if a text for a specific font
		unsigned int getTextWidth(const std::string &text,
				unsigned int font_size = FONT_SIZE_UNSPECIFIED,
				FontMode mode = FM_Unspecified)
		{
			return getTextWidth(utf8_to_wide(text));
		}

		// Get text width if a text for a specific font
		unsigned int getTextWidth(const std::wstring &text,
				unsigned int font_size = FONT_SIZE_UNSPECIFIED,
				FontMode mode = FM_Unspecified);

		// Get line height for a specific font (including empty room between lines)
		unsigned int getLineHeight(unsigned int font_size = FONT_SIZE_UNSPECIFIED,
				FontMode mode = FM_Unspecified);

		// Get default font size
		unsigned int getDefaultFontSize();

		// Initialize font engine
		void initialize(Settings *main_settings, gui::IGUIEnvironment *env);

		// Update internal parameters from settings
		void readSettings();

	private:
		// Update content of font cache in case of a setting change made it invalid
		void updateFontCache();

		// Initialize a new font
		void initFont(unsigned int basesize, FontMode mode=FM_Unspecified);

		// Initialize a font without freetype
		void initSimpleFont(unsigned int basesize, FontMode mode);

		// Update current minetest skin with font changes
		void updateSkin();

		// Clean cache
		void cleanCache();

		// Pointer to settings for registering callbacks or reading config
		Settings* m_settings = nullptr;

		// Pointer to irrlicht gui environment
		gui::IGUIEnvironment* m_env = nullptr;

		// Internal storage for caching fonts of different size
		std::map<unsigned int, irr::gui::IGUIFont*> m_font_cache[FM_MaxMode];

		// Default font size to use
		unsigned int m_default_size[FM_MaxMode];

		// Current font engine mode
		FontMode m_currentMode = FM_Standard;

		// Font mode of last request
		FontMode m_lastMode;

		// Size of last request
		unsigned int m_lastSize = 0;

		// Last font returned
		irr::gui::IGUIFont* m_lastFont = nullptr;

		DISABLE_CLASS_COPY(FontEngine);
};

// Interface to access main font engine
extern FontEngine* g_fontengine;

#endif // FONTENGINE_HPP_
