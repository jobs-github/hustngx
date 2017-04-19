Case: hustdict
--

### Introduction ###

`hustdict` is a `key-value` cache module developed based on `hustngx` used for testing of [`lib_ngx_shm_dict`](../advanced/lib_hustngx/core_module.md).  

### Project configuration ###

Path: `hustngx/samples/schema/hustdict.json`

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

Run command:  

    python hustngx.py nginx-1.12.0.tar.gz samples/schema/hustdict.json

It will generate the project template: `hustngx/samples/schema/hustdict`

In subsequent development, you can use the text comparison tool (for example: `BeyondCompare`) to synchronize the generated new code into the project directory.  

### Nginx configuration ###

Path: `hustngx/samples/hustdict/nginx/conf/nginx.json`

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

Run command:  

    python genconf.py

It will generate `nginx.conf`.  

### Summary on interfaces ###
The types of value supported by `hustdict` include numeric, string and binary. Specific reference: [types of value](hustdict/types.md)  

The following is the http interfaces provided by `hustdict`:  

* [set](hustdict/set.md)
* [get](hustdict/get.md)
* [del](hustdict/del.md)
* [flush_all](hustdict/flush_all.md)
* [flush_expired](hustdict/flush_expired.md)
* [keys](hustdict/keys.md)
* [get_keys](hustdict/get_keys.md)
* [incr](hustdict/incr.md)
* [decr](hustdict/decr.md)

### Test script ###

* `autotest.py`  
Unit test script for `hustdict`. Path: `hustngx/samples/hustdict/nginx/autotest.py`  

* `mutitest.py`  
Stress test script for `hustdict`. Path：`hustngx/samples/hustdict/nginx/mutitest.py`  

* `mutikill.sh`  
To kill processes of stress test script for `hustdict`. Path: `hustngx/samples/hustdict/nginx/mutikill.sh`  

[Previous](index.md)

[Home](../index.md)