# AddFlagsHelpers.cmake
#
# Provides a few helper functions for AddFlags.cmake:
#   _int_add_flags_linker
#   _int_add_flags_compiler
#   _int_add_flag_options_compiler
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

cmake_minimum_required(VERSION 3.13...3.14)

include_guard(DIRECTORY)

# Put list of enabled languages in accessible variable.
get_property(enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)

if("C" IN_LIST enabled_languages)
    include(CheckCCompilerFlag)
endif()
if("CXX" IN_LIST enabled_languages)
    include(CheckCXXCompilerFlag)
endif()
if("Fortran" IN_LIST enabled_languages)
    include(CheckFortranCompilerFlag)
endif()


# internal helper - adds given linker flags (if accepted by linker) to given configurations.
# _int_add_flags_linker(FLAGS <linker flags> [CONFIGS <linker target types>])
#   FLAGS:   linker flags to add
#   CONFIGS: kinds of linker products (EXE, SHARED, and/or MODULE) that these flags should be used for. Default: all configs
function(_int_add_flags_linker)
    cmake_parse_arguments(arg "" "" "FLAGS;CONFIGS" ${ARGN})

    if(NOT arg_FLAGS)
        return()
    endif()

    if(NOT arg_CONFIGS)
        set(arg_CONFIGS
            EXE
            SHARED
            MODULE
        )
    else()
        list(TRANSFORM ${arg_CONFIGS} TOUPPER)
    endif()

    foreach(flag ${arg_FLAGS})
        # Check to see if linker flag is supported (use C if enabled, or CXX if enabled, or Fortran if enabled).
        #
        # Use flag in variable name to ensure uniqueness, normalize to a valid C identifier to
        # make sure that CMake will accept it as a variable name.
        string(MAKE_C_IDENTIFIER "HAVE_${flag}" var)
        set(CMAKE_REQUIRED_LIBRARIES "${flag}")
        if("C" IN_LIST enabled_languages)
            check_c_compiler_flag("" ${var})
        elseif("CXX" IN_LIST enabled_languages)
            check_cxx_compiler_flag("" ${var})
        elseif("Fortran" IN_LIST enabled_languages)
            check_fortran_compiler_flag("" ${var})
        else()
            set(${var} FALSE)
        endif()

        # If linker flag is supported, add it to the linker flag lists for every requested configuration.
        if(${var})
            foreach(config ${arg_CONFIGS})
                set(dest_name CMAKE_${config}_LINKER_FLAGS)
                if(${dest_name})
                    # If destination var isn't empty, add a space before adding the new flag.
                    string(APPEND ${dest_name} " ")
                endif()
                string(APPEND ${dest_name} "${flag}")
            endforeach()
        endif()
    endforeach()

    # Push any changes to linker flags variables up to caller's scope.
    foreach(config ${arg_CONFIGS})
        set(dest_name CMAKE_${config}_LINKER_FLAGS)
        set(${dest_name} "${${dest_name}}" PARENT_SCOPE)
    endforeach()
endfunction()



# internal helper - adds given compile flags (if accepted by compiler) to given configurations of given languages.
# _int_add_flags_compiler(FLAGS <compiler flags> [LANGS <languages>] [CONFIGS <build types>])
#   FLAGS:   compiler flags to add
#   LANGS:   languages for which the compiler flags should be used (C, CXX, and/or Fortran)
#   CONFIGS: build types for which the compiler flags should be used (Debug, Release, etc.). Default: all configs
function(_int_add_flags_compiler)
    cmake_parse_arguments(arg "" "" "FLAGS;LANGS;CONFIGS" ${ARGN})

    if(NOT arg_FLAGS)
        return()
    endif()

    # Filter out langs that aren't enabled.
    set(temp_langs "${arg_LANGS}")
    set(arg_LANGS)
    foreach(lang ${temp_langs})
        if(lang IN_LIST enabled_languages)
            list(APPEND arg_LANGS "${lang}")
        endif()
    endforeach()

    if(NOT arg_LANGS)
        return()
    endif()

    if(NOT arg_CONFIGS)
        set(arg_CONFIGS "NONE")
    else()
        # Change all config names to uppercase. This is required because the per-config versions
        # of the various variables that store flags use uppercase for the config name
        # (e.g. CMAKE_CXX_FLAGS_RELEASE).
        list(TRANSFORM ${configs_var} TOUPPER)
    endif()

    foreach(lang ${arg_LANGS})
        foreach(flag ${arg_FLAGS})
            # Use flag in variable name to ensure uniqueness, normalize to a valid C identifier to
            # make sure that CMake will accept it as a variable name.
            string(MAKE_C_IDENTIFIER "HAVE_${flag}" var)

            # Check to see if this language's compiler accepts this flag.
            set(varlang ${var}_${lang})
            if(    lang STREQUAL "C")
                check_c_compiler_flag("${flag}" ${varlang})
            elseif(lang STREQUAL "CXX")
                check_cxx_compiler_flag("${flag}" ${varlang})
            elseif(lang STREQUAL "Fortran")
                check_fortran_compiler_flag("${flag}" ${varlang})
            else()
                set(${varlang} FALSE)
            endif()

            # If it does accept it, add it to the language-specific flag lists for every enabled configuration.
            if(${varlang})
                foreach(config ${arg_CONFIGS})
                    set(dest_name CMAKE_${lang}_FLAGS)
                    if(NOT config STREQUAL "NONE")
                        string(APPEND dest_name "_${config}")
                    endif()
                    if(${dest_name})
                        # If destination var isn't empty, add a space before adding the new flag.
                        string(APPEND ${dest_name} " ")
                    endif()
                    string(APPEND ${dest_name} "${flag}")
                endforeach()
            endif()
         endforeach()
    endforeach()

    # Push any changes to compiler flags variables up to caller's scope.
    foreach(lang ${arg_LANGS})
        set(dest_name CMAKE_${lang}_FLAGS)
        set(${dest_name} "${${dest_name}}" PARENT_SCOPE)
    endforeach()
endfunction()



# internal helper - adds one of the given compile flags (if accepted by compiler) to given configurations
#                   of given languages. Will only use the first flag in the list that's accepted by the compiler.
#
# _int_add_flag_options_compiler(FLAG_OPTIONS <compiler flags> [LANGS <languages>] [CONFIGS <build types>])
#   FLAG_OPTIONS: list of compiler flag options: if first entry fails, will try second, then third, and so on.
#   LANGS:        languages for which the compiler flags should be used (C, CXX, and/or Fortran)
#   CONFIGS:      build types for which the compiler flags should be used (Debug, Release, etc.). Default: all configs
function(_int_add_flag_options_compiler)
    cmake_parse_arguments(arg "" "" "FLAG_OPTIONS;LANGS;CONFIGS" ${ARGN})

    if(NOT arg_FLAG_OPTIONS)
        return()
    endif()

    # Filter out langs that aren't enabled.
    set(temp_langs "${arg_LANGS}")
    set(arg_LANGS)
    foreach(lang ${temp_langs})
        if(lang IN_LIST enabled_languages)
            list(APPEND arg_LANGS "${lang}")
        endif()
    endforeach()

    if(NOT arg_LANGS)
        return()
    endif()

    if(NOT arg_CONFIGS)
        set(arg_CONFIGS "NONE")
    else()
        # Change all config names to uppercase. This is required because the per-config versions
        # of the various variables that store flags use uppercase for the config name
        # (e.g. CMAKE_CXX_FLAGS_RELEASE).
        list(TRANSFORM ${configs_var} TOUPPER)
    endif()

    foreach(lang ${arg_LANGS})
        foreach(flag ${arg_FLAG_OPTIONS})
            # Use flag in variable name to ensure uniqueness, normalize to a valid C identifier to
            # make sure that CMake will accept it as a variable name.
            string(MAKE_C_IDENTIFIER "HAVE_${flag}" var)

            # Check to see if the current language's compiler accepts this flag.
            set(varlang ${var}_${lang})
            if(    lang STREQUAL "C")
                check_c_compiler_flag("${flag}" ${varlang})
            elseif(lang STREQUAL "CXX")
                check_cxx_compiler_flag("${flag}" ${varlang})
            elseif(lang STREQUAL "Fortran")
                check_fortran_compiler_flag("${flag}" ${varlang})
            else()
                set(${varlang} FALSE)
            endif()

            # If it does accept it, add it to the language-specific flag lists for every enabled configuration.
            if(${varlang})
                foreach(config ${arg_CONFIGS})
                    set(dest_name CMAKE_${lang}_FLAGS)
                    if(NOT config STREQUAL "NONE")
                        string(APPEND dest_name "_${config}")
                    endif()
                    if(${dest_name})
                        # If destination var isn't empty, add a space before adding the new flag.
                        string(APPEND ${dest_name} " ")
                    endif()
                    string(APPEND ${dest_name} "${flag}")
                endforeach()

                break() # Once we hit a set of flags that works for this language, stop the loop.
            endif()
         endforeach()
    endforeach()

    # Push any changes to compiler flags variables up to caller's scope.
    foreach(lang ${arg_LANGS})
        set(dest_name CMAKE_${lang}_FLAGS)
        set(${dest_name} "${${dest_name}}" PARENT_SCOPE)
    endforeach()
endfunction()
