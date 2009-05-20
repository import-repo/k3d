#ifndef K3DSDK_VECTOR3_SOURCE_H
#define K3DSDK_VECTOR3_SOURCE_H

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

#include "data.h"
#include "ivector3_source.h"
#include "vector3.h"

namespace k3d
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// vector3_source

template<typename derived_t>
class vector3_source :
	public ivector3_source
{
public:
	iproperty& vector3_source_output()
	{
		return m_output_vector3;
	}

	sigc::slot<void, ihint*> make_reset_vector3_slot()
	{
		return m_output_vector3.make_reset_slot();
	}

protected:
	vector3_source() :
		m_output_vector3(
			init_owner(*static_cast<derived_t*>(this))
			+ init_name("output_vector")
			+ init_label(_("Output Vector"))
			+ init_description("Output vector")
			+ init_slot(sigc::mem_fun(*this, &vector3_source<derived_t>::create_vector3)))
	{
	}


private:
	vector3 create_vector3()
	{
		return on_create_vector3();
	}

	virtual vector3 on_create_vector3() = 0;

	k3d_data(vector3, data::immutable_name, data::change_signal, data::no_undo, data::computed_storage, data::no_constraint, data::read_only_property, data::no_serialization) m_output_vector3;
};

} // namespace k3d

#endif // !K3DSDK_VECTOR3_SOURCE_H
