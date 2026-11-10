# torustiq-plugins

Sample dynamic plugins for [Torustiq](../torustiq), built on the SDK headers (plus cpp-httplib for the HTTP server input).

| Plugin ID | Kind | Behaviour |
|-----------|------|-----------|
| `hello_input` | input | `config.n` (positive integer): emits `n` messages `hello, {i}!`, header `loop_number={i}` |
| `http_server_input` | input | HTTP server; each POST (any path) is a message: body = payload, HTTP headers = message headers. `config.host` (default `127.0.0.1`), `config.port` (required, 1-65535). Stops gracefully when the host stops the input (SIGINT/SIGTERM) |
| `uppercase_transformer` | transformer | upper-cases the payload (ASCII), keeps headers |
| `print_output` | output | prints the payload, then headers as `  key=value` lines |

The plugin ID is the `type` used in a pipeline definition.

## Build, test, try
```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFETCHCONTENT_SOURCE_DIR_TORUSTIQ_SDK=$PWD/../torustiq-sdk
cmake --build build && ctest --test-dir build --output-on-failure
scripts/link_dev.sh            # symlinks the libraries into ../torustiq/dev/torustiq_home/plugins
../torustiq/build/torustiq examples/simple_pipeline.yaml
```

## Examples
Pipelines in `examples/` (run after `scripts/link_dev.sh`, from this directory):
- `simple_pipeline.yaml` — `hello_input` -> two `uppercase_transformer` steps -> `print_output`.
- `http_echo.yaml` — `http_server_input` on `127.0.0.1:8080` -> `print_output`, which prints the payload and all
  headers. Try `curl -X POST -H 'X-Demo: 1' -d hello http://127.0.0.1:8080/`; stop with Ctrl+C.

```sh
../torustiq/build/torustiq examples/http_echo.yaml
```

## Writing a plugin
Export `torustiq_plugin_get_info` (ID + ABI), `torustiq_plugin_init` (called once; register vtables named
after the ID) and optionally `torustiq_plugin_shutdown`. See `torustiq/sdk.h` and `src/plugins/`.
New plugin: create `src/plugins/<kind>/<name>/` with `<name>.cpp`, `<name>_test.cpp` and a `CMakeLists.txt`
(declare its own dependencies, call `torustiq_add_plugin` and `torustiq_add_plugin_test`; see
`cmake/TorustiqPlugin.cmake`). The root `CMakeLists.txt` discovers it automatically; also add the name to
`scripts/link_dev.sh`.
