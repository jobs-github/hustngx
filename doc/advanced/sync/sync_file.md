`sync_file`
----------

**类型:** `object`

**值:** `["src": <path>, "des": <path>]`

**属性:** 必须

**父节点:** [sync_files](sync_files.md)

描述要同步的文件项目，其中 `src` 代表源文件信息， `des` 代表目标文件信息。两者的值都是 [path](path.md) 。

`sync` 将会把 `src` 定义的 [path](path.md) 拷贝到 `des` 定义的 [path](path.md) 中。

[上一级](../sync.md)

[回首页](../../index.md)