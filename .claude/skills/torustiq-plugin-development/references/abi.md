# Plugin ABI (`torustiq/sdk.h`, ABI version 1)

Source of truth: `../torustiq-sdk/include/torustiq/sdk.h`. A layout or signature change there bumps
`TORUSTIQ_SDK_ABI_VERSION` (currently `1u`). C++ helper: `torustiq/sdk.hpp` (`torustiq::ConfigGet(cfg, key, default)`).

## Status codes
| Constant | Value | Meaning |
|----------|-------|---------|
| `TORUSTIQ_OK` | 0 | success / continue |
| `TORUSTIQ_ERROR` | -1 | failure; the host fails the pipeline |
| `TORUSTIQ_STOPPED` | 2 | returned by `emit`: downstream wants no more data |

Errors are status codes only; the plugin cannot pass a message to the host. Print diagnostics to stderr.

## Data structs
```c
typedef struct { const uint8_t* data; size_t len; } torustiq_bytes;        // body may contain NUL bytes
typedef struct { const char* key; const char* value; } torustiq_header;     // also used for config items
typedef struct { torustiq_bytes body; const torustiq_header* headers; size_t header_count; } torustiq_message;
typedef struct { const torustiq_header* items; size_t count; } torustiq_config;
const char* torustiq_config_get(const torustiq_config*, const char* key);    // NULL if missing
typedef int32_t (*torustiq_emit_fn)(void* user_data, const torustiq_message*);
```
All strings are NUL-terminated UTF-8. `body.data` may be null for an empty body: check before dereferencing.

## Vtables
All have `uint32_t abi_version` (set to `TORUSTIQ_SDK_ABI_VERSION`) and `const char* name` (the plugin ID).
```c
input:     int32_t create(const torustiq_config*, void** out_handle);
           int32_t run(void* h, torustiq_emit_fn emit, void* user_data);   // blocking loop
           void    stop(void* h);                                           // optional (may be NULL)
           void    destroy(void* h);
transform: create; int32_t process(void* h, const torustiq_message*, torustiq_emit_fn, void* user_data); destroy
output:    create; int32_t write(void* h, const torustiq_message*); int32_t flush(void* h) /*optional*/; destroy
```
The host validates at registration: `abi_version` matches, `name` non-empty, `create`/`destroy` non-null, and `run`
(input) / `process` (transform) / `write` (output) non-null. Duplicate names within one kind are rejected. The host
copies the vtable, so a local `vtable{}` in `torustiq_plugin_init` is fine.

## Exported symbols
All `extern "C"` and marked `TORUSTIQ_PLUGIN_EXPORT` (symbol visibility is hidden by default):
```c
const torustiq_plugin_info* torustiq_plugin_get_info(void);   // mandatory: {abi_version, id}; return a static
int32_t torustiq_plugin_init(const torustiq_registrar*);      // mandatory: register vtables, once per library
void    torustiq_plugin_shutdown(void);                       // optional: runs before dlclose
```
`torustiq_registrar` has `context` plus `register_input/transform/output(context, &vtable)`. One library may register
several kinds under the same ID.

## Ownership
Everything passed into a callback (message, headers, config, strings) belongs to the caller and is valid only for the
duration of that call. Copy what you keep. Strings you pass to `emit` only need to live until `emit` returns (stack
locals are fine). A NULL header value becomes `""` in the host and a NULL header key is skipped; duplicate header keys
collapse (the host stores headers in a `std::map`).
