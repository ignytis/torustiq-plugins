# uppercase_transformer

Transformer plugin (ID: `uppercase_transformer`). Upper-cases the message payload and passes headers through
unchanged.

> **Note:** this plugin is for demo purposes only.

## Configuration
None.

## Behaviour
- payload: converted to upper case (ASCII characters only)
- headers: kept as is

## Example
```yaml
type: uppercase_transformer
```
See `examples/simple_pipeline.yaml`, where two of these are chained.
