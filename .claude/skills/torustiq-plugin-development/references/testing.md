# Testing plugins

GoogleTest (as in the host), registered with CTest, 30 s timeout per test binary.

## Rules
- Each test binary links **exactly one plugin**: all plugins export the same C symbols (`torustiq_plugin_init`, ...).
  `torustiq_add_plugin_test(<id> <id>_test.cpp)` takes care of this by linking `<id>_objects`.
- Tests must not start servers, open sockets or block. Cover config validation and early-return paths only (for
  example `stop` before `run`). `http_server_input_test.cpp` is the model for network plugins.
- Call `destroy` on every handle that `create` returned.

## Helpers (`tests/test_util.h`, namespace `test`)
- `test::Registered` holds the registered `input`, `transform` and `output` vtables.
- `test::MakeRegistrar(&registered)` returns a `torustiq_registrar` that stores them.
- `test::Captured{body, headers (map), header_order}` and `test::Collect` - an emit callback that copies a message
  into a `std::vector<test::Captured>*` passed as `user_data`.
- Build config inline: `const torustiq_header items[] = {{"n", "3"}}; const torustiq_config config{items, 1};`
  and an empty one as `torustiq_config{nullptr, 0}`.

## Standard cases (see `assets/test.cpp.tmpl`)
1. **Info and registration**: `torustiq_plugin_get_info()->id`, `abi_version == TORUSTIQ_SDK_ABI_VERSION`,
   `vtable().name` equal to the ID.
2. **Happy path** through the vtable: inputs - `run` with `test::Collect`, assert bodies/headers; transformers -
   `process` with a crafted `torustiq_message`, assert what is emitted (and that headers pass through if intended);
   outputs - `write`, with `testing::internal::CaptureStdout()` / `GetCapturedStdout()` for stdout.
3. **Config validation** (when the plugin has config): every bad value returns `TORUSTIQ_ERROR`, including missing
   key and an empty config. Typical bad values: `"0"`, `"-1"`, `"abc"`, `""`, `"1.5"`, an overflowing number.
4. **Independent handles**: two `create` calls give different handles.
5. **Stop before run** (inputs): `stop` then `run` returns `TORUSTIQ_OK` and emits nothing. Call `stop` twice when
   the plugin owns a server, to cover idempotence.
6. **Null-safety**: a message with `body.data == nullptr` and `len == 0` (transformers and outputs).
7. **Early-return paths** that need no I/O, e.g. emit returning `TORUSTIQ_STOPPED` is mapped to `TORUSTIQ_OK` by an input.

Use a `TEST_F` fixture whose `SetUp` calls `torustiq_plugin_init` (as in `hello_input_test.cpp`); a plain `TEST` is
fine for trivial plugins.
