// K-3D
// Copyright (c) 2005-2009 Timothy M. Shead
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
	\author Bart Janssens (bart.janssens@lid.kviv.be)
*/

#include <k3dsdk/attribute_array_copier.h>
#include <k3dsdk/basic_math.h>
#include <k3dsdk/document_plugin_factory.h>
#include <k3dsdk/euler_operations.h>
#include <k3dsdk/geometry.h>
#include <k3dsdk/imaterial.h>
#include <k3dsdk/ipipeline_profiler.h>
#include <k3dsdk/measurement.h>
#include <k3dsdk/mesh_modifier.h>
#include <k3dsdk/mesh_selection_sink.h>
#include <k3dsdk/node.h>
#include <k3dsdk/polyhedron.h>
#include <k3dsdk/selection.h>
#include <k3dsdk/utility.h>
#include <k3dsdk/vectors.h>

#include <boost/scoped_ptr.hpp>

namespace module
{

namespace euler
{

/////////////////////////////////////////////////////////////////////////////
// kill_edge_and_vertex

class kill_edge_and_vertex :
	public k3d::mesh_selection_sink<k3d::mesh_modifier<k3d::node > >
{
	typedef k3d::mesh_selection_sink<k3d::mesh_modifier<k3d::node > > base;

public:
	kill_edge_and_vertex(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document)
	{
		m_mesh_selection.changed_signal().connect(make_reset_mesh_slot());
	}

	void on_create_mesh(const k3d::mesh& Input, k3d::mesh& Output)
	{
		Output = Input;

		k3d::geometry::merge_selection(m_mesh_selection.pipeline_value(), Output);
	
		for(k3d::mesh::primitives_t::iterator primitive = Output.primitives.begin(); primitive != Output.primitives.end(); ++primitive)
		{
      boost::scoped_ptr<k3d::polyhedron::primitive> polyhedron(k3d::polyhedron::validate(*primitive));
      if(!polyhedron)
        continue;

      k3d::mesh::indices_t edge_list;
      const k3d::uint_t edge_begin = 0;
      const k3d::uint_t edge_end = edge_begin + polyhedron->edge_selections.size();
      for(k3d::uint_t edge = edge_begin; edge != edge_end; ++edge)
      {
        if(polyhedron->edge_selections[edge])
          edge_list.push_back(edge);
      }
      
      const k3d::mesh::points_t& points = *Input.points;
      
      k3d::mesh::bools_t boundary_edges;
      k3d::mesh::indices_t companions;
      k3d::polyhedron::create_edge_adjacency_lookup(polyhedron->edge_points, polyhedron->clockwise_edges, boundary_edges, companions);
      
      k3d::euler::kill_edge_and_vertex(*polyhedron, edge_list, boundary_edges, companions, points.size());
    }
		
		k3d::mesh::delete_unused_points(Output);
	}

	void on_update_mesh(const k3d::mesh& Input, k3d::mesh& Output)
	{
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<kill_edge_and_vertex,
			k3d::interface_list<k3d::imesh_source,
			k3d::interface_list<k3d::imesh_sink > > > factory(
				k3d::uuid(0x42c80daa, 0x214fc707, 0x030c24a5, 0x665fcd4b),
				"EulerKillEdgeAndVertex",
				_("Apply the Kill Edge And Vertex (KEV) Euler operation to the selected edges"),
				"Mesh",
				k3d::iplugin_factory::EXPERIMENTAL);

		return factory;
	}
};

/////////////////////////////////////////////////////////////////////////////
// kill_edge_and_vertex_factory

k3d::iplugin_factory& kill_edge_and_vertex_factory()
{
	return kill_edge_and_vertex::get_factory();
}

} // namespace euler

} // namespace module

