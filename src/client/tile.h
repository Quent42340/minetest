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

#pragma once

#include "irrlicht/irrlichttypes.h"
#include "irrlicht/irr_v3d.h"
#include <ITexture.h>
#include <string>
#include <vector>
#include <SMaterial.h>
#include <memory>
#include "util/numeric.h"

#if __ANDROID__
#include <IVideoDriver.h>
#endif

class IGameDef;
struct TileSpec;
struct TileDef;

typedef std::vector<video::SColor> Palette;

/*
	tile.{h,cpp}: Texture handling stuff.
*/

/*
	Find out the full path of an image by trying different filename
	extensions.

	If failed, return "".

	TODO: Should probably be moved out from here, because things needing
	      this function do not need anything else from this header
*/
std::string getImagePath(std::string path);

/*
	Gets the path to a texture by first checking if the texture exists
	in texture_path and if not, using the data path.

	Checks all supported extensions by replacing the original extension.

	If not found, returns "".

	Utilizes a thread-safe cache.
*/
std::string getTexturePath(const std::string &filename);

void clearTextureNameCache();

/*
	TextureSource creates and caches textures.
*/

class ISimpleTextureSource
{
public:
	ISimpleTextureSource() = default;

	virtual ~ISimpleTextureSource() = default;

	virtual video::ITexture* getTexture(
			const std::string &name, u32 *id = nullptr) = 0;
};

class ITextureSource : public ISimpleTextureSource
{
public:
	ITextureSource() = default;

	virtual ~ITextureSource() = default;

	virtual u32 getTextureId(const std::string &name)=0;
	virtual std::string getTextureName(u32 id)=0;
	virtual video::ITexture* getTexture(u32 id)=0;
	virtual video::ITexture* getTexture(
			const std::string &name, u32 *id = nullptr)=0;
	virtual video::ITexture* getTextureForMesh(
			const std::string &name, u32 *id = nullptr) = 0;
	/*!
	 * Returns a palette from the given texture name.
	 * The pointer is valid until the texture source is
	 * destructed.
	 * Should be called from the main thread.
	 */
	virtual Palette* getPalette(const std::string &name) = 0;
	virtual bool isKnownSourceImage(const std::string &name)=0;
	virtual video::ITexture* getNormalTexture(const std::string &name)=0;
	virtual video::SColor getTextureAverageColor(const std::string &name)=0;
	virtual video::ITexture *getShaderFlagsTexture(bool normalmap_present)=0;
};

class IWritableTextureSource : public ITextureSource
{
public:
	IWritableTextureSource() = default;

	virtual ~IWritableTextureSource() = default;

	virtual u32 getTextureId(const std::string &name)=0;
	virtual std::string getTextureName(u32 id)=0;
	virtual video::ITexture* getTexture(u32 id)=0;
	virtual video::ITexture* getTexture(
			const std::string &name, u32 *id = nullptr)=0;
	virtual bool isKnownSourceImage(const std::string &name)=0;

	virtual void processQueue()=0;
	virtual void insertSourceImage(const std::string &name, video::IImage *img)=0;
	virtual void rebuildImagesAndTextures()=0;
	virtual video::ITexture* getNormalTexture(const std::string &name)=0;
	virtual video::SColor getTextureAverageColor(const std::string &name)=0;
	virtual video::ITexture *getShaderFlagsTexture(bool normalmap_present)=0;
};

IWritableTextureSource *createTextureSource();

#ifdef __ANDROID__
video::IImage * Align2Npot2(video::IImage * image, irr::video::IVideoDriver* driver);
#endif

#include "client/tile/TileLayer.h"
#include "client/tile/TileSpec.h"

std::vector<std::string> getTextureDirs();
