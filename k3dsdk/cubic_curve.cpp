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

#include "cubic_curve.h"
#include "metadata_keys.h"
#include "primitive_validation.h"
#include "selection.h"
#include "string_cast.h"

#include <numeric>

namespace k3d
{

namespace cubic_curve
{

/////////////////////////////////////////////////////////////////////////////////////////////
// const_primitive

const_primitive::const_primitive(
	const mesh::bools_t& Periodic,
	const mesh::materials_t& Material,
	const mesh::indices_t& CurveFirstPoints,
	const mesh::counts_t& CurvePointCounts,
	const mesh::selection_t& CurveSelections,
	const mesh::indices_t& CurvePoints,
	const mesh::attribute_arrays_t& ConstantData,
	const mesh::attribute_arrays_t& UniformData,
	const mesh::attribute_arrays_t& VaryingData
		) :
	periodic(Periodic),
	material(Material),
	curve_first_points(CurveFirstPoints),
	curve_point_counts(CurvePointCounts),
	curve_selections(CurveSelections),
	curve_points(CurvePoints),
	constant_data(ConstantData),
	uniform_data(UniformData),
	varying_data(VaryingData)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
// primitive

primitive::primitive(
	mesh::bools_t& Periodic,
	mesh::materials_t& Material,
	mesh::indices_t& CurveFirstPoints,
	mesh::counts_t& CurvePointCounts,
	mesh::selection_t& CurveSelections,
	mesh::indices_t& CurvePoints,
	mesh::attribute_arrays_t& ConstantData,
	mesh::attribute_arrays_t& UniformData,
	mesh::attribute_arrays_t& VaryingData
		) :
	periodic(Periodic),
	material(Material),
	curve_first_points(CurveFirstPoints),
	curve_point_counts(CurvePointCounts),
	curve_selections(CurveSelections),
	curve_points(CurvePoints),
	constant_data(ConstantData),
	uniform_data(UniformData),
	varying_data(VaryingData)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
// create

primitive* create(mesh& Mesh)
{
	mesh::primitive& generic_primitive = Mesh.primitives.create("cubic_curve");

	primitive* const result = new primitive(
		generic_primitive.structure.create<mesh::bools_t>("periodic"),
		generic_primitive.structure.create<mesh::materials_t>("material"),
		generic_primitive.structure.create<mesh::indices_t>("curve_first_points"),
		generic_primitive.structure.create<mesh::counts_t>("curve_point_counts"),
		generic_primitive.structure.create<mesh::selection_t>("curve_selections"),
		generic_primitive.structure.create<mesh::indices_t>("curve_points"),
		generic_primitive.attributes["constant"],
		generic_primitive.attributes["uniform"],
		generic_primitive.attributes["varying"]
		);

	result->curve_selections.set_metadata_value(metadata::key::selection_component(), string_cast(selection::UNIFORM));
	result->curve_points.set_metadata_value(metadata::key::domain(), metadata::value::mesh_point_indices_domain());

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// validate

const_primitive* validate(const mesh::primitive& Primitive)
{
	if(Primitive.type != "cubic_curve")
		return 0;

	try
	{
		const mesh::bools_t& periodic = require_const_array<mesh::bools_t>(Primitive, "periodic");
		const mesh::materials_t& material = require_const_array<mesh::materials_t>(Primitive, "material");
		const mesh::indices_t& curve_first_points = require_const_array<mesh::indices_t>(Primitive, "curve_first_points");
		const mesh::counts_t& curve_point_counts = require_const_array<mesh::counts_t>(Primitive, "curve_point_counts");
		const mesh::selection_t& curve_selections = require_const_array<mesh::selection_t>(Primitive, "curve_selections");
		const mesh::indices_t& curve_points = require_const_array<mesh::indices_t>(Primitive, "curve_points");

		const mesh::attribute_arrays_t& constant_data = require_const_attribute_arrays(Primitive, "constant");
		const mesh::attribute_arrays_t& uniform_data = require_const_attribute_arrays(Primitive, "uniform");
		const mesh::attribute_arrays_t& varying_data = require_const_attribute_arrays(Primitive, "varying");

		require_metadata(Primitive, curve_selections, "curve_selections", metadata::key::selection_component(), string_cast(selection::UNIFORM));
		require_metadata(Primitive, curve_points, "curve_points", metadata::key::domain(), metadata::value::mesh_point_indices_domain());

		require_array_size(Primitive, periodic, "periodic", 1);
		require_array_size(Primitive, material, "material", 1);
		require_array_size(Primitive, curve_point_counts, "curve_point_counts", curve_first_points.size());
		require_array_size(Primitive, curve_selections, "curve_selections", curve_first_points.size());
		require_array_size(Primitive, curve_points, "curve_points", std::accumulate(curve_point_counts.begin(), curve_point_counts.end(), 0));

		require_attribute_arrays_size(Primitive, constant_data, "constant", 1);
		require_attribute_arrays_size(Primitive, uniform_data, "uniform", curve_first_points.size());
		require_attribute_arrays_size(Primitive, varying_data, "varying", std::accumulate(curve_point_counts.begin(), curve_point_counts.end(), 0));

		return new const_primitive(periodic, material, curve_first_points, curve_point_counts, curve_selections, curve_points, constant_data, uniform_data, varying_data);
	}
	catch(std::exception& e)
	{
		log() << error << e.what() << std::endl;
	}

	return 0;
}

primitive* validate(mesh::primitive& Primitive)
{
	if(Primitive.type != "cubic_curve")
		return 0;

	try
	{
		mesh::bools_t& periodic = require_array<mesh::bools_t>(Primitive, "periodic");
		mesh::materials_t& material = require_array<mesh::materials_t>(Primitive, "material");
		mesh::indices_t& curve_first_points = require_array<mesh::indices_t>(Primitive, "curve_first_points");
		mesh::counts_t& curve_point_counts = require_array<mesh::counts_t>(Primitive, "curve_point_counts");
		mesh::selection_t& curve_selections = require_array<mesh::selection_t>(Primitive, "curve_selections");
		mesh::indices_t& curve_points = require_array<mesh::indices_t>(Primitive, "curve_points");

		mesh::attribute_arrays_t& constant_data = require_attribute_arrays(Primitive, "constant");
		mesh::attribute_arrays_t& uniform_data = require_attribute_arrays(Primitive, "uniform");
		mesh::attribute_arrays_t& varying_data = require_attribute_arrays(Primitive, "varying");

		require_metadata(Primitive, curve_selections, "curve_selections", metadata::key::selection_component(), string_cast(selection::UNIFORM));
		require_metadata(Primitive, curve_points, "curve_points", metadata::key::domain(), metadata::value::mesh_point_indices_domain());

		require_array_size(Primitive, periodic, "periodic", 1);
		require_array_size(Primitive, material, "material", 1);
		require_array_size(Primitive, curve_point_counts, "curve_point_counts", curve_first_points.size());
		require_array_size(Primitive, curve_selections, "curve_selections", curve_first_points.size());
		require_array_size(Primitive, curve_points, "curve_points", std::accumulate(curve_point_counts.begin(), curve_point_counts.end(), 0));

		require_attribute_arrays_size(Primitive, constant_data, "constant", 1);
		require_attribute_arrays_size(Primitive, uniform_data, "uniform", curve_first_points.size());
		require_attribute_arrays_size(Primitive, varying_data, "varying", std::accumulate(curve_point_counts.begin(), curve_point_counts.end(), 0));

		return new primitive(periodic, material, curve_first_points, curve_point_counts, curve_selections, curve_points, constant_data, uniform_data, varying_data);
	}
	catch(std::exception& e)
	{
		log() << error << e.what() << std::endl;
	}

	return 0;
}

primitive* validate(pipeline_data<mesh::primitive>& Primitive)
{
  if(!Primitive.get())
    return 0;

	if(Primitive->type != "cubic_curve")
		return 0;

  return validate(Primitive.writable());
}

} // namespace cubic_curve

} // namespace k3d

