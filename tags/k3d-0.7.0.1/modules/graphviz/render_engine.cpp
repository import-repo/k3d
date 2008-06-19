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
		\author Tim Shead <tshead@k-3d.com>
*/

#include <k3d-i18n-config.h>
#include <k3dsdk/document_plugin_factory.h>
#include <k3dsdk/fstream.h>
#include <k3dsdk/inetwork_render_farm.h>
#include <k3dsdk/inetwork_render_frame.h>
#include <k3dsdk/inetwork_render_job.h>
#include <k3dsdk/inode_collection_property.h>
#include <k3dsdk/inode_visibility.h>
#include <k3dsdk/ipipeline.h>
#include <k3dsdk/irender_frame.h>
#include <k3dsdk/irender_preview.h>
#include <k3dsdk/module.h>
#include <k3dsdk/network_render_farm.h>
#include <k3dsdk/node.h>
#include <k3dsdk/options.h>
#include <k3dsdk/persistent.h>

#include <iomanip>
#include <iterator>

namespace k3d
{

namespace data
{

/////////////////////////////////////////////////////////////////////////////
// node_collection_serialization

/// Serialization policy for data containers that can be serialized as XML
template<typename value_t, class property_policy_t>
class node_collection_serialization :
	public property_policy_t,
	public ipersistent
{
public:
	void save(xml::element& Element, const ipersistent::save_context& Context)
	{
		std::stringstream buffer;

		const inode_collection_property::nodes_t& nodes = property_policy_t::internal_value();
		for(inode_collection_property::nodes_t::const_iterator node = nodes.begin(); node != nodes.end(); ++node)
		{
			if(*node)
				buffer << " " << string_cast(Context.lookup.lookup_id(*node));
			else
				buffer << " 0";
		}

		Element.append(xml::element("property", buffer.str(), xml::attribute("name", property_policy_t::name())));
	}

	void load(xml::element& Element, const ipersistent::load_context& Context)
	{
		inode_collection_property::nodes_t nodes;

		std::stringstream buffer(Element.text);
		std::string node;
		while(buffer >> node)
			nodes.push_back(dynamic_cast<inode*>(Context.lookup.lookup_object(from_string(node, static_cast<ipersistent_lookup::id_type>(0)))));
		nodes.erase(std::remove(nodes.begin(), nodes.end(), static_cast<inode*>(0)), nodes.end());

		property_policy_t::set_value(nodes);
	}

protected:
	template<typename init_t>
	node_collection_serialization(const init_t& Init) :
		property_policy_t(Init)
	{
		Init.persistent_container().enable_serialization(Init.name(), *this);
	}
};

} // namespace data

} // namespace k3d

namespace module
{

namespace graphviz
{

/////////////////////////////////////////////////////////////////////////////
// render_engine

class render_engine :
	public k3d::persistent<k3d::node>,
	public k3d::inode_visibility,
	public k3d::irender_preview,
	public k3d::irender_frame
{
	typedef k3d::persistent<k3d::node> base;

public:
	render_engine(k3d::iplugin_factory& Factory, k3d::idocument& Document) :
		base(Factory, Document),
		m_visible_nodes(init_owner(*this) + init_name("visible_nodes") + init_label(_("Visible Nodes")) + init_description(_("Visible Nodes")) + init_value(std::vector<k3d::inode*>())),
		m_render_engine(init_owner(*this) + init_name("render_engine") + init_label(_("Render engine")) + init_description(_("Render engine name")) + init_value(k3d::string_t("dot")) + init_values(render_engine_values()))
	{
	}

	k3d::iproperty& visible_nodes()
	{
		return m_visible_nodes;
	}

	bool render_preview()
	{
		// Start a new render job ...
		k3d::inetwork_render_job& job = k3d::network_render_farm().create_job("k3d-preview");

		// Add a single render frame to the job ...
		k3d::inetwork_render_frame& frame = job.create_frame("frame");

		// Create an output image path ...
		const k3d::filesystem::path outputimagepath = frame.add_output_file("world.ps");
		return_val_if_fail(!outputimagepath.empty(), false);

		// View the output image when it's done ...
		frame.add_view_operation(outputimagepath);

		// Render it ...
		return_val_if_fail(render(frame, outputimagepath), false);

		// Start the job running ...
		k3d::network_render_farm().start_job(job);

		return true;
	}

	bool render_frame(const k3d::filesystem::path& OutputImage, const bool ViewImage)
	{
		// Sanity checks ...
		return_val_if_fail(!OutputImage.empty(), false);

		// Start a new render job ...
		k3d::inetwork_render_job& job = k3d::network_render_farm().create_job("k3d-render-frame");

		// Add a single render frame to the job ...
		k3d::inetwork_render_frame& frame = job.create_frame("frame");

		// Create an output image path ...
		const k3d::filesystem::path outputimagepath = frame.add_output_file("world.ps");
		return_val_if_fail(!outputimagepath.empty(), false);

		// Copy the output image to its requested destination ...
		frame.add_copy_operation(outputimagepath, OutputImage);

		// View the output image when it's done ...
		if(ViewImage)
			frame.add_view_operation(OutputImage);

		// Render it ...
		return_val_if_fail(render(frame, outputimagepath), false);

		// Start the job running ...
		k3d::network_render_farm().start_job(job);

		return true;
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::document_plugin_factory<render_engine,
			k3d::interface_list<k3d::irender_frame,
			k3d::interface_list<k3d::irender_preview> > > factory(
				k3d::uuid(0xbe72cb50, 0x011f41d8, 0x90449ae0, 0x4c24ace5),
				"GraphVizEngine",
				_("GraphViz Render Engine"),
				"RenderEngines",
				k3d::iplugin_factory::STABLE);

		return factory;
	}

private:
	template<typename T>
	static const k3d::string_t pointer_id(const T RHS)
	{
		// If this fails, it means that a pointer can't fit in a long int on your platform - you need to adjust the return type of this function
		BOOST_STATIC_ASSERT(sizeof(T) <= sizeof(k3d::uint64_t));

		std::stringstream buffer;
		buffer << reinterpret_cast<k3d::uint64_t>(RHS);
		return buffer.str();
	}

	static const k3d::string_t escaped_string(const k3d::string_t& Source)
	{
		k3d::string_t result(Source);

		for(k3d::string_t::size_type i = result.find('\"'); i != k3d::string_t::npos; i = result.find('\"', i+2))
			result.replace(i, 1, "\\\"");

		return result;
	}

	bool render(k3d::inetwork_render_frame& Frame, const k3d::filesystem::path& OutputImagePath)
	{
		// Sanity checks ...
		return_val_if_fail(!OutputImagePath.empty(), false);

		// Start our GraphViz DOT file ...
		const k3d::filesystem::path filepath = Frame.add_input_file("world.dot");
		return_val_if_fail(!filepath.empty(), false);

		// Open the DOT file stream ...
		k3d::filesystem::ofstream stream(filepath);
		return_val_if_fail(stream.good(), false);

		// Setup the frame for GraphViz rendering ...
		Frame.add_render_operation("graphviz", m_render_engine.pipeline_value(), filepath, false);

		stream << "digraph \"" << boost::any_cast<k3d::ustring>(document().title().property_value()).raw() << "\"\n";
		stream << "{\n\n";

		stream << "rankdir=TD\n\n";

		stream << "node [shape=box,style=filled,width=0,height=0]\n\n";


		// Create a mapping of properties-to-nodes as we go ...
		typedef std::map<k3d::iproperty*, k3d::inode*> property_node_map_t;
		property_node_map_t property_node_map;

		// Draw a vertex for every visible node ...
		const k3d::inode_collection_property::nodes_t visible_nodes = boost::any_cast<k3d::inode_collection_property::nodes_t>(m_visible_nodes.property_value());
		for(k3d::inode_collection_property::nodes_t::const_iterator node = visible_nodes.begin(); node != visible_nodes.end(); ++node)
		{
			stream << pointer_id(*node) << " [label=\"" << escaped_string((*node)->name()) << "\"]\n";

			if(k3d::iproperty_collection* const property_collection = dynamic_cast<k3d::iproperty_collection*>(*node))
			{
				const k3d::iproperty_collection::properties_t properties = property_collection->properties();
				for(k3d::iproperty_collection::properties_t::const_iterator property = properties.begin(); property != properties.end(); ++property)
					property_node_map.insert(std::make_pair(*property, *node));
			}
		}

		// Draw an edge for every property dependency between visible nodes ...
		stream << "\n";
		const k3d::ipipeline::dependencies_t dependencies = document().pipeline().dependencies();
		for(k3d::ipipeline::dependencies_t::const_iterator dependency = dependencies.begin(); dependency != dependencies.end(); ++dependency)
		{
			if(property_node_map.count(dependency->first) && property_node_map.count(dependency->second))
			{
				stream << pointer_id(property_node_map[dependency->second]) << " -> " << pointer_id(property_node_map[dependency->first]);
				stream << " [headlabel=\"" << escaped_string(dependency->first->property_name()) << "\" taillabel=\"" << escaped_string(dependency->second->property_name()) << "\"]\n";
			}
		}

		// Draw an edge for every property whose value is another property ...
		stream << "\n";
		for(property_node_map_t::const_iterator property = property_node_map.begin(); property != property_node_map.end(); ++property)
		{
			if(typeid(k3d::inode*) == property->first->property_type())
			{
				if(k3d::inode* const referenced_node = boost::any_cast<k3d::inode*>(property->first->property_value()))
				{
					if(std::count(visible_nodes.begin(), visible_nodes.end(), referenced_node))
					{
						stream << pointer_id(referenced_node) << " -> " << pointer_id(property->second);
						stream << " [style=dotted,label=\"" << escaped_string(property->first->property_name()) << "\"]\n";
					}
				}
			}
		}

		stream << "\n}\n";

		return true;
	}

	k3d_data(k3d::inode_collection_property::nodes_t, immutable_name, change_signal, with_undo, local_storage, no_constraint, writable_property, node_collection_serialization) m_visible_nodes;
	k3d_data(k3d::string_t, immutable_name, change_signal, with_undo, local_storage, no_constraint, list_property, with_serialization) m_render_engine;

	const k3d::ilist_property<k3d::string_t>::values_t& render_engine_values()
	{
		static k3d::ilist_property<k3d::string_t>::values_t values;
		if(values.empty())
		{
			const k3d::options::render_engines_t engines = k3d::options::render_engines();
			for(k3d::options::render_engines_t::const_iterator engine = engines.begin(); engine != engines.end(); ++engine)
			{
				if(engine->type == "graphviz")
					values.push_back(engine->name);
			}
		}
		return values;
	}
};

} // namespace graphviz

} // namespace module

K3D_MODULE_START(Registry)
	Registry.register_factory(module::graphviz::render_engine::get_factory());
K3D_MODULE_END
