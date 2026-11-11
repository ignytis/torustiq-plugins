# Lifecycle and threading (host behaviour)

Based on `../torustiq/src/core/pipeline.cpp`, `plugin_loader.cpp`, `plugin_registry.cpp` and `main.cpp`.

## Order of calls
1. Library loaded (`dlopen`) only if its ID is used as a pipeline `type`; `torustiq_plugin_init` runs **once per
   library**; it must return `TORUSTIQ_OK` or the pipeline never starts.
2. `create` runs **per pipeline step** (several steps may use the same plugin) and negotiates the payload kind:
   an input declares what it emits, a transformer gets the upstream kind and declares its own, an output gets the
   upstream kind. Return `TORUSTIQ_UNSUPPORTED_PAYLOAD` for a kind you cannot handle.
   If `create` returns non-OK the host nulls the handle and does **not** call `destroy`: free whatever you already
   allocated before returning an error.
3. Each node runs on its **own thread**: one instance's `run`/`process`/`write` is called from a single thread,
   different steps run concurrently.
4. `destroy(handle)` per step, then `torustiq_plugin_shutdown()` and `dlclose` in reverse load order.

## Return codes
| Callback | OK | STOPPED | ERROR / other |
|----------|----|---------|---------------|
| input `run` | stream ends normally | treated as OK | pipeline fails |
| transform `process` | continue | tolerated | pipeline fails |
| output `write` | continue | - | pipeline fails (must return OK) |
| `emit` (what you get back) | accepted | downstream is done: stop producing | pipeline is failing: stop |

Pattern in an input: `status = emit(...); if (status != OK) return status == STOPPED ? OK : status;`.

## Input specifics
- `run` blocks and loops, calling `emit` per message; it returns when exhausted, stopped, or `emit` is non-OK.
- `stop` is called from a different thread (signal watcher or failure path), possibly **before** `run`, possibly
  **during** it, possibly **more than once**. Use `std::atomic<bool> stopped`, check it every iteration, and unblock
  any blocking call (socket, condition variable, sleep) from `stop`. `stop` must not free the handle.
- `emit` can block: queues between nodes are bounded (256 messages) so a slow consumer applies backpressure.
- If `stop` is invoked before `run`, `run` should emit nothing and return OK.

## Output specifics
`flush` is called once when the stream ends (if non-NULL). `write` must not assume `flush` runs between messages.

## Fan-out and wiring
A message emitted by one node is copied to every consumer. End of stream propagates when all producers of a node
have finished, so downstream nodes drain their queues and finish.

## Multi-threaded plugins (e.g. `http_server_input`)
- `emit` from several worker threads needs an external mutex (see `emit_mutex` in `http_server_input.cpp`).
- Anything that cannot be interrupted directly (library stop is a no-op before listen) needs a watcher thread that
  polls `stopped`; join all threads before `run` returns.
- Map `TORUSTIQ_STOPPED` from `emit` to a clean shutdown, and any other non-OK to an error return from `run`.
