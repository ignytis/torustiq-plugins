
#include <torustiq_sdk/typedefs.h>

#include "stdio.hpp"

extern "C" const TorustiqPluginInfo torustiq_plugin_get_info() {
    return TorustiqPluginInfo{
        .host_app = APP_NAME,
        .api_version = API_VERSION,
        .id = "stdio_lib",
        .name = "Standard input/oputput, but as a library",
    };
}

extern "C" const TorustiqPlugin torustiq_plugin_init(TorustiqHostGlobals globals) {
    return TorustiqPlugins::Stdio::InitPlugin(globals);
}