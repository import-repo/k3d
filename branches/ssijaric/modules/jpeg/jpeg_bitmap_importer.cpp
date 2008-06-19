// K-3D
// Copyright (c) 1995-2007, Timothy M. Shead
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
// License along with this program; if not, read to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/** \file
	\author Timothy M. Shead
*/

#include <k3dsdk/application_plugin_factory.h>
#include <k3dsdk/i18n.h>
#include <k3dsdk/ibitmap_read_format.h>
#include <k3dsdk/ideletable.h>
#include <k3dsdk/ifile_format.h>
#include <k3dsdk/path.h>
#include <k3dsdk/string_modifiers.h>

#include <boost/gil/extension/io/jpeg_io.hpp>

namespace libk3djpeg
{

/////////////////////////////////////////////////////////////////////////////
// jpeg_bitmap_importer

class jpeg_bitmap_importer :
	public k3d::ifile_format,
	public k3d::ibitmap_read_format,
	public k3d::ideletable
{
public:
	jpeg_bitmap_importer()
	{
	}

	unsigned long priority()
	{
		return 128;
	}

	bool query_can_handle(const k3d::filesystem::path& Path)
	{
		return k3d::filesystem::extension(Path).lowercase().raw() == ".jpeg" || k3d::filesystem::extension(Path).lowercase().raw() == ".jpg";
	}

	bool read_file(const k3d::filesystem::path& Path, k3d::bitmap& Bitmap)
	{
		try
		{
			k3d::log() << info << "Reading " << Path.native_console_string() << " using " << get_factory().name() << std::endl;
			boost::gil::jpeg_read_and_convert_image(Path.native_filesystem_string(), Bitmap);
			return true;
		}
		catch(std::exception& e)
		{
			k3d::log() << error << k3d_file_reference << ": caught exception: " << e.what() << std::endl;
			return false;
		}
		catch(...)
		{
			k3d::log() << error << k3d_file_reference << ": caught unknown exception" << std::endl;
			return false;
		}
	}

	static k3d::iplugin_factory& get_factory()
	{
		static k3d::application_plugin_factory<jpeg_bitmap_importer,
			k3d::interface_list<k3d::ibitmap_read_format> > factory(
				k3d::uuid(0xfb924031, 0x25c242af, 0xa2e1398e, 0x35000e3c),
				"JPEGBitmapImporter",
				_("JPEG (*.jpeg)"),
				"Bitmap BitmapImporter");

		return factory;
	}
};

/////////////////////////////////////////////////////////////////////////////
// jpeg_bitmap_importer_factory

k3d::iplugin_factory& jpeg_bitmap_importer_factory()
{
	return jpeg_bitmap_importer::get_factory();
}

} // namespace libk3djpeg
