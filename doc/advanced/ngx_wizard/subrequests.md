`subrequests`
----------

**类型:** `object`

**值:** `{ "<key-str>": <val>, "<key-str>": <val>, ... , "<key-str>": <val> }`

**属性:** 可选

**父节点:** [sequential_subrequests](sequential_subrequests.md)

如果 `upstream` 使用了 `sequential subrequests` ( **多轮子请求** )，该字段可以用于定制 `sequential subrequests` 的行为。

其中 `<key-str>` 包含如下字段：

- [`use_subrequest_peer`](use_subrequest_peer.md)

- [`request_all_peers`](request_all_peers.md)

**如果忽略该字段，则不会生成 `sequential subrequests` 相关的代码**。

[上一级](../ngx_wizard.md)

[根目录](../../index.md)