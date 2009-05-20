#ifndef K3DSDK_PYTHON_MESH_PYTHON_H
#define K3DSDK_PYTHON_MESH_PYTHON_H

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

#include "instance_wrapper_python.h"

#include <k3dsdk/mesh.h>
#include <boost/python/object.hpp>

namespace k3d
{

namespace python
{

typedef instance_wrapper<k3d::mesh> mesh_wrapper;

typedef instance_wrapper<k3d::mesh::primitive> mesh_primitive_wrapper;

typedef instance_wrapper<const k3d::mesh::primitive> const_mesh_primitive_wrapper;

class mesh :
	public instance_wrapper<k3d::mesh>
{
	typedef instance_wrapper<k3d::mesh> base;
public:
	mesh();
	mesh(k3d::mesh* Mesh);

	void copy(const mesh& RHS);

	boost::python::object create_nurbs_curve_groups();
	boost::python::object create_nurbs_patches();
	boost::python::object create_point_selection();
	boost::python::object create_points();
	boost::python::object create_polyhedra();
	boost::python::object nurbs_curve_groups();
	boost::python::object nurbs_patches();
	boost::python::object vertex_data();
	boost::python::object point_selection();
	boost::python::object points();
	boost::python::object polyhedra();
	boost::python::object writable_nurbs_curve_groups();
	boost::python::object writable_nurbs_patches();
	boost::python::object writable_vertex_data();
	boost::python::object writable_point_selection();
	boost::python::object writable_points();
	boost::python::object writable_polyhedra();

	const std::string repr();
	const std::string str();
};

void define_class_mesh();

} // namespace python

} // namespace k3d

#endif // !K3DSDK_PYTHON_MESH_PYTHON_H
