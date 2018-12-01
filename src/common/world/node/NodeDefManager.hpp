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
#ifndef NODEDEFMANAGER_HPP_
#define NODEDEFMANAGER_HPP_

#include "common/map/MapNode.hpp"
#include "common/world/nameidmapping.h"
#include "common/world/node/ContentFeatures.hpp"

class IItemDefManager;
class NodeResolver;

/*!
 * @brief This class is for getting the actual properties of nodes from their
 * content ID.
 *
 * @details The nodes on the map are represented by three numbers (see MapNode).
 * The first number (param0) is the type of a node. All node types have own
 * properties (see ContentFeatures). This class is for storing and getting the
 * properties of nodes.
 * The manager is first filled with registered nodes, then as the game begins,
 * functions only get `const` pointers to it, to prevent modification of
 * registered nodes.
 */
class NodeDefManager {
	public:
		/*!
		 * Creates a NodeDefManager, and registers three ContentFeatures:
		 * \ref CONTENT_AIR, \ref CONTENT_UNKNOWN and \ref CONTENT_IGNORE.
		 */
		NodeDefManager();
		~NodeDefManager();

		/*!
		 * Returns the properties for the given content type.
		 * @param c content type of a node
		 * @return properties of the given content type, or \ref CONTENT_UNKNOWN
		 * if the given content type is not registered.
		 */
		inline const ContentFeatures& get(content_t c) const {
			return
				c < m_content_features.size() ?
				m_content_features[c] : m_content_features[CONTENT_UNKNOWN];
		}

		/*!
		 * Returns the properties of the given node.
		 * @param n a map node
		 * @return properties of the given node or @ref CONTENT_UNKNOWN if the
		 * given content type is not registered.
		 */
		inline const ContentFeatures& get(const MapNode &n) const {
			return get(n.getContent());
		}

		/*!
		 * Returns the node properties for a node name.
		 * @param name name of a node
		 * @return properties of the given node or @ref CONTENT_UNKNOWN if
		 * not found
		 */
		const ContentFeatures& get(const std::string &name) const;

		/*!
		 * Returns the content ID for the given name.
		 * @param name a node name
		 * @param[out] result will contain the content ID if found, otherwise
		 * remains unchanged
		 * @return true if the ID was found, false otherwise
		 */
		bool getId(const std::string &name, content_t &result) const;

		/*!
		 * Returns the content ID for the given name.
		 * @param name a node name
		 * @return ID of the node or @ref CONTENT_IGNORE if not found
		 */
		content_t getId(const std::string &name) const;

		/*!
		 * Returns the content IDs of the given node name or node group name.
		 * Group names start with "group:".
		 * @param name a node name or node group name
		 * @param[out] result will be appended with matching IDs
		 * @return true if `name` is a valid node name or a (not necessarily
		 * valid) group name
		 */
		bool getIds(const std::string &name, std::vector<content_t> &result) const;

		/*!
		 * Returns the smallest box in integer node coordinates that
		 * contains all nodes' selection boxes. The returned box might be larger
		 * than the minimal size if the largest node is removed from the manager.
		 */
		inline core::aabbox3d<s16> getSelectionBoxIntUnion() const {
			return m_selection_box_int_union;
		}

		/*!
		 * Checks whether a node connects to an adjacent node.
		 * @param from the node to be checked
		 * @param to the adjacent node
		 * @param connect_face a bit field indicating which face of the node is
		 * adjacent to the other node.
		 * Bits: +y (least significant), -y, -z, -x, +z, +x (most significant).
		 * @return true if the node connects, false otherwise
		 */
		bool nodeboxConnects(MapNode from, MapNode to,
				u8 connect_face) const;

		/*!
		 * Registers a NodeResolver to wait for the registration of
		 * ContentFeatures. Once the node registration finishes, all
		 * listeners are notified.
		 */
		void pendNodeResolve(NodeResolver *nr) const;

		/*!
		 * Stops listening to the NodeDefManager.
		 * @return true if the listener was registered before, false otherwise
		 */
		bool cancelNodeResolveCallback(NodeResolver *nr) const;

		/*!
		 * Registers a new node type with the given name and allocates a new
		 * content ID.
		 * Should not be called with an already existing name.
		 * @param name name of the node, must match with `def.name`.
		 * @param def definition of the registered node type.
		 * @return ID of the registered node or @ref CONTENT_IGNORE if
		 * the function could not allocate an ID.
		 */
		content_t set(const std::string &name, const ContentFeatures &def);

		/*!
		 * Allocates a blank node ID for the given name.
		 * @param name name of a node
		 * @return allocated ID or @ref CONTENT_IGNORE if could not allocate
		 * an ID.
		 */
		content_t allocateDummy(const std::string &name);

		/*!
		 * Removes the given node name from the manager.
		 * The node ID will remain in the manager, but won't be linked to any name.
		 * @param name name to be removed
		 */
		void removeNode(const std::string &name);

		/*!
		 * Regenerates the alias list (a map from names to node IDs).
		 * @param idef the item definition manager containing alias information
		 */
		void updateAliases(IItemDefManager *idef);

		/*!
		 * Reads the used texture pack's override.txt, and replaces the textures
		 * of registered nodes with the ones specified there.
		 *
		 * Format of the input file: in each line
		 * `node_name top|bottom|right|left|front|back|all|*|sides texture_name.png`
		 *
		 * @param override_filepath path to 'texturepack/override.txt'
		 */
		void applyTextureOverrides(const std::string &override_filepath);

		/*!
		 * Only the client uses this. Loads textures and shaders required for
		 * rendering the nodes.
		 * @param gamedef must be a Client.
		 * @param progress_cbk called each time a node is loaded. Arguments:
		 * `progress_cbk_args`, number of loaded ContentFeatures, number of
		 * total ContentFeatures.
		 * @param progress_cbk_args passed to the callback function
		 */
		void updateTextures(IGameDef *gamedef,
				void (*progress_cbk)(void *progress_args, u32 progress, u32 max_progress),
				void *progress_cbk_args);

		/*!
		 * Writes the content of this manager to the given output stream.
		 * @param protocol_version serialization version of ContentFeatures
		 */
		void serialize(std::ostream &os, u16 protocol_version) const;

		/*!
		 * Restores the manager from a serialized stream.
		 * This clears the previous state.
		 * @param is input stream containing a serialized NodeDefManager
		 */
		void deSerialize(std::istream &is);

		/*!
		 * Used to indicate that node registration has finished.
		 * @param completed tells whether registration is complete
		 */
		inline void setNodeRegistrationStatus(bool completed) {
			m_node_registration_complete = completed;
		}

		/*!
		 * Notifies the registered NodeResolver instances that node registration
		 * has finished, then unregisters all listeners.
		 * Must be called after node registration has finished!
		 */
		void runNodeResolveCallbacks();

		/*!
		 * Sets the registration completion flag to false and unregisters all
		 * NodeResolver instances listening to the manager.
		 */
		void resetNodeResolveState();

		/*!
		 * Resolves the IDs to which connecting nodes connect from names.
		 * Must be called after node registration has finished!
		 */
		void mapNodeboxConnections();

	private:
		/*!
		 * Resets the manager to its initial state.
		 * See the documentation of the constructor.
		 */
		void clear();

		/*!
		 * Allocates a new content ID, and returns it.
		 * @return the allocated ID or \ref CONTENT_IGNORE if could not allocate
		 */
		content_t allocateId();

		/*!
		 * Binds the given content ID and node name.
		 * Registers them in \ref m_name_id_mapping and
		 * \ref m_name_id_mapping_with_aliases.
		 * @param i a content ID
		 * @param name a node name
		 */
		void addNameIdMapping(content_t i, std::string name);

		/*!
		 * Recalculates m_selection_box_int_union based on
		 * m_selection_box_union.
		 */
		void fixSelectionBoxIntUnion();

		//! Features indexed by ID.
		std::vector<ContentFeatures> m_content_features;

		//! A mapping for fast conversion between names and IDs
		NameIdMapping m_name_id_mapping;

		/*!
		 * Like @ref m_name_id_mapping, but maps only from names to IDs, and
		 * includes aliases too. Updated by \ref updateAliases().
		 * Note: Not serialized.
		 */
		std::unordered_map<std::string, content_t> m_name_id_mapping_with_aliases;

		/*!
		 * A mapping from group names to a vector of content types that belong
		 * to it. Necessary for a direct lookup in \ref getIds().
		 * Note: Not serialized.
		 */
		std::unordered_map<std::string, std::vector<content_t>> m_group_to_items;

		/*!
		 * The next ID that might be free to allocate.
		 * It can be allocated already, because \ref CONTENT_AIR,
		 * \ref CONTENT_UNKNOWN and \ref CONTENT_IGNORE are registered when the
		 * manager is initialized, and new IDs are allocated from 0.
		 */
		content_t m_next_id;

		//! True if all nodes have been registered.
		bool m_node_registration_complete;

		/*!
		 * The union of all nodes' selection boxes.
		 * Might be larger if big nodes are removed from the manager.
		 */
		aabb3f m_selection_box_union;

		/*!
		 * The smallest box in integer node coordinates that
		 * contains all nodes' selection boxes.
		 * Might be larger if big nodes are removed from the manager.
		 */
		core::aabbox3d<s16> m_selection_box_int_union;

		/*!
		 * NodeResolver instances to notify once node registration has finished.
		 * Even constant NodeDefManager instances can register listeners.
		 */
		mutable std::vector<NodeResolver *> m_pending_resolve_callbacks;
};

NodeDefManager *createNodeDefManager();

#endif // NODEDEFMANAGER_HPP_
