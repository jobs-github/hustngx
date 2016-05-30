`action_for_request_body`
----------

**类型:** `string`

**值:** `read | discard | default`

**属性:** 可选

**父节点:** [handler](handler.md)

nginx 模块各个 `handler` 对于http请求的包体的处理方法。各个值的含义如下：

- `read`: 读取请求包体

- `discard`: 丢弃请求包体

- `default`: 对请求包体不做任何处理

各个 `handler` 可以根据具体的业务逻辑配置所需要的值。

**如果忽略该字段，则按照 `default` 行为进行处理**。

[上一级](../ngx_wizard.md)

[根目录](../../index.md)