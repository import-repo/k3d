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
		\author Tim Shead (tshead@k-3d.com)
*/

#include "document_window.h"
#include "document_state.h"

#include <gdk/gdkkeysyms.h>

namespace libk3dngui
{

/////////////////////////////////////////////////////////////////////////////
// document_window

document_window::document_window(document_state& Document, const std::string& Name) :
	base(Gtk::WINDOW_TOPLEVEL),
	ui_component(Name, dynamic_cast<k3d::icommand_node*>(&Document.document())),
	m_document(Document)
{
	Document.document().close_signal().connect(sigc::mem_fun(*this, &document_window::close));
}

k3d::idocument& document_window::document()
{
	return m_document.document();
}

bool document_window::on_key_press_event(GdkEventKey* event)
{
	if(event->keyval == GDK_Escape)
	{
		close();
		return true;
	}

	return base::on_key_press_event(event);
}

bool document_window::on_delete_event(GdkEventAny* event)
{
	close();
	return true;
}

void document_window::close()
{
	on_close();
	delete this;
}

void document_window::on_close()
{
}

} // namespace libk3dngui
