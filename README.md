# torustiq-plugins

Sample dynamic plugins for [Torustiq](../torustiq), dependency-free apart from the SDK headers.

| Plugin ID | Kind | Behaviour |
|-----------|------|-----------|
| `hello_input` | input | `config.n` (positive integer): emits `n` messages `hello, {i}!`, header `loop_number={i}` |
| `uppercase_transformer` | transformer | upper-cases the payload (ASCII), keeps headers |
| `print_output` | output | prints the payload, then headers as `  key=value` lines |

The plugin ID is the `type` used in a pipeline definition.

## Build, test, try
```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFETCHCONTENT_SOURCE_DIR_TORUSTIQ_SDK=$PWD/../torustiq-sdk
cmake --build build && ctest --test-dir build --output-on-failure
scripts/link_dev.sh            # symlinks the libraries into ../torustiq/dev/torustiq_home/plugins
../torustiq/build/torustiq ../torustiq/examples/plugins.yaml
```

## Writing a plugin
Export `torustiq_plugin_get_info` (ID + ABI), `torustiq_plugin_init` (called once; register vtables named
after the ID) and optionally `torustiq_plugin_shutdown`. See `torustiq/sdk.h` and `src/plugins/`.
New plugin: add `src/plugins/<kind>/<name>/<name>.cpp`, a `torustiq_add_plugin` line in `CMakeLists.txt`,
a test in `tests/`, and the name in `scripts/link_dev.sh`.
