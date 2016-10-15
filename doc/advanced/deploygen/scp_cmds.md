`scp_cmds`
----------

**类型:** `array`

**值:** `[<scp_cmd>, <scp_cmd>, ... , <scp_cmd>]`

**属性:** [`key`](key.md) 如果定义为 `scp` ，则 [`value`](value.md) 只能是 [`scp_cmds`](scp_cmds.md)

**父节点:** 无

需要执行 `scp` 命令的文件列表（从本地拷贝到远程），其中每一个子项都是 [scp_cmd](scp_cmd.md)

[上一级](../deploygen.md)

[根目录](../../index.md)