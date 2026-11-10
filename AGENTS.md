# AGENTS.md

Sample plugins for Torustiq (`../torustiq`), using only the C API from `../torustiq-sdk` (`torustiq/sdk.h`).
- Layout: `src/plugins/{inputs,outputs,transformers}/<name>/<name>.cpp`; one library per plugin, each folder with its own `CMakeLists.txt` and `<name>_test.cpp` (auto-discovered by the root).
- No dependencies besides the SDK, except cpp-httplib (header-only, FetchContent) for `http_server_input`. C++20, Google style (`.clang-format`), warning-free (`-Wall -Wextra -Wpedantic`).
- Never let exceptions cross the C boundary; copy data received in callbacks.
- Tests use GoogleTest (as in the host). Each test links exactly one plugin (they export the same C symbols).
- Tests must not start servers or block: cover config validation and early-return paths only.
- `examples/` — sample pipelines that use these plugins (the host keeps only builtin-plugin examples).
