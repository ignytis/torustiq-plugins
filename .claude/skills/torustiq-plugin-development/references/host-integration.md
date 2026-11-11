# Host integration

Host repo: `../torustiq`. This is what determines whether a built plugin is found and how it is configured.

## Discovery and loading
- Plugins directory: `$TORUSTIQ_PLUGINS_DIR`, else `$TORUSTIQ_HOME/plugins`. In dev mode (default for Debug host
  builds) `TORUSTIQ_HOME` defaults to `../torustiq/dev/torustiq_home`, which is where `scripts/link_dev.sh` symlinks
  the built libraries.
- The scan is non-recursive and takes regular files (symlinks followed) ending in `.so` / `.dylib` / `.dll`. The file
  name does not matter (`lib<id>.so` is only a CMake convention): the ID comes from `torustiq_plugin_get_info()`.
- `get_info` is called on **every** library in the directory, so it must have no side effects.
- Skipped with a warning on stderr: missing symbols, NULL/empty ID, ABI mismatch, duplicate ID (first file wins).
  The pipeline then fails later with an "unknown ... type" style error, so check stderr first.
- The ID must equal the vtable `name` and the pipeline `type`. IDs that collide with built-ins (`stdin`,
  `directory`, `stdout`, `lua`) are ignored because built-ins win.

## Pipeline YAML
```yaml
name: optional
inputs:                      # required, at least one
  src: {type: hello_input, config: {n: 3}}
transforms:                  # optional
  up: {type: uppercase_transformer, inputs: [src]}
outputs:                     # required, at least one
  out: {type: print_output, inputs: [up]}
```
- `type` is mandatory; ids are unique across sections; `config` must be a map.
- `inputs:` on a transform/output takes a scalar or list and may only reference earlier ids (inputs, then earlier
  transforms). If omitted it defaults to the previous transform or the sole input. With several inputs it must be
  explicit.
- Examples in this repo: `examples/simple_pipeline.yaml`, `examples/http_echo.yaml` (start with header comments that
  give the run command).

## How config reaches `create`
Every config value becomes a **string**: nested maps get dotted keys (`auth.user`), sequences get indexed keys
(`list.0`, `list.1`), null becomes `""`. Items arrive sorted by key. There are no typed values, so validate and parse
numbers/booleans yourself and return `TORUSTIQ_ERROR` for bad input. Missing keys give `NULL` from
`torustiq_config_get`; treat `NULL` and `""` alike for optional keys.

## Running in the host
```sh
scripts/link_dev.sh              # needs the plugin built; ID must be in the script's name list
../torustiq/build/torustiq examples/<x>.yaml
```
Exit code 1 and `error: <msg>` on stderr mean the pipeline failed. SIGINT/SIGTERM call `stop` on inputs.
