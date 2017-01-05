includes
----------

**类型:** `array`

**值:** `ngx_shm_dict | ngx_http_fetch`

**属性:** 可选

**父节点:** 无

引入的内置 `lib_hustngx` 模块列表，目前支持如下模块：  

* `ngx_shm_dict`
* `ngx_http_fetch`

加入 `ngx_shm_dict` 模块之后，可以以 `key-value` 的形式操作共享内存，相关的接口可参考 [`lib_ngx_shm_dict`](../lib_hustngx/core_module.md) 。

加入 `ngx_http_fetch` 模块之后，可以以 `context-free` 的方式创建 `ngx_http_request_t` ，相关的接口可参考 [`lib_ngx_http_fetch`](../lib_hustngx/http_module.md) 。

[上一级](../ngx_wizard.md)

[回首页](../../index.md)