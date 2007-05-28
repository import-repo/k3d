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

/** \file
		\author Timothy M. Shead (tshead@k-3d.com)
		\author Romain Behar (romainbehar@yahoo.com)
*/

#include <k3dsdk/document_plugin_factory.h>
#include <k3dsdk/axis.h>
#include <k3dsdk/bitmap_modifier.h>
#include <k3dsdk/classes.h>
#include <k3dsdk/file_filter.h>
#include <k3dsdk/i18n.h>
#include <k3dsdk/ibitmap_write_format.h>
#include <k3dsdk/irender_engine_ri.h>
#include <k3dsdk/irender_frame.h>
#include <k3dsdk/itexture_ri.h>
#include <k3dsdk/measurement.h>
#include <k3dsdk/node.h>
#include <k3dsdk/persistent.h>
#include <k3dsdk/types_ri.h>

#include <iterator>

#ifdef K3D_PLATFORM_WIN32
	#define COPY_COMMAND "copy"
#else
	#define COPY_COMMAND "cp"
#endif

namespace libk3drenderman
{

/////////////////////////////////////////////////////////////////////////////
// lat_long_environment_map

class lat_long_environment_map :
	public k3d::bitmap_modifier<k3d::persistent<k3d::node> >,
	public k3d::ri::itexture
{
	typedef k3d::bitmap_modifier<k3d::persistent<k3d::node> > base;

public:
	lat_long_environment_map(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document),
		m_filter(init_owner(*this) + init_name("filter") + init_label(_("filter")) + init_description(_("Filter")) + init_value(k3d::ri::RI_GAUSSIAN()) + init_values(filter_values())),
		m_swidth(init_owner(*this) + init_name("swidth") + init_label(_("swidth")) + init_description(_("Filter S Width")) + init_value(2.0) + init_constraint(constraint::minimum(std::numeric_limits<double>::epsilon())) + init_step_increment(1.0) + init_units(typeid(k3d::measurement::scalar))),
		m_twidth(init_owner(*this) + init_name("twidth") + init_label(_("twidth")) + init_description(_("Filter T Width")) + init_value(2.0) + init_constraint(constraint::minimum(std::numeric_limits<double>::epsilon())) + init_step_increment(1.0) + init_units(typeid(k3d::measurement::scalar)))
	{
	}
	
	void setup_renderman_texture(k3d::irender_frame& Frame, k3d::ri::irender_engine& Engine, k3d::ri::ishader_collection& Shaders)
	{
		m_ri_image_path = k3d::filesystem::path();
		m_ri_texture_path = k3d::filesystem::path();

		// If we don't have an input bitmap, we're done ...
		const k3d::bitmap* const input = m_input_bitmap.value();
		if(!input)
			return;

		m_ri_image_path = Frame.add_input_file("texture");
		return_if_fail(!m_ri_image_path.empty());
		
		m_ri_texture_path = Frame.add_input_file("texture");
		return_if_fail(!m_ri_texture_path.empty());

/*
		// If "render from source" is enabled, just copy the source file to the frame directory ...
		if(m_render_from_source.value())
		{
			// Copy the file ...
			const std::string copycommand = COPY_COMMAND " " + m_image_path.value().native_string() + " " + m_ri_image_path.native_string();
			k3d::system::run_process(copycommand);

		}
		// Otherwise, save the in-memory image data as a TIFF file ...
		else
*/
		{
			k3d::ibitmap_write_format* const filter = k3d::file_filter<k3d::ibitmap_write_format>(k3d::classes::TIFFWriter());
			return_if_fail(filter);
			return_if_fail(filter->write_file(m_ri_image_path, *input));
		}

		Engine.RiMakeLatLongEnvironmentV(m_ri_image_path.native_filesystem_string(), m_ri_texture_path.native_filesystem_string(), m_filter.value(), m_swidth.value(), m_twidth.value());
	}

	const k3d::filesystem::path renderman_texture_path(const k3d::ri::render_state& State)
	{
		return m_ri_texture_path;
	}

	void on_create_bitmap(const k3d::bitmap& Input, k3d::bitmap& Output)
	{
		Output = Input;
	}

	void on_update_bitmap(const k3d::bitmap& Input, k3d::bitmap& Output)
	{
	}
	
	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<lat_long_environment_map,
			k3d::interface_list<k3d::ibitmap_source,
			k3d::interface_list<k3d::ibitmap_sink,
			k3d::interface_list<k3d::ri::itexture> > > > factory(
				k3d::uuid(0x04354dba, 0x91a7411c, 0x88b73725, 0x5808a415),
				"RenderManLatLongEnvironmentMap",
				_("Converts a bitmap into a RenderMan LatLong Environment Map"),
				"RenderMan",
				k3d::iplugin_factory::STABLE);

		return factory;
	}

private:
	k3d_data(std::string, immutable_name, change_signal, with_undo, local_storage, no_constraint, list_property, with_serialization) m_filter;
	k3d_data(double, immutable_name, change_signal, with_undo, local_storage, with_constraint, measurement_property, with_serialization) m_swidth;
	k3d_data(double, immutable_name, change_signal, with_undo, local_storage, with_constraint, measurement_property, with_serialization) m_twidth;

	const k3d::ilist_property<std::string>::values_t& filter_values()
	{
		static k3d::ilist_property<std::string>::values_t values;
		if(values.empty())
		{
			values.push_back(k3d::ri::RI_GAUSSIAN());
			values.push_back(k3d::ri::RI_BOX());
			values.push_back(k3d::ri::RI_TRIANGLE());
			values.push_back(k3d::ri::RI_CATMULL_ROM());
			values.push_back(k3d::ri::RI_SINC());
		}
		return values;
	}

	/// Stores the absolute path to the saved TIFF version of this lat_long_environment_map (for use during RenderMan rendering)
	k3d::filesystem::path m_ri_image_path;
	/// Stores the absolute path to the version of this lat_long_environment_map converted by the renderer (via RiMakeLatLongEnvironment())
	k3d::filesystem::path m_ri_texture_path;
};

/////////////////////////////////////////////////////////////////////////////
// lat_long_environment_map_factory

k3d::iplugin_factory& lat_long_environment_map_factory()
{
	return lat_long_environment_map::get_factory();
}

} // namespace libk3drenderman

