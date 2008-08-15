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

#include "array_python.h"
#include "named_arrays_python.h"
#include "utility_python.h"

#include <k3dsdk/named_array_types.h>
#include <k3dsdk/type_registry.h>
#include <k3dsdk/typed_array.h>

#include <boost/python.hpp>
using namespace boost::python;

namespace k3d
{

namespace python
{

static object wrap_array(const k3d::array* const Array)
{
	throw std::runtime_error("wrap_array() not implemented");
	return object();
}

class named_arrays_array_factory
{
public:
	named_arrays_array_factory(const string_t& Name, const string_t& Type, boost::python::object& Array, k3d::named_arrays& Arrays) :
		name(Name),
		type(Type),
		array(Array),
		arrays(Arrays)
	{
	}

	template<typename T>
	void operator()(T) const
	{
		if(array != boost::python::object())
			return;

		if(type != k3d::type_string<T>())
			return;

		k3d::typed_array<T>* const new_array = new k3d::typed_array<T>();
		arrays[name].reset(new_array);
		array = boost::python::object(wrap(*new_array));
	}

private:
	string_t name;
	string_t type;
	boost::python::object& array;
	k3d::named_arrays& arrays;
};

static list array_names(named_arrays_wrapper& Self)
{
	list results;

	for(k3d::named_arrays::const_iterator array = Self.wrapped().begin(); array != Self.wrapped().end(); ++array)
		results.append(array->first);

	return results;
}

static object get_array(named_arrays_wrapper& Self, const string_t& Name)
{
	if(!Self.wrapped().count(Name))
		throw std::runtime_error("Unknown array name: " + Name);

	return wrap_array(Self.wrapped().find(Name)->second.get());
}

static object create_array(named_arrays_wrapper& Self, const string_t& Name, const string_t& Type)
{
	if(Name.empty())
		throw std::runtime_error("Empty array name");

	boost::python::object result;
	boost::mpl::for_each<k3d::named_array_types>(named_arrays_array_factory(Name, Type, result, Self.wrapped()));
	if(result == boost::python::object())
		throw std::runtime_error("Cannot create array [" + Name + "] with unknown type [" + Type + "]");

	return result;
}

static object get_item(named_arrays_wrapper& Self, int Item)
{
	if(Item < 0 || Item >= Self.wrapped().size())
		throw std::out_of_range("index out-of-range");

	k3d::named_arrays::const_iterator iterator = Self.wrapped().begin();
	std::advance(iterator, Item);

	return wrap_array(iterator->second.get());
}

void define_class_named_arrays()
{
	class_<named_arrays_wrapper>("named_arrays", 
		"Stores a mutable (read-write) collection of named arrays (named arrays of varying length).", no_init)
		.def("array_names", &array_names,
			"Returns a list containing names for all the arrays in the collection.")
		.def("array", &get_array,
			"Returns the array with the given name, or throws an exception.")
		.def("create_array", &create_array,
			"Creates an array with given name and type.")
		.def("__len__", &utility::wrapped_len<named_arrays_wrapper>)
		.def("__getitem__", &get_item);
}

} // namespace python

} // namespace k3d

