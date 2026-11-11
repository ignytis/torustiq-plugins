# hello_input

Input plugin (ID: `hello_input`). Emits a fixed number of greeting messages, then finishes.

> **Note:** this plugin is for demo purposes only.

## Configuration
| Key | Type | Required | Description |
|-----|------|----------|-------------|
| `n` | positive integer | yes | Number of messages to emit |

## Output
For each `i` in `1..n` a message is emitted:
- payload: `hello, {i}!`
- header: `loop_number={i}`

## Example
```yaml
type: hello_input
config:
  n: 3
```
See `examples/simple_pipeline.yaml`.
