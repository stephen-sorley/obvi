# Search common locations for Qt, and add the most recent version to CMAKE_MODULE_PATH if found.
# 
# You still need to call find_package(Qt ...) after including this file. This file just tries to
# ensure that the find_package call will find the newest version of Qt that matches the compiler
# you're building with. This is entirely optional - if no matching version is found, no changes
# are made and find_package works just like it would if you weren't using this file.
#
# Example usage:
#
# include(AddQtPath)
#
# find_package(Qt5 REQUIRED COMPONENTS Widgets)
#
# add_executable(myapp WIN32
#     myapp.cpp
#     mydialog_box.cpp # cmake will detect included ui_*.h header files and run uic for us (AUTOUIC option below).
#     resources.qrc # cmake will run Qt's resource compiler (rcc) on any listed *.qrc files (AUTORCC option below).
# )
#
# target_link_libraries(myapp PRIVATE
#    Qt5::Widgets
# )
#
# set_target_properties(myapp PROPERTIES
#     AUTOMOC TRUE
#     AUTOUIC TRUE
#     AUTORCC TRUE
# )
#

cmake_minimum_required(VERSION 3.13...3.14)

include_guard(DIRECTORY)

if(WIN32)
    if(NOT EXISTS "C:\Qt")
        return()
    endif()
    
elseif(APPLE)
    
else() #Linux, BSD, Cygwin, etc.
    
endif()