find_package(GTest QUIET)
if(NOT GTest_FOUND)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2)
    FetchContent_MakeAvailable(googletest)
endif()

# Each plugin exports the same C symbols, so every test links exactly one plugin.
foreach(plugin hello_input uppercase_transformer print_output)
    add_executable(${plugin}_test ${plugin}_test.cpp)
    target_include_directories(${plugin}_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${plugin}_test PRIVATE ${plugin}_objects GTest::gtest_main)
    if(MSVC)
        target_compile_options(${plugin}_test PRIVATE /W4)
    else()
        target_compile_options(${plugin}_test PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME ${plugin}_test COMMAND ${plugin}_test)
endforeach()
