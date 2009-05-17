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

#include "color.h"
#include "imaterial.h"
#include "iomanip.h"
#include "legacy_mesh.h"
#include "mesh.h"
#include "metadata_keys.h"
#include "type_registry.h"

#include <boost/mpl/for_each.hpp>

#include <iterator>
#include <map>

namespace k3d
{

namespace detail
{

////////////////////////////////////////////////////////////////////////////////////////////
// indentation

/// Inserts whitespace into a stream, proportional to its indentation level
std::ostream& indentation(std::ostream& Stream)
{
	Stream << std::string(2 * current_indent(Stream), ' '); 
	return Stream;
}

////////////////////////////////////////////////////////////////////////////////////////////
// block_output

template<typename output_type, typename iterator_type>
void block_output(iterator_type Begin, iterator_type End, std::ostream& Stream, const string_t& Delimiter, const uint_t BlockSize = 8)
{
	if(Begin == End)
		return;

	uint_t count = 0;
	for(; Begin != End; ++count, ++Begin)
	{
		if(0 == count % BlockSize)
			Stream << indentation;

		Stream << static_cast<output_type>(*Begin) << Delimiter;

		if(BlockSize - 1 == count % BlockSize)
			Stream << "\n";
	}

	if(count % BlockSize)
		Stream << "\n";
};

template<typename iterator_type>
void string_block_output(iterator_type Begin, iterator_type End, std::ostream& Stream, const string_t& Delimiter, const uint_t BlockSize = 8)
{
	if(Begin == End)
		return;

	uint_t count = 0;
	for(; Begin != End; ++count, ++Begin)
	{
		if(0 == count % BlockSize)
			Stream << indentation;

		Stream << "\"" << *Begin << "\"" << Delimiter;

		if(BlockSize - 1 == count % BlockSize)
			Stream << "\n";
	}

	if(count % BlockSize)
		Stream << "\n";
};

////////////////////////////////////////////////////////////////////////////////////////////
// print

template<typename pointer_type>
void print(std::ostream& Stream, const string_t& Label, const pointer_type& Pointer)
{
	if(Pointer)
	{
		typedef typename pointer_type::element_type array_type;
		typedef typename array_type::value_type value_type;

		Stream << indentation << Label;
		if(type_registered<value_type>())
			Stream << " [" << type_string<value_type>() <<  "]";
		Stream << " (" << Pointer->size() <<  "):\n";
		Stream << push_indent;

		block_output<typename pointer_type::element_type::value_type>(Pointer->begin(), Pointer->end(), Stream, " ");

		Stream << pop_indent;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// print_array

class print_array
{
public:
	print_array(std::ostream& Stream, const string_t& ArrayName, const array& Array, bool_t& Printed) :
		m_stream(Stream),
		m_array_name(ArrayName),
		m_array(Array),
		m_printed(Printed)
	{
		// Special-case k3d::uint_t_array so the type is labeled correctly ...
		if(const uint_t_array* const array = dynamic_cast<const uint_t_array*>(&m_array))
		{
			m_printed = true;
			m_stream << indentation << "array \"" << m_array_name << "\" [k3d::uint_t] (" << m_array.size() << "):\n";
			m_stream << push_indent;
			
			block_output<uint_t>(array->begin(), array->end(), m_stream, " ");
			print_metadata();

			m_stream << pop_indent;
		}
	}

	template<typename T>
	void operator()(T)
	{
		if(m_printed)
			return;

		if(const typed_array<T>* const array = dynamic_cast<const typed_array<T>*>(&m_array))
		{
			m_printed = true;
			m_stream << indentation << "array \"" << m_array_name << "\" [" << type_string<T>() << "] (" << m_array.size() << "):\n";
			m_stream << push_indent;

			block_output<T>(array->begin(), array->end(), m_stream, " ");
			print_metadata();

			m_stream << pop_indent;
		}
	}

	/// Special-case printing of 8-bit integers so they aren't printed as characters
	void operator()(int8_t)
	{
		typedef int8_t T;

		if(m_printed)
			return;

		if(const typed_array<T>* const array = dynamic_cast<const typed_array<T>*>(&m_array))
		{
			m_printed = true;
			m_stream << indentation << "array \"" << m_array_name << "\" [" << type_string<T>() << "] (" << m_array.size() << "):\n";
			m_stream << push_indent;

			block_output<int16_t>(array->begin(), array->end(), m_stream, " ");
			print_metadata();

			m_stream << pop_indent;
		}
	}

	/// Special-case printing of 8-bit unsigned integers so they aren't printed as characters
	void operator()(uint8_t)
	{
		typedef uint8_t T;

		if(m_printed)
			return;

		if(const typed_array<T>* const array = dynamic_cast<const typed_array<T>*>(&m_array))
		{
			m_printed = true;
			m_stream << indentation << "array \"" << m_array_name << "\" [" << type_string<T>() << "] (" << m_array.size() << "):\n";
			m_stream << push_indent;

			block_output<uint16_t>(array->begin(), array->end(), m_stream, " ");
			print_metadata();

			m_stream << pop_indent;
		}
	}

	/// Special-case printing of strings so whitespace is handled correctly
	void operator()(string_t)
	{
		typedef string_t T;

		if(m_printed)
			return;

		if(const typed_array<T>* const array = dynamic_cast<const typed_array<T>*>(&m_array))
		{
			m_printed = true;
			m_stream << indentation << "array \"" << m_array_name << "\" [" << type_string<T>() << "] (" << m_array.size() << "):\n";
			m_stream << push_indent;

			string_block_output(array->begin(), array->end(), m_stream, " ");
			print_metadata();

			m_stream << pop_indent;
		}
	}

private:
	void print_metadata()
	{
		const array::metadata_t metadata = m_array.get_metadata();
		for(array::metadata_t::const_iterator pair = metadata.begin(); pair != metadata.end(); ++pair)
			m_stream << indentation << "metadata: " << pair->first << " = " << pair->second << "\n";
	}

	std::ostream& m_stream;
	const string_t& m_array_name;
	const array& m_array;
	bool_t& m_printed;
};

////////////////////////////////////////////////////////////////////////////////////////////
// print

void print(std::ostream& Stream, const string_t& Label, const mesh::named_arrays_t& Arrays)
{
	Stream << indentation << Label << " (" << Arrays.size() << "):\n" << push_indent;
	for(mesh::attribute_arrays_t::const_iterator array_iterator = Arrays.begin(); array_iterator != Arrays.end(); ++array_iterator)
	{
		bool_t printed = false;
		boost::mpl::for_each<named_array_types>(print_array(Stream, array_iterator->first, *array_iterator->second, printed));
		if(!printed)
			log() << error << k3d_file_reference << ": array [" << array_iterator->first << "] with unknown type [" << demangle(typeid(*array_iterator->second)) << "] will not be printed" << std::endl;
	}
	Stream << pop_indent;
}

////////////////////////////////////////////////////////////////////////////////////////////
// print

void print(std::ostream& Stream, const string_t& Label, const mesh::attribute_arrays_t& Arrays)
{
	Stream << indentation << Label << " (" << Arrays.size() << "):\n" << push_indent;
	for(mesh::attribute_arrays_t::const_iterator array_iterator = Arrays.begin(); array_iterator != Arrays.end(); ++array_iterator)
	{
		bool_t printed = false;
		boost::mpl::for_each<named_array_types>(print_array(Stream, array_iterator->first, *array_iterator->second, printed));
		if(!printed)
			log() << error << k3d_file_reference << ": array [" << array_iterator->first << "] with unknown type [" << demangle(typeid(*array_iterator->second)) << "] will not be printed" << std::endl;
	}
	Stream << pop_indent;
}

////////////////////////////////////////////////////////////////////////////////////////////
// print

void print(std::ostream& Stream, const string_t& Label, const mesh::named_attribute_arrays_t& Attributes)
{
	Stream << indentation << Label << " (" << Attributes.size() << "):\n" << push_indent;
	for(mesh::named_attribute_arrays_t::const_iterator attributes = Attributes.begin(); attributes != Attributes.end(); ++attributes)
		print(Stream, "attributes \"" + attributes->first + "\"", attributes->second);
	Stream << pop_indent;
}

////////////////////////////////////////////////////////////////////////////////////////////
// almost_equal

/// Return true iff two shared arrays are equivalent (handles cases where they point to the same memory, and handles "fuzzy" floating-point comparisons).
template<typename T>
bool_t almost_equal(const pipeline_data<typed_array<T> >& A, const pipeline_data<typed_array<T> >& B, const uint64_t Threshold)
{
	if(A.get() == B.get())
		return true;

	if(A && B)
		return A->almost_equal(*B, Threshold);

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
// almost_equal

/// Return true iff two shared arrays are equivalent (handles cases where they point to the same memory, and handles "fuzzy" floating-point comparisons).
bool_t almost_equal(const pipeline_data<uint_t_array>& A, const pipeline_data<uint_t_array>& B, const uint64_t Threshold)
{
	if(A.get() == B.get())
		return true;

	if(A && B)
		return A->almost_equal(*B, Threshold);

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
// almost_equal

/// Return true iff two shared objects are equivalent (handles cases where they point to the same memory, and handles "fuzzy" floating-point comparisons).
template<typename T>
bool_t almost_equal(const pipeline_data<T>& A, const pipeline_data<T>& B, const uint64_t Threshold)
{
	if(A.get() == B.get())
		return true;

	if(A && B)
		return k3d::almost_equal<T>(Threshold)(*A, *B);

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
// almost_equal

/// Return true iff two sets of attributes arrays are equivalent (we provide this function mainly for consistency).

bool_t almost_equal(const mesh::attribute_arrays_t& A, const mesh::attribute_arrays_t& B, const uint64_t Threshold)
{
	return k3d::almost_equal<mesh::attribute_arrays_t>(Threshold)(A, B);
}

////////////////////////////////////////////////////////////////////////////////////////////
// almost_equal

/// Return true iff two sets of primitives are equivalent.

bool_t almost_equal(const mesh::primitives_t& A, const mesh::primitives_t& B, const uint64_t Threshold)
{
	// If we have differing numbers of primitives, they definitely aren't equal
	if(A.size() != B.size())
		return false;

	for(mesh::primitives_t::const_iterator a = A.begin(), b = B.begin(); a != A.end() && b != B.end(); ++a, ++b)
	{
		// If both primitives point to the same memory, they're equal
		if(a->get() == b->get())
			continue;

		// Perform element-wise comparisons of the two primitives ...
		if(a->get() && b->get())
		{
			if(!(**a).almost_equal((**b), Threshold))
				return false;
		}
		// One array was NULL and the other wasn't
		else if(a->get() || b->get())
		{
			return false;
		}
	}

	return true;
}


} // namespace detail

////////////////////////////////////////////////////////////////////////////////////
// mesh

mesh::mesh()
{
}

bool_t mesh::almost_equal(const mesh& Other, const uint64_t Threshold) const
{
	return
		detail::almost_equal(points, Other.points, Threshold) &&
		detail::almost_equal(point_selection, Other.point_selection, Threshold) &&
		detail::almost_equal(vertex_data, Other.vertex_data, Threshold) &&
		detail::almost_equal(primitives, Other.primitives, Threshold)
    ;
}

mesh& mesh::operator=(const legacy::mesh& RHS)
{
	// Convert points ...
	std::map<legacy::point*, uint_t> point_map;

	const uint_t point_size = RHS.points.size();
	points_t& points = this->points.create(new points_t(point_size));
	selection_t& point_selection = this->point_selection.create(new selection_t(point_size));

	for(uint_t i = 0; i != point_size; ++i)
	{
		points[i] = RHS.points[i]->position;
		point_selection[i] = RHS.points[i]->selection_weight;
		point_map[RHS.points[i]] = i;
	}

	// Convert primitives ...
	primitives = RHS.primitives;

/*
	// Convert polyhedra ...
	if(RHS.polyhedra.size())
	{
		polyhedra_t& polyhedra = this->polyhedra.create();
		indices_t& first_faces = polyhedra.first_faces.create();
		counts_t& face_counts = polyhedra.face_counts.create();
		polyhedra_t::types_t& types = polyhedra.types.create();
		indices_t& face_first_loops = polyhedra.face_first_loops.create();
		counts_t& face_loop_counts = polyhedra.face_loop_counts.create();
		selection_t& face_selection = polyhedra.face_selection.create();
		materials_t& face_materials = polyhedra.face_materials.create();
		indices_t& loop_first_edges = polyhedra.loop_first_edges.create();
		indices_t& edge_points = polyhedra.edge_points.create();
		indices_t& clockwise_edges = polyhedra.clockwise_edges.create();
		selection_t& edge_selection = polyhedra.edge_selection.create();

		for(legacy::mesh::polyhedra_t::const_iterator polyhedron = RHS.polyhedra.begin(); polyhedron != RHS.polyhedra.end(); ++polyhedron)
		{
			uint_t first_face = face_first_loops.size();
			uint_t face_count = 0;
			mesh::polyhedra_t::polyhedron_type type = (*polyhedron)->type == legacy::polyhedron::POLYGONS ? mesh::polyhedra_t::POLYGONS : mesh::polyhedra_t::CATMULL_CLARK;

			for(legacy::polyhedron::faces_t::const_iterator face = (*polyhedron)->faces.begin(); face != (*polyhedron)->faces.end(); ++face)
			{
				++face_count;

				uint_t face_first_loop = loop_first_edges.size();
				uint_t face_loop_count = 1 + (*face)->holes.size();

				const uint_t first_edge = edge_points.size();

				loop_first_edges.push_back(first_edge);
				for(legacy::split_edge* edge = (*face)->first_edge; edge; edge = edge->face_clockwise)
				{
					if(edge->vertex && edge->face_clockwise)
					{
						edge_points.push_back(point_map[edge->vertex]);
						clockwise_edges.push_back(edge_points.size());
						edge_selection.push_back(edge->selection_weight);
					}

					if(edge->face_clockwise == (*face)->first_edge)
					{
						clockwise_edges.back() = first_edge;
						break;
					}
				}

				for(legacy::face::holes_t::iterator hole = (*face)->holes.begin(); hole != (*face)->holes.end(); ++hole)
				{
					const uint_t first_edge = edge_points.size();

					loop_first_edges.push_back(first_edge);
					for(legacy::split_edge* edge = *hole; edge; edge = edge->face_clockwise)
					{
						if(edge->vertex && edge->face_clockwise && edge->face_clockwise->vertex)
						{
							edge_points.push_back(point_map[edge->vertex]);
							clockwise_edges.push_back(edge_points.size());
							edge_selection.push_back(edge->selection_weight);
						}

						if(edge->face_clockwise == (*hole))
						{
							clockwise_edges.back() = first_edge;
							break;
						}
					}
				}

				face_first_loops.push_back(face_first_loop);
				face_loop_counts.push_back(face_loop_count);
				face_selection.push_back((*face)->selection_weight);
				face_materials.push_back((*face)->material);
			}

			first_faces.push_back(first_face);
			face_counts.push_back(face_count);
			types.push_back(type);
		}
	}
*/

	return *this;
}

namespace detail
{

/// Helper function used by lookup_unused_points()
void mark_used_points(const mesh::indices_t& PrimitivePoints, mesh::bools_t& UnusedPoints)
{
	const uint_t begin = 0;
	const uint_t end = PrimitivePoints.size();
	for(uint_t i = begin; i != end; ++i)
		UnusedPoints[PrimitivePoints[i]] = false;
}

/// Helper object used by lookup_unused_points()
struct mark_used_primitive_points
{
	mark_used_primitive_points(mesh::bools_t& UnusedPoints) :
		unused_points(UnusedPoints)
	{
	}

	void operator()(const string_t&, const pipeline_data<array>& Array)
	{
		if(Array->get_metadata_value(metadata::key::domain()) != metadata::value::mesh_point_indices_domain())
			return;

		if(const mesh::indices_t* const array = dynamic_cast<const mesh::indices_t*>(Array.get()))
			mark_used_points(*array, unused_points);
	}

	mesh::bools_t& unused_points;
};

} // namespace detail

const bounding_box3 mesh::bounds(const mesh& Mesh)
{
	return Mesh.points ? bounds(*Mesh.points) : bounding_box3();
}

const bounding_box3 mesh::bounds(const points_t& Points)
{
	bounding_box3 results;

	const uint_t point_begin = 0;
	const uint_t point_end = point_begin + Points.size();
	for(uint_t point = point_begin; point != point_end; ++point)
		results.insert(Points[point]);

	return results;
}

void mesh::deep_copy(const mesh& From, mesh& To)
{
	To = From;
	assert_not_implemented(); // Need to ensure that all storage is unique
}

void mesh::lookup_unused_points(const mesh& Mesh, mesh::bools_t& UnusedPoints)
{
	UnusedPoints.assign(Mesh.points ? Mesh.points->size() : 0, true);
	visit_arrays(Mesh, detail::mark_used_primitive_points(UnusedPoints));
}

namespace detail
{

/// Helper function used by delete_unused_points()
void remap_points(mesh::indices_t& PrimitivePoints, const mesh::indices_t& PointMap)
{
	const uint_t begin = 0;
	const uint_t end = PrimitivePoints.size();
	for(uint_t i = begin; i != end; ++i)
		PrimitivePoints[i] = PointMap[PrimitivePoints[i]];
}

/// Helper object used by delete_unused_points()
struct remap_primitive_points
{
	remap_primitive_points(mesh::indices_t& PointMap) :
		point_map(PointMap)
	{
	}

	void operator()(const string_t&, pipeline_data<array>& Array)
	{
		if(Array->get_metadata_value(metadata::key::domain()) != metadata::value::mesh_point_indices_domain())
			return;

		if(mesh::indices_t* const array = dynamic_cast<mesh::indices_t*>(&Array.writable()))
			remap_points(*array, point_map);
	}

	mesh::indices_t& point_map;
};

} // namespace detail

void mesh::delete_unused_points(mesh& Mesh)
{
	// Create a bitmap marking which points are unused ...
	mesh::bools_t unused_points;
	lookup_unused_points(Mesh, unused_points);

	// Count how many points will be left when we're done ...
	const uint_t points_remaining = std::count(unused_points.begin(), unused_points.end(), false);

	// Create an array that will map from current-point-indices to new-point-indices,
	// taking into account the points that will be removed.
	mesh::indices_t point_map(unused_points.size());

	const uint_t begin = 0;
	const uint_t end = unused_points.size();
	for(uint_t current_index = begin, new_index = begin; current_index != end; ++current_index)
	{
		point_map[current_index] = new_index;
		if(!unused_points[current_index])
			++new_index;
	}

	// Move leftover points (and point selections) into their final positions ...
	mesh::points_t& points = Mesh.points.writable();
	mesh::selection_t& point_selection = Mesh.point_selection.writable();
	for(uint_t i = begin; i != end; ++i)
	{
		if(!unused_points[i])
		{
			points[point_map[i]] = points[i];
			point_selection[point_map[i]] = point_selection[i];
		}
	}

	// Update generic mesh primitives so they use the correct indices ...
	visit_arrays(Mesh, detail::remap_primitive_points(point_map));

	// Free leftover memory ...
	points.resize(points_remaining);
	point_selection.resize(points_remaining);
}

////////////////////////////////////////////////////////////////////////////////////
// mesh::primitive

mesh::primitive::primitive()
{
}

mesh::primitive::primitive(const string_t& Type) :
	type(Type)
{
}

bool_t mesh::primitive::almost_equal(const primitive& Other, const uint64_t Threshold) const
{
	return
		k3d::almost_equal<string_t>(Threshold)(type, Other.type) &&
		k3d::almost_equal<named_arrays_t>(Threshold)(structure, Other.structure) &&
		k3d::almost_equal<named_attribute_arrays_t>(Threshold)(attributes, Other.attributes);
}

////////////////////////////////////////////////////////////////////////////////////
// mesh::primitives_t

mesh::primitive& mesh::primitives_t::create(const string_t& Type)
{
	push_back(pipeline_data<mesh::primitive>());
	return back().create(new mesh::primitive(Type));
}

////////////////////////////////////////////////////////////////////////////////////
// mesh::selection

mesh::selection::selection()
{
}

mesh::selection::selection(const string_t& Type) :
	type(Type)
{
}

bool_t mesh::selection::almost_equal(const selection& Other, const uint64_t Threshold) const
{
	return
		k3d::almost_equal<string_t>(Threshold)(type, Other.type) &&
		k3d::almost_equal<named_arrays_t>(Threshold)(structure, Other.structure)
		;
}

////////////////////////////////////////////////////////////////////////////////////
// mesh::selection_set

mesh::selection& mesh::selection_set::create(const string_t& Type)
{
	push_back(pipeline_data<mesh::selection>());
	return back().create(new mesh::selection(Type));
}

bool_t mesh::selection_set::almost_equal(const selection_set& Other, const uint64_t Threshold) const
{
	if(size() != Other.size())
		return false;

	const selection_set& self = *this;
	for(uint_t i = 0; i != self.size(); ++i)
	{
		if(!k3d::almost_equal<selection_set>(Threshold)(*this, Other))
			return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// operator<<

/** \todo Print materials */
std::ostream& operator<<(std::ostream& Stream, const mesh& RHS)
{
	Stream << detail::indentation << "mesh:\n" << push_indent;

	detail::print(Stream, "points", RHS.points);
	detail::print(Stream, "point_selection", RHS.point_selection);
	detail::print(Stream, "vertex_data", RHS.vertex_data);

	Stream << detail::indentation << "primitives (" << RHS.primitives.size() << "):\n" << push_indent;
	for(mesh::primitives_t::const_iterator primitive = RHS.primitives.begin(); primitive != RHS.primitives.end(); ++primitive)
	{
		Stream << detail::indentation << "primitive \"" << (*primitive)->type << "\"\n" << push_indent;
		detail::print(Stream, "structure", (*primitive)->structure);
		detail::print(Stream, "attributes", (*primitive)->attributes);
		Stream << pop_indent;
	}
	Stream << pop_indent;
	Stream << pop_indent;

	return Stream;
}

std::ostream& operator<<(std::ostream& Stream, const mesh::selection& RHS)
{
	Stream << detail::indentation << "mesh::selection:\n" << push_indent;
	Stream << detail::indentation << "type: " << RHS.type << "\n";
	detail::print(Stream, "structure", RHS.structure);
	Stream << pop_indent;

	return Stream;
}

std::ostream& operator<<(std::ostream& Stream, const mesh::selection_set& RHS)
{
	Stream << detail::indentation << "mesh::selection_set:\n" << push_indent;
	for(mesh::selection_set::const_iterator selection = RHS.begin(); selection != RHS.end(); ++selection)
		Stream << **selection << "\n";
	Stream << pop_indent;

	return Stream;
}

} // namespace k3d

