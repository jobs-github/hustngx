nginx 配置文件生成器
--

`genconf` 用于自动生成 `nginx.conf` 文件，其输入参数通过配置文件设定。

工具路径：`hustngx/lib_hustngx/nginx/conf/genconf.py`

使用范例：

    usage sample:
        python genconf.py nginx.json

以下是一个完整的配置文件，包含 `genconf` 所支持的所有字段：

    {
        "module": "hustmqha",
        "worker_connections": 1048576,
        "listen": 8080,
        "keepalive_timeout": 540,
        "keepalive": 32768,
        "http_basic_auth_file": "/data/hustmqha/conf/htpasswd",
        "nginx_root": "/data/hustmqha/html",
        "auth_filter": ["test"],
        "local_cmds": ["getconf", "test"],
        "main_conf": 
        [
            ["long_polling_timeout", "180s"],
            ["status_cache", "off"],
            ["keepalive_cache_size", 128],
            ["autost_uri", "/hustmq/stat_all"],
            ["fetch_buffer_size", "64m"],
            ["autost_interval", "200ms"]
        ],
        "proxy":
        {
            "health_check": 
            [
                "check interval=5000 rise=1 fall=3 timeout=5000 type=http",
                "check_http_send \"GET /status.html HTTP/1.1\\r\\n\\r\\n\"",
                "check_http_expect_alive http_2xx"
            ],
            "auth": "aHVzdGRieGM6cWlob29odXN0eHJiY3pkYg==",
            "proxy_connect_timeout": "2s",
            "proxy_send_timeout": "60s",
            "proxy_read_timeout": "60s",
            "proxy_buffer_size": "64m",
            "backends": ["backend:8087"],
            "proxy_cmds": ["/backend/test"]
        }
    }

**其中必填的字段包括：**

- `module`： 模块名称

- `worker_connections`: `worker` 进程处理的最大连接数

- `listen`: 监听端口

- `local_cmds`: 模块支持的命令列表

**除此之外的其他字段都是可选的**。

其中 `local_cmds` 表示模块所支持的命令列表，每一条命令的含义等同于 [ngx_wizard 配置文件中所定义的 uri](../ngx_wizard/uri.md)

大部分可选字段的含义以及配置方法和 nginx 官方的配置文件的含义一致，包括：

- `keepalive_timeout`

- `keepalive`

- `proxy_connect_timeout`

- `proxy_send_timeout`

- `proxy_read_timeout`

- `proxy_buffer_size`

`health_check` 的配置可参考 [`nginx_upstream_check_module`](https://github.com/yaoweibin/nginx_upstream_check_module)

以下是自定义的可选字段：

[`http_basic_auth_file`](http_basic_auth_file.md)  
[`nginx_root`](nginx_root.md)  
[`auth_filter`](auth_filter.md)  
[`main_conf`](main_conf.md)  
[`proxy`](proxy.md)  
　　[`auth`](auth.md)  
　　[`backends`](backends.md)  
　　[`proxy_cmds`](proxy_cmds.md)  

[上一级](../lib_hustngx.md)

[根目录](../../index.md)