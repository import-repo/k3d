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

#include "iunknown_python.h"
#include "mesh_python.h"
#include "owned_instance_wrapper_python.h"
#include "polyhedron_python.h"
#include "utility_python.h"

#include <k3dsdk/imaterial.h>
#include <k3dsdk/polyhedron.h>

#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>
using namespace boost::python;
#include <boost/scoped_ptr.hpp>

namespace k3d
{

namespace python
{

class polyhedron
{
public:
	class const_primitive
	{
	public:
		typedef owned_instance_wrapper<k3d::polyhedron::const_primitive> wrapper;

		static object shell_first_faces(wrapper& Self) { return wrap(Self.wrapped().shell_first_faces); }
		static object shell_face_counts(wrapper& Self) { return wrap(Self.wrapped().shell_face_counts); }
		static object shell_types(wrapper& Self) { return wrap(Self.wrapped().shell_types); }
		static object face_first_loops(wrapper& Self) { return wrap(Self.wrapped().face_first_loops); }
		static object face_loop_counts(wrapper& Self) { return wrap(Self.wrapped().face_loop_counts); }
		static object face_selections(wrapper& Self) { return wrap(Self.wrapped().face_selections); }
		static object face_materials(wrapper& Self) { return wrap(Self.wrapped().face_materials); }
		static object loop_first_edges(wrapper& Self) { return wrap(Self.wrapped().loop_first_edges); }
		static object edge_points(wrapper& Self) { return wrap(Self.wrapped().edge_points); }
		static object clockwise_edges(wrapper& Self) { return wrap(Self.wrapped().clockwise_edges); }
		static object edge_selections(wrapper& Self) { return wrap(Self.wrapped().edge_selections); }
		static object constant_data(wrapper& Self) { return wrap(Self.wrapped().constant_data); }
		static object uniform_data(wrapper& Self) { return wrap(Self.wrapped().uniform_data); }
		static object face_varying_data(wrapper& Self) { return wrap(Self.wrapped().face_varying_data); }
	};

	class primitive
	{
	public:
		typedef owned_instance_wrapper<k3d::polyhedron::primitive> wrapper;

		static object shell_first_faces(wrapper& Self) { return wrap(Self.wrapped().shell_first_faces); }
		static object shell_face_counts(wrapper& Self) { return wrap(Self.wrapped().shell_face_counts); }
		static object shell_types(wrapper& Self) { return wrap(Self.wrapped().shell_types); }
		static object face_first_loops(wrapper& Self) { return wrap(Self.wrapped().face_first_loops); }
		static object face_loop_counts(wrapper& Self) { return wrap(Self.wrapped().face_loop_counts); }
		static object face_selections(wrapper& Self) { return wrap(Self.wrapped().face_selections); }
		static object face_materials(wrapper& Self) { return wrap(Self.wrapped().face_materials); }
		static object loop_first_edges(wrapper& Self) { return wrap(Self.wrapped().loop_first_edges); }
		static object edge_points(wrapper& Self) { return wrap(Self.wrapped().edge_points); }
		static object clockwise_edges(wrapper& Self) { return wrap(Self.wrapped().clockwise_edges); }
		static object edge_selections(wrapper& Self) { return wrap(Self.wrapped().edge_selections); }
		static object constant_data(wrapper& Self) { return wrap(Self.wrapped().constant_data); }
		static object uniform_data(wrapper& Self) { return wrap(Self.wrapped().uniform_data); }
		static object face_varying_data(wrapper& Self) { return wrap(Self.wrapped().face_varying_data); }
	};


	static object create(mesh& Mesh)
	{
		return wrap_owned(k3d::polyhedron::create(Mesh.wrapped()));
	}

	static object create2(mesh& Mesh, list Vertices, list VertexCounts, list VertexIndices, object Material)
	{
		k3d::mesh::points_t vertices;
		k3d::mesh::counts_t vertex_counts;
		k3d::mesh::indices_t vertex_indices;
		k3d::imaterial* const material = Material ? &dynamic_cast<k3d::imaterial&>(boost::python::extract<iunknown_wrapper>(Material)().wrapped())  : 0;

		utility::copy(Vertices, vertices);
		utility::copy(VertexCounts, vertex_counts);
		utility::copy(VertexIndices, vertex_indices);

		return wrap_owned(k3d::polyhedron::create(Mesh.wrapped(), vertices, vertex_counts, vertex_indices, material));
	}

	static object validate(mesh_primitive_wrapper& Primitive)
	{
		return wrap_owned(k3d::polyhedron::validate(Primitive.wrapped()));
	}

	static object validate_const(const_mesh_primitive_wrapper& Primitive)
	{
		return wrap_owned(k3d::polyhedron::validate(Primitive.wrapped()));
	}

	static bool_t is_triangles(polyhedron::const_primitive::wrapper& Polyhedron)
	{
		return k3d::polyhedron::is_triangles(Polyhedron.wrapped());
	}

	static bool_t is_triangles2(polyhedron::primitive::wrapper& Polyhedron)
	{
		return k3d::polyhedron::is_triangles(Polyhedron.wrapped());
	}

	static bool_t is_solid(polyhedron::const_primitive::wrapper& Polyhedron)
	{
		return k3d::polyhedron::is_solid(Polyhedron.wrapped());
	}

	static bool_t is_solid2(polyhedron::primitive::wrapper& Polyhedron)
	{
		return k3d::polyhedron::is_solid(Polyhedron.wrapped());
	}
};

void define_namespace_polyhedron()
{
	scope outer = class_<polyhedron>("polyhedron",
		"Provides functionality to create and manipulate polyhedron mesh primitives.", no_init)
		.def("create", &polyhedron::create,
			"Creates an empty polyhedron, returning an object that can be used to access all of its arrays.")
		.def("create", &polyhedron::create2,
			"Creates a polyhedron, populating it from a list of vertices, a list of per-face vertex counts, a list of per-face vertices, and an optional material node.")
		.staticmethod("create")
		.def("validate", &polyhedron::validate)
		.def("validate", &polyhedron::validate_const)
		.staticmethod("validate")
		.def("is_triangles", &polyhedron::is_triangles)
		.def("is_triangles", &polyhedron::is_triangles2)
		.staticmethod("is_triangles")
		.def("is_solid", &polyhedron::is_solid)
		.def("is_solid", &polyhedron::is_solid2)
		.staticmethod("is_solid")
		;

	enum_<k3d::polyhedron::polyhedron_type>("shell_type")
		.value("POLYGONS", k3d::polyhedron::POLYGONS)
		.value("CATMULL_CLARK", k3d::polyhedron::CATMULL_CLARK)
		.attr("__module__") = "k3d";

	class_<polyhedron::const_primitive::wrapper>("const_primitive", no_init)
		.def("shell_first_faces", &polyhedron::const_primitive::shell_first_faces)
		.def("shell_face_counts", &polyhedron::const_primitive::shell_face_counts)
		.def("shell_types", &polyhedron::const_primitive::shell_types)
		.def("face_first_loops", &polyhedron::const_primitive::face_first_loops)
		.def("face_loop_counts", &polyhedron::const_primitive::face_loop_counts)
		.def("face_selections", &polyhedron::const_primitive::face_selections)
		.def("face_materials", &polyhedron::const_primitive::face_materials)
		.def("loop_first_edges", &polyhedron::const_primitive::loop_first_edges)
		.def("edge_points", &polyhedron::const_primitive::edge_points)
		.def("clockwise_edges", &polyhedron::const_primitive::clockwise_edges)
		.def("edge_selections", &polyhedron::const_primitive::edge_selections)
		.def("constant_data", &polyhedron::const_primitive::constant_data)
		.def("uniform_data", &polyhedron::const_primitive::uniform_data)
		.def("face_varying_data", &polyhedron::const_primitive::face_varying_data)
		;

	class_<polyhedron::primitive::wrapper>("primitive", no_init)
		.def("shell_first_faces", &polyhedron::primitive::shell_first_faces)
		.def("shell_face_counts", &polyhedron::primitive::shell_face_counts)
		.def("shell_types", &polyhedron::primitive::shell_types)
		.def("face_first_loops", &polyhedron::primitive::face_first_loops)
		.def("face_loop_counts", &polyhedron::primitive::face_loop_counts)
		.def("face_selections", &polyhedron::primitive::face_selections)
		.def("face_materials", &polyhedron::primitive::face_materials)
		.def("loop_first_edges", &polyhedron::primitive::loop_first_edges)
		.def("edge_points", &polyhedron::primitive::edge_points)
		.def("clockwise_edges", &polyhedron::primitive::clockwise_edges)
		.def("edge_selections", &polyhedron::primitive::edge_selections)
		.def("constant_data", &polyhedron::primitive::constant_data)
		.def("uniform_data", &polyhedron::primitive::uniform_data)
		.def("face_varying_data", &polyhedron::primitive::face_varying_data)
		;
}

} // namespace python

} // namespace k3d

