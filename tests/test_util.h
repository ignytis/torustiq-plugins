#ifndef TORUSTIQ_PLUGINS_TESTS_TEST_UTIL_H_
#define TORUSTIQ_PLUGINS_TESTS_TEST_UTIL_H_

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "torustiq/sdk.h"

extern "C" {
const torustiq_plugin_info* torustiq_plugin_get_info(void);
int32_t torustiq_plugin_init(const torustiq_registrar* registrar);
}

namespace test {

struct Captured {
    uint32_t kind = TORUSTIQ_VALUE_NULL; // payload kind
    std::string body;                    // bytes payloads only
    std::map<std::string, std::string> headers;
    std::vector<std::string> header_order;
};

inline int32_t Collect(void* user_data, const torustiq_message* message) {
    Captured c;
    c.kind = message->payload.kind;
    if (message->payload.kind == TORUSTIQ_VALUE_BYTES) {
        c.body.assign(reinterpret_cast<const char*>(message->payload.as.bytes.data),
                      message->payload.as.bytes.len);
    }
    for (size_t i = 0; i < message->header_count; ++i) {
        c.headers[message->headers[i].key] = message->headers[i].value;
        c.header_order.push_back(message->headers[i].key);
    }
    static_cast<std::vector<Captured>*>(user_data)->push_back(std::move(c));
    return TORUSTIQ_OK;
}

// Registrar that keeps the registered vtable of each kind.
struct Registered {
    torustiq_input_vtable input{};
    torustiq_transform_vtable transform{};
    torustiq_output_vtable output{};
};

inline torustiq_registrar MakeRegistrar(Registered* registered) {
    torustiq_registrar r{};
    r.context = registered;
    r.register_input = [](void* ctx, const torustiq_input_vtable* v) {
        static_cast<Registered*>(ctx)->input = *v;
        return TORUSTIQ_OK;
    };
    r.register_transform = [](void* ctx, const torustiq_transform_vtable* v) {
        static_cast<Registered*>(ctx)->transform = *v;
        return TORUSTIQ_OK;
    };
    r.register_output = [](void* ctx, const torustiq_output_vtable* v) {
        static_cast<Registered*>(ctx)->output = *v;
        return TORUSTIQ_OK;
    };
    return r;
}

} // namespace test

#endif // TORUSTIQ_PLUGINS_TESTS_TEST_UTIL_H_
