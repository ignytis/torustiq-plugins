# Gotchas

1. **Stale SDK README.** `../torustiq-sdk/README.md` still sketches a `torustiq_plugin_register` export. The real
   contract is `torustiq_plugin_get_info` + `torustiq_plugin_init` (+ optional `torustiq_plugin_shutdown`) from
   `sdk.h`. A plugin built from the README sketch is skipped by the host ("no torustiq_plugin_get_info").
2. **Silent skips.** ABI mismatch, empty ID or a duplicate ID only log a warning; the failure shows up later as an
   unknown plugin type. IDs equal to built-ins (`stdin`, `stdout`, `directory`, `lua`) are ignored.
3. **ID triple.** Plugin ID = vtable `name` = YAML `type`. A mismatch means "not found" or a registration error.
4. **`get_info` must be pure** and return a pointer to a static: it runs on every library in the plugins dir.
5. **No `destroy` after a failed `create`.** Free partial allocations yourself before returning `TORUSTIQ_ERROR`.
6. **`stop` races.** It can arrive before `run`, during it, and repeatedly, from another thread. Atomic flag,
   idempotent, never free the handle there. A blocking `run` must be woken by `stop`.
7. **`emit` can block** (bounded queue). An input stuck in `emit` still needs `stop` to end it; check `stopped`
   between messages and treat non-OK from `emit` as "stop producing".
8. **Callback data is borrowed.** Copy messages, headers and config values you keep. Config pointers die when
   `create` returns.
9. **Config is all strings.** Parse and validate numbers/booleans with exception-free code; `std::stoi` and friends
   throw, and exceptions must not cross the C boundary.
10. **Null bodies.** `body.data` can be `nullptr`; guard before building a `std::string`/writing it.
11. **Duplicate header keys collapse** in the host (headers are stored in a map), so do not rely on repeated keys.
12. **Globals are shared** by every step that uses the library. `init` runs once per library, `create` per step:
    keep instance state in the handle.
13. **Tests link one plugin**: adding two plugins to one test binary causes duplicate-symbol errors.
14. **Visibility is hidden by default.** Only the `TORUSTIQ_PLUGIN_EXPORT` functions are exported; do not export more.
15. **Dev workflow.** After adding a plugin: re-run CMake (glob), add the ID to `scripts/link_dev.sh`, rebuild, then
    link. `link_dev.sh` exits with an error if a library has not been built.
