`action_for_request_body`
----------

**Type:** `string`

**Value:** `read | discard | default`

**Attribute:** Optional

**Parent:** [handler](handler.md)

The action of `handler` for request body in nginx module. Details of value:  

- `read`: read request body

- `discard`: discard request body

- `default`: no action for request body

Please set value according to requirement of specific business.  

**If this field is ignored, it will be interpreted as `default`**  

[Previous](../ngx_wizard.md)

[Home](../../index.md)