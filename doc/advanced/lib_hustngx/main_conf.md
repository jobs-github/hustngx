`main_conf`
----------

**类型:** `array`

**值:** `[ ["<key>", <value>], ["<key>", <value>], ... , ["<key>", <value>] ]`

**范例:** `"main_conf": [ ["test_num", 1], ["test_flag", "on"] ]`

**父节点:** 无

用于配置 `main_conf` 相关字段的值。

`<key>` 的名称请参考 [main_conf](../ngx_wizard/main_conf.md) 所定义的字段。前后需要保持一致。

`int` 对应的 nginx 数据类型为 `ngx_int_t`，配置样例：

    {
        "main_conf":
        [
            ["keepalive_cache_size", 128]
        ]
    }

`size` 对应的 nginx 数据类型为 `ssize_t`，配置样例：

    {
        "main_conf":
        [
            ["fetch_buffer_size", "64m"]
        ]
    }

`time` 对应的 nginx 数据类型为 `ngx_int_t`，配置样例：

    {
        "main_conf":
        [
            ["long_polling_timeout", "180s"],
            ["autost_interval", "200ms"]
        ]
    }

`bool` 对应的 nginx 数据类型为 `ngx_bool_t`，配置样例：

    {
        "main_conf":
        [
            ["status_cache", "off"]
        ]
    }

`string` 对应的 nginx 数据类型为 `ngx_str_t`，配置样例：

    {
        "main_conf":
        [
            ["autost_uri", "/hustmq/stat_all"]
        ]
    }

[上一级](genconf.md)

[回首页](../../index.md)