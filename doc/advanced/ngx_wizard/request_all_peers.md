`request_all_peers`
----------

**类型:** `bool`

**值:** `true | false`

**属性:** 可选

**父节点:** [subrequests](subrequests.md)

执行 `sequential subrequests` 的时候，是否将命令转发给所有的 `backend` 节点。

如果配置为 `false` , 或者忽略该字段，`handler` 在转发命令的时候，只要有一台 `backend` 节点处理成功，就会返回结果给客户端， 例如：

	ngx_int_t hustmqha_put_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
	{
	    hustmqha_put_ctx_t * ctx = ngx_http_get_addon_module_ctx(r);
	    if (!ctx)
	    {
	        return __first_put_handler(backend_uri, r);
	    }
	    if (NGX_HTTP_OK != r->headers_out.status)
	    {
	        ++ctx->count;
	        return (ctx->count < ctx->peer_count) ? ngx_http_run_subrequest(r, &ctx->base, NULL)
	            : ngx_http_send_response_imp(NGX_HTTP_NOT_FOUND, NULL, r);
	    }
	    // TODO: you decide the return value
	    return ngx_http_send_response_imp(NGX_HTTP_OK, &ctx->base.response, r);
	}

请注意这一行代码：

    if (NGX_HTTP_OK != r->headers_out.status)

可以看到，如果 `subrequest` 返回的状态码是200，则 `handler` 主体不会继续转发命令给下一个节点，而是将处理结果直接返回给客户端。

如果配置为 `true` ，则 `handler` 会把命令转发给所有 `backend` 节点，例如：

    ngx_int_t hustmqha_pub_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
	{
	    hustmqha_pub_ctx_t * ctx = ngx_http_get_addon_module_ctx(r);
	    if (!ctx)
	    {
	        return __first_pub_handler(backend_uri, r);
	    }
	    ctx->subrequest_peer = ngx_http_get_next_peer(ctx->subrequest_peer);
	    if (ctx->subrequest_peer)
	    {
	        return ngx_http_run_subrequest(r, &ctx->base, ctx->subrequest_peer->peer);
	    }
	    // TODO: you decide the return value
	    return ngx_http_send_response_imp(NGX_HTTP_OK, &ctx->base.response, r);
	}

可以看到，这种模式下, `handler` 不会对上一轮 `subrequest` 返回的状态码进行判断。

因此，在进行 `sequential subrequests` 的时候：

- **如果你只需要把命令转发给某一台 `backend` 节点，则可以忽略该字段**，或者将之设置成 `false`；
- **如果你需要把命令转发给所有 `backend` 节点，则需要将该字段设置成 `true`** 。


**如果忽略该字段，则 `handler` 在转发命令的时候，只要有一台 `backend` 节点处理成功，就会返回结果给客户端**。

[上一级](../ngx_wizard.md)

[根目录](../../index.md)