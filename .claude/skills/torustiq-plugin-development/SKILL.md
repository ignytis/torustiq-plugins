---
name: torustiq-plugin-development
description: Use when creating, changing, building, testing or documenting a Torustiq plugin (input, transformer or output) in this repo, or when touching code built on torustiq/sdk.h, plugin CMake, plugin unit tests, per-plugin READMEs or example pipelines in examples/.
---

# Torustiq plugin development

Plugins are shared libraries that talk to the Torustiq host through the C API in `../torustiq-sdk`
(`torustiq/sdk.h`). Three kinds exist: **input** (produces messages), **transformer** (message in, 0..n
messages out) and **output** (consumes messages).

## Workflow
1. Pick the kind and copy the matching template from `assets/` (see "Templates").
2. Fill in the placeholders, then implement the callbacks. Keep the exports and the vtable wiring as in the template.
3. Write `<id>_test.cpp` (see [references/testing.md](references/testing.md)).
4. Do the registration/docs steps from the checklist below.
5. Build, run tests, format (commands below).

## New-plugin checklist
1. Directory: `src/plugins/<inputs|transformers|outputs>/<name>/`. The folder may be kebab-case (existing
   `http-server`); the plugin ID, CMake target and file stems are snake_case (`http_server_input`).
2. Files in it: `<id>.cpp`, `<id>_test.cpp`, `CMakeLists.txt`, `README.md`. The root `CMakeLists.txt` discovers the
   folder automatically; do not edit it.
3. Declare third-party dependencies in the plugin's own `CMakeLists.txt` (`find_package` with a `FetchContent`
   fallback), not in the root. The SDK is the only dependency otherwise.
4. Root `README.md`: add a table row with a 1-2 sentence description and a link to the plugin's README. Details
   belong in the plugin README only.
5. `scripts/link_dev.sh`: add the plugin ID to the hardcoded `for name in ...` list.
6. Optional: `examples/<x>.yaml` using the plugin, mentioned in the root README's Examples section.
7. Plugins that only exist as samples carry `> **Note:** this plugin is for demo purposes only.` in their README.

## Build, test, format
```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFETCHCONTENT_SOURCE_DIR_TORUSTIQ_SDK=$PWD/../torustiq-sdk
cmake --build build && ctest --test-dir build --output-on-failure
clang-format -i <changed .cpp/.h files>      # .clang-format at the repo root
scripts/link_dev.sh                          # symlink libs into ../torustiq/dev/torustiq_home/plugins
../torustiq/build/torustiq examples/<x>.yaml
```
Re-run the first command after adding a plugin folder (the discovery glob is `CONFIGURE_DEPENDS`). The build must be
warning-free under `-Wall -Wextra -Wpedantic`.

## Hard rules
- Use only the SDK C API (`torustiq/sdk.h`); C++20, Google style.
- Never let an exception cross the C boundary: wrap every callback that can throw in `try { ... } catch (...) {
  return TORUSTIQ_ERROR; }`. Prefer exception-free parsing (no `std::stoi`) for config values.
- Data passed into a callback is valid only during that call: copy anything you keep (including config in `create`).
- Vtable `name` == plugin ID == the `type` used in pipeline YAML. Zero-initialise vtables (`vtable{}`) and set named fields.
- Per-instance state lives in the handle returned by `create`, never in globals. Every step gets a non-null handle.
- `stop` (inputs) must be idempotent and thread-safe and must not free the handle; only `destroy` does.
- Unit tests must not start servers or block; test binaries link exactly one plugin.

## Templates (`assets/`)
Placeholders: `{{ID}}` plugin ID (snake_case), `{{Class}}` state struct name, `{{KIND}}` `input`|`transform`|`output`.
- `input.cpp.tmpl`, `transformer.cpp.tmpl`, `output.cpp.tmpl` - plugin sources (from `hello_input`,
  `uppercase_transformer`, `print_output`).
- `test.cpp.tmpl` - GoogleTest fixture plus the standard cases.
- `CMakeLists.txt.tmpl` - plugin CMake, including the variant with a FetchContent dependency.
- `README.md.tmpl` - per-plugin README in the repo's format.
Strip the `.tmpl` suffix when copying.

## References (read when needed)
- [references/abi.md](references/abi.md) - structs, vtables, status codes, exported symbols, ownership. Read before writing callbacks.
- [references/lifecycle-threading.md](references/lifecycle-threading.md) - what the host calls when, threads,
  backpressure, what each return code does. Read for inputs that block/loop, `stop`, `flush`, multi-threaded plugins.
- [references/host-integration.md](references/host-integration.md) - plugin discovery, ID rules, pipeline YAML, how
  config reaches `create`, running a plugin in the host. Read when wiring examples or debugging "plugin not found".
- [references/testing.md](references/testing.md) - test helpers and required cases.
- [references/gotchas.md](references/gotchas.md) - pitfalls, including a stale SDK README. Skim before finishing.

When the SDK header and any prose disagree, `../torustiq-sdk/include/torustiq/sdk.h` wins.
