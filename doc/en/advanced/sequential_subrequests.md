Sequential Subrequests
--

In this chapter, we will introduce the details on `sequential subrequests`.  

### Peer selector ###
`sequential subrequests` is one of the most complicated technology in nginx http module development. The key point on  `sequential subrequests` programming is to take control of `peer selection` on backend machines for dispatching subrequest to the specified backend machine as your wishes, which is implemented in nginx core by default as a black-box for you. `ngx_http_peer_selector_module` in `lib_hustngx` is designed for this problem. Configured by `ngx_http_peer_selector`, you can **take full control of peer selection on backend machines** in your nginx module when generating subrequest. You can customize peer selection on backend machines freely.  

For example:  

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

Pay attention to the codes as below:  

    ngx_http_gen_subrequest(backend_uri, r, ctx->peer, &ctx->base, __post_subrequest_handler);
    ngx_http_run_subrequest(r, &ctx->base, ctx->peer);

The argument `ctx->peer` will be passed to `upstream` module, and the subrequest will be dispatched to the machine stored in it.  

### State machine ###

Another key point is the `state machine` of `subrequest`.  When a subrequest is posted, nginx will return to main event loop, waiting for the next event. So all context across subrequest should be saved by yourself (the type name of context end with `ctx` in code generated by hustngx).  

Besides, to post `sequential subrequests`, the traditional `for-loop` is **NOT** work:  

    for (subrequest in subrequests):
        # do your business

We should use state machine programming instead:  

	ngx_int_t hustmq_ha_autost_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
	{
		......
		hustmq_ha_autost_ctx_t * ctx = ngx_http_get_addon_module_ctx(r);
		if (!ctx)
		{
			......
			hustmq_ha_autost_ctx_t * ctx = ngx_palloc(r->pool, sizeof(hustmq_ha_autost_ctx_t));
			if (!ctx)
			{
				return NGX_ERROR;
			}
			memset(ctx, 0, sizeof(hustmq_ha_autost_ctx_t));
			ngx_http_set_addon_module_ctx(r, ctx);
	
			......
	
			ctx->peer = ngx_http_first_peer(peers->peer);
			return ngx_http_gen_subrequest(backend_uri, r, ctx->peer,
			        &ctx->base, ngx_http_post_subrequest_handler);
		}
	
		ctx->peer = ngx_http_next_peer(ctx->peer);
		if (ctx->peer)
		{
			return ngx_http_run_subrequest(r, &ctx->base, ctx->peer);
		}
        ...
	
		return ngx_http_send_response_imp(r->headers_out.status, NULL, r);
	}

If it is the first time you attempt to write `sequential subrequests`, you may be confused by the `return`s occur in 3 different statements enclosed by mutually exclusive conditions. It is hard to image nginx will execute them all.  

That's why it is important to understand the state machine of `sequential subrequests`.  

For example, in the code above, after a subrequest is generated by calling `ngx_http_gen_subrequest`, the event loop of nginx will take control of current process (nginx is asynchronous & event-driven, all blocking and time-consuming functions or instructions should be forbidden).  

When a subrequest is done, nginx will call `ngx_http_post_subrequest_handler` defined in [`ngx_http_utils_module`](lib_hustngx/http_module.md), then the entry `hustmq_ha_autost_handler` will be called later.  
**As a result, nginx will call the entry `hustmq_ha_autost_handler` again after a subrequest is done, sequential by sequential, which is the key to implement `sequential subrequests`**.  

The next problem is how to distinguish that the current stage of `hustmq_ha_autost_handler` is before the first subrequest, or the second, or the last?  

Pay attention to the code below:  

    if (!ctx)
    ...
    if (ctx->peer)

This is a traditional implement of state machine programming.  
**For every new instance of `ngx_http_request_t`**, the value of `ctx` is NULL as it has not been created yet when `hustmq_ha_autost_handler` is called by nginx for the first time. **According to this we could confirm that whether it is the first calling on `hustmq_ha_autost_handler` (before the first subrequest)**.  
In the meanwhile, we need to define a field in `ctx` to get current state by its value for the latter subrequests. The example above takes the existence of next backend machine as flag for decision. We could stop `sequential subrequests` and response the result to client when `ctx->peer` is NULL, which means all backend machines have been traversed.  

In conclusion, state machine programming breaks down the sequential executed statement `for (i = 0; i < N; ++i)` into three segments:  

- The first segment, initialize `ctx` (equivalent to `i = 0`)  

- The second segment, update `ctx->peer` to the next node (equivalent to `++i`)

- The third segment, response the result to client if all nodes have been traversed (equivalent to `i < N`)

### Reference counter ###

Another hot potato of implementation on `sequential subrequests` is the management of resources. The primary issue is the collection on instance of `ngx_http_request_t`.  
Reference counter is used by nginx for management. As the operations on reference counter are distributed across multiple modules, we must pay attention to the variation of `r->main->count`.  
If you found that its value is always increase, never decrease, or always decrease, never increase when debugging, your program had bug, **which would make instance of `ngx_http_request_t` out of collection, result in critical resource leak**.  Because the value of `r->main->count` should never changed every time nginx called the entry `hustmq_ha_autost_handler`, which you could toggle breakpoint for watch.  

**You will never confront such problems, as most of them have been resolved correctly by hustngx**.  

If your code is not generated by hustngx, please manage the variation of `r->main->count` by yourself, otherwise your code is unusable.  

Please pay attention to the variation of `r->main->count` as well if the final business is entirely different with be beginning (after all, hustngx could not generate code for extremely exceptional business) even your initial code is generated by hustngx. **Because the usability of code depends on its value**.     

[Previous](index.md)

[Home](../index.md)