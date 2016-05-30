`use_subrequest_peer`
----------

**类型:** `bool`

**值:** `true | false`

**属性:** 可选

**父节点:** [subrequests](subrequests.md)

是否使用 `ngx_http_subrequest_peer_t` 所定义的类型来缓存 `subrequest` 节点。

**如果 [`use_round_robin`](use_round_robin.md) 被配置成 `true`，则该字段失效。**

如果配置为 `false` , 或者忽略该字段，则使用 nginx 内置的 `ngx_http_upstream_rr_peer_t` 作为节点的缓存类型，例如：

    typedef struct
	{
	    ngx_http_subrequest_ctx_t base;
	    ngx_http_upstream_rr_peer_t * peer;
	    // TODO: add your fields here
	} hustmqha_autost_ctx_t;

如果配置为 `true` ，则使用 `lib_hustngx` 所定义的 `ngx_http_subrequest_peer_t` 作为节点的缓存类型，例如：

    typedef struct
	{
	    ngx_http_subrequest_ctx_t base;
	    ngx_http_subrequest_peer_t * subrequest_peer;
	    // TODO: add your fields here
	} hustmqha_pub_ctx_t;

nginx 在读取配置文件的 `upstream` 字段之后，会将 `backend` 机器列表解析成内置的链表类型，也就是 `ngx_http_upstream_rr_peer_t` ，其顺序是固定的。

因此，在进行 `sequential subrequests` 的时候：

- **如果你只需要以固定的顺序来遍历 `backend` 节点列表，则可以忽略该字段**，或者将之设置成 `false`；
- **如果你遍历 `backend` 节点列表的顺序是不确定的，需要定制，则需要将该字段设置成 `true`** 。

更进一步，如果你将该字段设置成 `true`, 则需要在进行 `sequential subrequests` 之前构建链表，例如：

    static ngx_int_t __first_pub_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
	{
	    ......
	    ngx_http_subrequest_peer_t * peer = NULL;
	    // TODO: you can initialize the peer list here
	    // peer = ngx_http_init_peer_list(r->pool, ngx_http_get_backends());
	    if (!peer)
	    {
	        return NGX_ERROR;
	    }
	
	    hustmqha_pub_ctx_t * ctx = ngx_palloc(r->pool, sizeof(hustmqha_pub_ctx_t));
	    if (!ctx)
	    {
	        return NGX_ERROR;
	    }
	    memset(ctx, 0, sizeof(hustmqha_pub_ctx_t));
	    ngx_http_set_addon_module_ctx(r, ctx);
	    // TODO: you can initialize ctx here
	
	    ctx->subrequest_peer = ngx_http_get_first_peer(peer);
	
	    ......
	}

请注意被注释的一行代码：

    // peer = ngx_http_init_peer_list(r->pool, ngx_http_get_backends());

`ngx_http_init_peer_list` 是 `lib_hustngx` 所提供的函数，以默认的方式构建链表（其节点顺序和 `ngx_http_upstream_rr_peer_t` 所存储的链表顺序是一样的）。你可以参考这个函数的实现来自行决定如何构建链表。

**如果忽略该字段，则使用 nginx 内置的 `ngx_http_upstream_rr_peer_t` 作为节点的缓存类型，以固定的顺序来遍历 `backend` 机器列表**。

[上一级](../ngx_wizard.md)

[根目录](../../index.md)