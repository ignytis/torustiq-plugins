cmake_minimum_required(VERSION 3.20)

# Read project info
file(READ project.json PROJECT_INFO)
string(JSON APP_NAME GET ${PROJECT_INFO} name)
string(JSON APP_VERSION GET ${PROJECT_INFO} version)
string(JSON APP_DESCRIPTION GET ${PROJECT_INFO} description)

string(TOLOWER ${APP_NAME} APP_NAME_LOWERCASE)

project(${APP_NAME} VERSION ${APP_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "build") # for static library

# Helps with debugging
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# SDK library
find_path(TORUSTIQ_SDK_INCLUDE_DIR
    NAMES torustiq_sdk/typedefs.h
    HINTS
        "$ENV{TORUSTIQ_SDK_DIR}/include"
        "$ENV{TORUSTIQ_SDK_DIR}"
    PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/../../torustiq-sdk/include"
    REQUIRED
)

add_library(torustiq_sdk STATIC
    # Could this list be generated the same way as SOURCES?
    ${TORUSTIQ_SDK_INCLUDE_DIR}/../src/message.c
)
target_include_directories(torustiq_sdk PUBLIC ${TORUSTIQ_SDK_INCLUDE_DIR})

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_library(${APP_NAME_LOWERCASE} SHARED ${SOURCES})
target_link_libraries(${APP_NAME_LOWERCASE} PRIVATE torustiq_sdk)

add_compile_definitions(APP_NAME="${APP_NAME}")
add_compile_definitions(APP_VERSION="${APP_VERSION}")
add_compile_definitions(APP_DESCRIPTION="${APP_DESCRIPTION}")
