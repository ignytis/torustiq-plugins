cmake_minimum_required(VERSION 3.21)
project(torustiq_plugins VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Keep all plugin libraries in the build root (scripts/link_dev.sh expects them there).
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(TORUSTIQ_PLUGINS_BUILD_TESTS "Build unit tests" ON)

include(FetchContent)

# For a local checkout: -DFETCHCONTENT_SOURCE_DIR_TORUSTIQ_SDK=/path/to/torustiq-sdk
find_package(torustiq_sdk QUIET)
if(NOT torustiq_sdk_FOUND)
    FetchContent_Declare(torustiq_sdk
        GIT_REPOSITORY git@github.com:ignytis/torustiq-sdk.git
        GIT_TAG main)
    FetchContent_MakeAvailable(torustiq_sdk)
endif()

include(cmake/TorustiqPlugin.cmake)

if(TORUSTIQ_PLUGINS_BUILD_TESTS)
    enable_testing()
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2)
        FetchContent_MakeAvailable(googletest)
    endif()
    add_subdirectory(tests)
endif()

# Every plugin is a self-contained directory with its own CMakeLists.txt (dependencies, target,
# test): src/plugins/<kind>/<name>/CMakeLists.txt. Drop a new folder in and re-run CMake.
file(GLOB plugin_lists CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/*/*/CMakeLists.txt)
foreach(list_file ${plugin_lists})
    get_filename_component(plugin_dir ${list_file} DIRECTORY)
    add_subdirectory(${plugin_dir})
endforeach()
