// HTTP server input: every POST request is emitted as one message (body = payload,
// HTTP headers = message headers). Config: `host` (default 127.0.0.1), `port` (1..65535).
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <httplib.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "torustiq/sdk.h"

namespace {

constexpr const char* kPluginId = "http_server_input";
constexpr const char* kDefaultHost = "127.0.0.1";
constexpr size_t kMaxPayloadBytes = 16 * 1024 * 1024;
constexpr int kIoTimeoutSeconds = 5;
constexpr auto kStopPollInterval = std::chrono::milliseconds(50);

struct HttpServerInput {
    std::string host;
    int port = 0;
    httplib::Server server;
    std::atomic<bool> stopped{false};
    std::atomic<bool> failed{false};
    std::mutex emit_mutex; // Serializes emit() calls made from the server's worker threads.
    std::mutex wake_mutex;
    std::condition_variable wake;
    bool finished = false; // Guarded by wake_mutex.
};

// Returns 0 on invalid input. Exception-free decimal parser limited to the port range.
int ParsePort(const char* text) {
    if (text == nullptr || *text == '\0') {
        return 0;
    }
    int value = 0;
    for (const char* p = text; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return 0;
        }
        value = value * 10 + (*p - '0');
        if (value > 65535) {
            return 0;
        }
    }
    return value;
}

int32_t
Create(const torustiq_config* config, torustiq_payload_spec* out_output, void** out_handle) {
    try {
        const int port = ParsePort(torustiq_config_get(config, "port"));
        if (port == 0 || out_output == nullptr || out_handle == nullptr) {
            return TORUSTIQ_ERROR;
        }
        out_output->kind = TORUSTIQ_PAYLOAD_BYTES;
        const char* host = torustiq_config_get(config, "host");
        auto input = std::make_unique<HttpServerInput>();
        input->host = (host == nullptr || *host == '\0') ? kDefaultHost : host;
        input->port = port;
        input->server.set_read_timeout(kIoTimeoutSeconds);
        input->server.set_write_timeout(kIoTimeoutSeconds);
        input->server.set_payload_max_length(kMaxPayloadBytes);
        *out_handle = input.release();
        return TORUSTIQ_OK;
    } catch (...) {
        return TORUSTIQ_ERROR;
    }
}

void HandlePost(HttpServerInput* self,
                torustiq_emit_fn emit,
                void* user_data,
                const httplib::Request& req,
                httplib::Response& res) {
    try {
        std::vector<torustiq_header> headers;
        headers.reserve(req.headers.size());
        for (const auto& header : req.headers) {
            // The strings live in `req` for the duration of this call, so pointers stay valid.
            headers.push_back({header.first.c_str(), header.second.c_str()});
        }
        torustiq_message message{};
        message.payload = torustiq_value_from_bytes(
            reinterpret_cast<const uint8_t*>(req.body.data()), req.body.size());
        message.headers = headers.data();
        message.header_count = headers.size();

        int32_t status;
        {
            std::lock_guard<std::mutex> lock(self->emit_mutex);
            status = emit(user_data, &message);
        }
        if (status == TORUSTIQ_OK) {
            res.status = 200;
        } else if (status == TORUSTIQ_STOPPED) {
            // Downstream accepts no more data: refuse the request and shut the server down.
            res.status = 503;
            self->stopped.store(true);
        } else {
            res.status = 500;
            self->failed.store(true);
            self->stopped.store(true);
        }
    } catch (...) {
        res.status = 500;
        self->failed.store(true);
        self->stopped.store(true);
    }
}

int32_t Run(void* handle, torustiq_emit_fn emit, void* user_data) {
    auto* self = static_cast<HttpServerInput*>(handle);
    try {
        if (self->stopped.load()) {
            return TORUSTIQ_OK;
        }
        self->server.Post(
            R"(.*)", [self, emit, user_data](const httplib::Request& req, httplib::Response& res) {
                HandlePost(self, emit, user_data, req, res);
            });
        if (!self->server.bind_to_port(self->host, self->port)) {
            return TORUSTIQ_ERROR;
        }

        // httplib's stop() is a no-op until the server is listening, so a stop() that arrives
        // between bind and listen would be lost. The watcher keeps retrying until listen returns.
        std::thread watcher([self] {
            std::unique_lock<std::mutex> lock(self->wake_mutex);
            while (!self->finished) {
                if (self->stopped.load()) {
                    self->server.stop();
                }
                self->wake.wait_for(lock, kStopPollInterval);
            }
        });
        const bool listened = self->server.listen_after_bind();
        {
            std::lock_guard<std::mutex> lock(self->wake_mutex);
            self->finished = true;
        }
        self->wake.notify_all();
        watcher.join();

        if (self->failed.load()) {
            return TORUSTIQ_ERROR;
        }
        return (listened || self->stopped.load()) ? TORUSTIQ_OK : TORUSTIQ_ERROR;
    } catch (...) {
        return TORUSTIQ_ERROR;
    }
}

void Stop(void* handle) {
    auto* self = static_cast<HttpServerInput*>(handle);
    self->stopped.store(true);
    self->server.stop();
    self->wake.notify_all();
}

void Destroy(void* handle) {
    delete static_cast<HttpServerInput*>(handle);
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
