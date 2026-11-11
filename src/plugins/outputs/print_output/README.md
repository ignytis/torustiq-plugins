# print_output

Output plugin (ID: `print_output`). Prints each message to standard output.

> **Note:** this plugin is for demo purposes only.

## Configuration
None.

## Output format
The payload is printed first, followed by the headers, one per line, formatted as `  key=value`.

## Example
```yaml
type: print_output
```
See `examples/simple_pipeline.yaml` and `examples/http_echo.yaml`.
