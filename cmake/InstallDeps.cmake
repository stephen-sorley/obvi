# InstallDeps.cmake
#
# Helper functions for installing library dependencies (and copying to the build dir on Windows).
#
# Accepts a list of paths and/or targets. Targets that are static libs or executables are silently
# ignored. Alias targets and import targets are handled naturally, and any interface link
# dependencies of interface targets are followed recursively and added as well.
#
# The "lib dest" and "runtime dest" parameters given to install_deps are allowed to contain
# generator expressions.
#
# On Windows, if given a path to an import library (.lib), this function will attempt to guess the
# path to the DLL file. If at all possible, please define import libraries as SHARED explicitly
# and provide dll path in LOCATION and import lib path in IMPORTED_IMPLIB, so that this guessing
# isn't necessary.
#
# On *nix, all symlinks are resolved before installing the file, so the name of the installed file
# will be the name of the physical file at the end of the symlink chain. If the file is an ELF
# shared library and SONAME is defined, a symlink will also be installed that points to the physical
# file and has the name stored in SONAME.
#
# For example, let's say "libz.so" is passed in, it's a symlink to "libz.so.1.2.11", and the SONAME
# stored in the ELF header is "libz.so.1". This function will install a file named "libz.so.1.2.11",
# and a symlink that points to that file named "libz.so.1".
#
# Functions:
# install_deps([lib dest] [runtime dest] [... lib files or import targets ...])
#     Install the given list of lib dependencies to CMAKE_INSTALL_PREFIX, alongside the project.
#     Can be passed either library paths or imported targets. On Windows, if given an import lib
#     path or an imported library of type UNKNOWN, it will try to guess the path to the DLL. If it
#     can't find the the DLL, it will throw an error.
#
#     By default, install_deps will also copy any DLL's to the build's runtime output directory
#     (CMAKE_RUNTIME_OUTPUT_DIR, usually set to [build dir]/bin). This behavior can be turned off
#     by setting INSTALL_DEPS_COPY_DLL to FALSE (see below).
#
# install_deps_system([lib dest] [runtime dest])
#     Installs any needed system dependencies (like Visual C++ runtime libraries). If OpenMP was
#     found by the project, any required OpenMP libraries are included.
#
#     Just like with install_deps, this will also copy any DLL's to the build directory when on
#     Windows. This behavior can be disabled by setting INSTALL_DEPS_COPY_DLL to FALSE.
#
# Extra variables that modify how the functions work:
# INSTALL_DEPS_COPY_DLL
#   If TRUE (the default), any DLL's installed with install_deps will also be copied to the build
#   directory (specifically, to CMAKE_RUNTIME_OUTPUT_DIR). Set to FALSE to disable DLL copies.
#
# INSTALL_DEPS_AUTO_MODE [NONE|{LIMITED}|ALL]
#   Determines how install_deps() will handle any additional dependencies listed in the
#   IMPORTED_LINK_DEPENDENT_LIBRARIES or INTERFACE_LINK_LIBRARIES properties of the import libs
#   that were explicitly passed in. Note that these properties are recursively expanded out to
#   their fullest extent (so deps of deps of deps can be pulled in).
#      NONE: only the libs explicitly passed into install_deps() will be installed.
#      LIMITED: any additional deps that are import libs with the same namespace as one of the
#               explicitly passed-in libraries are installed.
#      ALL: all additional deps that are import libs are installed, regardless of namespace.
#
#   For example, let's say you called install_deps(Qt5::Widgets), and Qt5::Widgets has Qt5::Core
#   and OpenSSL::OpenSSL listed in its INTERFACE_LINK_LIBRARIES property. Furthermore, let's say
#   OpenSSL::OpenSSL has ZLIB::ZLIB listed in its INTERFACE_LINK_LIBRARIES property.  So our
#   dependency graph looks like this:
#       Qt5::Widgets -> Qt5::Core
#                    -> OpenSSL::OpenSSL -> ZLIB::ZLIB
#   If mode is NONE, only Qt5::Widgets is installed.
#   If mode is LIMITED, Qt5::Widgets and Qt5::Core are installed.
#   If mode is ALL, Qt5::Widgets, Qt5::Core, OpenSSL::OpenSSL, and ZLIB::ZLIB are installed.
#
#   "LIMITED" is the default behavior.
#
# INSTALL_DEPS_EXCLUDE_LIST
#   Any libs in the exclusion list will be skipped if found during automatic dependency detection.
#
# # # # # # # # # # # #
# This file was originally adapted from the mstdlib project (also MIT licensed), found here:
#   https://github.com/Monetra/mstdlib/blob/master/CMakeModules/InstallDepLibs.cmake
#   (Monetra Technologies, LLC)
#
# # # # # # # # # # # #
# The MIT License (MIT)
#
# Copyright (c) 2019 Stephen Sorley
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# # # # # # # # # # # #
#
cmake_minimum_required(VERSION 3.14)

include_guard(DIRECTORY)


# These libraries are always added to the exclusion list (won't ever be picked up by automatic
# sub-dependency detection).
list(APPEND INSTALL_DEPS_EXCLUDE_LIST
    Qt5::Gui_GL
    Qt5::Gui_EGL
    Qt5::Gui_GLESv2
)



# READELF
# used to extract SONAME from shared libs on ELF platforms.
find_program(READELF
    NAMES readelf
          elfdump  #on Solaris, "elfdump -d" gives very similar output to "readelf -d" on Linux.
    DOC "readelf (unix/ELF only)"
)
mark_as_advanced(FORCE READELF)


# DUMPBIN
# used to get DLL name from import lib on Windows.
# (optional - will try other heuristics to guess DLL when dumpbin not available, plus the whole
#  guessing routine is never even run if the DLL location is stored in the imported library target)
set(dumpbin_hints)
foreach(lang CXX C)
    if(CMAKE_${lang}_COMPILER)
        get_filename_component(dir "${CMAKE_${lang}_COMPILER}" DIRECTORY)
        list(APPEND dumpbin_hints "${dir}")
    endif()
endforeach()
find_program(DUMPBIN
    NAMES dumpbin
    HINTS ${dumpbin_hints}
    DOC "dumpbin (Windows only)"
)
mark_as_advanced(FORCE DUMPBIN)


# Helper function for _install_deps_internal: try to find .dll using path of an import lib (VS or MinGW).
function(_install_deps_get_dll_from_implib out_dll path)
    # Get directory containing import lib, and try to guess root dir of install.
    get_filename_component(imp_dir "${path}" DIRECTORY)
    string(REGEX REPLACE "/[/0-9x_-]*lib[/0-9x_-]*(/.*|$)" "" root_dir "${imp_dir}")

    # Get library name by removing .lib or .dll.a from extension.
    get_filename_component(imp_file "${path}" NAME)
    string(REGEX REPLACE "\.lib$" "" libname "${imp_file}")
    string(REGEX REPLACE "\.dll\.a$" "" libname "${libname}")

    # If dumpbin is available, run it on the import lib to determine the DLL name.
    set(dumpbin_name)
    if(DUMPBIN)
        execute_process(COMMAND ${DUMPBIN} /ARCHIVEMEMBERS ${path}
            OUTPUT_VARIABLE dumpbin_out
            RESULT_VARIABLE res
            ERROR_QUIET
            ENCODING        auto
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(res EQUAL 0)
            string(REGEX MATCH "[: \t]+([^:\n]+\.dll)" dumpbin_out "${dumpbin_out}")
            if(CMAKE_MATCH_1)
                set(dumpbin_name ${CMAKE_MATCH_1})
                string(REGEX REPLACE "[ \t]*/[0-9a-fA-F]*[ \t]*" "" dumpbin_name "${dumpbin_name}")
            endif()
        endif()
    endif()

    # Get alternate library names by removing lib prefix, and/or d, MT, MDd, etc.
    # (These are common suffixes that indicate which visual studio build flags were used).
    string(REGEX REPLACE "^lib" "" nolibname "${libname}")

    set(alt_names)
    string(REGEX REPLACE "M[dDtT]+$" "" alt_name "${libname}")
    list(APPEND alt_names ${alt_name})
    string(REGEX REPLACE "M[dDtT]+$" "" alt_name "${nolibname}")
    list(APPEND alt_names ${alt_name})
    string(REGEX REPLACE "[dD]$" "" alt_name "${libname}")
    list(APPEND alt_names ${alt_name})
    string(REGEX REPLACE "[dD]$" "" alt_name "${nolibname}")
    list(APPEND alt_names ${alt_name})

    string(TOLOWER "${libname}" libname_lower)
    string(TOUPPER "${libname}" libname_upper)

    string(MAKE_C_IDENTIFIER "${libname_upper}" clibname)

    # Make list of possible subdirectory names that might contain DLL's.
    set(suffixes)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        list(APPEND suffixes
            64 bin64 64/bin bin/64 lib64 64/lib lib/64
            x86_64 x86_64/bin bin/x86_64 x86_64/lib lib/x86_64
            x64 binx64 x64/bin bin/x64 libx64 x64/lib lib/x64
        )
    else()
        list(APPEND suffixes
            32 bin32 32/bin bin/32 lib32 32/lib lib/32
            x86 x86/bin bin/x86 x86/lib lib/x86
            Win32 binWin32 Win32/bin bin/Win32 libWin32 Win32/lib lib/Win32
        )
    endif()
    list(APPEND suffixes bin lib)

    # Ask CMake to search for the DLL.
    set(CMAKE_FIND_LIBRARY_SUFFIXES .dll)
    find_library(${clibname}_DLL
        NAMES           ${dumpbin_name} ${libname} ${libname_lower} ${libname_upper} ${nolibname} ${alt_names}
        HINTS           "${imp_dir}"
                        "${root_dir}"
        NO_DEFAULT_PATH
        PATH_SUFFIXES   ${suffixes}
    )
    mark_as_advanced(FORCE ${clibname}_DLL)

    # If found, set result in parent scope. Otherwise, send a fatal error message with instructions
    # on how to manually specify the DLL location.
    if(${clibname}_DLL)
        set(${out_dll} "${${clibname}_DLL}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR
        "install_dep_libs() couldn't find DLL for given import lib \"${path}\" (set manually with -D${clibname}_DLL=...)"
        )
    endif()
endfunction()


# Helper function: remove libtag from given lib or path, return in separate variable.
function(_install_deps_take_libtag pathvar tagvar)
    set(libtag)
    if(${pathvar} MATCHES "^(@[^@]+@)(.+)")
        set(libtag ${CMAKE_MATCH_1})
        set(${pathvar} ${CMAKE_MATCH_2} PARENT_SCOPE) # Return pathvar without libtag.
    endif()
    if(tagvar)
        set(${tagvar} ${libtag} PARENT_SCOPE) # Return libtag by itself.
    endif()
endfunction()


# Helper function: extract paths from the given import lib, add to given variable. Handle any libtag paths, too.
function(_install_deps_append_paths_from_imported out_paths_name lib libtag is_multiconfig)
    # For shared/module/unknown imported libs, get the imported location (should be DLL, on Windows).
    # Add to list of paths.
    get_target_property(lib_path_debug ${lib} LOCATION_DEBUG)
    get_target_property(lib_path_any ${lib} LOCATION)
    if(WIN32)
        # On Windows, if there's no known DLL, use the import lib as the path instead.
        if(NOT lib_path_debug)
            get_target_property(lib_path_debug ${lib} IMPORTED_IMPLIB_DEBUG)
        endif()
        if(NOT lib_path_any)
            get_target_property(lib_path_any ${lib} IMPORTED_IMPLIB)
        endif()
    endif()

    # If the debug path points to the exact same library as the non-debug one, don't consider it separately.
    if(lib_path_any AND lib_path_debug AND (lib_path_debug STREQUAL lib_path_any))
        set(lib_path_debug)
    endif()

    set(types any)
    if(lib_path_debug)
        if(is_multiconfig)
            if(libtag STREQUAL "@DEBUG@")
                set(types debug)
            elseif(libtag)
                set(types any)
            else()
                set(types debug any)
            endif()
        elseif(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
            set(types debug)
        endif()
    endif()

    foreach(type ${types})
        set(lib_path "${lib_path_${type}}")
        if(NOT lib_path)
            continue()
        endif()
        if(NOT EXISTS "${lib_path}")
            message(FATAL_ERROR "Target ${lib} given to install_deps() contained bad path ${lib_path}")
        endif()

        if(is_multiconfig)
            if(libtag)
                string(PREPEND lib_path ${libtag})
            elseif(type STREQUAL debug)
                string(PREPEND lib_path @DEBUG@)
            elseif(lib_path_debug)
                string(PREPEND lib_path @NOTDEBUG@)
            endif()
        endif()
        list(APPEND ${out_paths_name} "${lib_path}")
    endforeach()

    set(${out_paths_name} "${${out_paths_name}}" PARENT_SCOPE)
endfunction()


# Helper function for _install_deps_internal: convert given list of libs into file paths.
function(_install_deps_get_paths_from_libs lib_dest runtime_dest component out_paths_name out_libs_name prefixes_name out_imports_name)
    # Check to see if we're building with a multi-config generator (like Visual Studio or XCode) or not.
    get_property(is_multiconfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

    set(automode LIMITED)
    if(INSTALL_DEPS_AUTO_MODE)
        string(TOUPPER "${INSTALL_DEPS_AUTO_MODE}" automode)
        if(NOT (INSTALL_DEPS_AUTO_MODE STREQUAL "NONE" OR INSTALL_DEPS_AUTO_MODE STREQUAL "ALL"))
            set(automode LIMITED)
        endif()
    endif()

    set(out_libs)
    foreach(lib IN LISTS ${out_libs_name})
        # Skip empty list elements, as well as "optimized" and "debug" keywords that might be in a <NAME>_LIBRARIES variable.
        if((NOT lib) OR (lib STREQUAL "optimized") OR (lib STREQUAL "debug"))
            continue()
        endif()

        # If this library was marked as config-specific by a previous run, split off the config tag for later use.
        _install_deps_take_libtag(lib libtag)
        if(NOT is_multiconfig)
            set(libtag)
        endif()

        if(TARGET "${lib}")
            # If this is an alias target, get the proper name of the target, then add the result back
            # onto the list of libs to process on the next invocation of this function.
            #
            # Make sure we keep the libtag the same, if the original target had one.
            get_target_property(alias ${lib} ALIASED_TARGET)
            if(alias)
                list(APPEND out_libs "${libtag}${alias}")
                continue()
            endif()

            get_target_property(is_imported ${lib} IMPORTED)

            # If this is an imported library, and it has dependencies listed using properties, add
            # them to the list of libs too. Will be processed on next invocation of this function.
            #
            # NOTE: this only adds deps that are import libs, filters out any paths or link flags.
            if(is_imported AND NOT automode STREQUAL "NONE")
                set(modes GENERAL)
                if(is_multiconfig)
                    if(libtag STREQUAL "@DEBUG@")
                        list(APPEND modes DEBUG)
                    elseif(libtag STREQUAL "@NOTDEBUG@")
                        list(APPEND modes NOTDEBUG)
                    else()
                        list(APPEND modes DEBUG NOTDEBUG)
                    endif()
                elseif(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
                    list(APPEND modes DEBUG)
                else()
                    list(APPEND modes NOTDEBUG)
                endif()
                foreach(mode ${modes})
                    if(mode STREQUAL DEBUG)
                        set(subtag "@DEBUG@")
                        set(props
                            IMPORTED_LINK_DEPENDENT_LIBRARIES_DEBUG
                        )
                    elseif(mode STREQUAL NOTDEBUG)
                        set(subtag "@NOTDEBUG@")
                        set(props
                            IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE
                        )
                    else() # GENERAL
                        set(subtag "${libtag}")
                        set(props
                            INTERFACE_LINK_LIBRARIES
                            IMPORTED_LINK_DEPENDENT_LIBRARIES
                        )
                    endif()
                    # Only add tags if this is a multiconfig generator.
                    if(NOT is_multiconfig)
                        set(subtag)
                    endif()
                    foreach(prop ${props})
                        get_target_property(dep_libs ${lib} ${prop})
                        if(dep_libs)
                            foreach(lib ${dep_libs})
                                if(lib MATCHES "([^ \t:]+)::[^ \t]+" AND TARGET ${lib})
                                    if(lib IN_LIST INSTALL_DEPS_EXCLUDE_LIST)
                                        continue()
                                    endif()
                                    if(automode STREQUAL "ALL" OR "${CMAKE_MATCH_1}" IN_LIST ${prefixes_name})
                                        list(APPEND out_libs "${subtag}${lib}")
                                    endif()
                                endif()
                            endforeach() #loop over dep_libs
                        endif()
                    endforeach() #loop over props
                endforeach() #loop over modes
            endif()

            # If this target isn't a shared, module, interface, or unknown imported library target, skip it silently
            # without doing anything.
            get_target_property(type ${lib} TYPE)
            if(NOT type STREQUAL "SHARED_LIBRARY" AND
               NOT type STREQUAL "MODULE_LIBRARY" AND
               NOT type STREQUAL "INTERFACE_LIBRARY" AND
               NOT type STREQUAL "UNKNOWN_LIBRARY")  #UNKNOWN is a special type that only applies to import libraries.
                continue()
            endif()

            # If this target isn't imported, install directly if shared or module, then skip regardless of type.
            if(NOT is_imported)
                if(type STREQUAL "SHARED_LIBRARY" OR type STREQUAL "MODULE_LIBRARY")
                    install(TARGETS ${lib}
                        LIBRARY DESTINATION "${lib_dest}"
                        RUNTIME DESTINATION "${runtime_dest}"
                        ${component}
                    )
                endif()
                continue()
            endif()

            # Add this imported library to our list of import targets.
            list(APPEND ${out_imports_name} "${libtag}${lib}")

            # If this was an imported interface library, nothing left to do after we read from the properties.
            if(type STREQUAL "INTERFACE_LIBRARY")
                continue()
            endif()

            # For shared/module/unknown imported libs, get the imported location (should be DLL, on Windows).
            # Add to list of paths.
            _install_deps_append_paths_from_imported(${out_paths_name} "${lib}" "${libtag}" "${is_multiconfig}")
        elseif(lib)
            # Handling for if this lib is a file path.
            if (NOT EXISTS "${lib}")
                message(FATAL_ERROR "Path ${lib} given to install_deps() was bad")
            endif()
            list(APPEND ${out_paths_name} "${libtag}${lib}")
        endif()
    endforeach()

    set(${out_paths_name} "${${out_paths_name}}" PARENT_SCOPE)
    set(${out_libs_name} "${out_libs}" PARENT_SCOPE)
    set(${out_imports_name} "${${out_imports_name}}" PARENT_SCOPE)
endfunction()


# Helper function for _install_deps_internal: if we're installing Qt runtime libraries, add extra
# libraries and plugins that those runtime libraries depend on.
#
# ARGN == list of import libs that were returned by _install_deps_get_paths_from_libs().
function(_install_deps_get_qt_extra_paths lib_dest runtime_dest component out_paths_name)
    set(tagged_imports ${ARGN})
    if((NOT tagged_imports) OR (NOT TARGET Qt5::Core) OR NOT ("${tagged_imports}" MATCHES "Qt[0-9]+::"))
        return()
    endif()

    # Check to see if we're building with a multi-config generator (like Visual Studio or XCode) or not.
    get_property(is_multiconfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

    # Remove any libtags from the list of import libraries. None of the stuff we add in here
    # needs to use them.
    list(TRANSFORM imports REPLACE "^(@[^@]+@)(.+)" "\\2" REGEX "^@[^@]+@.+")

    # Get root dir of Qt5 installation.
    #   Find absolute path to core library.
    get_target_property(qt5_root Qt5::Core LOCATION)
    #   Remove library file from path.
    get_filename_component(qt5_root "${qt5_root}" DIRECTORY)
    #   Remove last directory ("lib") from path.
    get_filename_component(qt5_root "${qt5_root}" DIRECTORY)

    # For each Qt lib we support, add any required plugin targets to "plugins", and add any required
    # libraries to "to_add". The plugins will be installed directly, while the libs in "to_add" will
    # get appended onto out_paths at the bottom of the function, so that SONAME softlinks will be
    # created as needed.
    set(plugins)
    set(to_add)


    if("Qt5::Core" IN_LIST imports)
        # Install any extra libs needed by Qt5::Core.
        if(NOT WIN32 AND NOT APPLE) # Linux
            # Qt can't rely on OS libs for internationalization on Linux, because Linux uses ICU, which
            # doesn't have a stable shared-library interface. So, Qt is built against one specific
            # version of ICU that must be bundled alongside its libraries. It has to be the exact
            # version bundled with Qt, so that the ABI matches.
            list(APPEND to_add
                libicui18n.so
                libicuuc.so
                libicudata.so
            )
        endif()
    endif()

    if("Qt5::Gui" IN_LIST imports)
        # Install platform plugin.
        if(WIN32)
            list(APPEND plugins Qt5::QWindowsIntegrationPlugin)
        elseif(APPLE)
            # TODO: support Qt on macOS
        else() # Linux/X11
            list(APPEND plugins Qt5::QXcbIntegrationPlugin)
        endif()

        # Install any extra libs needed by Qt5::Gui.
        if(NOT WIN32 AND NOT APPLE) # Linux
            # Linux XCB platform plugin requires a few additional libraries that aren't listed as deps
            # in Qt5's exported config files.
            list(APPEND to_add
                libQt5DBus.so
                libQt5XcbQpa.so
            )
        endif()
    endif()

    if("Qt5::PrintSupport" IN_LIST imports)
        if(WIN32)
            list(APPEND plugins Qt5::QWindowsPrinterSupportPlugin)
        elseif(APPLE)
            #TODO: support Qt on macOS
        else() # Linux/X11
            list(APPEND plugins Qt5::QCupsPrinterSupportPlugin)
        endif()
    endif()


    # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    # Add prefix to each lib name we want to add, then append to out_paths.
    list(TRANSFORM to_add PREPEND "${qt5_root}/lib/")
    foreach(lib IN LISTS to_add)
        if(EXISTS ${lib})
            list(APPEND ${out_paths_name} ${lib})
        endif()
    endforeach()

    # Resolve plugin imported targets to a list of tagged paths.
    set(plugin_paths)
    foreach(plugin ${plugins})
        if(NOT TARGET ${plugin})
            message(AUTHOR_WARNING "Expected plugin ${plugin} is not a valid target, so it won't be installed.")
            continue()
        endif()
        _install_deps_append_paths_from_imported(plugin_paths "${plugin}" "" "${is_multiconfig}")
    endforeach()

    # Install each tagged plugin path directly - can't just add them to out_paths, because we have to
    # install the plugins to special subdirs under runtime_dest.
    foreach(path ${plugin_paths})
        _install_deps_take_libtag(path libtag)

        # Get subdir(s) of 'plugins' that Qt had the plugin install under - we'll need
        # to install under these same subdir(s) in bin/ when we bundle them in our app.
        #
        # Example: Windows platform plugin is at <QT_ROOT>/plugins/platforms/qwindows.dll,
        #          so we'll need to install to <runtime_dest>/platforms/qwindows.dll.
        get_filename_component(plugin_dir "${path}" DIRECTORY)
        string(REGEX REPLACE "^.*/plugins/" "" plugin_dir "${plugin_dir}")

        set(to_inst "${path}")
        if(libtag)
            if(libtag STREQUAL "@DEBUG@")
                set(to_inst "$<$<CONFIG:DEBUG>:${to_inst}>")
            else()
                set(to_inst "$<$<NOT:$<CONFIG:DEBUG>>:${to_inst}>")
            endif()
        endif()
        install(FILES "${to_inst}" DESTINATION "${runtime_dest}/${plugin_dir}" ${component})
    endforeach()

    # Push changes to out_paths up to caller's scope.
    set(${out_paths_name} "${${out_paths_name}}" PARENT_SCOPE)
endfunction()


# Helper function for _install_deps_internal: retrieve soname of given lib file. If no soname, returns empty string.
function(_install_deps_read_soname outvarname path)
    # Set output variable to empty string - this is what will be returned on an error.
    set(${outvarname} "" PARENT_SCOPE)

    if(NOT READELF) # If this system doesn't provide the readelf command.
        return()
    endif()

    # Read the ELF header from the file.
    execute_process(COMMAND "${READELF}" -d ${path}
        RESULT_VARIABLE res
        OUTPUT_VARIABLE header
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT res EQUAL 0) # If the readelf command returned an error status code.
        return()
    endif()

    # Parse the SONAME out of the header.
    if(READELF MATCHES "readelf")
        # Linux (readelf) format:   0x000000000000000e (SONAME) Library soname: [libssl.so.1.0.0]
        if(NOT header MATCHES "\\(SONAME\\)[^\n]+\\[([^\n]+)\\]")
            return()
        endif()
    else()
        # Solaris (elfdump) format: [8] SONAME 0x49c1 libssl.so.1.0.0
        if(NOT header MATCHES "\\[[0-9]+\\][ \t]+SONAME[ \t]+[x0-9a-fA-F]+[ \t]+([^\n]+)")
            return()
        endif()
    endif()

    string(STRIP "${CMAKE_MATCH_1}" soname)
    set(${outvarname} "${soname}" PARENT_SCOPE)
endfunction()


# Helper function for install_deps.
# _install_deps_internal([lib dest] [runtime dest] [flag to turn file copy on/off] [flag to turn file install on/off] [... lib files or import targets ...]
function(_install_deps_internal lib_dest runtime_dest component do_copy do_install)
    if((NOT do_copy) AND (NOT do_install))
        return()
    endif()

    # Check to see if we're building with a multi-config generator (like Visual Studio or XCode) or not.
    get_property(is_multiconfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

    # Handle default destination for copied DLL's.
    set(copy_dest "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

    # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    # Construct list of library paths.
    set(lib_paths)
    if(NOT lib_dest)
        set(lib_dest lib)
    endif()
    if(NOT runtime_dest)
        set(runtime_dest bin)
    endif()

    set(libs ${ARGN})

    # Get list of import lib prefixes (used to support LIMITED auto mode).
    set(prefixes)
    foreach(lib IN LISTS libs)
        if(TARGET ${lib} AND lib MATCHES "([^: \t]+)::[^ \t]+")
            list(APPEND prefixes "${CMAKE_MATCH_1}")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES prefixes)

    # Convert list of libraries into a list of library paths. Will resolve any alias libraries,
    # import libs, and import lib dependencies into full paths. Will also filter out unwanted
    # stuff like static library targets.
    #
    # On each loop, this function removes everything from 'libs' and processes each item.
    # Paths get added to lib_paths. Non-imported targets are installed directly. Imported targets
    # are processed to calculate the path, then this path is added to 'lib_paths'.
    #
    # Any automatically detected sub-dependencies or resolved alias targets are added to 'libs'
    # to be processed on the next iteration of the loop. The loop stops when 'libs' is empty.
    #
    # If this is a multiconfig generator, paths of libraries that should only be installed in
    # DEBUG or NOTDEBUG configurations are prefixed by a tag: @DEBUG@ or @NOTDEBUG@.
    set(imports)
    while(libs)
        _install_deps_get_paths_from_libs("${lib_dest}" "${runtime_dest}" "${component}"
            lib_paths
            libs
            prefixes
            imports
        )
    endwhile()
    list(REMOVE_DUPLICATES imports)

    # If we're installing any Qt libs, add any extra plugins and libraries that are required for
    # them to work properly.
    _install_deps_get_qt_extra_paths("${lib_dest}" "${runtime_dest}" "${component}"
        lib_paths
        ${imports}
    )

    # Remove any obvious duplicates. This won't catch different symlinks that refer to the same file,
    # or the same files installed on different calls to install_deps(). However, duplicates don't
    # actually hurt anything, they just make cmake's output during an install noisier to look at.
    # So, it's OK that we don't catch 100% of duplicates, since it's just to make things look nicer.
    #
    #   NOTE: if we wanted to catch 100% of duplicates, we could. It would just take more code.
    list(REMOVE_DUPLICATES lib_paths)

    # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    # Process each library path, install to appropriate location (if not already installed).
    set(sonames)
    foreach(path IN LISTS lib_paths)
        # If this is an empty list element, skip it.
        if(NOT path)
            continue()
        endif()

        # If path has a tag in front of it, parse it off.
        _install_deps_take_libtag(path libtag)
        if(NOT is_multiconfig)
            set(libtag)
        endif()

        # If on Windows, try to replace import libraries with DLL's. Throws fatal error if it can't do it.
        if(WIN32 AND (${path} MATCHES "\.lib$" OR ${path} MATCHES "\.a$"))
           _install_deps_get_dll_from_implib(path "${path}")
        endif()

        # Resolve any symlinks in path to get actual physical name. If relative, evaluate relative to current binary dir.
        get_filename_component(path "${path}" REALPATH BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")

        # Figure out the destination (DLL's and manifest files go to runtime_dest, everything else goes to lib_dest).
        if(path MATCHES "\.[dD][lL][lL]$" OR path MATCHES "\.[mM][aA][nN][iI][fF][eE][sS][tT]$")
            set(dest "${runtime_dest}")
            if(path MATCHES "\.[dD][lL][lL]$")
                set(type PROGRAMS) # install DLL's as executable
            else()
                set(type FILES) # install manifests as a normal non-executable file
            endif()

            # If requested by caller, copy the DLL's to the build dir in addition to installing them.
            # If the file with the same name and timestamp already exists at the destination, nothing will be copied.
            if(do_copy AND type STREQUAL PROGRAMS)
                if(is_multiconfig)
                    set(configs)
                    if(libtag)
                        # If this wasn't a general-use library, only copy it to the proper configurations.
                        if(libtag STREQUAL "@DEBUG@")
                            # Copy to "Debug" directory only.
                            foreach(config ${CMAKE_CONFIGURATION_TYPES})
                                string(TOLOWER "${config}" config_lower)
                                if(config_lower STREQUAL "debug")
                                    set(configs ${config})
                                    break()
                                endif()
                            endforeach()
                        else()
                            # Copy to every directory besides "Debug".
                            set(configs)
                            foreach(config ${CMAKE_CONFIGURATION_TYPES})
                                string(TOLOWER "${config}" config_lower)
                                if(NOT config_lower STREQUAL "debug")
                                    list(APPEND configs "${config}")
                                endif()
                            endforeach()
                        endif()
                    else()
                        # If this was a general-use library (no libtag), copy it to every configuration.
                        set(configs ${CMAKE_CONFIGURATION_TYPES})
                    endif()
                    foreach(config ${configs})
                        file(MAKE_DIRECTORY "${copy_dest}/${config}")
                        file(COPY "${path}" DESTINATION "${copy_dest}/${config}")
                    endforeach()
                else()
                    file(COPY "${path}" DESTINATION "${copy_dest}")
                endif()
            endif()
        else()
            set(dest "${lib_dest}")
            set(type FILES)
        endif()

        if(NOT do_install)
            continue()
        endif()

        # Install the file.
        set(to_inst "${path}")
        if(libtag)
            if(libtag STREQUAL "@DEBUG@")
                set(to_inst "$<$<CONFIG:DEBUG>:${to_inst}>")
            else()
                set(to_inst "$<$<NOT:$<CONFIG:DEBUG>>:${to_inst}>")
            endif()
        endif()
        install(${type} "${to_inst}" DESTINATION "${dest}" ${component})

        # If the library has a soname that's different than the actual name of the file on disk, add a symlink.
        _install_deps_read_soname(soname "${path}")
        if(soname)
            get_filename_component(libname "${path}" NAME)
            if(NOT soname STREQUAL libname)
                # Generate a relative-path symlink in the build dir (doesn't have to be valid).
                set(tmpdir "${CMAKE_CURRENT_BINARY_DIR}/dep-lib-links")
                if(libtag)
                    string(REPLACE "@" "" libtagname "${libtag}")
                    string(APPEND tmpdir "${libtagname}")
                endif()
                file(MAKE_DIRECTORY "${tmpdir}")
                file(REMOVE "${tmpdir}/${soname}") # Remove any old symlink with the same name.
                execute_process(
                    COMMAND           ${CMAKE_COMMAND} -E create_symlink "${libname}" "${soname}"
                    WORKING_DIRECTORY "${tmpdir}"
                    RESULT_VARIABLE   res
                    ERROR_QUIET
                    OUTPUT_QUIET
                )

                if(NOT res EQUAL 0)
                    message(AUTHOR_WARNING "install_deplib: failed to create install symlink for \"${libname}\"")
                    continue()
                endif()

                # Install the symlink to the same directory as the library.
                set(to_inst "${tmpdir}/${soname}")
                if(libtag)
                    if(libtag STREQUAL "@DEBUG@")
                        set(to_inst "$<$<CONFIG:DEBUG>:${to_inst}>")
                    else()
                        set(to_inst "$<$<NOT:$<CONFIG:DEBUG>>:${to_inst}>")
                    endif()
                endif()
                install(${type} "${to_inst}" DESTINATION "${dest}" ${component})
            endif()
        endif()
    endforeach()
endfunction()


function(_install_deps_get_debug_system_libraries out_varname)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES          TRUE)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY     TRUE)

    include(InstallRequiredSystemLibraries)

    # BUGFIX: CMake doesn't find the OpenMP debug library properly on Windows, add a hack to work
    #         around this problem until we can get a patch accepted with upstream.
    if(CMAKE_INSTALL_OPENMP_LIBRARIES)
        #Loop through paths until we find vcruntime###d.dll (if present).
        set(omp_paths)
        foreach(path ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
            if(path MATCHES "vcruntime[0-9]+d\.dll")
                get_filename_component(path "${path}" DIRECTORY) # remove filename
                string(REPLACE ".DebugCRT" ".DebugOpenMP" path "${path}")
                file(GLOB omp_paths "${path}/vcomp*d.dll")
                break()
            endif()
        endforeach()
        if(omp_paths)
            list(GET omp_paths 0 omp_paths)
            list(APPEND CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${omp_paths})
        endif()
    endif()

    set(${out_varname} "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}" PARENT_SCOPE)
endfunction()


# install_deps([lib dest] [runtime dest] [... lib files or import targets ...]
function(install_deps lib_dest runtime_dest)
    # Handle default values for variables that control DLL copying.
    if(NOT DEFINED INSTALL_DEPS_COPY_DLL)
        set(INSTALL_DEPS_COPY_DLL TRUE)
    endif()

    set(libs ${ARGN})
    if(NOT libs)
        return()
    endif()

    # See if the user passed an optional "COMPONENT [component name]" to the install command.
    # If they did, remove those entries from the 'libs' list and add them to the 'component' list.
    set(component)
    list(FIND libs COMPONENT idx)
    if(idx GREATER -1)
        math(EXPR idx_next "${idx} + 1")
        list(GET libs ${idx} ${idx_next} component)
        list(REMOVE_AT libs ${idx} ${idx_next})
    endif()

    # Call internal helper
    _install_deps_internal("${lib_dest}" "${runtime_dest}" "${component}" ${INSTALL_DEPS_COPY_DLL} TRUE ${libs})
endfunction()


# install_deps_system([lib dest] [runtime dest])
function(install_deps_system lib_dest runtime_dest)
    # Check to see if we're building with a multi-config generator (like Visual Studio or XCode) or not.
    get_property(is_multiconfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

    if(OpenMP_FOUND)
        set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
    endif()
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE) # tell module not to install, just save to CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS

    # Before we do anything else, grab list of libraries installed when in debug-only mode.
    set(debug_libs)
    set(debug_tag)
    set(nondebug_tag)
    if(is_multiconfig OR CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
        _install_deps_get_debug_system_libraries(debug_libs)
        if(is_multiconfig)
            set(debug_tag "@DEBUG@")
            set(nondebug_tag "@NONDEBUG@")
        endif()
    endif()

    # Grab list of system libraries installed in normal non-debug mode.
    include(InstallRequiredSystemLibraries) # sets CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS

    # Get list of debug libs that are only installed when debug libs are requested.
    set(debug_only_libs)
    foreach(lib ${debug_libs})
        if(NOT lib IN_LIST CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
            list(APPEND debug_only_libs "${debug_tag}${lib}")
        endif()
    endforeach()

    # Remove any release libs that we're already installing the corresponding debug libs for.
    # If multiconfig, mark those libs with a nondebug libtag instead of removing them.
    set(other_libs)
    if(debug_only_libs)
        foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
            get_filename_component(libname "${lib}" NAME)
            string(REGEX REPLACE ".dll$" "d.dll" debuglib "${libname}")
            if(debug_only_libs MATCHES "/${debuglib}(;|$)")
                if(is_multiconfig)
                    list(APPEND other_libs "${nondebug_tag}${lib}")
                endif()
            else()
                list(APPEND other_libs "${lib}")
            endif()
        endforeach()
    else()
        set(other_libs "${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}")
    endif()

    install_deps("${lib_dest}" "${runtime_dest}" ${debug_only_libs} ${other_libs})
endfunction()
