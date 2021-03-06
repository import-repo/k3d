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
		\brief An object that acts as a property source for scalar properties
		\author Bart Janssens (bart.janssens@polytechnic.be)
*/

#include <k3d-i18n-config.h>
#include <k3dsdk/color.h>
#include <k3dsdk/document_plugin_factory.h>
#include <k3dsdk/node.h>

namespace module
{

namespace core
{

class color_property :
	public k3d::node
{
	typedef k3d::node base;
public:
	color_property(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document),
		m_value(init_owner(*this) + init_name("value") + init_label(_("Color")) + init_description(_("Outputs the value exposed by this property")) + init_value(k3d::color(1, 1, 1)))
	{
	}

	// Returns the factory at module registration time
	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<color_property > factory(
			k3d::uuid(0xba815d69, 0x003d4736, 0x877cf38c, 0xc0497228),
			"ColorProperty",
			_("Provides a source object to link color properties to"),
			"Color",
			k3d::iplugin_factory::STABLE);

		return factory;
	}

	// Implementation of the factory method required by the k3dinode interface:
private:
	// The color property itself:
	k3d_data(k3d::color, immutable_name, change_signal, with_undo, local_storage, no_constraint, writable_property, with_serialization) m_value;
};

k3d::iplugin_factory& color_property_factory()
{
	return color_property::get_factory();
}

} // namespace core

} // namespace module


