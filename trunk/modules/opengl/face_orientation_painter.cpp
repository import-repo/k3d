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
	\author Timothy M. Shead (tshead@k-3d.com)
*/

#include "utility.h"

#include <k3dsdk/document_plugin_factory.h>
#include <k3dsdk/i18n.h>
#include <k3dsdk/mesh_painter_gl.h>
#include <k3dsdk/mesh.h>
#include <k3dsdk/painter_render_state_gl.h>
#include <k3dsdk/persistent.h>
#include <k3dsdk/selection.h>

namespace libk3dopengl
{

/////////////////////////////////////////////////////////////////////////////
// face_orientation_painter

class face_orientation_painter :
	public k3d::gl::mesh_painter
{
	typedef k3d::gl::mesh_painter base;

public:
	face_orientation_painter(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document),
		m_draw_selected(init_owner(*this) + init_name("draw_selected") + init_label(_("Draw Selected")) + init_description(_("Draw normals for selected polygons")) + init_value(true)),
		m_draw_unselected(init_owner(*this) + init_name("draw_unselected") + init_label(_("Draw Unselected")) + init_description(_("Draw normals for unselected polygons")) + init_value(false)),
		m_selected_color(init_owner(*this) + init_name("selected_color") + init_label(_("Selected Color")) + init_description(_("Normal color for selected polygons")) + init_value(k3d::color(0, 1, 1))),
		m_unselected_color(init_owner(*this) + init_name("unselected_color") + init_label(_("Unselected Color")) + init_description(_("Normal color for unselected polygons")) + init_value(k3d::color(0, 0.6, 0.6)))
	{
		m_draw_selected.changed_signal().connect(make_async_redraw_slot());
		m_draw_unselected.changed_signal().connect(make_async_redraw_slot());
		m_selected_color.changed_signal().connect(make_async_redraw_slot());
		m_unselected_color.changed_signal().connect(make_async_redraw_slot());
	}

	template<typename FunctorT>
	void draw(const k3d::dev::mesh& Mesh, const k3d::typed_array<k3d::point3>& Centers, const k3d::color& Color, const FunctorT& FaceTest)
	{
		const k3d::dev::mesh::indices_t& face_first_loops = *Mesh.polyhedra->face_first_loops;
		const k3d::dev::mesh::indices_t& loop_first_edges = *Mesh.polyhedra->loop_first_edges;
		const k3d::dev::mesh::indices_t& edge_points = *Mesh.polyhedra->edge_points;
		const k3d::dev::mesh::indices_t& clockwise_edges = *Mesh.polyhedra->clockwise_edges;
		const k3d::dev::mesh::points_t& points = *Mesh.points;

		const size_t face_count = face_first_loops.size();

		k3d::gl::color3d(Color);

		glBegin(GL_LINES);
		for(size_t face = 0; face != face_count; ++face)
		{
			if(FaceTest(face))
			{
				const k3d::point3 first_point = points[edge_points[loop_first_edges[face_first_loops[face]]]];
				const k3d::point3 second_point = points[edge_points[clockwise_edges[loop_first_edges[face_first_loops[face]]]]];
				const k3d::point3 center_point = Centers[face];

				const k3d::point3 glyph_point1 = ((first_point - center_point) * 0.9) + center_point;
				const k3d::point3 glyph_point2 = ((second_point - center_point) * 0.9) + center_point;
				const k3d::point3 glyph_point3 = ((second_point - center_point) * 0.8) + center_point;

				k3d::gl::vertex3d(glyph_point1);
				k3d::gl::vertex3d(glyph_point2);
				k3d::gl::vertex3d(glyph_point2);
				k3d::gl::vertex3d(glyph_point3);
			}
		}
		glEnd();
	}

	void on_paint_mesh(const k3d::dev::mesh& Mesh, const k3d::gl::painter_render_state& RenderState)
	{
		const bool draw_selected = m_draw_selected.value() && RenderState.show_component_selection;
		const bool draw_unselected = m_draw_unselected.value();
		if(!draw_selected && !draw_unselected)
			return;

		if(!k3d::dev::validate_polyhedra(Mesh))
			return;

		const k3d::dev::mesh::indices_t& face_first_loops = *Mesh.polyhedra->face_first_loops;
		const k3d::dev::mesh::indices_t& loop_first_edges = *Mesh.polyhedra->loop_first_edges;
		const k3d::dev::mesh::indices_t& edge_points = *Mesh.polyhedra->edge_points;
		const k3d::dev::mesh::indices_t& clockwise_edges = *Mesh.polyhedra->clockwise_edges;
		const k3d::dev::mesh::points_t& points = *Mesh.points;

		const size_t face_count = face_first_loops.size();

		// Calculate face centers ...
		k3d::typed_array<k3d::point3> centers(face_count);
		for(size_t face = 0; face != face_count; ++face)
			centers[face] = k3d::dev::center(edge_points, clockwise_edges, points, loop_first_edges[face_first_loops[face]]);

		k3d::gl::store_attributes attributes;
		glDisable(GL_LIGHTING);

		if(draw_selected)
			draw(Mesh, centers, m_selected_color.value(), selected_faces(Mesh));

		if(draw_unselected)
			draw(Mesh, centers, m_unselected_color.value(), unselected_faces(Mesh));
	}
	
	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<face_orientation_painter, k3d::interface_list<k3d::gl::imesh_painter > > factory(
			k3d::uuid(0x7bc1ee5f, 0x45802906, 0x323984bf, 0xcbe858d6),
			"OpenGLFaceOrientationPainter",
			_("Draws arrows to show polyhedron face orientation"),
			"OpenGL Painters",
			k3d::iplugin_factory::EXPERIMENTAL);

		return factory;
	}

private:
	k3d_data(bool, immutable_name, change_signal, with_undo, local_storage, no_constraint, writable_property, with_serialization) m_draw_selected;
	k3d_data(bool, immutable_name, change_signal, with_undo, local_storage, no_constraint, writable_property, with_serialization) m_draw_unselected;
	k3d_data(k3d::color, immutable_name, change_signal, with_undo, local_storage, no_constraint, writable_property, with_serialization) m_selected_color;
	k3d_data(k3d::color, immutable_name, change_signal, with_undo, local_storage, no_constraint, writable_property, with_serialization) m_unselected_color;
};

/////////////////////////////////////////////////////////////////////////////
// face_orientation_painter_factory

k3d::iplugin_factory& face_orientation_painter_factory()
{
	return face_orientation_painter::get_factory();
}

} // namespace libk3dopengl

