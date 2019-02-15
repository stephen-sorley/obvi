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
add_subdirectory(tests)

add_executable(obvi
    main.cpp
)

target_link_libraries(obvi PRIVATE
    Qt5::Widgets
)

target_include_directories(obvi PRIVATE
    include
)
