cmake_minimum_required(VERSION 3.13...3.14)

include_guard(DIRECTORY)

# Don't allow user to build in the source directory.
get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)
if(srcdir STREQUAL bindir)
    message(FATAL_ERROR "\

Don't run CMake in the source directory - please create a new build directory, and invoke cmake from there. \
To clean up after this build attempt, please run the following in your source directory:\
\n\
cmake -E remove -f CMakeCache.txt && cmake -E remove_directory CMakeFiles
    ")
endif()


# Define the list of build types. The first one in the list is the default build type.
set(build_types
	Release
	RelWithDebInfo
	Debug
)

# If we're targeting a multi-config generator like Visual Studio or XCode, all we need to do
# is pass it our list of build types, and we're done.
if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES "${build_types}" CACHE STRING "Build types available to use in IDE")
	return()
endif()

# If we reach here, we're targeting a normal single-config generator.

# If the user requested a particular build type when they invoked CMake, and it's on the list, use that.
# Otherwise, use the default build type (first in the list).
#
# Note: the matching is case-insensitive - i.e., if the user passes in RELEASE, it will match "Release"
#       from the list, and the value of CMAKE_BUILD_TYPE after this file is run will be "Release".
list(GET build_types 0 selected_type)
if(CMAKE_BUILD_TYPE)
	string(TOLOWER "${CMAKE_BUILD_TYPE}" _val_lower)
	list(TRANSFORM build_types TOLOWER OUTPUT_VARIABLE _list_lower)
	list(FIND _list_lower "${_val_lower}" _idx)
	if(_idx GREATER -1)
		list(GET build_types ${_idx} selected_type)
	else()
		message(WARNING "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} is invalid, using default (${selected_type})")
	endif()
endif()

# Set CMAKE_BUILD_TYPE to selected type, and add help text and drop-down list entries for cmake-gui.
string(REPLACE ";" ", " build_types_help "${build_types}")
set(CMAKE_BUILD_TYPE "${selected_type}" CACHE STRING ${build_types_help} FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${build_types}")


# Put build artifacts in standard subdirectories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
