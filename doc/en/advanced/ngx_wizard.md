ngx_wizard
--

### Usage ###

    usage:
        python ngx_wizard.py [conf]
    sample:
        python ngx_wizard.py sample.json

### Configuration ###

A complete example of configuration file:  

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
                "uri": "test",
                "args":
                [
                    ["int", "int_val", "-1"],
                    ["bool", "bool_val", "true"],
                    ["size", "size_val", "1024"],
                    ["time", "time_val", "60"],
                    ["string", "str_val"]
                ]
            },
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

### Structure ###

[`module`](ngx_wizard/module.md)  
[`includes`](ngx_wizard/includes.md)  
[`main_conf`](ngx_wizard/main_conf.md)  
　　[`conf`](ngx_wizard/conf.md)  
[`handlers`](ngx_wizard/handlers.md)  
　　[`handler`](ngx_wizard/handler.md)  
　　　　[`uri`](ngx_wizard/uri.md)  
　　　　[`args`](ngx_wizard/args.md)  
　　　　　　[`arg`](ngx_wizard/arg.md)  
　　　　[`action_for_request_body`](ngx_wizard/action_for_request_body.md)  
　　　　[`methods`](ngx_wizard/methods.md)  
　　　　　　[`method`](ngx_wizard/method.md)  
　　　　[`upstream`](ngx_wizard/upstream.md)  
　　　　　　[`backend_uri`](ngx_wizard/backend_uri.md)  
　　　　　　[`parallel_subrequests`](ngx_wizard/parallel_subrequests.md)  
　　　　　　[`sequential_subrequests`](ngx_wizard/sequential_subrequests.md)  

**Limitation:**

* Required tags:  

	*  [`module`](ngx_wizard/module.md)

	*  [`handlers`](ngx_wizard/handlers.md)

	*  [`uri`](ngx_wizard/uri.md)

* [`parallel_subrequests`](ngx_wizard/parallel_subrequests.md)  and [`sequential_subrequests`](ngx_wizard/sequential_subrequests.md) **could not coexist**, please select one to configure [`upstream`](ngx_wizard/upstream.md).  

[Previous](index.md)

[Home](../index.md)