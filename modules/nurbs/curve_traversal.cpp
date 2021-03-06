// K-3D
// Copyright (c) 1995-2004, Timothy M. Shead
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
	\author Carsten Haubold (CarstenHaubold@web.de)
*/

#include "nurbs_patch_modifier.h"

#include <k3dsdk/data.h>
#include <k3dsdk/document_plugin_factory.h>
#include <k3dsdk/geometry.h>
#include <k3dsdk/log.h>
#include <k3dsdk/material_sink.h>
#include <k3dsdk/measurement.h>
#include <k3dsdk/mesh.h>
#include <k3dsdk/mesh_modifier.h>
#include <k3dsdk/mesh_selection_sink.h>
#include <k3dsdk/mesh_source.h>
#include <k3dsdk/module.h>
#include <k3dsdk/node.h>
#include <k3dsdk/nurbs_curve.h>
#include <k3dsdk/point3.h>
#include <k3dsdk/selection.h>

#include <boost/scoped_ptr.hpp>

#include <iostream>
#include <vector>

namespace module
{

namespace nurbs
{

class curve_traversal :
	public k3d::mesh_selection_sink<k3d::mesh_modifier<k3d::node > >
{
	typedef k3d::mesh_selection_sink<k3d::mesh_modifier<k3d::node > > base;
public:
	curve_traversal(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document),
		m_create_caps(init_owner(*this) + init_name(_("create_caps")) + init_label(_("Create caps?")) + init_description(_("Create caps at both ends of the revolved curve?")) + init_value(false)),
		m_delete_original(init_owner(*this) + init_name(_("delete_original")) + init_label(_("Delete the Curves")) + init_description(_("Delete the curves used to construct the surface")) + init_value(true))
	{
		m_mesh_selection.changed_signal().connect(make_update_mesh_slot());
		m_create_caps.changed_signal().connect(make_update_mesh_slot());
		m_delete_original.changed_signal().connect(make_update_mesh_slot());
	}

	void on_create_mesh(const k3d::mesh& Input, k3d::mesh& Output)
	{
		Output = Input;
	}

	void on_update_mesh(const k3d::mesh& Input, k3d::mesh& Output)
	{
		Output = Input;

		boost::scoped_ptr<k3d::nurbs_curve::primitive> nurbs(get_first_nurbs_curve(Output));
		if(!nurbs)
			return;

		k3d::geometry::merge_selection(m_mesh_selection.pipeline_value(), Output);

		std::vector<k3d::uint_t> curves;

		const k3d::uint_t curve_begin = 0;
		const k3d::uint_t curve_end = nurbs->curve_first_points.size();
		for (k3d::uint_t curve = curve_begin; curve != curve_end; ++curve)
		{
			if (nurbs->curve_selections[curve] > 0.0)
				curves.push_back(curve);
		}

		if (curves.size() != 2)
		{
			k3d::log() << error << nurbs_debug << "You need to select 2 curves!\n" << std::endl;
		}
		else
		{
			nurbs_curve_modifier mod(Output, *nurbs);
			mod.traverse_curve(curves[0], curves[1], m_create_caps.pipeline_value());

			if (m_delete_original.pipeline_value())
			{
				mod.delete_curve(curves[0]);
				mod.delete_curve(curves[1]);
			}
		}
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<curve_traversal, k3d::interface_list<k3d::imesh_source, k3d::interface_list<k3d::imesh_sink > > > factory(
		  k3d::uuid(0x9e316d5f, 0x9945f986, 0xbb2f70b6, 0xec54bada),
		  "NurbsCurveTraversal",
		  _("Creates a NURBS surface while traversing one NURBS curve along another"),
		  "NURBS",
		  k3d::iplugin_factory::EXPERIMENTAL);

		return factory;
	}

private:
	k3d_data(k3d::bool_t, immutable_name, change_signal, with_undo, local_storage, no_constraint, writable_property, with_serialization) m_create_caps;
	k3d_data(k3d::bool_t, immutable_name, change_signal, with_undo, local_storage, no_constraint, writable_property, with_serialization) m_delete_original;
};

//Create connect_curve factory
k3d::iplugin_factory& curve_traversal_factory()
{
	return curve_traversal::get_factory();
}

} //namespace nurbs

} //namespace module

