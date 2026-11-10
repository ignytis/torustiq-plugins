# AGENTS.md

Sample plugins for Torustiq (`../torustiq`), using only the C API from `../torustiq-sdk` (`torustiq/sdk.h`).
- Layout: `src/plugins/{inputs,outputs,transformers}/<name>/<name>.cpp`; one library per plugin.
- No dependencies besides the SDK. C++20, Google style (`.clang-format`), warning-free (`-Wall -Wextra -Wpedantic`).
- Never let exceptions cross the C boundary; copy data received in callbacks.
- Tests use GoogleTest (as in the host). Each test links exactly one plugin (they export the same C symbols).
