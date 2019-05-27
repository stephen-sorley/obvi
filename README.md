# Obvi

### Project Overview
A basic 3D object viewer. Written mainly to get back into practice with Qt,
CMake, and modern C++, and to create an example that I can refer to later.

Currently only tested to build with VS2017 and GCC 7.3 (Ubuntu 18.04).
I'll probably include support for a CI system like AppVeyor at a later date.

Note that the CMake modules located in the *cmake/* subdir are actually from
the [cmake-common](https://github.com/stephen-sorley/cmake-common) project.
They've been pulled into this project as a subrepo by using
[git-subrepo](https://github.com/ingydotnet/git-subrepo/tree/master).


### Third-party dependencies

[Catch2](https://github.com/catchorg/Catch2):
Header-only unit testing framework (bundled into repo)

[Qt](https://www.qt.io/download):
Cross-platform GUI library
