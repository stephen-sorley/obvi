cmake_minimum_required(VERSION 3.13...3.14)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ProjectSetup)

project(Obvi
	VERSION   1.0
    LANGUAGES CXX
)

set(ADD_FLAGS_STRICT_WARNINGS TRUE) # Enable verbose warnings.
set(ADD_FLAGS_HARDEN          TRUE) # Add security-hardening flags.
include(AddFlags)



add_executable(obvi
    main.cpp
)
