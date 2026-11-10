# Shared helpers for plugin unit tests (test_util.h).
add_library(torustiq_test_util INTERFACE)
target_include_directories(torustiq_test_util INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(torustiq_test_util INTERFACE torustiq::sdk GTest::gtest)
