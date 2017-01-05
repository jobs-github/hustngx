`sequential_subrequests`
----------

**类型:** `object`

**值:** `true | false`

**属性:** 可选

**父节点:** [upstream](upstream.md)

如果配置了该字段，`hustngx` 会自动生成 `sequential subrequests`（串行子请求） 的代码模板。nginx 模块会通过 `proxy` 所定义的规则将 `http` 请求转发给 `backend` 节点。

[上一级](../ngx_wizard.md)

[根目录](../../index.md)