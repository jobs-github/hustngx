`upstream`
----------

**类型:** `object`

**值:** `{ "<key-str>": <val>, "<key-str>": <val>, ... , "<key-str>": <val> }`

**属性:** 可选

**父节点:** [handler](handler.md)

nginx 模块各个 `handler` 对 `upstream` 的支持的配置信息，该字段会自动生成 `subrequest` 的请求代码模板。

其中 `subrequest` 包括两种范式：[`parallel subrequests`](../parallel_subrequests.md) 和 [`sequential subrequests`](../sequential_subrequests.md)。  
前者在性能上更加出色，后者在配置上更加简单。

**如果忽略该字段，则不会生成和 `upstream` 相关的代码** 。

其中 `<key-str>` 包含如下字段：

* [`backend_uri`](backend_uri.md)

* [`parallel_subrequests`](parallel_subrequests.md)  

* [`sequential_subrequests`](sequential_subrequests.md)

[上一级](../ngx_wizard.md)

[回首页](../../index.md)