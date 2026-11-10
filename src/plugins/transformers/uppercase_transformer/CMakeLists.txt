torustiq_add_plugin(uppercase_transformer uppercase_transformer.cpp)

if(TORUSTIQ_PLUGINS_BUILD_TESTS)
    torustiq_add_plugin_test(uppercase_transformer uppercase_transformer_test.cpp)
endif()
