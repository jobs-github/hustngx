`sequential_subrequests`
----------

**类型:** `object`

**值:** `{ "<key-str>": <val>, "<key-str>": <val>, ... , "<key-str>": <val> }`

**属性:** 可选

**父节点:** [upstream](upstream.md)

如果配置了该字段，`hustngx` 会自动生成 `sequential subrequests`（串行子请求） 的代码模板。nginx 模块会通过 `proxy` 所定义的规则将 `http` 请求转发给 `backend` 节点。

其中 `<key-str>` 包含如下字段：

- [`gen_post_subrequest`](gen_post_subrequest.md)

- [`use_round_robin`](use_round_robin.md)

- [`subrequests`](subrequests.md)

[上一级](../ngx_wizard.md)

[根目录](../../index.md)