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

#include "nurbs_curve_modifier.h"

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
void connect_at_points(k3d::mesh& Mesh, k3d::uint_t curve1, k3d::uint_t curve2, k3d::uint_t point1, k3d::uint_t point2);

class connect_curves :
	public k3d::mesh_selection_sink<k3d::mesh_modifier<k3d::node > >
{
	typedef k3d::mesh_selection_sink<k3d::mesh_modifier<k3d::node > > base;
public:
	connect_curves(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document)
	{
		m_mesh_selection.changed_signal().connect(make_update_mesh_slot());
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
		std::vector<k3d::uint_t> points;

		const k3d::uint_t curve_begin = 0;
		const k3d::uint_t curve_end = nurbs->curve_first_points.size();
		for (k3d::uint_t curve = curve_begin; curve != curve_end; ++curve)
		{
			const k3d::uint_t curve_point_begin = nurbs->curve_first_points[curve];
			const k3d::uint_t curve_point_end = curve_point_begin + nurbs->curve_point_counts[curve];

			const k3d::mesh::weights_t& orig_weights = nurbs->curve_point_weights;

			boost::shared_ptr<k3d::mesh::weights_t> curve_point_weights(new k3d::mesh::weights_t());

			for (k3d::uint_t curve_point = curve_point_begin; curve_point != curve_point_end; ++curve_point)
			{
				if ((*Output.point_selection)[nurbs->curve_points[curve_point]])
				{
					curves.push_back(curve);
					points.push_back(curve_point);
				}
			}
		}

		if (curves.size() != 2 || points.size() != 2 || curves[0] == curves[1])
		{
			k3d::log() << error << "You need to select exactly 2 points on 2 different curves!\n" << "Selected: " << points.size() << " points on " << curves.size() << " curves" << std::endl;
		}
		else
		{
			nurbs_curve_modifier mod(Output, *nurbs);
			mod.join_curves(points[0], curves[0], points[1], curves[1]);
		}
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<connect_curves, k3d::interface_list<k3d::imesh_source, k3d::interface_list<k3d::imesh_sink > > > factory(
		  k3d::uuid(0x959eb84b, 0x544d0672, 0xd53d899b, 0x6a568e86),
		  "NurbsConnectCurves",
		  _("Connects a set of NURBS curves"),
		  "NURBS",
		  k3d::iplugin_factory::EXPERIMENTAL);

		return factory;
	}

private:
};

//Create connect_curve factory
k3d::iplugin_factory& connect_curves_factory()
{
	return connect_curves::get_factory();
}

}//namespace nurbs
} //namespace module
