// K-3D
// Copyright (c) 1995-2007, Timothy M. Shead
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
	\brief Implements the context menu
	\author Timothy M. Shead (tshead@k-3d.com)
	\author Romain Behar (romainbehar@yahoo.com)
*/

#include <gtkmm/image.h>
#include <gtkmm/menu.h>
#include <gtkmm/stock.h>

#include "check_menu_item.h"
#include "context_menu.h"
#include "detail.h"
#include "document_state.h"
#include "file_chooser_dialog.h"
#include "icons.h"
#include "image_menu_item.h"
#include "menu_item.h"
#include "menus.h"
#include "messages.h"
#include "modifiers.h"
#include "render.h"
#include "viewport.h"
#include "widget_manip.h"

#include <k3dsdk/classes.h>
#include <k3dsdk/dependencies.h>
#include <k3dsdk/fstream.h>
#include <k3dsdk/i18n.h>
#include <k3dsdk/ianimation_render_engine.h>
#include <k3dsdk/icamera.h>
#include <k3dsdk/idag.h>
#include <k3dsdk/imesh_selection_sink.h>
#include <k3dsdk/imesh_sink.h>
#include <k3dsdk/imesh_source.h>
#include <k3dsdk/imesh_storage.h>
#include <k3dsdk/inode_collection.h>
#include <k3dsdk/ipreview_render_engine.h>
#include <k3dsdk/iproperty_collection.h>
#include <k3dsdk/iselectable.h>
#include <k3dsdk/istill_render_engine.h>
#include <k3dsdk/itransform_sink.h>
#include <k3dsdk/itransform_source.h>
#include <k3dsdk/legacy_mesh.h>
#include <k3dsdk/mesh_selection.h>
#include <k3dsdk/nodes.h>
#include <k3dsdk/options.h>
#include <k3dsdk/persistent_lookup.h>
#include <k3dsdk/plugins.h>
#include <k3dsdk/property.h>
#include <k3dsdk/serialization.h>
#include <k3dsdk/state_change_set.h>
#include <k3dsdk/utility_gl.h>

namespace libk3dngui
{

namespace detail
{

/// Provides a standardized context menu for nodes
class node_context_menu :
	public Gtk::Menu
{
public:
	node_context_menu(document_state& DocumentState, k3d::icommand_node& Parent) :
		m_document_state(DocumentState)
	{
		// Create viewport-specific functionality ...
		m_viewport1 = new Gtk::MenuItem(*Gtk::manage(
			new Gtk::Label() <<
			alignment(0, 0.5) <<
			set_markup(_("<b><i>Viewport:</i></b>"))));
		append(*Gtk::manage(m_viewport1));

		// Render-related functionality ...
		Gtk::Menu* const render_submenu = new Gtk::Menu();

		render_submenu->items().push_back(*Gtk::manage(
			new image_menu_item::control(Parent, "render_preview",
			*Gtk::manage(new Gtk::Image(load_icon("render_preview", Gtk::ICON_SIZE_MENU))),
			_("Render Preview")) <<
				connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_render_preview))));

		render_submenu->items().push_back(*Gtk::manage(
			new image_menu_item::control(Parent, "render_frame",
			*Gtk::manage(new Gtk::Image(load_icon("render_frame", Gtk::ICON_SIZE_MENU))),
			_("Render Image")) <<
				connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_render_frame))));

		render_submenu->items().push_back(*Gtk::manage(
			new image_menu_item::control(Parent, "render_animation",
			*Gtk::manage(new Gtk::Image(load_icon("render_animation", Gtk::ICON_SIZE_MENU))),
			_("Render Animation")) <<
				connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_render_animation))));

		items().push_back(Gtk::Menu_Helpers::MenuElem(_("Render"), *Gtk::manage(render_submenu)));

		items().push_back(*Gtk::manage(
			new menu_item::control(Parent, "set_viewport_camera", _("Set Camera ...")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_set_viewport_camera))));

		// Set render engines
		Gtk::Menu* const engines_submenu = new Gtk::Menu();

		engines_submenu->items().push_back(*Gtk::manage(
			new menu_item::control(Parent, "set_viewport_preview_engine", _("Set Preview Engine ...")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_set_viewport_preview_engine))));

		engines_submenu->items().push_back(*Gtk::manage(
			new menu_item::control(Parent, "set_viewport_still_engine", _("Set Still Engine ...")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_set_viewport_still_engine))));

		engines_submenu->items().push_back(*Gtk::manage(
			new menu_item::control(Parent, "set_viewport_animation_engine", _("Set Animation Engine ...")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_set_viewport_animation_engine))));

		engines_submenu->items().push_back(*Gtk::manage(
			new menu_item::control(Parent, "set_viewport_gl_engine", _("Set OpenGL Engine ...")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_set_viewport_gl_engine))));

		items().push_back(Gtk::Menu_Helpers::MenuElem(_("Render Engine"), *Gtk::manage(engines_submenu)));

		// Viewport section end
		items().push_back(Gtk::Menu_Helpers::SeparatorElem());

		// Functionality common to all nodes ...
		m_delete_nodes = new menu_item::control(Parent, "delete", _("Delete")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_delete));
		items().push_back(*Gtk::manage(m_delete_nodes));

		// Persistence-related functionality ...
/*		if(dynamic_cast<k3d::ipersistent*>(&Object))
		{
			items().push_back(Gtk::Menu_Helpers::SeparatorElem());
			items().push_back(*Gtk::manage(
				new image_menu_item::control(Parent, "save_node",
					*Gtk::manage(new Gtk::Image(Gtk::Stock::SAVE, Gtk::ICON_SIZE_MENU)),
					_("Save Object")) <<
				connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_save_node))));
			items().push_back(*Gtk::manage(
				new image_menu_item::control(Parent, "load_node",
					*Gtk::manage(new Gtk::Image(Gtk::Stock::OPEN, Gtk::ICON_SIZE_MENU)),
					_("Load Object")) <<
				connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_load_node))));
		}
*/

		// Mesh-specific functionality ...
		m_instantiate_meshes = new menu_item::control(Parent, "instantiate_meshes", _("Instantiate")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_instantiate));
		items().push_back(*Gtk::manage(m_instantiate_meshes));

		m_duplicate_meshes = new menu_item::control(Parent, "duplicate_meshes", _("Duplicate")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_duplicate));
		items().push_back(*Gtk::manage(m_duplicate_meshes));

		// Mesh modifiers menu
		const factories_t& mesh_modifier_factories = mesh_modifiers();
		if(!mesh_modifier_factories.empty())
		{
			m_mesh_modifiers = new Gtk::Menu();

			for(factories_t::const_iterator modifier = mesh_modifier_factories.begin(); modifier != mesh_modifier_factories.end(); ++modifier)
			{
				k3d::iplugin_factory& factory = **modifier;

				m_mesh_modifiers->items().push_back(*Gtk::manage(
					create_menu_item(Parent, "mesh_modifier_", factory) <<
					connect_menu_item(sigc::bind(sigc::mem_fun(*this, &node_context_menu::on_modify_meshes), &factory))));
			}
			items().push_back(Gtk::Menu_Helpers::MenuElem(_("Mesh Modifier"), *manage(m_mesh_modifiers)));
		}

		// Transform-specific functionality ...
		const factories_t& transform_modifier_factories = transform_modifiers();
		if(!transform_modifier_factories.empty())
		{
			m_transform_modifiers = new Gtk::Menu();

			for(factories_t::const_iterator modifier = transform_modifier_factories.begin(); modifier != transform_modifier_factories.end(); ++modifier)
			{
				k3d::iplugin_factory& factory = **modifier;

				m_transform_modifiers->items().push_back(*Gtk::manage(
					create_menu_item(Parent, "transform_modifier_", factory) <<
					connect_menu_item(sigc::bind(sigc::mem_fun(*this, &node_context_menu::on_modifier_transform), &factory))));
			}
			items().push_back(Gtk::Menu_Helpers::MenuElem(_("Transform Modifier"), *manage(m_transform_modifiers)));
		}

		// Viewport visibility ...
		items().push_back(Gtk::Menu_Helpers::SeparatorElem());

		items().push_back(*Gtk::manage(
			new menu_item::control(Parent, "hide_selection", _("Hide Selection")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_hide_selection))));

		items().push_back(*Gtk::manage(
			new menu_item::control(Parent, "show_selection", _("Show Selection")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_show_selection))));

		items().push_back(*Gtk::manage(
			new menu_item::control(Parent, "hide_unselected", _("Hide Unselected")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_hide_unselected))));

		items().push_back(*Gtk::manage(
			new menu_item::control(Parent, "show_all", _("Show All")) <<
			connect_menu_item(sigc::mem_fun(*this, &node_context_menu::on_show_all))));

		show_all();
	}

	void update_menu()
	{
		// Display viewport-specific functionality ...
		m_viewport = m_document_state.get_focus_viewport();
		const bool display_viewport = (m_viewport && m_viewport->gl_engine());
		m_viewport1->set_sensitive(display_viewport);

		// Mesh specific functionality
		unsigned long selected_node_counter = 0;
		unsigned long selected_mesh_counter = 0;
		unsigned long selected_transform_counter = 0;
		const k3d::nodes_t selected_nodes = m_document_state.selected_nodes();
		for(k3d::nodes_t::const_iterator node = selected_nodes.begin(); node != selected_nodes.end(); ++node)
		{
			++selected_node_counter;

			if(dynamic_cast<k3d::imesh_sink*>(*node))
				++selected_mesh_counter;
			if(dynamic_cast<k3d::itransform_sink*>(*node))
				++selected_transform_counter;
		}

		const bool non_empty_selection = (selected_node_counter > 0);
		m_delete_nodes->set_sensitive(non_empty_selection);
		m_instantiate_meshes->set_sensitive(non_empty_selection);
		m_duplicate_meshes->set_sensitive(non_empty_selection);

		const bool display_mesh_modifiers = (selected_mesh_counter > 0);
		m_mesh_modifiers->set_sensitive(display_mesh_modifiers);

		const bool display_transform_modifiers = (selected_transform_counter > 0);
		m_transform_modifiers->set_sensitive(display_transform_modifiers);
	}

	void on_show()
	{
		update_menu();
		Gtk::Widget::on_show();
	}

private:
	void on_set_viewport_camera()
	{
		return_if_fail(m_viewport);

		k3d::icamera* const camera = pick_camera(m_document_state, m_viewport->camera());
		if(!camera)
			return;

		m_viewport->set_camera(camera);
	}

	void on_set_viewport_preview_engine()
	{
		return_if_fail(m_viewport);

		k3d::icamera_preview_render_engine* const engine = pick_camera_preview_render_engine(m_document_state);
		if(!engine)
			return;

		m_viewport->set_camera_preview_engine(engine);
	}

	void on_set_viewport_still_engine()
	{
		return_if_fail(m_viewport);

		k3d::icamera_still_render_engine* const engine = pick_camera_still_render_engine(m_document_state);
		if(!engine)
			return;

		m_viewport->set_camera_still_engine(engine);
	}

	void on_set_viewport_animation_engine()
	{
		return_if_fail(m_viewport);

		k3d::icamera_animation_render_engine* const engine = pick_camera_animation_render_engine(m_document_state);
		if(!engine)
			return;

		m_viewport->set_camera_animation_engine(engine);
	}

	void on_set_viewport_gl_engine()
	{
		return_if_fail(m_viewport);

		k3d::gl::irender_engine* const engine = pick_gl_render_engine(m_document_state);
		if(!engine)
			return;

		m_viewport->set_gl_engine(engine);
	}

	void on_render_preview()
	{
		return_if_fail(m_viewport);

		k3d::icamera* camera = m_viewport->camera();
		if(!camera)
			camera = pick_camera(m_document_state);
		if(!camera)
			return;

		k3d::icamera_preview_render_engine* render_engine = m_viewport->camera_preview_engine();
		if(!render_engine)
			render_engine = pick_camera_preview_render_engine(m_document_state);
		if(!render_engine)
			return;

		m_viewport->set_camera(camera);
		m_viewport->set_camera_preview_engine(render_engine);

		render_camera_preview(*camera, *render_engine);
	}

	void on_render_frame()
	{
		k3d::icamera* camera = m_viewport ? m_viewport->camera() : 0;
		if(!camera)
			camera = pick_camera(m_document_state);
		if(!camera)
			return;

		k3d::icamera_still_render_engine* render_engine = m_viewport ? m_viewport->camera_still_engine() : 0;
		if(!render_engine)
			render_engine = pick_camera_still_render_engine(m_document_state);
		if(!render_engine)
			return;

		if(m_viewport)
		{
			m_viewport->set_camera(camera);
			m_viewport->set_camera_still_engine(render_engine);
		}

		render_camera_frame(*camera, *render_engine);
	}

	void on_render_animation()
	{
		k3d::icamera* camera = m_viewport ? m_viewport->camera() : 0;
		if(!camera)
			camera = pick_camera(m_document_state);
		if(!camera)
			return;

		k3d::icamera_animation_render_engine* render_engine = m_viewport ? m_viewport->camera_animation_engine() : 0;
		if(!render_engine)
			render_engine = pick_camera_animation_render_engine(m_document_state);
		if(!render_engine)
			return;

		if(m_viewport)
		{
			m_viewport->set_camera(camera);
			m_viewport->set_camera_animation_engine(render_engine);
		}

		render_camera_animation(m_document_state, *camera, *render_engine);
	}

/*
	void on_save_node()
	{
		k3d::ipersistent* const persistent = dynamic_cast<k3d::ipersistent*>(&m_node);
		return_if_fail(persistent);

		k3d::filesystem::path path;
		if(!get_file_path(k3d::ipath_property::WRITE, m_node.factory().name(), _("Save Object Properties:"), k3d::filesystem::path(), path))
			return;

		k3d::log() << info << "Writing .k3d file: " << path.native_string() << std::endl;
		k3d::filesystem::ofstream file(path);
		if(!file)
			{
				k3d::log() << error << k3d_file_reference << ": error opening [" << path.native_string() << "]" << std::endl;
				return;
			}

		const k3d::filesystem::path root_path = path.branch_path();
		k3d::dependencies dependencies;
		k3d::persistent_lookup lookup;
		k3d::ipersistent::save_context context(root_path, dependencies, lookup);

		k3d::xml::element xml("k3dml");
		k3d::save_node(*persistent, xml, context);

		file << k3d::xml::declaration() << xml;
	}

	void on_load_node()
	{
		k3d::ipersistent* const persistent = dynamic_cast<k3d::ipersistent*>(&m_node);
		return_if_fail(persistent);

		k3d::filesystem::path path;
		if(!get_file_path(k3d::ipath_property::READ, m_node.factory().name(), _("Load Object Properties:"), k3d::filesystem::path(), path))
			return;

		k3d::log() << info << "Reading .k3d file: " << path.native_string() << std::endl;
		k3d::filesystem::ifstream file(path);
		if(!file)
		{
			k3d::log() << error << k3d_file_reference << ": error opening [" << path.native_string() << "]" << std::endl;
			return;
		}

		k3d::xml::element xml("k3dml");
		try
		{
			k3d::xml::hide_progress progress;
			k3d::xml::parse(xml, file, path.native_string(), progress);
		}
		catch(std::exception& e)
		{
			k3d::log() << error << e.what() << std::endl;
			return;
		}

		return_if_fail(xml.name == "k3dml");

		k3d::xml::element* const xml_node = k3d::xml::find_element(xml, "node");
		return_if_fail(xml_node);

		const k3d::uuid class_id = k3d::xml::attribute_value<k3d::uuid>(*xml_node, "class", k3d::uuid::null());
		if(class_id != m_node.factory().class_id())
		{
			error_message(_("Wrong node type"), _("Load Object Properties:"));
			return;
		}

		k3d::record_state_change_set changeset(m_document_state.document(), k3d::string_cast(boost::format(_("Load %1%")) % path.native_string()));

		const k3d::filesystem::path root_path = path.branch_path();
		k3d::persistent_lookup lookup;
		k3d::ipersistent::load_context context(root_path, lookup);

		persistent->load(*xml_node, context);
	}
*/

	void on_delete()
	{
		k3d::record_state_change_set changeset(m_document_state.document(), _("Delete nodes"), K3D_CHANGE_SET_CONTEXT);

		k3d::nodes_t nodes = m_document_state.selected_nodes();

		k3d::delete_nodes(m_document_state.document(), nodes);
		k3d::gl::redraw_all(m_document_state.document(), k3d::gl::irender_engine::ASYNCHRONOUS);
	}

	/// Instantiates selected meshes
	void on_instantiate()
	{
		instantiate_selected_nodes(m_document_state);
	}

	/// Duplicates selected meshes
	void on_duplicate()
	{
		duplicate_selected_nodes(m_document_state);
	}

	/// Modify selected meshes
	void on_modify_meshes(k3d::iplugin_factory* Modifier)
	{
		return_if_fail(Modifier);

		k3d::inode* new_modifier = 0;

		const k3d::nodes_t selected_nodes = m_document_state.selected_nodes();
		for(k3d::nodes_t::const_iterator node = selected_nodes.begin(); node != selected_nodes.end(); ++node)
		{
			new_modifier = modify_mesh(m_document_state, **node, Modifier);
			assert_warning(new_modifier);
		}

		// Show the new modifier properties if only one was processed
		if(selected_nodes.size() == 1)
			m_document_state.view_node_properties_signal().emit(new_modifier);

		k3d::gl::redraw_all(m_document_state.document(), k3d::gl::irender_engine::ASYNCHRONOUS);
	}

	/// Modify selected transforms
	void on_modifier_transform(k3d::iplugin_factory* Modifier)
	{
		return_if_fail(Modifier);

		k3d::nodes_t selected_nodes = m_document_state.selected_nodes();

		k3d::inode* new_modifier;
		for(k3d::nodes_t::iterator node = selected_nodes.begin(); node != selected_nodes.end(); ++node)
		{
			new_modifier = modify_transformation(m_document_state.document(), **node, Modifier);
			assert_warning(new_modifier);
		}

		// Show the new modifier properties if only one was processed
		if(selected_nodes.size() == 1)
			m_document_state.view_node_properties_signal().emit(new_modifier);

		k3d::gl::redraw_all(m_document_state.document(), k3d::gl::irender_engine::ASYNCHRONOUS);
	}

	void on_hide_selection()
	{
		k3d::record_state_change_set change_set(m_document_state.document(), _("Hide selection"), K3D_CHANGE_SET_CONTEXT);
		m_document_state.hide_selection();
	}

	void on_show_selection()
	{
		k3d::record_state_change_set change_set(m_document_state.document(), _("Show selection"), K3D_CHANGE_SET_CONTEXT);
		m_document_state.show_selection();
	}

	void on_hide_unselected()
	{
		k3d::record_state_change_set change_set(m_document_state.document(), _("Hide unselected"), K3D_CHANGE_SET_CONTEXT);
		m_document_state.hide_unselected();
	}

	void on_show_all()
	{
		k3d::record_state_change_set change_set(m_document_state.document(), _("Show all"), K3D_CHANGE_SET_CONTEXT);
		m_document_state.show_all_nodes();
	}

	// Viewport items
	Gtk::MenuItem* m_viewport1;

	// Mesh functionality
	menu_item::control* m_delete_nodes;
	menu_item::control* m_instantiate_meshes;
	menu_item::control* m_duplicate_meshes;
	Gtk::Menu* m_mesh_modifiers;
	Gtk::Menu* m_transform_modifiers;

	document_state& m_document_state;
	viewport::control* m_viewport;
};

} // namespace detail

Gtk::Menu* create_context_menu(document_state& DocumentState, k3d::icommand_node& Parent)
{
	return new detail::node_context_menu(DocumentState, Parent);
}

} // namespace libk3dngui

