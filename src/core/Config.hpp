/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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
#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)

/*
	If CMake is used, includes the cmake-generated cmake_config.h.
	Otherwise use default values
*/
#if defined USE_CMAKE_CONFIG_H
	#include "cmake_config.h"
#elif defined (__ANDROID__) || defined (ANDROID)
	#define PROJECT_NAME "minetest"
	#define PROJECT_NAME_C "Minetest"
	#define STATIC_SHAREDIR ""
	#include "android_version.h"
	#ifdef NDEBUG
		#define BUILD_TYPE "Release"
	#else
		#define BUILD_TYPE "Debug"
	#endif
#else
	#ifdef NDEBUG
		#define BUILD_TYPE "Release"
	#else
		#define BUILD_TYPE "Debug"
	#endif
#endif

#define BUILD_INFO \
	"BUILD_TYPE=" BUILD_TYPE "\n"          \
	"RUN_IN_PLACE=" STR(RUN_IN_PLACE) "\n" \
	"USE_GETTEXT=" STR(USE_GETTEXT) "\n"   \
	"USE_SOUND=" STR(USE_SOUND) "\n"       \
	"USE_CURL=" STR(USE_CURL) "\n"         \
	"USE_FREETYPE=" STR(USE_FREETYPE) "\n" \
	"USE_LUAJIT=" STR(USE_LUAJIT) "\n"     \
	"STATIC_SHAREDIR=" STR(STATIC_SHAREDIR);

#endif // CONFIG_HPP_
