includes
----------

**Type:** `array`

**Value:** `ngx_shm_dict | ngx_http_fetch`

**Attribute:** Optional

**Parent:** None

To import built-in modules of `lib_hustngx`. The following modules are currently supported:  

* `ngx_shm_dict`
* `ngx_http_fetch`

Using `ngx_shm_dict`, you can operate share memory by simple `key-value` interfaces, which could be found at [`lib_ngx_shm_dict`](../lib_hustngx/core_module.md).  

Using `ngx_http_fetch`, you can construct instance of `ngx_http_request_t` by `context-free` way. See interfaces from [`lib_ngx_http_fetch`](../lib_hustngx/http_module.md).  

[Previous](../ngx_wizard.md)

[Home](../../index.md)