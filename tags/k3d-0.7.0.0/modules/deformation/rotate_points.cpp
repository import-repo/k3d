// K-3D
// Copyright (c) 1995-2006, Timothy M. Shead
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
		\author Romain Behar (romainbehar@yahoo.com)
*/

#include <k3dsdk/algebra.h>
#include <k3dsdk/document_plugin_factory.h>
#include <k3d-i18n-config.h>
#include <k3dsdk/measurement.h>
#include <k3dsdk/mesh_simple_deformation_modifier.h>

namespace libk3ddeformation
{

/////////////////////////////////////////////////////////////////////////////
// rotate_points

class rotate_points :
	public k3d::mesh_simple_deformation_modifier
{
	typedef k3d::mesh_simple_deformation_modifier base;

public:
	rotate_points(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document),
		m_x(init_owner(*this) + init_name("x") + init_label(_("X")) + init_description(_("X rotation angle")) + init_value(0.0) + init_step_increment(k3d::radians(1.0)) + init_units(typeid(k3d::measurement::angle))),
		m_y(init_owner(*this) + init_name("y") + init_label(_("Y")) + init_description(_("Y rotation angle")) + init_value(0.0) + init_step_increment(k3d::radians(1.0)) + init_units(typeid(k3d::measurement::angle))),
		m_z(init_owner(*this) + init_name("z") + init_label(_("Z")) + init_description(_("Z rotation angle")) + init_value(0.0) + init_step_increment(k3d::radians(1.0)) + init_units(typeid(k3d::measurement::angle)))
	{
		m_mesh_selection.changed_signal().connect(make_update_mesh_slot());
		m_x.changed_signal().connect(make_update_mesh_slot());
		m_y.changed_signal().connect(make_update_mesh_slot());
		m_z.changed_signal().connect(make_update_mesh_slot());
	}

	void on_deform_mesh(const k3d::mesh::points_t& InputPoints, const k3d::mesh::selection_t& PointSelection, k3d::mesh::points_t& OutputPoints)
	{
		const k3d::matrix4 matrix = k3d::rotation3D(k3d::point3(m_x.pipeline_value(), m_y.pipeline_value(), m_z.pipeline_value()));

		const size_t point_begin = 0;
		const size_t point_end = point_begin + OutputPoints.size();
		for(size_t point = point_begin; point != point_end; ++point)
			OutputPoints[point] = k3d::mix(InputPoints[point], matrix * InputPoints[point], PointSelection[point]);
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<rotate_points,
			k3d::interface_list<k3d::imesh_source,
			k3d::interface_list<k3d::imesh_sink > > > factory(
				k3d::uuid(0x89a6a016, 0x3db04f26, 0xb626bb37, 0x6c812533),
				"RotatePoints",
				_("Rotates mesh points"),
				"Deformation",
				k3d::iplugin_factory::STABLE);

		return factory;
	}

private:
	k3d_data(double, immutable_name, change_signal, with_undo, local_storage, no_constraint, measurement_property, with_serialization) m_x;
	k3d_data(double, immutable_name, change_signal, with_undo, local_storage, no_constraint, measurement_property, with_serialization) m_y;
	k3d_data(double, immutable_name, change_signal, with_undo, local_storage, no_constraint, measurement_property, with_serialization) m_z;
};

/////////////////////////////////////////////////////////////////////////////
// rotate_points_factory

k3d::iplugin_factory& rotate_points_factory()
{
	return rotate_points::get_factory();
}

} // namespace libk3ddeformation
