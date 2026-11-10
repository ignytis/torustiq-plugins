torustiq_add_plugin(print_output print_output.cpp)

if(TORUSTIQ_PLUGINS_BUILD_TESTS)
    torustiq_add_plugin_test(print_output print_output_test.cpp)
endif()
