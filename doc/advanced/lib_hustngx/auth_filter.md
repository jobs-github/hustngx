`auth_filter`
----------

**类型:** `array`

**值:** `["<localcmd>", "<localcmd>", ... , "<localcmd>"]`

**范例:** `"auth_filter": ["test"]`

**父节点:** 无

`local_cmds` 中关闭 `http_basic_auth_module` 验证的命令列表。

例如，你需要 `test` 无需验证即可访问 nginx， 则可以将它添加到 `auth_filter` 中，最终生成的 `nginx.conf` 该字段的内容如下：

    location /test {
        hustmqha;
        #http_basic_auth_file /data/hustmqha/conf/htpasswd;
    }

可以看到， `http_basic_auth_file` 被注释掉了。

[回上页](genconf.md)

[回首页](../../index.md)