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

/** \file Painter that triangulates its input first
 * 	\author Bart Janssens (bart.janssens@lid.kviv.be)
 */

#include <k3d-i18n-config.h>
#include <k3dsdk/array.h>
#include <k3dsdk/document_plugin_factory.h>
#include <k3dsdk/extension_gl.h>
#include <k3dsdk/gl.h>
#include <k3dsdk/hints.h>
#include <k3dsdk/imesh_painter_gl.h>
#include <k3dsdk/mesh_operations.h>
#include <k3dsdk/named_array_operations.h>
#include <k3dsdk/node.h>
#include <k3dsdk/painter_render_state_gl.h>
#include <k3dsdk/painter_selection_state_gl.h>
#include <k3dsdk/selection.h>
#include <k3dsdk/utility_gl.h>

#include "cached_triangulation.h"
#include "colored_selection_painter_gl.h"
#include "normal_cache.h"

namespace module
{

namespace opengl
{

namespace painters
{

/////////////////////////////////////////////////////////////////////////////
// face_painter

class face_painter :
	public colored_selection_painter
{
	typedef colored_selection_painter base;

public:
	face_painter(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document, k3d::color(0.2,0.2,0.2), k3d::color(0.6,0.6,0.6))
	{
	}

	void on_paint_mesh(const k3d::mesh& Mesh, const k3d::gl::painter_render_state& RenderState)
	{
		if(!validate_polyhedra(Mesh))
			return;
			
		if(k3d::is_sds(Mesh))
			return;
		
		k3d::gl::store_attributes attributes;

		glFrontFace(RenderState.inside_out ? GL_CCW : GL_CW);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		k3d::gl::set(GL_CULL_FACE, RenderState.draw_two_sided);

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);
		
		const color_t color = RenderState.node_selection ? selected_mesh_color() : unselected_mesh_color(RenderState.parent_selection);
		const color_t selected_color = RenderState.show_component_selection ? selected_component_color() : color;
		
		cached_triangulation& triangles = get_data<cached_triangulation>(&Mesh, this);
		const k3d::mesh::indices_t& face_starts = triangles.face_starts();
		if (face_starts.empty())
			return;
		const k3d::mesh::points_t& points = triangles.points();
		const cached_triangulation::indices_t& indices = triangles.indices();
		
		glEnable(GL_LIGHTING);
		enable_blending();
		
		const k3d::mesh::indices_t& edge_points = *Mesh.polyhedra->edge_points;
		const k3d::mesh::indices_t& clockwise_edges = *Mesh.polyhedra->clockwise_edges;
		const k3d::mesh::indices_t& loop_first_edges = *Mesh.polyhedra->loop_first_edges;
		const k3d::mesh::indices_t& face_first_loops = *Mesh.polyhedra->face_first_loops;
		const k3d::mesh::indices_t& face_loop_counts = *Mesh.polyhedra->face_loop_counts;
		const k3d::mesh::points_t& mesh_points = *Mesh.points;
		const k3d::mesh::selection_t& face_selection = *Mesh.polyhedra->face_selection;
		normal_cache& n_cache = get_data<normal_cache>(&Mesh, this);

		glBegin(GL_TRIANGLES);
		for (k3d::uint_t face = 0; face != face_starts.size(); ++face)
		{
			k3d::uint_t startindex = face_starts[face];
			k3d::uint_t endindex = face+1 == (face_starts.size()) ? indices.size() : face_starts[face+1];
			const color_t& face_color = face_selection[face] ? selected_color : color;
			k3d::gl::normal3d(n_cache.face_normals(this).at(face));
			k3d::gl::material(GL_FRONT_AND_BACK, GL_DIFFUSE, k3d::color(face_color.red, face_color.green, face_color.blue), face_color.alpha);
			for (k3d::uint_t corner = startindex; corner != endindex; ++corner)
				k3d::gl::vertex3d(points[indices[corner]]);
		}
		glEnd();
		
		disable_blending();
	}
	
	void on_select_mesh(const k3d::mesh& Mesh, const k3d::gl::painter_render_state& RenderState, const k3d::gl::painter_selection_state& SelectionState)
	{
		if(!validate_polyhedra(Mesh))
			return;
			
		if (k3d::is_sds(Mesh))
			return;
			
		if (!SelectionState.select_faces)
			return;

		k3d::gl::store_attributes attributes;
		
		glDisable(GL_LIGHTING);

		glFrontFace(RenderState.inside_out ? GL_CCW : GL_CW);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		k3d::gl::set(GL_CULL_FACE, !SelectionState.select_backfacing);

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);
		
		cached_triangulation& triangles = get_data<cached_triangulation>(&Mesh, this); 
		
		const k3d::mesh::indices_t& face_starts = triangles.face_starts();
		if (face_starts.empty())
			return;
		const k3d::mesh::points_t& points = triangles.points();
		const cached_triangulation::indices_t& indices = triangles.indices();
		
		size_t face_count = Mesh.polyhedra->face_first_loops->size();
		for(size_t face = 0; face != face_count; ++face)
		{
			k3d::gl::push_selection_token(k3d::selection::ABSOLUTE_FACE, face);

			k3d::uint_t startindex = face_starts[face];
			k3d::uint_t endindex = face+1 == (face_starts.size()) ? indices.size() : face_starts[face+1];
			glBegin(GL_TRIANGLES);
			for (k3d::uint_t corner = startindex; corner != endindex; ++corner)
				k3d::gl::vertex3d(points[indices[corner]]);
			glEnd();

			k3d::gl::pop_selection_token(); // ABSOLUTE_FACE
		}
	}
	
	void on_mesh_changed(const k3d::mesh& Mesh, k3d::ihint* Hint)
	{
		if(!k3d::validate_polyhedra(Mesh))
			return;
			
		if (k3d::is_sds(Mesh))
			return;
		
		schedule_data<cached_triangulation>(&Mesh, Hint, this);
		schedule_data<normal_cache>(&Mesh, Hint, this);
	}
	
	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<face_painter, k3d::interface_list<k3d::gl::imesh_painter > > factory(
			k3d::uuid(0xd4abf63c, 0x2242c17e, 0x2afcb18d, 0x0a8ebdd5),
			"OpenGLFacePainter",
			_("Renders mesh faces, after trianglulating them (OpenGL 1.1)"),
			"OpenGL Painter",
			k3d::iplugin_factory::EXPERIMENTAL);

		return factory;
	}
};

/////////////////////////////////////////////////////////////////////////////
// face_painter_factory

k3d::iplugin_factory& face_painter_factory()
{
	return face_painter::get_factory();
}

} // namespace opengl

} // namespace painters

} // namespace module

