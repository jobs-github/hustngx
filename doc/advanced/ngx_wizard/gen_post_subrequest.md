`gen_post_subrequest`
----------

**类型:** `bool`

**值:** `true | false`

**属性:** 可选

**父节点:** [sequential_subrequests](sequential_subrequests.md)

是否生成 `subrequest` 处理完毕之后的回调函数。

如果配置为 `false` , 或者忽略该字段，则使用 **公共的回调函数(默认行为)** ；

如果配置为 `true` ，则会在对应的 [handler](handler.md) 的代码中生成该文件 **专有的回调函数** ，例如：

    static ngx_int_t __post_subrequest_handler(
        ngx_http_request_t * r, void * data, ngx_int_t rc)
	{
	    hustmqha_autost_ctx_t * ctx = ngx_http_get_addon_module_ctx(r->parent);
	    if (ctx && NGX_HTTP_OK == r->headers_out.status)
	    {
	        ctx->base.response.len = ngx_http_get_buf_size(
	            &r->upstream->buffer);
	        ctx->base.response.data = r->upstream->buffer.pos;
	        // TODO: you can process the response from backend server here
	    }
	    return ngx_http_finish_subrequest(r);
	}


通常 nginx 模块对于不同命令的处理逻辑都是不一样的， 因此 **大多数情况下，你需要将该字段配置成 true , 除非你确定公共的回调函数可以 100% 满足你的需求** 。

[上一级](../ngx_wizard.md)

[根目录](../../index.md)