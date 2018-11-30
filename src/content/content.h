/*
Minetest
Copyright (C) 2018 rubenwardy <rw@rubenwardy.com>

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
#include "core/config.h"
#include "util/convert_json.h"
#include "irrlichttypes.h"

struct ContentSpec
{
	std::string type;
	std::string author;
	u32 release = 0;
	std::string name;
	std::string desc;
	std::string path;
};

void parseContentInfo(ContentSpec &spec);
