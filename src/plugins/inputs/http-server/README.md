# http_server_input

Input plugin (ID: `http_server_input`). Runs an HTTP server; every POST request (to any path) becomes a pipeline
message. Built on [cpp-httplib](https://github.com/yhirose/cpp-httplib).

## Configuration
| Key | Type | Required | Default | Description |
|-----|------|----------|---------|-------------|
| `host` | string | no | `127.0.0.1` | Address to bind to |
| `port` | integer (1-65535) | yes | — | Port to listen on |

## Message mapping
- payload: the request body
- headers: the HTTP request headers

## Lifecycle
The server stops gracefully when the host stops the input (e.g. on SIGINT/SIGTERM).

## Example
```yaml
type: http_server_input
config:
  host: 127.0.0.1
  port: 8080
```
See `examples/http_echo.yaml`. Try it with:
```sh
curl -X POST -H 'X-Demo: 1' -d hello http://127.0.0.1:8080/
```
