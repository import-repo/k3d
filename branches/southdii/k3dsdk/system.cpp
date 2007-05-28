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
		\brief Implements system related functions
		\author Tim Shead (tshead@k-3d.com)
*/

#include <errno.h>
#include "path.h"
#include "result.h"
#include "string_modifiers.h"
#include "system.h"

#include <glibmm/spawn.h>

#include <boost/tokenizer.hpp>

#include <cmath>

#include <sys/stat.h>

// Define some platform-specific odds-and-ends
#ifdef K3D_PLATFORM_WIN32

	#define SEARCHPATH_SEPARATOR_STRING ";"
	#define DEFAULT_TEMP_DIRECTORY "c:\\"
	#define DEFAULT_HOME_DIRECTORY "c:\\"
	#define popen _popen
	#include <io.h>
	#include <windows.h>

#else // K3D_PLATFORM_WIN32

	#define SEARCHPATH_SEPARATOR_STRING ":"
	#define DEFAULT_TEMP_DIRECTORY "/tmp"
	#define DEFAULT_HOME_DIRECTORY "/"

	#include <time.h>

#endif // !K3D_PLATFORM_WIN32

namespace k3d
{

namespace system
{

const std::string getenv(const std::string& Variable)
{
	if(const char* variable = ::getenv(Variable.c_str()))
		return std::string(variable);

	return std::string();
}

void setenv(const std::string& Name, const std::string& Value)
{
#ifdef K3D_PLATFORM_WIN32

	// Use putenv on Win32 because it's the only thing available, and it copies its inputs
	::putenv((Name + "=" + Value).c_str());

#else // K3D_PLATFORM_WIN32

	// Use setenv where possible because it copies its inputs
	::setenv(Name.c_str(), Value.c_str(), true);

#endif // !K3D_PLATFORM_WIN32
}

void setenv(const std::string& Variable)
{
#ifdef K3D_PLATFORM_WIN32

	// Use putenv on Win32 because it's the only thing available, and it copies its inputs
	::putenv(Variable.c_str());

#else // K3D_PLATFORM_WIN32

	// On unix, we have to make a copy of the string to pass to putenv()
	::putenv(::strdup(Variable.c_str()));

#endif // !K3D_PLATFORM_WIN32
}

const filesystem::path get_home_directory()
{
	static filesystem::path home;

#ifdef K3D_PLATFORM_WIN32

	if(home.empty())
		home = filesystem::native_path(ustring::from_utf8(getenv("APPDATA")));

	if(home.empty())
		home = filesystem::native_path(ustring::from_utf8(getenv("USERPROFILE")));

#endif // K3D_PLATFORM_WIN32

	if(home.empty())
		home = filesystem::native_path(ustring::from_utf8(getenv("HOME")));

	if(home.empty())
	{
		home = filesystem::native_path(ustring::from_utf8(DEFAULT_HOME_DIRECTORY));
		log() << warning << "Using default home directory [" << home.native_console_string() << "]" << std::endl;
	}

	return home;
}

const filesystem::path get_temp_directory()
{
	static filesystem::path temp_directory;

	if(temp_directory.empty())
	{
		if(!getenv("TMPDIR").empty())
		{
			temp_directory = filesystem::native_path(ustring::from_utf8(getenv("TMPDIR")));
		}
		else if(!getenv("TMP").empty())
		{
			temp_directory = filesystem::native_path(ustring::from_utf8(getenv("TMP")));
		}
		else if(!getenv("TEMP").empty())
		{
			temp_directory = filesystem::native_path(ustring::from_utf8(getenv("TEMP")));
		}
		else
		{
			temp_directory = filesystem::native_path(ustring::from_utf8(DEFAULT_TEMP_DIRECTORY));
		}
	}

	return temp_directory;
}

const filesystem::path generate_temp_file()
{
#ifdef K3D_PLATFORM_WIN32

	std::string buffer(MAX_PATH, '\0');
	return_val_if_fail(GetTempFileName(get_temp_directory().native_filesystem_string().c_str(), "k3d", 0, const_cast<char*>(buffer.c_str())), filesystem::path());
	buffer.resize(strlen(buffer.c_str()));

	return filesystem::native_path(ustring::from_utf8(buffer));

#else // K3D_PLATFORM_WIN32

	std::string buffer = (get_temp_directory() / filesystem::generic_path("k3d-XXXXXX")).native_filesystem_string();
	int fd = mkstemp(const_cast<char*>(buffer.c_str()));
	return_val_if_fail(fd, filesystem::path());
	close(fd);

	return filesystem::native_path(ustring::from_utf8(buffer));

#endif // !K3D_PLATFORM_WIN32
}

bool file_modification_time(const filesystem::path& File, time_t& ModificationTime)
{
	struct stat statistics;
	if(-1 == stat(File.native_filesystem_string().c_str(), &statistics))
		return false;

	ModificationTime = statistics.st_mtime;

	return true;
}

bool spawn_async(const std::string& CommandLine)
{
	return_val_if_fail(!CommandLine.empty(), false);

	log() << info << "spawn_async: " << CommandLine << std::endl;
	log() << info << "PATH=" << getenv("PATH") << std::endl;

	try
	{
		Glib::spawn_command_line_async(CommandLine);
		return true;
	}
	catch(Glib::Exception& e)
	{
		k3d::log() << error << e.what() << std::endl;
		return false;
	}
}

bool spawn_sync(const std::string& CommandLine)
{
	return_val_if_fail(!CommandLine.empty(), false);

	log() << info << "spawn_sync: " << CommandLine << std::endl;
	log() << info << "PATH=" << getenv("PATH") << std::endl;

	try
	{
		Glib::spawn_command_line_sync(CommandLine);
		return true;
	}
	catch(Glib::Exception& e)
	{
		k3d::log() << error << e.what() << std::endl;
		return false;
	}
}

const paths_t decompose_path_list(const std::string Input)
{
	paths_t results;

	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> separator(SEARCHPATH_SEPARATOR_STRING);
	tokenizer input(Input, separator);

	for(tokenizer::iterator path = input.begin(); path != input.end(); ++path)
	{
		const std::string path_string = k3d::trim(*path);
		if(!path_string.empty())
			results.push_back(filesystem::native_path(ustring::from_utf8(path_string)));
	}

	return results;
}

#ifdef K3D_PLATFORM_WIN32

const std::string executable_name(const std::string& Executable)
{
	return Executable + ".exe";
}

#else // K3D_PLATFORM_WIN32

const std::string executable_name(const std::string& Executable)
{
	return Executable;
}

#endif // !K3D_PLATFORM_WIN32

#ifdef K3D_PLATFORM_WIN32

void sleep(const double Seconds)
{
	if(Seconds <= 0.0)
		return;

	Sleep(static_cast<DWORD>(Seconds * 1000));
}

#else // K3D_PLATFORM_WIN32

void sleep(const double Seconds)
{
	if(Seconds <= 0.0)
		return;

	timespec t;
	t.tv_sec = static_cast<long>(std::floor(Seconds));
	t.tv_nsec = static_cast<long>((Seconds - std::floor(Seconds)) * 1000000000);

	nanosleep(&t, 0);
}

#endif // !K3D_PLATFORM_WIN32

} // namespace system

} // namespace k3d

