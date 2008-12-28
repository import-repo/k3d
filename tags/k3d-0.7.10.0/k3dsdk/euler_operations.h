#ifndef K3DSDK_EULER_OPERATIONS_H
#define K3DSDK_EULER_OPERATIONS_H

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

#include "mesh.h"

namespace k3d
{

namespace euler
{

/// Returns the "Euler Number" for a polyhedron, which we define as:
///
/// V - E + F - (L - F)
///
/// with the following definitions:
///
/// V = number of vertices in the polyhedron.
/// E = number of whole edges (i.e. number of split-edges / 2) in the polyhedron.
/// F = number of faces in the polyhedron.
/// L = number of loops in the polyhedron.
///
/// Note the relation between the above definition and the traditional
/// Euler - Poincare formula:
///
/// V - E + F - (L - F) - 2(S - G) = 0
///
/// with the same definitions as above, plus:
///
/// S = number of shells (solid manifolds) in the polyhedron.
/// G = genus (number of holes) in the polyhedron.
///
/// Thus, the result of euler::number() will equal the 2(S - G) term
/// for a given polyhedron.  If you know the number of shells in the
/// polyhedron (almost always 1), you can easily determine the number of holes.

const uint_t number(const mesh& Mesh, const uint_t Polyhedron);

} // namespace euler

} // namespace k3d

#endif // !K3DSDK_EULER_OPERATIONS_H
