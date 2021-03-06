// K-3D
// Copyright (c) 1995-2008, Timothy M. Shead
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
	\author Timothy M. Shead (tshead@k-3d.com)
*/

#include <k3d-i18n-config.h>
#include <k3dsdk/document_plugin_factory.h>
#include <k3dsdk/imaterial.h>
#include <k3dsdk/imesh_painter_ri.h>
#include <k3dsdk/node.h>
#include <k3dsdk/polyhedron.h>
#include <k3dsdk/renderable_ri.h>
#include <k3dsdk/selection.h>
#include <k3dsdk/utility_gl.h>

#include <boost/scoped_ptr.hpp>

#include <set>

namespace module
{

namespace renderman
{

namespace painters
{

/////////////////////////////////////////////////////////////////////////////
// subdivision_surface_painter

class subdivision_surface_painter :
	public k3d::node,
	public k3d::ri::imesh_painter
{
	typedef k3d::node base;
	typedef k3d::typed_array<k3d::string_t> strings_t;
public:
	subdivision_surface_painter(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document)
	{
	}

	void paint_mesh(const k3d::mesh& Mesh, const k3d::ri::render_state& RenderState)
	{
		for(k3d::mesh::primitives_t::const_iterator primitive = Mesh.primitives.begin(); primitive != Mesh.primitives.end(); ++primitive)
		{
      boost::scoped_ptr<k3d::polyhedron::const_primitive> polyhedron(k3d::polyhedron::validate(**primitive));
      if(!polyhedron)
        continue;

      const k3d::mesh::points_t& points = *Mesh.points;
    
      const strings_t* const interpolateboundary_tags = polyhedron->constant_data.lookup<strings_t>("interpolateboundary");

      const k3d::uint_t shell_begin = 0;
      const k3d::uint_t shell_end = shell_begin + polyhedron->shell_types.size();
      for(k3d::uint_t shell = shell_begin; shell != shell_end; ++shell)
      {
        if(polyhedron->shell_types[shell] != k3d::polyhedron::CATMULL_CLARK)
          continue;

        // Get the set of all materials used in this polyhedron ...
        typedef std::set<k3d::imaterial*> materials_t;
        materials_t materials;

        const k3d::uint_t faces_begin = polyhedron->shell_first_faces[shell];
        const k3d::uint_t faces_end = faces_begin + polyhedron->shell_face_counts[shell];
        for(k3d::uint_t face = faces_begin; face != faces_end; ++face)
          materials.insert(polyhedron->face_materials[face]);

        // Iterate over each material, rendering all the faces that use that material in a single pass
        for(materials_t::iterator m = materials.begin(); m != materials.end(); ++m)
        {
          k3d::imaterial* const material = *m;

          k3d::ri::unsigned_integers vertex_counts;
          k3d::ri::unsigned_integers vertex_ids;

          k3d::typed_array<k3d::ri::point>* const ri_points = new k3d::typed_array<k3d::ri::point>(points);

          const k3d::uint_t faces_begin = polyhedron->shell_first_faces[shell];
          const k3d::uint_t faces_end = faces_begin + polyhedron->shell_face_counts[shell];
          for(k3d::uint_t face = faces_begin; face != faces_end; ++face)
          {
            k3d::uint_t vertex_count = 0;

            const k3d::uint_t loop = polyhedron->face_first_loops[face];
            const k3d::uint_t first_edge = polyhedron->loop_first_edges[loop];
            for(k3d::uint_t edge = first_edge; ; )
            {
              ++vertex_count;
              vertex_ids.push_back(polyhedron->edge_points[edge]);

              edge = polyhedron->clockwise_edges[edge];
              if(edge == first_edge)
                break;
            }

            vertex_counts.push_back(vertex_count);
          }

          k3d::ri::strings tags;
          k3d::ri::unsigned_integers tag_counts;
          k3d::ri::integers tag_integers;
          k3d::ri::reals tag_reals;
          
          if (interpolateboundary_tags)
          {
            tags.push_back(interpolateboundary_tags->at(shell));
            tag_counts.push_back(0);
            tag_counts.push_back(0);
          }

          k3d::ri::parameter_list parameters;
          parameters.push_back(k3d::ri::parameter(k3d::ri::RI_P(), k3d::ri::VERTEX, 1, ri_points));

          k3d::ri::setup_material(material, RenderState);
          RenderState.stream.RiSubdivisionMeshV("catmull-clark", vertex_counts, vertex_ids, tags, tag_counts, tag_integers, tag_reals, parameters);
        }
      }
    }
	}
	
	void paint_complete(const k3d::mesh& Mesh, const k3d::ri::render_state& RenderState)
	{
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<subdivision_surface_painter, k3d::interface_list<k3d::ri::imesh_painter > > factory(
			k3d::uuid(0x4d6fae39, 0x723e4ed3, 0xbc5735c5, 0x3b75edc0),
			"RenderManSubdivisionSurfacePainter",
			_("Renders mesh subdivision surfaces"),
			"RenderMan Painter",
			k3d::iplugin_factory::EXPERIMENTAL);

		return factory;
	}
};

/////////////////////////////////////////////////////////////////////////////
// subdivision_surface_painter_factory

k3d::iplugin_factory& subdivision_surface_painter_factory()
{
	return subdivision_surface_painter::get_factory();
}

} // namespace painters

} // namespace renderman

} // namespace module

