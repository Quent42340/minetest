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
#ifndef NODEBOX_HPP_
#define NODEBOX_HPP_

#include <istream>
#include <vector>
#include "irrlicht/irr_aabb3d.h"

enum NodeBoxType
{
	NODEBOX_REGULAR, // Regular block; allows buildable_to
	NODEBOX_FIXED, // Static separately defined box(es)
	NODEBOX_WALLMOUNTED, // Box for wall mounted nodes; (top, bottom, side)
	NODEBOX_LEVELED, // Same as fixed, but with dynamic height from param2. for snow, ...
	NODEBOX_CONNECTED, // optionally draws nodeboxes if a neighbor node attaches
};

struct NodeBox {
	NodeBox() { reset(); }

	void reset();

	void serialize(std::ostream &os, u16 protocol_version) const;
	void deSerialize(std::istream &is);

	enum NodeBoxType type;
	// NODEBOX_REGULAR (no parameters)
	// NODEBOX_FIXED
	std::vector<aabb3f> fixed;
	// NODEBOX_WALLMOUNTED
	aabb3f wall_top;
	aabb3f wall_bottom;
	aabb3f wall_side; // being at the -X side
	// NODEBOX_CONNECTED
	std::vector<aabb3f> connect_top;
	std::vector<aabb3f> connect_bottom;
	std::vector<aabb3f> connect_front;
	std::vector<aabb3f> connect_left;
	std::vector<aabb3f> connect_back;
	std::vector<aabb3f> connect_right;
	std::vector<aabb3f> disconnected_top;
	std::vector<aabb3f> disconnected_bottom;
	std::vector<aabb3f> disconnected_front;
	std::vector<aabb3f> disconnected_left;
	std::vector<aabb3f> disconnected_back;
	std::vector<aabb3f> disconnected_right;
	std::vector<aabb3f> disconnected;
	std::vector<aabb3f> disconnected_sides;
};

#endif // NODEBOX_HPP_
