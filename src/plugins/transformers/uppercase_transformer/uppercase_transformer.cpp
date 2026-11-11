// Transformer plugin `uppercase_transformer`: converts the payload to upper case (ASCII).
// Headers are passed through unchanged.

#include <cstddef>
#include <cstdint>
#include <string>

#include "torustiq/sdk.h"

namespace {

constexpr const char* kPluginId = "uppercase_transformer";

// Bytes in, bytes out. Structured payloads are rejected when the pipeline is built.
int32_t Create(const torustiq_config*,
               const torustiq_payload_spec* input,
               torustiq_payload_spec* out_output,
               void** out_handle) {
    if (input->kind != TORUSTIQ_PAYLOAD_BYTES) {
        return TORUSTIQ_UNSUPPORTED_PAYLOAD;
    }
    out_output->kind = TORUSTIQ_PAYLOAD_BYTES;
    // The transformer is stateless, but every step still gets its own non-null handle.
    try {
        *out_handle = new int(0);
        return TORUSTIQ_OK;
    } catch (...) {
        return TORUSTIQ_ERROR;
    }
}

int32_t Process(void*, const torustiq_message* message, torustiq_emit_fn emit, void* user_data) {
    try {
        std::string body;
        const torustiq_bytes& in = message->payload.as.bytes;
        if (in.data != nullptr) {
            body.assign(reinterpret_cast<const char*>(in.data), in.len);
        }
        for (char& c : body) {
            if (c >= 'a' && c <= 'z') {
                c = static_cast<char>(c - 'a' + 'A');
            }
        }
        torustiq_message out = *message;
        out.payload =
            torustiq_value_from_bytes(reinterpret_cast<const uint8_t*>(body.data()), body.size());
        return emit(user_data, &out);
    } catch (...) {
        return TORUSTIQ_ERROR;
    }
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
    torustiq_transform_vtable vtable{};
    vtable.abi_version = TORUSTIQ_SDK_ABI_VERSION;
    vtable.name = kPluginId;
    vtable.create = Create;
    vtable.process = Process;
    vtable.destroy = Destroy;
    return registrar->register_transform(registrar->context, &vtable);
}

} // extern "C"
