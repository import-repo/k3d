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

#include <gtkmm/cellrenderertext.h>
#include <gtkmm/combobox.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>

#include "document_state.h"
#include "file_chooser_dialog.h"
#include "icons.h"
#include "menus.h"
#include "messages.h"
#include "viewport.h"

#include <k3dsdk/basic_math.h>
#include <k3dsdk/classes.h>
#include <k3dsdk/create_plugins.h>
#include <k3dsdk/file_range.h>
#include <k3dsdk/i18n.h>
#include <k3dsdk/ianimation_render_engine.h>
#include <k3dsdk/icamera.h>
#include <k3dsdk/icamera_animation_render_engine.h>
#include <k3dsdk/icamera_preview_render_engine.h>
#include <k3dsdk/icamera_still_render_engine.h>
#include <k3dsdk/ipreview_render_engine.h>
#include <k3dsdk/istill_render_engine.h>
#include <k3dsdk/nodes.h>
#include <k3dsdk/options.h>
#include <k3dsdk/path.h>
#include <k3dsdk/plugins.h>
#include <k3dsdk/property.h>
#include <k3dsdk/result.h>
#include <k3dsdk/share.h>
#include <k3dsdk/string_cast.h>
#include <k3dsdk/system.h>
#include <k3dsdk/time_source.h>

#include <boost/format.hpp>

namespace libk3dngui
{

namespace detail
{

class camera_columns :
	public Gtk::TreeModelColumnRecord
{
public:
	camera_columns()
	{
		add(node);
		add(factory);
		add(label);
		add(icon);
		add(separator);
	}

	Gtk::TreeModelColumn<k3d::inode*> node;
	Gtk::TreeModelColumn<k3d::iplugin_factory*> factory;
	Gtk::TreeModelColumn<Glib::ustring> label;
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
	Gtk::TreeModelColumn<bool> separator;
};

class camera_separators :
	public sigc::trackable
{
public:
	camera_separators(camera_columns& Columns) :
		columns(Columns)
	{
	}

	bool is_separator(const Glib::RefPtr<Gtk::TreeModel>& Model, const Gtk::TreeModel::iterator& Row)
	{
		return (*Row)[columns.separator];
	}

private:
	camera_columns& columns;
};

/// Prompt the user to choose an existing camera from within a document, or create a new one
k3d::icamera* pick_camera(document_state& DocumentState, const k3d::nodes_t& RenderEngines, const k3d::factories_t& Factories, const k3d::icamera* CurrentCamera, const std::string& Title, const std::string& Message)
{
	camera_columns columns;
	Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(columns);

	int index = 0;
	int active_row = 0;
	for(k3d::nodes_t::const_iterator node = RenderEngines.begin(); node != RenderEngines.end(); ++node)
	{
		Gtk::TreeRow row = *model->append();
		row[columns.node] = *node;
		row[columns.factory] = 0;
		row[columns.label] = (*node)->name();
		row[columns.icon] = quiet_load_icon((*node)->factory().name(), Gtk::ICON_SIZE_MENU);
		row[columns.separator] = false;

		if(CurrentCamera == dynamic_cast<k3d::icamera*>(*node))
			active_row = index;

		++index;
	}

	if(RenderEngines.size() && Factories.size())
	{
		Gtk::TreeRow row = *model->append();
		row[columns.node] = 0;
		row[columns.factory] = 0;
		row[columns.separator] = true;
	}

	for(k3d::factories_t::const_iterator factory = Factories.begin(); factory != Factories.end(); ++factory)
	{
		std::string markup;
		if(k3d::iplugin_factory::EXPERIMENTAL == (*factory)->quality())
		{
			markup = k3d::string_cast(boost::format(_("<span color=\"blue\">Create %1% (Experimental)</span>")) % (*factory)->name());
		}
		else if(k3d::iplugin_factory::DEPRECATED == (*factory)->quality())
		{
			markup = k3d::string_cast(boost::format(_("<span color=\"red\" strikethrough=\"true\">Create %1%</span><span color=\"red\"> (Deprecated)</span>")) % (*factory)->name());
		}
		else
		{
			markup = k3d::string_cast(boost::format(_("Create %1%")) % (*factory)->name());
		}

		Gtk::TreeRow row = *model->append();
		row[columns.node] = 0;
		row[columns.factory] = *factory;
		row[columns.label] = markup;
		row[columns.icon] = quiet_load_icon((*factory)->name(), Gtk::ICON_SIZE_MENU);
		row[columns.separator] = false;
	}

	Gtk::ComboBox combo(model);

	combo.pack_start(columns.icon, false);

	Gtk::CellRendererText* const label_renderer = new Gtk::CellRendererText();
	combo.pack_start(*manage(label_renderer));
	combo.add_attribute(label_renderer->property_markup(), columns.label);

	camera_separators separators(columns);
	combo.set_row_separator_func(sigc::mem_fun(separators, &camera_separators::is_separator));

	combo.set_active(active_row);

	Gtk::Dialog dialog(Title, true);
	dialog.set_border_width(5);
	dialog.get_vbox()->pack_start(*Gtk::manage(new Gtk::Label(Message)), Gtk::PACK_SHRINK, 5);
	dialog.get_vbox()->pack_start(combo, Gtk::PACK_SHRINK, 5);
	dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

	dialog.set_position(Gtk::WIN_POS_CENTER);
	dialog.show_all();

	if(Gtk::RESPONSE_OK == dialog.run())
	{
		return_val_if_fail(combo.get_active() != model->children().end(), 0);

		if(k3d::inode* const node = combo.get_active()->get_value(columns.node))
			return dynamic_cast<k3d::icamera*>(node);

		if(k3d::iplugin_factory* const factory = combo.get_active()->get_value(columns.factory))
			return dynamic_cast<k3d::icamera*>(DocumentState.create_node(factory));
	}

	return 0;
}

class render_engine_columns :
	public Gtk::TreeModelColumnRecord
{
public:
	render_engine_columns()
	{
		add(object);
		add(factory);
		add(label);
		add(icon);
		add(separator);
	}

	Gtk::TreeModelColumn<k3d::inode*> object;
	Gtk::TreeModelColumn<k3d::iplugin_factory*> factory;
	Gtk::TreeModelColumn<Glib::ustring> label;
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
	Gtk::TreeModelColumn<bool> separator;
};

class render_engine_separators :
	public sigc::trackable
{
public:
	render_engine_separators(render_engine_columns& Columns) :
		columns(Columns)
	{
	}

	bool is_separator(const Glib::RefPtr<Gtk::TreeModel>& Model, const Gtk::TreeModel::iterator& Row)
	{
		return (*Row)[columns.separator];
	}

private:
	render_engine_columns& columns;
};

/// Prompt the user to choose an existing render engine from within a document, or create a new one
template<typename interface_t>
interface_t* pick_render_engine(document_state& DocumentState, const k3d::nodes_t& RenderEngines, const k3d::factories_t& Factories, const std::string& Title, const std::string& Message)
{
	render_engine_columns columns;
	Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(columns);

	for(k3d::nodes_t::const_iterator object = RenderEngines.begin(); object != RenderEngines.end(); ++object)
	{
		Gtk::TreeRow row = *model->append();
		row[columns.object] = *object;
		row[columns.factory] = 0;
		row[columns.label] = (*object)->name();
		row[columns.icon] = quiet_load_icon((*object)->factory().name(), Gtk::ICON_SIZE_MENU);
		row[columns.separator] = false;
	}

	if(RenderEngines.size() && Factories.size())
	{
		Gtk::TreeRow row = *model->append();
		row[columns.object] = 0;
		row[columns.factory] = 0;
		row[columns.separator] = true;
	}

	for(k3d::factories_t::const_iterator factory = Factories.begin(); factory != Factories.end(); ++factory)
	{
		std::string markup;
		if(k3d::iplugin_factory::EXPERIMENTAL == (*factory)->quality())
		{
			markup = k3d::string_cast(boost::format(_("<span color=\"blue\">Create %1% (Experimental)</span>")) % (*factory)->name());
		}
		else if(k3d::iplugin_factory::DEPRECATED == (*factory)->quality())
		{
			markup = k3d::string_cast(boost::format(_("<span color=\"red\" strikethrough=\"true\">Create %1%</span><span color=\"red\"> (Deprecated)</span>")) % (*factory)->name());
		}
		else
		{
			markup = k3d::string_cast(boost::format(_("Create %1%")) % (*factory)->name());
		}

		Gtk::TreeRow row = *model->append();
		row[columns.object] = 0;
		row[columns.factory] = *factory;
		row[columns.label] = markup;
		row[columns.icon] = quiet_load_icon((*factory)->name(), Gtk::ICON_SIZE_MENU);
		row[columns.separator] = false;
	}

	Gtk::ComboBox combo(model);

	combo.pack_start(columns.icon, false);

	Gtk::CellRendererText* const label_renderer = new Gtk::CellRendererText();
	combo.pack_start(*manage(label_renderer));
	combo.add_attribute(label_renderer->property_markup(), columns.label);

	render_engine_separators separators(columns);
	combo.set_row_separator_func(sigc::mem_fun(separators, &render_engine_separators::is_separator));

	combo.set_active(0);

	Gtk::Dialog dialog(Title, true);
	dialog.set_border_width(5);
	dialog.get_vbox()->pack_start(*Gtk::manage(new Gtk::Label(Message)), Gtk::PACK_SHRINK, 5);
	dialog.get_vbox()->pack_start(combo, Gtk::PACK_SHRINK, 5);
	dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

	dialog.set_position(Gtk::WIN_POS_CENTER);
	dialog.show_all();

	if(Gtk::RESPONSE_OK == dialog.run())
	{

		return_val_if_fail(combo.get_active() != model->children().end(), 0);

		if(k3d::inode* const object = combo.get_active()->get_value(columns.object))
			return dynamic_cast<interface_t*>(object);

		if(k3d::iplugin_factory* const factory = combo.get_active()->get_value(columns.factory))
		{
			k3d::inode* const node = k3d::create_plugin<k3d::inode>(*factory, DocumentState.document(), k3d::unique_name(DocumentState.document().nodes(), factory->name()));
			DocumentState.view_node_properties_signal().emit(node);
			return dynamic_cast<interface_t*>(node);
		}
	}

	return 0;
}

/// Returns the specific RenderMan implementation (if any) to be used by a render engine, or an empty string
const std::string renderman_type(k3d::iunknown& Engine)
{
	if(k3d::inode* const node = dynamic_cast<k3d::inode*>(&Engine))
	{
		if(node->factory().class_id() == k3d::classes::RenderManEngine())
		{
			return boost::any_cast<std::string>(k3d::get_value(Engine, "render_engine"));
		}
	}

	return std::string();
}

/// Returns the path to a binary executable by searching the contents of the PATH environment variable, or an empty path
const k3d::filesystem::path find_executable(const std::string& Executable)
{
	const std::string executable_name = k3d::system::executable_name(Executable);

	k3d::filesystem::path result;

	const k3d::filesystem::path_list paths = k3d::filesystem::split_native_paths(k3d::ustring::from_utf8(k3d::system::getenv("PATH")));
	for(k3d::filesystem::path_list::const_iterator path = paths.begin(); path != paths.end(); ++path)
	{
		const k3d::filesystem::path test_path = (*path) / k3d::filesystem::generic_path(k3d::ustring::from_utf8(executable_name));
		if(k3d::filesystem::exists(test_path))
		{
			result = test_path;
			break;
		}
	}

	return result;
}

/// Performs sanity checks to see if Aqsis is installed and usable
void test_aqsis_render_engine(k3d::iunknown& Engine)
{
	static bool test_performed = false;
	if(test_performed)
		return;

	if(renderman_type(Engine) != "aqsis")
		return;

	test_performed = true;

	if(find_executable("aqsis").empty())
	{
		error_message(
			_("Could not locate the aqsis executable."),
			_("Without it, RIB files cannot be rendered.  Check to ensure that you have Aqsis installed, and that the PATH envrionment variable points to the Aqsis binary installation directory."));
		return;
	}

	if(find_executable("aqsl").empty())
	{
		error_message(
			_("Could not locate the aqsl executable."),
			_("Without it, shaders cannot be compiled.  Check to ensure that you have Aqsis installed, and that the PATH envrionment variable points to the Aqsis binary installation directory."));
		return;
	}
}

/// Performs sanity checks to see if Pixie is installed and usable
void test_pixie_render_engine(k3d::iunknown& Engine)
{
	static bool test_performed = false;
	if(test_performed)
		return;

	if(renderman_type(Engine) != "pixie")
		return;

	test_performed = true;

	if(find_executable("rndr").empty())
	{
		error_message(
			_("Could not locate the rndr executable."),
			_("Without it, RIB files cannot be rendered.  Check to ensure that you have Pixie installed, and that the PATH environment variable points to the Pixie binary installation directory."));
		return;
	}

	if(find_executable("sdrc").empty())
	{
		error_message(
			_("Could not locate the sdrc executable."),
			_("Without it, shaders cannot be compiled.  Check to ensure that you have Pixie installed, and that the PATH environment variable points to the Pixie binary installation directory."));
		return;
	}
}

/// Performs sanity checks to see if Yafray is installed and usable
void test_yafray_render_engine(k3d::iunknown& Engine)
{
	static bool test_performed = false;
	if(test_performed)
		return;

	k3d::inode* const node = dynamic_cast<k3d::inode*>(&Engine);
	if(!node)
		return;

	if(node->factory().class_id() != k3d::uuid(0xef38bf93, 0x66654f9f, 0x992ca91b, 0x62bae139))
		return;

	test_performed = true;

	if(find_executable("yafray").empty())
	{
		error_message(
			_("Could not locate the yafray executable."),
			_("Check to ensure that you have Yafray installed, and that the PATH environment variable points to the Yafray installation directory."));
		return;
	}
}

class animation_sample_columns :
	public Gtk::TreeModelColumnRecord
{
public:
	animation_sample_columns()
	{
		add(file);
	}

	Gtk::TreeModelColumn<Glib::ustring> file;
};

/// Prompt the user to choose a range of files for saving an animation
class animation_chooser_dialog :
	public Gtk::FileChooserDialog
{
	typedef Gtk::FileChooserDialog base;

public:
	animation_chooser_dialog() :
		base(_("Choose animation output files:"), Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
		model(Gtk::ListStore::create(columns))
	{
	}

	bool get_files(k3d::file_range& Files)
	{
		files = &Files;

		// Create widgets that will be added to the dialog.  We do this on the stack
		// so the same buttons don't get added to the widget multiple times if get_files() is
		// called more-than-once.

		Gtk::Label file_label(_("Choose the format for output files:"));
		file_label.set_alignment(0.0, 0.5);

		before.set_text(Files.before.raw());
		before.signal_changed().connect(sigc::mem_fun(*this, &animation_chooser_dialog::before_changed));

		Gtk::Entry digits;
		digits.set_editable(false);
		digits.set_text(Glib::ustring(Files.digits, '0'));
		digits.set_max_length(Files.digits);

		after.set_text(Files.after.raw());
		after.signal_changed().connect(sigc::mem_fun(*this, &animation_chooser_dialog::after_changed));

		Gtk::HBox file(false, 0);
		file.pack_start(before);
		file.pack_start(digits);
		file.pack_start(after);

		Gtk::Label samples_label(_("Generated filenames:"));
		samples_label.set_alignment(0.0, 0.5);

		generate_samples();

		Gtk::TreeView samples(model);
		samples.set_headers_visible(false);
		samples.set_reorderable(false);
		samples.append_column("", columns.file);

		Gtk::ScrolledWindow samples_window;
		samples_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		samples_window.add(samples);

		Gtk::VBox extra_widget(false, 5);
		extra_widget.pack_start(file_label);
		extra_widget.pack_start(file);
		extra_widget.pack_start(samples_label);
		extra_widget.pack_start(samples_window, Gtk::PACK_EXPAND_WIDGET);

		set_extra_widget(extra_widget);

		Gtk::Button cancel_widget(Gtk::Stock::CANCEL);
		cancel_widget.set_flags(cancel_widget.get_flags() | Gtk::CAN_DEFAULT);
		cancel_widget.show();

		Gtk::Button save_widget(Gtk::Stock::SAVE);
		save_widget.set_flags(save_widget.get_flags() | Gtk::CAN_DEFAULT);
		save_widget.show();

		// Add the K-3D share path as a shortcut ...
		add_shortcut_folder(k3d::share_path().native_utf8_string().raw());

		add_action_widget(cancel_widget, Gtk::RESPONSE_CANCEL);

		add_action_widget(save_widget, Gtk::RESPONSE_OK);
		set_default_response(Gtk::RESPONSE_OK);

		// Setup the initial path to display ...
		k3d::filesystem::path start_path = Files.directory;
		if(start_path.empty())
			start_path = k3d::options::get_path(k3d::options::path::render_animation());

		if(start_path.empty())
			start_path = k3d::system::get_home_directory();

		if(k3d::filesystem::exists(start_path) && k3d::filesystem::is_directory(start_path))
			set_current_folder(start_path.native_filesystem_string());

		// Run the dialog ...
		set_position(Gtk::WIN_POS_CENTER);
		extra_widget.show_all();
		if(Gtk::RESPONSE_OK != run())
			return false;

		Files.directory = k3d::filesystem::native_path(k3d::ustring::from_utf8(Glib::filename_to_utf8(get_filename()).raw()));

/*
		// Prompt the user if they're about to overwrite an existing file ...
		if(!prompt_file_overwrite(Result))
			return false;
*/

		// Record the path for posterity ...
		k3d::options::set_path(k3d::options::path::render_animation(), Files.directory);

		return true;
	}

private:
	void before_changed()
	{
		files->before = k3d::ustring::from_utf8(before.get_text());
		generate_samples();
	}

	void after_changed()
	{
		files->after = k3d::ustring::from_utf8(after.get_text());
		generate_samples();
	}

	void generate_samples()
	{
		model->clear();

		for(size_t file = files->begin; file != files->end; ++file)
		{
			Gtk::TreeRow row = *model->append();
			row[columns.file] = files->file(file).native_utf8_string().raw();
		}
	}

	k3d::file_range* files;
	Gtk::Entry before;
	Gtk::Entry after;

	animation_sample_columns columns;
	const Glib::RefPtr<Gtk::ListStore> model;
};

} // namespace detail

k3d::icamera* default_camera(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::icamera>(DocumentState.document().nodes());
	return render_engines.size() == 1 ? dynamic_cast<k3d::icamera*>(*render_engines.begin()) : 0;
}

k3d::ipreview_render_engine* default_preview_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::ipreview_render_engine>(DocumentState.document().nodes());
	return render_engines.size() == 1 ? dynamic_cast<k3d::ipreview_render_engine*>(*render_engines.begin()) : 0;
}

k3d::istill_render_engine* default_still_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::istill_render_engine>(DocumentState.document().nodes());
	return render_engines.size() == 1 ? dynamic_cast<k3d::istill_render_engine*>(*render_engines.begin()) : 0;
}

k3d::ianimation_render_engine* default_animation_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::ianimation_render_engine>(DocumentState.document().nodes());
	return render_engines.size() == 1 ? dynamic_cast<k3d::ianimation_render_engine*>(*render_engines.begin()) : 0;
}

k3d::icamera_preview_render_engine* default_camera_preview_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::icamera_preview_render_engine>(DocumentState.document().nodes());
	return render_engines.size() == 1 ? dynamic_cast<k3d::icamera_preview_render_engine*>(*render_engines.begin()) : 0;
}

k3d::icamera_still_render_engine* default_camera_still_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::icamera_still_render_engine>(DocumentState.document().nodes());
	return render_engines.size() == 1 ? dynamic_cast<k3d::icamera_still_render_engine*>(*render_engines.begin()) : 0;
}

k3d::icamera_animation_render_engine* default_camera_animation_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::icamera_animation_render_engine>(DocumentState.document().nodes());
	return render_engines.size() == 1 ? dynamic_cast<k3d::icamera_animation_render_engine*>(*render_engines.begin()) : 0;
}

k3d::gl::irender_engine* default_gl_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::gl::irender_engine>(DocumentState.document().nodes());
	return render_engines.size() == 1 ? dynamic_cast<k3d::gl::irender_engine*>(*render_engines.begin()) : 0;
}

k3d::icamera* pick_camera(document_state& DocumentState, const k3d::icamera* CurrentCamera)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::icamera>(DocumentState.document().nodes());
	const k3d::factories_t factories = k3d::plugins<k3d::icamera>();

	return detail::pick_camera(DocumentState, render_engines, factories, CurrentCamera, _("Pick Camera:"), _("Choose a camera"));
}

k3d::ipreview_render_engine* pick_preview_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::ipreview_render_engine>(DocumentState.document().nodes());
	const k3d::factories_t factories = k3d::plugins<k3d::ipreview_render_engine>();

	return detail::pick_render_engine<k3d::ipreview_render_engine>(DocumentState, render_engines, factories, _("Pick Preview Render Engine:"), _("Choose a render engine to be used for preview image rendering"));
}

k3d::istill_render_engine* pick_still_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::istill_render_engine>(DocumentState.document().nodes());
	const k3d::factories_t factories = k3d::plugins<k3d::istill_render_engine>();

	return detail::pick_render_engine<k3d::istill_render_engine>(DocumentState, render_engines, factories, _("Pick Still Render Engine:"), _("Choose a render engine to be used for still image rendering"));
}

k3d::ianimation_render_engine* pick_animation_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::ianimation_render_engine>(DocumentState.document().nodes());
	const k3d::factories_t factories = k3d::plugins<k3d::ianimation_render_engine>();

	return detail::pick_render_engine<k3d::ianimation_render_engine>(DocumentState, render_engines, factories, _("Pick Animation Render Engine:"), _("Choose a render engine to be used for animation rendering"));
}

k3d::icamera_preview_render_engine* pick_camera_preview_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::icamera_preview_render_engine>(DocumentState.document().nodes());
	const k3d::factories_t factories = k3d::plugins<k3d::icamera_preview_render_engine>();

	return detail::pick_render_engine<k3d::icamera_preview_render_engine>(DocumentState, render_engines, factories, _("Pick Preview Render Engine:"), _("Choose a render engine to be used for preview image rendering"));
}

k3d::icamera_still_render_engine* pick_camera_still_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::icamera_still_render_engine>(DocumentState.document().nodes());
	const k3d::factories_t factories = k3d::plugins<k3d::icamera_still_render_engine>();

	return detail::pick_render_engine<k3d::icamera_still_render_engine>(DocumentState, render_engines, factories, _("Pick Still Render Engine:"), _("Choose a render engine to be used for still image rendering"));
}

k3d::icamera_animation_render_engine* pick_camera_animation_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::icamera_animation_render_engine>(DocumentState.document().nodes());
	const k3d::factories_t factories = k3d::plugins<k3d::icamera_animation_render_engine>();

	return detail::pick_render_engine<k3d::icamera_animation_render_engine>(DocumentState, render_engines, factories, _("Pick Animation Render Engine:"), _("Choose a render engine to be used for animation rendering"));
}

k3d::gl::irender_engine* pick_gl_render_engine(document_state& DocumentState)
{
	const k3d::nodes_t render_engines = k3d::find_nodes<k3d::gl::irender_engine>(DocumentState.document().nodes());
	const k3d::factories_t factories = k3d::plugins<k3d::gl::irender_engine>();

	return detail::pick_render_engine<k3d::gl::irender_engine>(DocumentState, render_engines, factories, _("Pick OpenGL Render Engine:"), _("Choose an OpenGL render engine to be used for drawing"));
}

void test_render_engine(k3d::iunknown& Engine)
{
	detail::test_aqsis_render_engine(Engine);
	detail::test_pixie_render_engine(Engine);
	detail::test_yafray_render_engine(Engine);
}

void render_preview(k3d::ipreview_render_engine& Engine)
{
	test_render_engine(Engine);
	assert_warning(Engine.render_preview());
}

void render_frame(k3d::istill_render_engine& Engine)
{
	k3d::filesystem::path file;
	{
		file_chooser_dialog dialog(_("Render Frame:"), k3d::options::path::render_frame(), Gtk::FILE_CHOOSER_ACTION_SAVE);
		if(!dialog.get_file_path(file))
			return;
	}

	test_render_engine(Engine);
	assert_warning(Engine.render_frame(file, true));
}

void render_animation(document_state& DocumentState, k3d::ianimation_render_engine& Engine)
{
	// Ensure that the document has animation capabilities, first ...
	k3d::iproperty* const start_time_property = k3d::get_start_time(DocumentState.document());
	k3d::iproperty* const end_time_property = k3d::get_end_time(DocumentState.document());
	k3d::iproperty* const frame_rate_property = k3d::get_frame_rate(DocumentState.document());
	return_if_fail(start_time_property && end_time_property && frame_rate_property);

	const double start_time = boost::any_cast<double>(k3d::get_value(DocumentState.document().dag(), *start_time_property));
	const double end_time = boost::any_cast<double>(k3d::get_value(DocumentState.document().dag(), *end_time_property));
	const double frame_rate = boost::any_cast<double>(k3d::get_value(DocumentState.document().dag(), *frame_rate_property));

	const long start_frame = static_cast<long>(k3d::round(frame_rate * start_time));
	const long end_frame = static_cast<long>(k3d::round(frame_rate * end_time));

	k3d::file_range files;
	files.before = k3d::ustring::from_utf8("output");
	files.begin = start_frame;
	files.end = end_frame + 1;

	// Make sure the supplied filepath has enough digits to render the entire animation ...
	while(files.max_file_count() <= end_frame)
		files.digits += 1;

	{
		// Prompt the user for a base filename ...
		detail::animation_chooser_dialog dialog;

		if(!dialog.get_files(files))
			return;
	}

	// See if the user wants to view frames as they're completed ...
	std::vector<std::string> buttons;
	buttons.push_back("Yes");
	buttons.push_back("No");
	buttons.push_back("Cancel");

	const unsigned long result = query_message("Do you want to see rendered frames as they're completed?", 1, buttons);
	if(0 == result || 3 == result)
		return;

	const bool viewcompleted = (1 == result);

	test_render_engine(Engine);
	assert_warning(Engine.render_animation(files, viewcompleted));
}

void render_camera_preview(k3d::icamera& Camera, k3d::icamera_preview_render_engine& Engine)
{
	test_render_engine(Engine);
	assert_warning(Engine.render_camera_preview(Camera));
}

void render_camera_frame(k3d::icamera& Camera, k3d::icamera_still_render_engine& Engine)
{
	k3d::filesystem::path file;
	{
		file_chooser_dialog dialog(_("Render Frame:"), k3d::options::path::render_frame(), Gtk::FILE_CHOOSER_ACTION_SAVE);

		// Try to infer the correct file extension behavior ...
		if(dynamic_cast<viewport::control*>(&Engine))
		{
			dialog.add_pattern_filter(_("PNM Image (*.pnm)"), "*.pnm");
			dialog.add_all_files_filter();
			dialog.append_extension(".pnm");
		}
		else if(k3d::inode* const node = dynamic_cast<k3d::inode*>(&Engine))
		{
			if(node->factory().class_id() == k3d::classes::RenderManEngine())
			{
				dialog.add_pattern_filter(_("TIFF Image (*.tiff)"), "*.tiff");
				dialog.add_all_files_filter();
				dialog.append_extension(".tiff");
			}
			else if(node->factory().class_id() == k3d::uuid(0xef38bf93, 0x66654f9f, 0x992ca91b, 0x62bae139))
			{
				dialog.add_pattern_filter(_("Targa Image (*.tga)"), "*.tga");
				dialog.add_all_files_filter();
				dialog.append_extension(".tga");
			}
		}

		if(!dialog.get_file_path(file))
			return;
	}

	test_render_engine(Engine);
	assert_warning(Engine.render_camera_frame(Camera, file, true));
}

void render_camera_animation(document_state& DocumentState, k3d::icamera& Camera, k3d::icamera_animation_render_engine& Engine)
{
	// Ensure that the document has animation capabilities, first ...
	k3d::iproperty* const start_time_property = k3d::get_start_time(DocumentState.document());
	k3d::iproperty* const end_time_property = k3d::get_end_time(DocumentState.document());
	k3d::iproperty* const frame_rate_property = k3d::get_frame_rate(DocumentState.document());
	return_if_fail(start_time_property && end_time_property && frame_rate_property);

	const double start_time = boost::any_cast<double>(k3d::get_value(DocumentState.document().dag(), *start_time_property));
	const double end_time = boost::any_cast<double>(k3d::get_value(DocumentState.document().dag(), *end_time_property));
	const double frame_rate = boost::any_cast<double>(k3d::get_value(DocumentState.document().dag(), *frame_rate_property));

	const long start_frame = static_cast<long>(k3d::round(frame_rate * start_time));
	const long end_frame = static_cast<long>(k3d::round(frame_rate * end_time));

	k3d::file_range files;
	files.before = k3d::ustring::from_utf8("output");
	files.begin = start_frame;
	files.end = end_frame + 1;

	// Try to infer the correct file extension behavior ...
	if(dynamic_cast<viewport::control*>(&Engine))
	{
		files.after = k3d::ustring::from_utf8(".pnm");
	}
	else if(k3d::inode* const node = dynamic_cast<k3d::inode*>(&Engine))
	{
		if(node->factory().class_id() == k3d::classes::RenderManEngine())
		{
			files.after = k3d::ustring::from_utf8(".tiff");
		}
		else if(node->factory().class_id() == k3d::uuid(0xef38bf93, 0x66654f9f, 0x992ca91b, 0x62bae139))
		{
			files.after = k3d::ustring::from_utf8(".tga");
		}
	}

	// Make sure the supplied filepath has enough digits to render the entire animation ...
	while(files.max_file_count() <= end_frame)
		files.digits += 1;

	{
		// Prompt the user for a base filename ...
		detail::animation_chooser_dialog dialog;
		if(!dialog.get_files(files))
			return;
	}

	// See if the user wants to view frames as they're completed ...
	std::vector<std::string> buttons;
	buttons.push_back("Yes");
	buttons.push_back("No");
	buttons.push_back("Cancel");

	const unsigned long result = query_message("Do you want to see rendered frames as they're completed?", 1, buttons);
	if(0 == result || 3 == result)
		return;

	const bool viewcompleted = (1 == result);

	test_render_engine(Engine);
	assert_warning(Engine.render_camera_animation(Camera, files, viewcompleted));
}

} // namespace libk3dngui