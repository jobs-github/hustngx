`auth`
----------

**类型:** `string`

**值:** `<base64-encoded-user-str>`

**范例:** `"auth": "aHVzdGRieGM6cWlob29odXN0eHJiY3pkYg=="`

**父节点:** [proxy](proxy.md)

如果 nginx 在访问 `backend` 节点的时候需要进行 `http basic authentication`, 则该字段用于配置 `base64` 编码后的用户信息。

例如，假设访问 `backend` 节点的用户名和密码为：

    jobs:p@ssword

则该字段的值为：

    am9iczpwQHNzd29yZA==

[上一级](genconf.md)

[根目录](../../index.md)