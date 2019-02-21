# Unit tests.
#
# These tests use the Catch2 unit testing framework: https://github.com/catchorg/Catch2
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

add_executable(tests
    tests_main.cpp

    test_vec3.cpp
    test_mat3.cpp
)

target_link_libraries(tests PRIVATE
    util
)

target_compile_definitions(tests PRIVATE
    CATCH_CONFIG_FAST_COMPILE   # Turn off exception translation in normal test cases, ~20% faster
    #CATCH_CONFIG_WINDOWS_CRTDBG # Enable leak checking using Windows' CRT Debug Heap
)

# Pass unit tests to CTest using Catch's CMake integration scripts.
list(APPEND CMAKE_MODULE_PATH "${Obvi_SOURCE_DIR}/include/catch2")
include(Catch)
catch_discover_tests(tests)
