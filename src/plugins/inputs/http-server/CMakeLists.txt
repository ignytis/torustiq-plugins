# cpp-httplib (header-only HTTP server).
find_package(httplib QUIET)
if(NOT httplib_FOUND)
    set(HTTPLIB_COMPILE OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_REQUIRE_OPENSSL OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.65.0
        SYSTEM)
    FetchContent_MakeAvailable(httplib)
endif()

torustiq_add_plugin(http_server_input http_server_input.cpp LIBS httplib::httplib)

if(TORUSTIQ_PLUGINS_BUILD_TESTS)
    torustiq_add_plugin_test(http_server_input http_server_input_test.cpp)
endif()
