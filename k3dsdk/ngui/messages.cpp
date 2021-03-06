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
		\author Tim Shead (tshead@k-3d.com)
*/

#include <k3d-i18n-config.h>

#include "application_state.h"
#include "options.h"
#include "widget_manip.h"

#include <k3dsdk/batch_mode.h>

#include <gtkmm/alignment.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>

namespace k3d
{

namespace ngui
{

void message(const std::string& Message, const std::string& SecondaryMessage)
{
	if(k3d::batch_mode())
		return;

	Gtk::MessageDialog dialog(Message, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
	if(!SecondaryMessage.empty())
		dialog.set_secondary_text(SecondaryMessage);
	dialog.run();
}

void warning_message(const std::string& Message, const std::string& SecondaryMessage)
{
	if(k3d::batch_mode())
		return;

	Gtk::MessageDialog dialog(Message, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
	if(!SecondaryMessage.empty())
		dialog.set_secondary_text(SecondaryMessage);
	dialog.run();
}

void error_message(const std::string& Message, const std::string& SecondaryMessage)
{
	if(k3d::batch_mode())
		return;

	Gtk::MessageDialog dialog(Message, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	if(!SecondaryMessage.empty())
		dialog.set_secondary_text(SecondaryMessage);
	dialog.run();
}

unsigned int query_message(const std::string& Message, const unsigned int DefaultOption, const std::vector<std::string>& Options)
{
	return_val_if_fail(!k3d::batch_mode(), 0);

	Gtk::MessageDialog dialog(Message, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, true);

	for(unsigned long i = 0; i != Options.size(); ++i)
		dialog.add_button(Options[i], i+1);

	if(DefaultOption)
		dialog.set_default_response(DefaultOption-1);
	else
		dialog.set_default_response(Options.size());

	dialog.set_position(Gtk::WIN_POS_CENTER);

	dialog.show_all();
	int result = dialog.run();
	if(Gtk::RESPONSE_DELETE_EVENT == result)
		result = 0;

	return result;
}

void nag_message(const std::string& Type, const k3d::ustring& Message, const k3d::ustring& SecondaryMessage)
{
	if(k3d::batch_mode())
		return;

	if(!options::nag(Type))
		return;

	Gtk::MessageDialog dialog(Message.raw(), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
	if(!SecondaryMessage.empty())
		dialog.set_secondary_text(SecondaryMessage.raw());

	Gtk::CheckButton show_message(_("Display this message again in the future"));
	show_message.set_active(true);
	show_message.show();

	Gtk::Alignment alignment(0.5, 0.5, 0, 0);
	alignment.add(show_message);
	alignment.show();

	dialog.get_vbox()->pack_start(alignment);

	dialog.run();

	options::enable_nag(Type, show_message.get_active());
}

} // namespace ngui

} // namespace k3d

