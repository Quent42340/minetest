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
#ifndef NODERESOLVER_HPP_
#define NODERESOLVER_HPP_

#include "map/MapNode.hpp"

class NodeResolver {
	public:
		NodeResolver();
		virtual ~NodeResolver();

		virtual void resolveNodeNames() = 0;

		bool getIdFromNrBacklog(content_t *result_out,
				const std::string &node_alt, content_t c_fallback,
				bool error_on_fallback = true);
		bool getIdsFromNrBacklog(std::vector<content_t> *result_out,
				bool all_required = false, content_t c_fallback = CONTENT_IGNORE);

		void nodeResolveInternal();

		u32 m_nodenames_idx = 0;
		u32 m_nnlistsizes_idx = 0;
		std::vector<std::string> m_nodenames;
		std::vector<size_t> m_nnlistsizes;
		const NodeDefManager *m_ndef = nullptr;
		bool m_resolve_done = false;
};

#endif // NODERESOLVER_HPP_
