`scp_cmd`
----------

**类型:** `object`

**值:** `["local": <file-url>, "remote": <file-url>]`

**属性:** 参考 [`scp_cmds`](scp_cmds.md)

**父节点:** [`scp_cmds`](scp_cmds.md)

`scp` 命令拷贝的文件描述。`local` 代表本地的文件，`remote` 代表远程路径。

该命令会将 `local` 所定义的 `<file-url>` 拷贝到 `remote` 所指定的路径。

**目前仅支持单文件拷贝，如果需要拷贝文件夹，请打包成单个文件再拷贝。**

[回上页](../deploygen.md)

[回首页](../../index.md)