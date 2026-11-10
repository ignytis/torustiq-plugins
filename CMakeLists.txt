cmake_minimum_required(VERSION 3.21)
project(torustiq_plugins VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# torustiq_add_plugin(<target> <source>)
# Builds a plugin as a shared library that the host loads at run time. The sources are also
# compiled into a static `<target>_objects` library for unit tests.
function(torustiq_add_plugin target source)
    add_library(${target}_objects OBJECT ${source})
    set_target_properties(${target}_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(${target}_objects PUBLIC torustiq::sdk)
    add_library(${target} MODULE $<TARGET_OBJECTS:${target}_objects>)
    target_link_libraries(${target} PRIVATE torustiq::sdk)
    foreach(t ${target}_objects ${target})
        if(MSVC)
            target_compile_options(${t} PRIVATE /W4)
        else()
            target_compile_options(${t} PRIVATE -Wall -Wextra -Wpedantic)
        endif()
    endforeach()
endfunction()

torustiq_add_plugin(hello_input src/plugins/inputs/hello_input/hello_input.cpp)
torustiq_add_plugin(uppercase_transformer
    src/plugins/transformers/uppercase_transformer/uppercase_transformer.cpp)
torustiq_add_plugin(print_output src/plugins/outputs/print_output/print_output.cpp)

if(TORUSTIQ_PLUGINS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
