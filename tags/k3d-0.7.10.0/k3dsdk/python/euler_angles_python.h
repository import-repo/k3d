#ifndef K3DSDK_PYTHON_EULER_ANGLES_PYTHON_H
#define K3DSDK_PYTHON_EULER_ANGLES_PYTHON_H

// K-3D
// Copyright (c) 1995-2006, Timothy M. Shead
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

#include <k3dsdk/algebra.h>

namespace k3d
{

namespace python
{

class euler_angles :
	public k3d::euler_angles
{
	typedef k3d::euler_angles base;
public:
	euler_angles(const k3d::euler_angles& EulerAngles);
	euler_angles(const AngleOrder Order, const double A, const double B, const double C);

	const int len() const;
	const double getitem(const int Item) const;
	void setitem(const int Item, const double Value);
	const std::string str() const;

	static void define_class();
};

const euler_angles operator+(const euler_angles& LHS, const euler_angles& RHS);
const euler_angles operator*(const euler_angles& LHS, const double RHS);
const euler_angles operator*(const double LHS, const euler_angles& RHS);

} // namespace python

} // namespace k3d

#endif // !K3DSDK_PYTHON_EULER_ANGLES_PYTHON_H

