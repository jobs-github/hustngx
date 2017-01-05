`proxy`
----------

**类型:** `object`

**值:** `{ "<key>": <val>, "<key>": <val>, ... , "<key>": <val> }`

**范例:** [参考这里](genconf.md)

**父节点:** 无

`http_basic_auth_module` 所使用的密钥文件路径。密钥文件使用明文存储用户名密码，例如：

    jobs:p@ssword

其中 `<key>` 包含如下自定义的值：

- [auth](auth.md)

- [backends](backends.md)

- [proxy_cmds](proxy_cmds.md)

[回上页](genconf.md)

[回首页](../../index.md)