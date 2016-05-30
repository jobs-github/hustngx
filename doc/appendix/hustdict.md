案例：hustdict
--

### 简介 ###

`hustdict` 是基于 `hustngx` 开发的 `key-value` 缓存模块，同时也是 [`lib_ngx_shm_dict`](../advanced/lib_hustngx/core_module.md) 的测试套件。

### 工程配置文件 ###

文件路径：`hustngx/sample/hustdict.json`

    {
	    "module": "hustdict",
	    "includes": ["ngx_shm_dict"],
	    "main_conf":
	    [
	        ["string", "shm_name"],
	        ["int", "shm_size"]
	    ],
	    "handlers":
	    [
	        { "uri": "set", "action_for_request_body": "read", "methods": ["POST"] },
	        { "uri": "get" },
	        { "uri": "del" },
	        { "uri": "flush_all" },
	        { "uri": "flush_expired" },
	        { "uri": "keys" },
	        { "uri": "get_keys" },
	        { "uri": "incr" },
	        { "uri": "decr" }
	    ]
	}

运行如下命令：

    python hustngx.py nginx-1.9.4.tar.gz sample/hustdict.json

即可生成对应的工程模板：`hustngx/sample/hustdict`

之后的开发可以利用文本比对工具（例如：`BeyondCompare`），将生成的新代码同步至项目目录之中。

### nginx 配置文件 ###

文件路径：`hustngx/hustdict/nginx/conf/nginx.json`

    {
	    "module": "hustdict",
	    "worker_connections": 1048576,
	    "listen": 8085,
	    "keepalive_timeout": 540,
	    "keepalive": 32768,
	    "http_basic_auth_file": "/data/hustdict/conf/htpasswd",
	    "main_conf":
	    [
	        ["shm_name", "hust_dict_share_memory"],
	        ["shm_size", "268435456"]
	    ],
	    "auth_filter": [],
	    "local_cmds": ["set", "get", "del", "flush_all", "flush_expired", "keys", "get_keys", "incr", "decr"]
	}

运行如下命令：

    python genconf.py

即可生成对应的配置文件 `nginx.conf`

### 接口概要 ###
`hustdict` 支持的值类型包括数字、字符串、二进制三类，具体参考：[值类型列表](hustdict/types.md)

以下是 `hustdict` 提供的 http 接口：

* [set](hustdict/set.md)
* [get](hustdict/get.md)
* [del](hustdict/del.md)
* [flush_all](hustdict/flush_all.md)
* [flush_expired](hustdict/flush_expired.md)
* [keys](hustdict/keys.md)
* [get_keys](hustdict/get_keys.md)
* [incr](hustdict/incr.md)
* [decr](hustdict/decr.md)

### 测试脚本 ###

* `autotest.py`  
`hustdict` 单元测试脚本。

* `mutitest.py`  
`hustdict` 压力测试脚本。

* `mutikill.sh`  
用于杀掉 `hustdict` 压力测试脚本进程。

[上一级](index.md)

[根目录](../index.md)