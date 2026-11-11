// Input plugin `hello_input`.
// Config: `n` - a positive integer. Emits `n` messages `hello, {i}!` (i = 1..n), each with the
// single header `loop_number` = i.

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "torustiq/sdk.h"

namespace {

constexpr const char* kPluginId = "hello_input";

struct HelloInput {
    uint64_t count = 0;
    std::atomic<bool> stopped{false};
};

// Parses a positive decimal integer without exceptions. Returns 0 if `text` is not one.
uint64_t ParsePositive(const char* text) {
    if (text == nullptr || *text == '\0') {
        return 0;
    }
    uint64_t value = 0;
    for (const char* c = text; *c != '\0'; ++c) {
        if (*c < '0' || *c > '9') {
            return 0;
        }
        const uint64_t digit = static_cast<uint64_t>(*c - '0');
        if (value > (std::numeric_limits<uint64_t>::max() - digit) / 10) {
            return 0;
        }
        value = value * 10 + digit;
    }
    return value;
}

int32_t
Create(const torustiq_config* config, torustiq_payload_spec* out_output, void** out_handle) {
    try {
        out_output->kind = TORUSTIQ_PAYLOAD_BYTES;
        const uint64_t count = ParsePositive(torustiq_config_get(config, "n"));
        if (count == 0) {
            return TORUSTIQ_ERROR; // `n` is missing or not a positive integer
        }
        auto* input = new HelloInput();
        input->count = count;
        *out_handle = input;
        return TORUSTIQ_OK;
    } catch (...) {
        return TORUSTIQ_ERROR;
    }
}

int32_t Run(void* handle, torustiq_emit_fn emit, void* user_data) {
    auto* self = static_cast<HelloInput*>(handle);
    try {
        for (uint64_t i = 1; i <= self->count && !self->stopped.load(); ++i) {
            const std::string number = std::to_string(i);
            const std::string body = "hello, " + number + "!";
            const torustiq_header header{"loop_number", number.c_str()};
            torustiq_message message{};
            message.payload = torustiq_value_from_bytes(
                reinterpret_cast<const uint8_t*>(body.data()), body.size());
            message.headers = &header;
            message.header_count = 1;
            const int32_t status = emit(user_data, &message);
            if (status != TORUSTIQ_OK) {
                return status == TORUSTIQ_STOPPED ? TORUSTIQ_OK : status;
            }
        }
        return TORUSTIQ_OK;
    } catch (...) {
        return TORUSTIQ_ERROR;
    }
}

void Stop(void* handle) {
    static_cast<HelloInput*>(handle)->stopped.store(true);
}

void Destroy(void* handle) {
    delete static_cast<HelloInput*>(handle);
}

const torustiq_plugin_info kInfo = {TORUSTIQ_SDK_ABI_VERSION, kPluginId};

} // namespace

extern "C" {

TORUSTIQ_PLUGIN_EXPORT const torustiq_plugin_info* torustiq_plugin_get_info(void) {
    return &kInfo;
}

TORUSTIQ_PLUGIN_EXPORT int32_t torustiq_plugin_init(const torustiq_registrar* registrar) {
    torustiq_input_vtable vtable{};
    vtable.abi_version = TORUSTIQ_SDK_ABI_VERSION;
    vtable.name = kPluginId;
    vtable.create = Create;
    vtable.run = Run;
    vtable.stop = Stop;
    vtable.destroy = Destroy;
    return registrar->register_input(registrar->context, &vtable);
}

} // extern "C"
