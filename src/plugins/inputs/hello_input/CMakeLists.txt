torustiq_add_plugin(hello_input hello_input.cpp)

if(TORUSTIQ_PLUGINS_BUILD_TESTS)
    torustiq_add_plugin_test(hello_input hello_input_test.cpp)
endif()
