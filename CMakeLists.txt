# Root CMake file for Obvi project.
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
# # # # # # # # # # # # #

cmake_minimum_required(VERSION 3.13...3.14)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ProjectSetup)

project(Obvi
    VERSION   1.0
    LANGUAGES CXX
)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# Set up compiler and linker options.
set(ADD_FLAGS_STRICT_WARNINGS TRUE) # Enable verbose warnings.
set(ADD_FLAGS_HARDEN          TRUE) # Add security-hardening flags.
include(AddFlags)

# Require C++14, and disable compiler-specific extensions (if possible).
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# Find third-party dependencies.
include(AddQtPath)

find_package(Qt5 REQUIRED COMPONENTS Widgets)


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# Build rules.
add_subdirectory(util)
add_subdirectory(tests)

add_executable(obvi
    main.cpp
)

target_link_libraries(obvi PRIVATE
    Qt5::Widgets
    util
)

include(CTest) # Adds a 'test' phony target for running unit tests (must be in root to work).
