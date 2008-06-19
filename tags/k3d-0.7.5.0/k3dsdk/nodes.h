#ifndef K3DSDK_NODES_H
#define K3DSDK_NODES_H

// K-3D
// Copyright (c) 1995-2005, Timothy M. Shead
//
// Contact: tshead@k-3d.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/** \file
		\author Tim Shead (tshead@k-3d.com)
*/

#include "uuid.h"
#include "inode_collection.h"
#include "inode.h"

#include "utility.h"

#include <vector>

#ifdef interface
#undef interface
#endif // inteface

namespace k3d
{

// Forward declarations
class idocument;
class iproperty;

/// Defines a collection of nodes
typedef inode_collection::nodes_t nodes_t;

/// Returns the node that matches the given name (returns NULL if no node matches or if more-than-one node matches)
inode* find_node(inode_collection& Nodes, const std::string& NodeName);

/// Returns the node that owns the given property (could return NULL)
inode* find_node(inode_collection& Nodes, iproperty& Property);

/// Returns the set of nodes that match a specific uuid (could return empty set!)
const nodes_t find_nodes(inode_collection& Nodes, const uuid ClassID);

/// Returns the set of nodes that match the given name (be prepared to handle zero, one, or many results)
const nodes_t find_nodes(inode_collection& Nodes, const std::string& NodeName);

/// Returns the set of nodes that implement a specific interface type (could return empty set!)
template<typename interface_t>
const nodes_t find_nodes(inode_collection& Nodes)
{
	nodes_t nodes;

	const nodes_t::const_iterator end(Nodes.collection().end());
	for(nodes_t::const_iterator node = Nodes.collection().begin(); node != end; ++node)
	{
		if(dynamic_cast<interface_t*>(*node))
			nodes.insert(nodes.end(), *node);
	}

	return nodes;
}

/// Returns a unique node name based on the one supplied
const std::string unique_name(inode_collection& Nodes, const std::string& Name);

/// Deletes a collection of nodes, cleaning-up all references and resources (this operation is undo-able, if state change recording is in effect)
void delete_nodes(idocument& Document, const nodes_t& Nodes);

} // namespace k3d

#endif // !K3DSDK_NODES_H
