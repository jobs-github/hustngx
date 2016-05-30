sequential subrequests
--

本节重点讲解 `sequential subrequests` （多轮串行子请求）相关的技术原理。

### 节点选择 ###
`sequential subrequests` 是 nginx http 模块开发中难度最大的技术点之一。正确进行 `sequential subrequests` 编程的关键，是将 `backend` 节点选择的控制权从 nginx 内核转移到客户端 （nginx 扩展模块），这样你可以自己决定将子请求转发给哪一个后端机器。 `lib_hustngx` 中的 [`ngx_http_peer_selector_module`](lib_hustngx/http_module.md) 可以完美地解决此问题。

### 状态机 ###
另外一个关键点是要理解 `subrequest` 的状态机模型。每一个子请求发出去之后，nginx 将回到主循环中，等待下一个事件的触发。因此所有跨越子请求的上下文，都需要由你自己保存下来（`hustngx`生成的代码中以 `ctx` 作为类型定义的结尾）。

此外，`sequential subrequests` 的投递方法 **并非** 用我们所熟悉的 for 循环方式实现：

    for (subrequest in subrequests):
        # do your business

而是用如下的状态机编程的方式实现：

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

如果你是第一次写 `sequential subrequests` 的程序，你很难理解为什么上面的代码有三处 `return`，因为这三处 `return` 前后的条件分支看上去根本无法全部执行！

这就需要理解 `sequential subrequests` 的状态机模型。

以上述代码为例子，当调用 `ngx_http_gen_subrequest` 产生子请求之后，程序的流程就转移到 nginx 主循环之中了（nginx 是全异步，事件驱动的，不能包含任何阻塞和耗时的调用）。  
当子请求完成之后，nginx 会调用 `ngx_http_post_subrequest_handler`（该函数定义在 [`ngx_http_utils_module`](lib_hustngx/http_module.md) 中），并最终调用 `hustmq_ha_autost_handler`，也就是入口函数。  
**因此，每一轮子请求结束之后，nginx 都会再次调用入口函数 `hustmq_ha_autost_handler`，这正是实现 `sequential subrequests` 的关键**。

接下来的问题就是，如何区分当前 `hustmq_ha_autost_handler` 是处于第一个子请求开始前，还是第二个，还是最后一个？  
请留意代码中的如下两行：

    if (!ctx)
    ...
    if (ctx->peer)

这两行就是状态机编程的一种典型的实现。  
**对于每一个新的 `ngx_http_request_t` 对象**，当 nginx 第一次调用 `hustmq_ha_autost_handler`的时候，`ctx` 对象是不存在的，因此其值为空， **利用这个特点可以区分当前是否是第一次调用 `hustmq_ha_autost_handler`（第一个子请求开始前）** 。  
之后的子请求，就需要在 `ctx` 里面定义字段，通过该字段的值来判定当前处于什么阶段。上面的代码以下一个 `backend` 节点是否存在作为判定标志。当 `ctx->peer` 为空的时候，说明已经遍历完毕，可以将处理结果返回给客户端了， `sequential subrequests` 将终止在这一轮调用。

因此，基于状态机编程的方式本质上是将原本串行执行的 `for (i = 0; i < N; ++i)` 拆分成了三段：

- 第一段，初始化 `ctx`（等价于 i = 0）

- 第二段，更新 `ctx->peer` 为下一个节点（等价于 ++i）

- 第三段，如果遍历完毕，返回给客户端结果（等价于 i < N）

### 引用计数 ###

进行 `sequential subrequests` 编程遇到的另外一个棘手的问题是对资源的管理（主要是 `ngx_http_request_t` 对象的回收）。  
nginx 内部是通过引用计数来进行管理的。由于 nginx 对于引用计数的操作分散在很多个模块，因此必须格外留意 `r->main->count` 的变化。  
如果在进行调试（例如，在 `hustmq_ha_autost_handler` 入口处下断点，每一次调用进来 `r->main->count` 都应该是同一个值）的时候发现这个值只增不减，或者只减不增，那么程序一定有问题。 **这将使得 `ngx_http_request_t` 对象永远没有机会得到回收，从而导致严重的资源泄露** 。

**关于引用计数， `hustngx` 对于生成的代码中全部做了正确的处理，因此大部分引用计数的问题你都不会在该框架生成的代码中遇到。**

如果你没有采用 `hustngx` 生成代码，请自行管理好 `r->main->count` 的变化，否则你写出来的模块将是不可用的。

即使你采用了 `hustngx` 生成代码，如果最终的业务逻辑和原始的代码有了非常大的出入（毕竟 `hustngx` 无法帮你生成极其特殊的业务逻辑），也请留意 `r->main->count` 的变化， **它的值是你的程序是否可用的关键** 。

[上一级](index.md)

[根目录](../index.md)