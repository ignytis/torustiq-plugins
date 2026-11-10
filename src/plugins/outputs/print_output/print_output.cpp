// Output plugin `print_output`: prints the payload, then each header on its own line, indented
// with two spaces, as `key=value`.

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "torustiq/sdk.h"

namespace {

constexpr const char* kPluginId = "print_output";

int32_t Create(const torustiq_config*, void** out_handle) {
    try {
        *out_handle = new int(0);
        return TORUSTIQ_OK;
    } catch (...) {
        return TORUSTIQ_ERROR;
    }
}

int32_t Write(void*, const torustiq_message* message) {
    try {
        if (message->body.data != nullptr) {
            std::cout.write(reinterpret_cast<const char*>(message->body.data),
                            static_cast<std::streamsize>(message->body.len));
        }
        std::cout << '\n';
        for (size_t i = 0; i < message->header_count; ++i) {
            std::cout << "  " << message->headers[i].key << '=' << message->headers[i].value
                      << '\n';
        }
        return std::cout.good() ? TORUSTIQ_OK : TORUSTIQ_ERROR;
    } catch (...) {
        return TORUSTIQ_ERROR;
    }
}

int32_t Flush(void*) {
    std::cout.flush();
    return std::cout.good() ? TORUSTIQ_OK : TORUSTIQ_ERROR;
}

void Destroy(void* handle) {
    delete static_cast<int*>(handle);
}

const torustiq_plugin_info kInfo = {TORUSTIQ_SDK_ABI_VERSION, kPluginId};

} // namespace

extern "C" {

TORUSTIQ_PLUGIN_EXPORT const torustiq_plugin_info* torustiq_plugin_get_info(void) {
    return &kInfo;
}

TORUSTIQ_PLUGIN_EXPORT int32_t torustiq_plugin_init(const torustiq_registrar* registrar) {
    torustiq_output_vtable vtable{};
    vtable.abi_version = TORUSTIQ_SDK_ABI_VERSION;
    vtable.name = kPluginId;
    vtable.create = Create;
    vtable.write = Write;
    vtable.flush = Flush;
    vtable.destroy = Destroy;
    return registrar->register_output(registrar->context, &vtable);
}

} // extern "C"
