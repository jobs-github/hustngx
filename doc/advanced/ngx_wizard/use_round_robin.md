`use_round_robin`
----------

**类型:** `bool`

**值:** `true | false`

**属性:** 可选

**父节点:** [sequential_subrequests](sequential_subrequests.md)

发起 `subrequest` 的时候，是否使用 nginx 内置的 `ngx_http_upstream_round_robin` 所定义的算法来选择 `backend` 节点。

如果配置为 `false` , 或者忽略该字段，则 `backend` 节点选择的控制权属于客户端；

如果配置为 `true` ，则 `backend` 节点选择的控制权属于 nginx 内核，此时客户端无法干预节点的选择。

**大多数情况下，你不需要配置该字段**。

`ngx_http_peer_selector` 是把 `backend` 节点选择的控制权交给客户端的 **唯一方法** 。

只有当你 100% 确定需要将节点选择的控制权交给 nginx 内核，才需要将其配置为 `true` 。

[上一级](../ngx_wizard.md)

[根目录](../../index.md)