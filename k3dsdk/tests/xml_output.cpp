// K-3D
// Copyright (c) 1995-2004, Timothy M. Shead
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

#include <k3dsdk/xml.h>
using namespace k3d::xml;

#include <iostream>

int main()
{
	element a("a");
	a.append(attribute("b", "c"));
	a.append(attribute("d", 1));
	
	element& e = a.append(element("e", "g"));
	e.append(attribute("f", 2.5));

	/*element& h =*/ e.append(element("h", "i"));

	/*element& j =*/ e.append(element("j"));

	std::cout << a;

	return 0;
}


