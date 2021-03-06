// K-3D
// Copyright (c) 1995-2006, Timothy M. Shead
//
// Contact: tshead@k-3d.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your argument) any later version.
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

#include <k3dsdk/application_plugin_factory.h>
#include <k3dsdk/iuser_interface.h>
#include <k3dsdk/ievent_loop.h>
#include <k3dsdk/log.h>
#include <k3dsdk/module.h>

#include <iostream>

namespace module
{

namespace nui
{

/////////////////////////////////////////////////////////////////////////////
// user_interface

/// Provides the Null User Interface, a do-nothing user interface plugin useful for executing scripts and as a starting-point for user interface authors
class user_interface :
	public k3d::ievent_loop,
	public k3d::iuser_interface
{
public:
	void get_command_line_arguments(boost::program_options::options_description& Description)
	{
	}

	const arguments_t parse_startup_arguments(const arguments_t& Arguments, bool& Quit, bool& Error)
	{
		return Arguments;
	}

	const arguments_t parse_runtime_arguments(const arguments_t& Arguments, bool& Quit, bool& Error)
	{
		return Arguments;
	}

	void startup_message_handler(const k3d::string_t& Message)
	{
	}

	void display_user_interface()
	{
	}

	void start_event_loop()
	{
	}

	void stop_event_loop()
	{
	}

	void open_uri(const k3d::string_t& URI)
	{
	}

	void message(const k3d::string_t& Message)
	{
	}

	void warning_message(const k3d::string_t& Message)
	{
	}

	void error_message(const k3d::string_t& Message)
	{
	}

	unsigned int query_message(const k3d::string_t& Message, const unsigned int DefaultOption, const std::vector<k3d::string_t>& Options)
	{
		return 0;
	}

	bool tutorial_message(const k3d::string_t& Message)
	{
		return false;
	}

	bool get_file_path(const k3d::ipath_property::mode_t Mode, const k3d::string_t& Type, const k3d::string_t& Prompt, const k3d::filesystem::path& OldPath, k3d::filesystem::path& Result)
	{
		return false;
	}

	bool show(iunknown& Object)
	{
		return false;
	}

	void synchronize()
	{
	}

	sigc::connection get_timer(const double FrameRate, sigc::slot<void> Slot)
	{
		return sigc::connection();
	}
	
	k3d::uint_t watch_path(const k3d::filesystem::path& Path, const sigc::slot<void>& Slot)
	{	
		return 0;
	}

	void unwatch_path(const k3d::uint_t WatchID)
	{
	}

	k3d::iplugin_factory& factory()
	{
		return get_factory();
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::application_plugin_factory<user_interface,
			k3d::interface_list<k3d::ievent_loop> > factory(
			k3d::uuid(0xe6ade3f1, 0x484ce7b1, 0x1c118cb2, 0x9a3da138),
			"NUI",
			"Null User Interface (NUI)",
			"Interface");

		return factory;
	}
};

} // namespace nui

} // namespace module

K3D_MODULE_START(Registry)
	Registry.register_factory(module::nui::user_interface::get_factory());
K3D_MODULE_END


