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
	\author Tim Shead (tshead@k-3d.com)
*/

#include "detail.h"

namespace module
{

namespace quadrics
{

/////////////////////////////////////////////////////////////////////////////
// legacy_cone

class legacy_cone :
	public quadric
{
	typedef quadric base;

public:
	legacy_cone(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document),
		m_radius(init_owner(*this) + init_name("radius") + init_label(_("Radius")) + init_description(_("Base radius")) + init_value(5.0) + init_step_increment(0.1) + init_units(typeid(k3d::measurement::distance))),
		m_height(init_owner(*this) + init_name("height") + init_label(_("Height")) + init_description(_("Cone height")) + init_value(10.0) + init_step_increment(0.1) + init_units(typeid(k3d::measurement::distance))),
		m_thetamax(init_owner(*this) + init_name("thetamax") + init_label(_("Theta max")) + init_description(_("From RenderMan specification")) + init_value(k3d::radians(360.0)) + init_step_increment(k3d::radians(1.0)) + init_units(typeid(k3d::measurement::angle)))
	{
		m_radius.changed_signal().connect(sigc::mem_fun(*this, &legacy_cone::reset_geometry));
		m_height.changed_signal().connect(sigc::mem_fun(*this, &legacy_cone::reset_geometry));
		m_thetamax.changed_signal().connect(sigc::mem_fun(*this, &legacy_cone::reset_geometry));

		m_input_matrix.changed_signal().connect(make_async_redraw_slot());
		m_material.changed_signal().connect(make_async_redraw_slot());

		m_gl_v_knot_vector.insert(m_gl_v_knot_vector.end(), 2, 0);
		m_gl_v_knot_vector.insert(m_gl_v_knot_vector.end(), 1);
		m_gl_v_knot_vector.insert(m_gl_v_knot_vector.end(), 2, 2);

		add_snap_target(new k3d::snap_target(_("Tip"), sigc::mem_fun(*this, &legacy_cone::tip_target_position), sigc::mem_fun(*this, &legacy_cone::tip_target_orientation)));
	}


	bool tip_target_position(const k3d::point3& Position, k3d::point3& TargetPosition)
	{
		TargetPosition = k3d::point3(0, 0, m_height.pipeline_value());
		return true;
	}

	bool tip_target_orientation(const k3d::point3& Position, k3d::vector3& Look, k3d::vector3& Up)
	{
		return false;
	}

	void reset_geometry(k3d::iunknown*)
	{
		m_gl_control_points.clear();
		k3d::gl::redraw_all(document(), k3d::gl::irender_viewport::ASYNCHRONOUS);
	}

	const k3d::bounding_box3 extents()
	{
		const double radius = m_radius.pipeline_value();
		const double height = m_height.pipeline_value();

		return k3d::bounding_box3(radius, -radius, radius, -radius, height, 0.0);
	}

	void draw(const nurbs_renderer_t Nurbs)
	{
		if(m_gl_control_points.empty())
		{
			const double radius = m_radius.pipeline_value();
			const double height = m_height.pipeline_value();
			const double thetamax = m_thetamax.pipeline_value();

			if(thetamax == 0.0)
				return;

			k3d::mesh::knots_t knots;
			k3d::mesh::weights_t weights;
			k3d::mesh::points_t arc_points;
			k3d::nurbs_curve::circular_arc(k3d::vector3(1, 0, 0), k3d::vector3(0, 1, 0), 0, thetamax, 4, knots, weights, arc_points);

			m_gl_u_knot_vector.assign(knots.begin(), knots.end());

			for(unsigned long i = 0; i <= 2; ++i)
			{
				const double radius2 = k3d::mix(radius, 0.001, static_cast<double>(i) / static_cast<double>(2));
				const k3d::point3 offset = k3d::mix(0.0, height, static_cast<double>(i) / static_cast<double>(2)) * k3d::point3(0, 0, 1);

				for(unsigned long j = 0; j != arc_points.size(); ++j)
				{
					m_gl_control_points.push_back(weights[j] * (radius2 * arc_points[j][0] + offset[0]));
					m_gl_control_points.push_back(weights[j] * (radius2 * arc_points[j][1] + offset[1]));
					m_gl_control_points.push_back(weights[j] * (radius2 * arc_points[j][2] + offset[2]));
					m_gl_control_points.push_back(weights[j]);
				}
			}
		}

		gluBeginSurface(Nurbs);
		gluNurbsSurface(Nurbs, m_gl_u_knot_vector.size(), &m_gl_u_knot_vector[0], m_gl_v_knot_vector.size(), &m_gl_v_knot_vector[0], 4, 36, &m_gl_control_points[0], 3, 2, GL_MAP2_VERTEX_4);
		gluEndSurface(Nurbs);
	}

	void on_gl_draw(const k3d::gl::render_state& State)
	{
		k3d::gl::setup_material(m_material.pipeline_value());

		const nurbs_renderer_t nurbs = nurbs_renderer(State);

		k3d::gl::color3d(State.node_selection ? k3d::color(1, 1, 1) : k3d::color(0, 0, 0));
		gluNurbsProperty(nurbs, GLU_DISPLAY_MODE, GLU_OUTLINE_PATCH);
		glDisable(GL_LIGHTING);
		glDisable(GL_AUTO_NORMAL);
		draw(nurbs);

		if(!State.draw_two_sided)
			glEnable(GL_CULL_FACE);

		gluNurbsProperty(nurbs, GLU_DISPLAY_MODE, GLU_FILL);
		glEnable(GL_LIGHTING);
		glEnable(GL_AUTO_NORMAL);
		glPolygonOffset(1.0, 1.0);
		glEnable(GL_POLYGON_OFFSET_FILL);
		draw(nurbs);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	void on_gl_select(const k3d::gl::render_state& State, const k3d::gl::selection_state& SelectState)
	{
		const nurbs_renderer_t nurbs = nurbs_renderer(State);
		gluNurbsProperty(nurbs, GLU_DISPLAY_MODE, GLU_FILL);
		glDisable(GL_LIGHTING);
		glDisable(GL_AUTO_NORMAL);
		glDisable(GL_CULL_FACE);

		k3d::gl::push_selection_token(this);
		draw(nurbs);
		k3d::gl::pop_selection_token();
	}

	void on_renderman_render(const k3d::ri::render_state& State)
	{
		const double radius = m_radius.pipeline_value();
		const double height = m_height.pipeline_value();
		const double thetamax = k3d::degrees(m_thetamax.pipeline_value());

		k3d::ri::setup_material(m_material.pipeline_value(), State);
		State.stream.RiConeV(height, radius, thetamax);
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<legacy_cone,
			k3d::interface_list<k3d::itransform_source,
			k3d::interface_list<k3d::itransform_sink > > > factory(
				k3d::classes::Cone(),
				"LegacyCone",
				_("Cone primitive"),
				"Quadric",
				k3d::iplugin_factory::DEPRECATED);

		return factory;
	}

private:
	k3d_data(double, immutable_name, change_signal, with_undo, local_storage, no_constraint, measurement_property, with_serialization) m_radius;
	k3d_data(double, immutable_name, change_signal, with_undo, local_storage, no_constraint, measurement_property, with_serialization) m_height;
	k3d_data(double, immutable_name, change_signal, with_undo, local_storage, no_constraint, measurement_property, with_serialization) m_thetamax;

	std::vector<GLfloat> m_gl_u_knot_vector;
	std::vector<GLfloat> m_gl_v_knot_vector;
	std::vector<GLfloat> m_gl_control_points;
};

/////////////////////////////////////////////////////////////////////////////
// legacy_cone_factory

k3d::iplugin_factory& legacy_cone_factory()
{
	return legacy_cone::get_factory();
}

} // namespace quadrics

} // namespace module
