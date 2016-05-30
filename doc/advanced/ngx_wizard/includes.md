includes
----------

**类型:** `array`

**值:** `ngx_http_peer_selector | ngx_shm_dict | ngx_http_fetch`

**属性:** 可选

**父节点:** 无

引入的内置 `lib_hustngx` 模块列表，目前支持如下模块：  

* `ngx_http_peer_selector`
* `ngx_shm_dict`
* `ngx_http_fetch`

加入 `ngx_shm_dict` 模块之后，可以以 `key-value` 的形式操作共享内存，相关的接口可参考 [`lib_ngx_shm_dict`](../lib_hustngx/core_module.md) 。

加入 `ngx_http_fetch` 模块之后，可以以 `context-free` 的方式创建 `ngx_http_request_t` ，相关的接口可参考 [`lib_ngx_http_fetch`](../lib_hustngx/http_module.md) 。

加入 `ngx_http_peer_selector` 之后，在进行 `subrequest` 的过程中，`backend` 机器节点的选择的 **控制权将属于客户端** （也就是你自己编写的 nginx 模块）。此时你可以自由控制 `backend` 节点的选择方法和顺序。

例如：

    static ngx_int_t __first_autost_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
	{
	    ......
	
	    ctx->peer = ngx_http_first_peer(peers->peer);
	    return ngx_http_gen_subrequest(backend_uri, r, ctx->peer,
	        &ctx->base, __post_subrequest_handler);
	}

    ngx_int_t hustmqha_autost_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
	{
	    hustmqha_autost_ctx_t * ctx = ngx_http_get_addon_module_ctx(r);
	    if (!ctx)
	    {
	        return __first_autost_handler(backend_uri, r);
	    }
	    if (NGX_HTTP_OK != r->headers_out.status)
	    {
	        ctx->peer = ngx_http_next_peer(ctx->peer);
	        return (ctx->peer) ? ngx_http_run_subrequest(r, &ctx->base, ctx->peer)
	            : ngx_http_send_response_imp(NGX_HTTP_NOT_FOUND, NULL, r);
	    }
	    // TODO: you decide the return value
	    return ngx_http_send_response_imp(NGX_HTTP_OK, &ctx->base.response, r);
	}

注意代码中的如下两处调用：

    ngx_http_gen_subrequest(backend_uri, r, ctx->peer, &ctx->base, __post_subrequest_handler);
    ngx_http_run_subrequest(r, &ctx->base, ctx->peer);

其中的 `ctx->peer` 将会直接传递给 `upstream` 模块，作为请求的转发节点。

**如果忽略该字段，则不会引入任何内置模块，建议加入 `ngx_http_peer_selector` 模块**。

[上一级](../ngx_wizard.md)

[根目录](../../index.md)