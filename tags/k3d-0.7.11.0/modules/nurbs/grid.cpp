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
#include <k3dsdk/material_sink.h>
#include <k3dsdk/measurement.h>
#include <k3dsdk/mesh_source.h>
#include <k3dsdk/node.h>
#include <k3dsdk/nurbs_patch.h>

#include <boost/scoped_ptr.hpp>

namespace module
{

namespace nurbs
{

/////////////////////////////////////////////////////////////////////////////
// grid

class grid :
	public k3d::material_sink<k3d::mesh_source<k3d::node > >
{
	typedef k3d::material_sink<k3d::mesh_source<k3d::node > > base;

public:
	grid(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document),
		m_u_order(init_owner(*this) + init_name("u_order") + init_label(_("u_order")) + init_description(_("U Order")) + init_value(4) + init_constraint(constraint::minimum<k3d::int32_t>(2)) + init_step_increment(1) + init_units(typeid(k3d::measurement::scalar))),
		m_v_order(init_owner(*this) + init_name("v_order") + init_label(_("v_order")) + init_description(_("V Order")) + init_value(4) + init_constraint(constraint::minimum<k3d::int32_t>(2)) + init_step_increment(1) + init_units(typeid(k3d::measurement::scalar))),
		m_u_segments(init_owner(*this) + init_name("u_segments") + init_label(_("u_segments")) + init_description(_("Columns")) + init_value(1) + init_constraint(constraint::minimum<k3d::int32_t>(1)) + init_step_increment(1) + init_units(typeid(k3d::measurement::scalar))),
		m_v_segments(init_owner(*this) + init_name("v_segments") + init_label(_("v_segments")) + init_description(_("Rows")) + init_value(1) + init_constraint(constraint::minimum<k3d::int32_t>(1)) + init_step_increment(1) + init_units(typeid(k3d::measurement::scalar))),
		m_width(init_owner(*this) + init_name("width") + init_label(_("width")) + init_description(_("Width")) + init_value(10.0) + init_step_increment(0.1) + init_units(typeid(k3d::measurement::distance))),
		m_height(init_owner(*this) + init_name("height") + init_label(_("height")) + init_description(_("Height")) + init_value(10.0) + init_step_increment(0.1) + init_units(typeid(k3d::measurement::distance)))
	{
		m_material.changed_signal().connect(k3d::hint::converter<
			k3d::hint::convert<k3d::hint::any, k3d::hint::none> >(make_update_mesh_slot()));
		m_u_order.changed_signal().connect(k3d::hint::converter<
			k3d::hint::convert<k3d::hint::any, k3d::hint::none> >(make_update_mesh_slot()));
		m_v_order.changed_signal().connect(k3d::hint::converter<
			k3d::hint::convert<k3d::hint::any, k3d::hint::none> >(make_update_mesh_slot()));
		m_u_segments.changed_signal().connect(k3d::hint::converter<
			k3d::hint::convert<k3d::hint::any, k3d::hint::none> >(make_update_mesh_slot()));
		m_v_segments.changed_signal().connect(k3d::hint::converter<
			k3d::hint::convert<k3d::hint::any, k3d::hint::none> >(make_update_mesh_slot()));
		m_width.changed_signal().connect(k3d::hint::converter<
			k3d::hint::convert<k3d::hint::any, k3d::hint::none> >(make_update_mesh_slot()));
		m_height.changed_signal().connect(k3d::hint::converter<
			k3d::hint::convert<k3d::hint::any, k3d::hint::none> >(make_update_mesh_slot()));
	}

	void on_update_mesh_topology(k3d::mesh& Output)
	{
		Output = k3d::mesh();

		k3d::imaterial* const material = m_material.pipeline_value();
		const k3d::int32_t u_order = m_u_order.pipeline_value();
		const k3d::int32_t v_order = m_v_order.pipeline_value();
		const k3d::int32_t u_segments = m_u_segments.pipeline_value();
		const k3d::int32_t v_segments = m_v_segments.pipeline_value();
		const k3d::double_t width = m_width.pipeline_value();
		const k3d::double_t height = m_height.pipeline_value();

		const k3d::int32_t u_degree = u_order - 1;
		const k3d::int32_t v_degree = v_order - 1;
		const k3d::int32_t u_points = (u_segments * u_degree) + 1;
		const k3d::int32_t v_points = (v_segments * v_degree) + 1;

		// Create patch ...
		k3d::mesh::points_t& points = Output.points.create();
		k3d::mesh::selection_t& point_selection = Output.point_selection.create();
		boost::scoped_ptr<k3d::nurbs_patch::primitive> primitive(k3d::nurbs_patch::create(Output));
		
		primitive->patch_first_points.push_back(primitive->patch_points.size());
		primitive->patch_u_point_counts.push_back(u_points);
		primitive->patch_v_point_counts.push_back(v_points);
		primitive->patch_u_orders.push_back(u_order);
		primitive->patch_v_orders.push_back(v_order);
		primitive->patch_u_first_knots.push_back(primitive->patch_u_knots.size());
		primitive->patch_v_first_knots.push_back(primitive->patch_v_knots.size());
		primitive->patch_selections.push_back(0);
		primitive->patch_materials.push_back(material);

		for(k3d::int32_t row = 0; row != v_points; ++row)
		{
			const k3d::double_t row_percent = 0.5 - (static_cast<k3d::double_t>(row) / static_cast<k3d::double_t>(v_points - 1));

			for(k3d::int32_t column = 0; column != u_points; ++column)
			{
				const k3d::double_t column_percent = (static_cast<k3d::double_t>(column) / static_cast<k3d::double_t>(u_points - 1)) - 0.5;

				primitive->patch_points.push_back(points.size());
				primitive->patch_point_weights.push_back(1);

				points.push_back(k3d::point3(width * column_percent, height * row_percent, 0));
				point_selection.push_back(0);
			}
		}

		primitive->patch_u_knots.insert(primitive->patch_u_knots.end(), u_order, 0);
		for(k3d::int32_t i = 1; i != u_segments; ++i)
			primitive->patch_u_knots.insert(primitive->patch_u_knots.end(), u_order - 1, i);
		primitive->patch_u_knots.insert(primitive->patch_u_knots.end(), u_order, u_segments);

		primitive->patch_v_knots.insert(primitive->patch_v_knots.end(), v_order, 0);
		for(k3d::int32_t i = 1; i != v_segments; ++i)
			primitive->patch_v_knots.insert(primitive->patch_v_knots.end(), v_order - 1, i);
		primitive->patch_v_knots.insert(primitive->patch_v_knots.end(), v_order, v_segments);

		primitive->patch_trim_loop_counts.push_back(0);
		primitive->patch_first_trim_loops.push_back(0);
	}

	void on_update_mesh_geometry(k3d::mesh& Output)
	{
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<grid, k3d::interface_list<k3d::imesh_source > > factory(
		  k3d::uuid(0x5aac4e72, 0xf9b04b61, 0xf8b1bdbc, 0x851cf62e),
		  "NurbsGrid",
		  _("Generates a NURBS grid"),
		  "NURBS",
		  k3d::iplugin_factory::STABLE);

		return factory;
	}

private:
	k3d_data(k3d::int32_t, immutable_name, change_signal, with_undo, local_storage, with_constraint, measurement_property, with_serialization) m_u_order;
	k3d_data(k3d::int32_t, immutable_name, change_signal, with_undo, local_storage, with_constraint, measurement_property, with_serialization) m_v_order;
	k3d_data(k3d::int32_t, immutable_name, change_signal, with_undo, local_storage, with_constraint, measurement_property, with_serialization) m_u_segments;
	k3d_data(k3d::int32_t, immutable_name, change_signal, with_undo, local_storage, with_constraint, measurement_property, with_serialization) m_v_segments;
	k3d_data(k3d::double_t, immutable_name, change_signal, with_undo, local_storage, no_constraint, measurement_property, with_serialization) m_width;
	k3d_data(k3d::double_t, immutable_name, change_signal, with_undo, local_storage, no_constraint, measurement_property, with_serialization) m_height;
};

/////////////////////////////////////////////////////////////////////////////
// grid_factory

k3d::iplugin_factory& grid_factory()
{
	return grid::get_factory();
}

} // namespace nurbs

} // namespace module


