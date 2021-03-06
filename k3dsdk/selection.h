#ifndef K3DSDK_SELECTION_H
#define K3DSDK_SELECTION_H

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

#include "gl.h"
#include "inode_collection.h"
#include "inode_selection.h"
#include "ipersistent.h"
#include "iselectable.h"
#include "named_arrays.h"
#include "nodes.h"
#include "serialization_xml.h"
#include "xml.h"

#include <vector>

namespace k3d
{

class inode;

class mesh;

/// Functor object for deciding whether an object is selected (i.e. has a non-zero selection weight)
struct is_selected
{
	template<typename T>
	bool operator()(T* Object)
	{
		return Object->selection_weight != 0 ? true : false;
	}

	template<typename T>
	bool operator()(T& Object)
	{
		return Object.selection_weight != 0 ? true : false;
	}
};

namespace selection
{

enum type
{

/** \note Never alter the values of these enumerations, they may be used for efficient serialization */

/// Used to mark "NULL" tokens
NONE = 0,
/// Storage for a zero-based node index (relative to the document node collection)
NODE = 1,
/// Storage for a zero-based mesh index (relative to a node)
MESH = 2,
//POINT_GROUP = 3,
//POINT = 4,
//ABSOLUTE_POINT = 5,
//POLYHEDRON = 6,
//FACE = 7,
//ABSOLUTE_FACE = 8,
//FACE_HOLE = 9,
//SPLIT_EDGE = 10,
//ABSOLUTE_SPLIT_EDGE = 11,
//LINEAR_CURVE_GROUP = 12,
//LINEAR_CURVE = 13,
//ABSOLUTE_LINEAR_CURVE = 14,
//CUBIC_CURVE_GROUP = 15,
//CUBIC_CURVE = 16,
//ABSOLUTE_CUBIC_CURVE = 17,
//NUCURVE_GROUP = 18,
//NUCURVE = 19,
//ABSOLUTE_NURBS_CURVE = 20,
//ABSOLUTE_BILINEAR_PATCH = 21,
//ABSOLUTE_BICUBIC_PATCH = 22,
//ABSOLUTE_NURBS_PATCH = 23,
/// Storage for a user-defined index
USER1 = 24,
/// Storage for a zero-based primitive index (relative to a mesh)
PRIMITIVE = 25,
/// Storage for a zero-based constant index (relative to a primitive)
CONSTANT = 26,
/// Storage for a zero-based uniform index (relative to a primitive)
UNIFORM = 27,
/// Storage for a zero-based varying index (relative to a primitive)
VARYING = 28,
/// Storage for a zero-based face-varying index (relative to a primitive)
FACE_VARYING = 29,
/// Storage for a zero-based split-edge index (relative to a primitive)
SPLIT_EDGE = 30,
/// Storage for a zero-based point index (relative to a mesh)
POINT = 31,

};

std::ostream& operator<<(std::ostream& Stream, const type& RHS);
std::istream& operator>>(std::istream& Stream, type& RHS);

typedef GLuint id;
id null_id();

struct token
{
	token();
	token(const selection::type Type, const selection::id ID);

	selection::type type;
	selection::id id;
};

bool operator==(const token& LHS, const token& RHS);
bool operator!=(const token& LHS, const token& RHS);
std::ostream& operator<<(std::ostream& Stream, const token& RHS);
std::istream& operator>>(std::istream& Stream, token& RHS);

struct record
{
	record();
	static const record empty_record();

	bool empty() const;
	id get_id(const selection::type Type) const;
	const token get_token(const selection::type Type) const;

	GLuint zmin;
	GLuint zmax;

	typedef std::vector<token> tokens_t;
	tokens_t tokens;
};
std::ostream& operator<<(std::ostream& Stream, const record& RHS);
std::istream& operator>>(std::istream& Stream, record& RHS);

typedef std::vector<record> records;

const record make_record(inode*);

inode* get_node(const record& Record);
mesh* get_mesh(const record& Record);

template<typename T>
bool is_selected(T* Object)
{
	iselectable* const selectable = dynamic_cast<iselectable*>(Object);
	return selectable && selectable->get_selection_weight();
}

/// Functor object for setting selection weight on a collection of objects - good with k3d::for_each_component
struct set_weight
{
	set_weight(const double SelectionWeight) :
		selection_weight(SelectionWeight)
	{
	}

	template<typename T>
	void operator()(T& Object) const
	{
		Object.selection_weight = selection_weight;
	}

	template<typename T>
	void operator()(T* Object) const
	{
		Object->selection_weight = selection_weight;
	}

	double selection_weight;
};

} // namespace selection

namespace gl
{

void push_selection_token(inode*);
void push_selection_token(const selection::token&);
void push_selection_token(const selection::type, const selection::id);

void pop_selection_token();

} // namespace gl

namespace selection
{

/// Defines storage for a generic selection.
class storage
{
public:
	storage();
	storage(const string_t& Type);

	/// Stores the selection type ("point", "component", "parameter", etc).
	string_t type;
	/// Stores array data that defines the selection.
	named_arrays structure;

	/// Compares two selections for equality using the fuzzy semantics of almost_equal.
	bool_t almost_equal(const storage& Other, const uint64_t Threshold) const;
};

/// Stream serialization
std::ostream& operator<<(std::ostream& Stream, const storage& RHS);

/// Defines a container of selection::storage objects
class set :
	public std::vector<pipeline_data<storage> >
{
public:
	/// Create a new selection, appending it to the collection.
	storage& create(const string_t& Type);

	/// Compares two selection sets for equality using the fuzzy semantics of almost_equal.
	bool_t almost_equal(const set& Other, const uint64_t Threshold) const;
};

/// Stream serialization
std::ostream& operator<<(std::ostream& Stream, const set& RHS);

} // namespace selection

/// Specialization of almost_equal that tests k3d::mesh::selection for equality
template<>
class almost_equal<selection::storage>
{
	typedef selection::storage T;

public:
	almost_equal(const uint64_t Threshold) :
		threshold(Threshold)
	{
	}

	inline bool_t operator()(const T& A, const T& B) const
	{
		return A.almost_equal(B, threshold);
	}

	const uint64_t threshold;
};

/// Specialization of almost_equal that tests k3d::mesh::selection_set for equality
template<>
class almost_equal<selection::set>
{
	typedef selection::set T;

public:
	almost_equal(const uint64_t Threshold) :
		threshold(Threshold)
	{
	}

	inline bool_t operator()(const T& A, const T& B) const
	{
		return A.almost_equal(B, threshold);
	}

	const uint64_t threshold;
};

/////////////////////////////////////////////////////////////////////////////
// selection_set_serialization

/// k3d::data serialization policy for use with selection data
template<typename value_t, class property_policy_t>
class selection_set_serialization :
	public property_policy_t,
	public ipersistent
{
public:
	void save(xml::element& Element, const ipersistent::save_context& Context)
	{
		xml::element& xml_property = Element.append(xml::element("property", xml::attribute("name", property_policy_t::name())));
		xml::save(property_policy_t::internal_value(), xml_property, Context);
	}

	void load(xml::element& Element, const ipersistent::load_context& Context)
	{
		xml::load(property_policy_t::internal_value(), Element, Context);
	}

protected:
	template<typename init_t>
	selection_set_serialization(const init_t& Init) :
		property_policy_t(Init)
	{
		Init.owner().enable_serialization(Init.name(), *this);
	}
};

} // namespace k3d

#endif // !K3DSDK_SELECTION_H

