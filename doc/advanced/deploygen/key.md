`key`
----------

**类型:** `string`

**值:** `scp | ssh`

**属性:** 必须

**父节点:** 无

命令的类型，目前支持标准的 `scp` 和 `ssh` 命令。

- 如果定义为 `scp` ，则 [`value`](value.md) 只能是 [`scp_cmds`](scp_cmds.md)

- 如果定义为 `ssh` ，则 [`value`](value.md) 只能是 [`ssh_cmds`](ssh_cmds.md)

[上一级](../deploygen.md)

[根目录](../../index.md)