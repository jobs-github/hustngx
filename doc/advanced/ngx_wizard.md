ngx_wizard
--

### 使用范例 ###

    usage:
        python ngx_wizard.py [conf]
    sample:
        python ngx_wizard.py ngx_wizard.json

### 配置范例 ###

以下是一个完整的配置文件，包含 `ngx_wizard` 所支持的所有字段：

    {
        "module": "hustmqha",
        "includes": 
        [
            "ngx_shm_dict", 
            "ngx_http_fetch"
        ],
        "main_conf":
        [
            ["time", "long_polling_timeout"],
            ["bool", "status_cache"],
            ["int", "keepalive_cache_size"],
            ["string", "autost_uri"],
            ["size", "fetch_buffer_size"],
            ["time", "autost_interval"]
        ],
        "handlers":
        [
            
            {
                "uri": "put",
                "action_for_request_body": "read",
                "methods": ["PUT", "POST"],
                "upstream":
                {
                    "backend_uri": "/hustmq/put",
                    "sequential_subrequests": true
                }
            },
            {
                "uri": "get",
                "action_for_request_body": "default",
                "methods": ["GET"],
                "upstream":
                {
                    "backend_uri": "/hustmq/get",
                    "sequential_subrequests": true
                }
            },
            {
                "uri": "autost",
                "action_for_request_body": "default",
                "methods": ["GET"],
                "upstream":
                {
                    "backend_uri": "/hustmq/stat_all",
                    "parallel_subrequests": true
                }
            },
            {
                "uri": "stat_all",
                "action_for_request_body": "discard",
                "methods": ["GET"],
                "upstream": null
            }
        ]
    }

### 字段结构 ###

[`module`](ngx_wizard/module.md)  
[`includes`](ngx_wizard/includes.md)  
[`main_conf`](ngx_wizard/main_conf.md)  
　　[`conf`](ngx_wizard/conf.md)  
[`handlers`](ngx_wizard/handlers.md)  
　　[`handler`](ngx_wizard/handler.md)  
　　　　[`uri`](ngx_wizard/uri.md)  
　　　　[`action_for_request_body`](ngx_wizard/action_for_request_body.md)  
　　　　[`methods`](ngx_wizard/methods.md)  
　　　　　　[`method`](ngx_wizard/method.md)  
　　　　[`upstream`](ngx_wizard/upstream.md)  
　　　　　　[`backend_uri`](ngx_wizard/backend_uri.md)  
　　　　　　[`parallel_subrequests`](ngx_wizard/parallel_subrequests.md)  
　　　　　　[`sequential_subrequests`](ngx_wizard/sequential_subrequests.md)  

**字段约束：**

* 以下字段必填：

	*  [`module`](ngx_wizard/module.md)

	*  [`handlers`](ngx_wizard/handlers.md)

	*  [`uri`](ngx_wizard/uri.md)

* [`parallel_subrequests`](ngx_wizard/parallel_subrequests.md)  和 [`sequential_subrequests`](ngx_wizard/sequential_subrequests.md) **不能共存** ，只能配置其中一种。  

[上一级](index.md)

[根目录](../index.md)