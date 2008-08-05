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
//
// ---------------------
//

#ifndef PREVIEWSPHERE_H
#define PREVIEWSPHERE_H

#include "PreviewObj.h"

namespace module
{
namespace ngui
{
namespace material_manager
{
namespace mechanics
{

using namespace libk3dngui;

class PreviewSphere : public PreviewObj
{
 public:
  
  PreviewSphere(k3d::string_t _combo_value, document_state *_document_state)
    :PreviewObj(_combo_value, _document_state)
    {
    }

  virtual ~PreviewSphere()
    {
    }

 public:

  //Initialization Of Object Contents Beyond Initial Values
  void init(k3d::string_t _node_name, k3d::string_t _meta_nametag);

 

  

};//PreviewSphere


}//namespace mechanics

}//namespace material_manager

}//namespace ngui

}//namespace module


#endif
