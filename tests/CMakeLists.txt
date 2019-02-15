# Unit tests.
#
# These tests use the Catch2 unit testing framework: https://github.com/catchorg/Catch2

add_executable(tests
    main.cpp
    geom.cpp
)

target_include_directories(tests PRIVATE
    "${Obvi_SOURCE_DIR}/include"
)

target_compile_definitions(tests PRIVATE
    CATCH_CONFIG_FAST_COMPILE   # Turn off exception translation in normal test cases, ~20% faster
    #CATCH_CONFIG_WINDOWS_CRTDBG # Enable leak checking using Windows' CRT Debug Heap
)
