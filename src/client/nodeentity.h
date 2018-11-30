/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2018 Unarelith, Quentin Bazin <quent42340@gmail.com>

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
#ifndef NODEENTITY_H_
#define NODEENTITY_H_

// #include <string>
// #include <vector>
// #include "irrlicht/irrlichttypes_extrabloated.h"
//
// class Client;
// class ITextureSource;
// struct ContentFeatures;
// struct ItemStack;

#include "client/wieldmesh.h"

/*
	Node entity scene node, renders a dynamic node through an entity
*/
class NodeEntitySceneNode : public WieldMeshSceneNode
{
public:
	NodeEntitySceneNode(scene::ISceneManager *mgr, s32 id = -1, bool lighting = false);
};

#endif // NODEENTITY_H_
