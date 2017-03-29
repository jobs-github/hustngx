nginx configuration generator
--

`genconf` is used to generate `nginx.conf` with arguments configured in file.  

Tool Path: `hustngx/lib_hustngx/nginx/conf/genconf.py`

Usage:  

    usage sample:
        python genconf.py nginx.json

A complete example of configuration file:  

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

**Required fields:**

- `module`: module name

- `worker_connections`: the maximum number of simultaneous connections that can be opened by a worker process.

- `listen`: listen port

- `local_cmds`: commands supported by module

**Other fields are all optional**  

`local_cmds` defines the commands supported by module, each command is equivalent to [uri](../ngx_wizard/uri.md) included by configuration of `ngx_wizard`.  

Most of optional fields could be found at nginx official configuration website, including:  

- `keepalive_timeout`

- `keepalive`

- `proxy_connect_timeout`

- `proxy_send_timeout`

- `proxy_read_timeout`

- `proxy_buffer_size`

Configuration on `health_check` : [`nginx_upstream_check_module`](https://github.com/yaoweibin/nginx_upstream_check_module)

Customized optional fields are as below:  

[`http_basic_auth_file`](http_basic_auth_file.md)  
[`nginx_root`](nginx_root.md)  
[`auth_filter`](auth_filter.md)  
[`main_conf`](main_conf.md)  
[`proxy`](proxy.md)  
　　[`auth`](auth.md)  
　　[`backends`](backends.md)  
　　[`proxy_cmds`](proxy_cmds.md)  

[Previous](../lib_hustngx.md)

[Home](../../index.md)