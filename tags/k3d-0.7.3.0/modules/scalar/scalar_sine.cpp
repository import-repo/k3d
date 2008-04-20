// K-3D
// Copyright (c) 1995-2005, Timothy M. Shead
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
		\brief Applyies sine() operator to input value
		\author Anders Dahnielson (anders@dahnielson.com)
*/

#include <k3dsdk/document_plugin_factory.h>
#include <k3d-i18n-config.h>
#include <k3dsdk/node.h>
#include <k3dsdk/persistent.h>

#include <cmath>

namespace libk3dcore
{

class scalar_sine :
	public k3d::persistent<k3d::node>
{
	typedef k3d::persistent<k3d::node> base;
public:
	scalar_sine(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document),
		m_input(init_owner(*this) + init_name("input") + init_label(_("Input value")) + init_description(_("Input value")) + init_value(0.0)),
		m_output(init_owner(*this) + init_name("output") + init_label(_("Output value")) + init_description(_("Sine wave function applied on input")) + init_slot(sigc::mem_fun(*this, &scalar_sine::get_value)))
	{
		m_input.changed_signal().connect(m_output.make_reset_slot());
	}

	double get_value()
	{
		return std::sin(m_input.pipeline_value());
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<scalar_sine > factory(
			k3d::uuid(0xb00248d6, 0x9d6843d5, 0xab5a24bc, 0x852519e5),
			"ScalarSine",
			_("Applies a sine function to its input"),
			"Scalar",
			k3d::iplugin_factory::STABLE);
		return factory;
	}

private:
	k3d_data(double, immutable_name, change_signal, with_undo, local_storage, no_constraint, writable_property, with_serialization) m_input;
	k3d_data(double, immutable_name, change_signal, no_undo, computed_storage, no_constraint, read_only_property, no_serialization) m_output;

};

k3d::iplugin_factory& scalar_sine_factory()
{
	return scalar_sine::get_factory();
}

} //namespace libk3dcore


