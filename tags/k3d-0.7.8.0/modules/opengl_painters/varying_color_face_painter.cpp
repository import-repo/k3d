// K-3D
// Copyright (c) 1995-2007, Timothy M. Shead
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
	\file Face varying color painter, adapted from uniform face color painter
	\author Timothy M. Shead (tshead@k-3d.com)
	\author Bart Janssens (bart.janssens@k-3d.com)
*/

#include <k3dsdk/document_plugin_factory.h>
#include <k3d-i18n-config.h>
#include <k3dsdk/mesh_painter_gl.h>
#include <k3dsdk/mesh_operations.h>
#include <k3dsdk/named_arrays.h>
#include <k3dsdk/named_array_operations.h>
#include <k3dsdk/painter_render_state_gl.h>
#include <k3dsdk/painter_selection_state_gl.h>
#include <k3dsdk/selection.h>

namespace module
{

namespace opengl
{

namespace painters
{

/////////////////////////////////////////////////////////////////////////////
// varying_color_face_painter

class varying_color_face_painter :
	public k3d::gl::mesh_painter
{
	typedef k3d::gl::mesh_painter base;

public:
	varying_color_face_painter(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document),
		m_color_array(init_owner(*this) + init_name("color_array") + init_label(_("Color Array")) + init_description(_("Specifies the array to be used for face colors")) + init_value(std::string("Cs")))
	{
	}

	void on_paint_mesh(const k3d::mesh& Mesh, const k3d::gl::painter_render_state& RenderState)
	{
		if(!k3d::validate_polyhedra(Mesh))
			return;

		if (k3d::is_sds(Mesh))
			return;

		const k3d::mesh::indices_t& face_first_loops = *Mesh.polyhedra->face_first_loops;
		const k3d::mesh::selection_t& face_selection = *Mesh.polyhedra->face_selection;
		const k3d::mesh::indices_t& loop_first_edges = *Mesh.polyhedra->loop_first_edges;
		const k3d::mesh::indices_t& edge_points = *Mesh.polyhedra->edge_points;
		const k3d::mesh::indices_t& clockwise_edges = *Mesh.polyhedra->clockwise_edges;
		const k3d::mesh::points_t& points = *Mesh.points;

		const k3d::uint_t edge_count = edge_points.size();
		const k3d::uint_t face_count = face_first_loops.size();

		// Calculate face normals ...
		k3d::typed_array<k3d::normal3> normals(face_count, k3d::normal3(0, 0, 1));
		for(k3d::uint_t face = 0; face != face_count; ++face)
			normals[face] = k3d::normal(edge_points, clockwise_edges, points, loop_first_edges[face_first_loops[face]]);

		// Define a default face color array (in case the user's choice of color array doesn't exist) ...
		k3d::typed_array<k3d::color> default_color_array;

		// Get the color array ...
		const k3d::typed_array<k3d::color>* color_array_p = k3d::get_array<k3d::typed_array<k3d::color> >(Mesh.polyhedra->face_varying_data, m_color_array.pipeline_value());
		if(!color_array_p)
		{
			default_color_array.resize(edge_count, k3d::color(0.9, 0.9, 0.9));
			color_array_p = &default_color_array;
		}

		const k3d::typed_array<k3d::color>& color_array = *color_array_p;
		return_if_fail(color_array.size() == edge_count);

		k3d::gl::store_attributes attributes;
		glEnable(GL_LIGHTING);

		glFrontFace(RenderState.inside_out ? GL_CCW : GL_CW);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		k3d::gl::set(GL_CULL_FACE, RenderState.draw_two_sided);

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);

		for(k3d::uint_t face = 0; face != face_count; ++face)
		{
			k3d::gl::normal3d(normals[face]);

			glBegin(GL_POLYGON);
			const k3d::uint_t first_edge = loop_first_edges[face_first_loops[face]];
			for(k3d::uint_t edge = first_edge; ; )
			{
				k3d::gl::material(GL_FRONT_AND_BACK, GL_DIFFUSE, color_array[edge]);
				k3d::gl::vertex3d(points[edge_points[edge]]);
				edge = clockwise_edges[edge];
				if(edge == first_edge)
					break;
			}
			glEnd();
		}
	}

	void on_select_mesh(const k3d::mesh& Mesh, const k3d::gl::painter_render_state& RenderState, const k3d::gl::painter_selection_state& SelectionState)
	{
		if(!SelectionState.select_faces)
			return;

		if(!k3d::validate_polyhedra(Mesh))
			return;

		if (k3d::is_sds(Mesh))
			return;

		const k3d::mesh::indices_t& face_first_loops = *Mesh.polyhedra->face_first_loops;
		const k3d::mesh::indices_t& loop_first_edges = *Mesh.polyhedra->loop_first_edges;
		const k3d::mesh::indices_t& edge_points = *Mesh.polyhedra->edge_points;
		const k3d::mesh::indices_t& clockwise_edges = *Mesh.polyhedra->clockwise_edges;
		const k3d::mesh::points_t& points = *Mesh.points;

		const k3d::uint_t face_count = face_first_loops.size();

		k3d::gl::store_attributes attributes;
		glDisable(GL_LIGHTING);

		glFrontFace(RenderState.inside_out ? GL_CCW : GL_CW);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		k3d::gl::set(GL_CULL_FACE, RenderState.draw_two_sided);

		for(k3d::uint_t face = 0; face != face_count; ++face)
		{
			k3d::gl::push_selection_token(k3d::selection::ABSOLUTE_FACE, face);

			glBegin(GL_POLYGON);
			const k3d::uint_t first_edge = loop_first_edges[face_first_loops[face]];
			for(k3d::uint_t edge = first_edge; ; )
			{
				k3d::gl::vertex3d(points[edge_points[edge]]);
				edge = clockwise_edges[edge];
				if(edge == first_edge)
					break;
			}
			glEnd();

			k3d::gl::pop_selection_token(); // ABSOLUTE_FACE
		}
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<varying_color_face_painter, k3d::interface_list<k3d::gl::imesh_painter > > factory(
			k3d::uuid(0xf766bef1, 0xa140be8b, 0x807de9a9, 0x1894693d),
			"OpenGLVaryingColorFacePainter",
			_("Renders mesh faces using a face-varying color array"),
			"OpenGL Painter",
			k3d::iplugin_factory::EXPERIMENTAL);

		return factory;
	}

private:
	k3d_data(std::string, immutable_name, change_signal, with_undo, local_storage, no_constraint, writable_property, with_serialization) m_color_array;
};

/////////////////////////////////////////////////////////////////////////////
// varying_color_face_painter_factory

k3d::iplugin_factory& varying_color_face_painter_factory()
{
	return varying_color_face_painter::get_factory();
}

} // namespace painters

} // namespace opengl

} // namespace module


