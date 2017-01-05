parallel subrequests
--

本节重点讲解 `parallel subrequests` （多轮并行子请求）相关的技术原理。在阅读本节之前，请先阅读 [`sequential subrequests`](sequential_subrequests.md) （多轮串行子请求）一节的内容。

### `parallel subrequests` 技术的现状 ###
基于 `nginx` 自身提供的 [`ngx_http_subrequest`](http://lxr.nginx.org/source/src/http/ngx_http_core_module.c) 很难写出正确的  `parallel subrequests` 的代码。事实上 [Emiller's Advanced Topics In Nginx Module Development](http://www.evanmiller.org/nginx-modules-guide-advanced.html) 一书中，对于这个主题做了极其简短的描述：

> It's also possible to issue several subrequests at once without waiting for previous subrequests to finish. This technique is, in fact, too advanced even for Emiller's Advanced Topics in Nginx Module Development. See the SSI module for an example.

翻译成一句话就是：这个主题太复杂了，三言两语讲不清，自己去看 [`SSI module`](http://lxr.nginx.org/source/src/http/modules/ngx_http_ssi_filter_module.c) 的源代码吧，接近3000行的一个文件。。。

曾经笔者尝试去读懂这个文件，以便探求 `parallel subrequests` 的技术原理，后来放弃了，因为里面处处充斥着 `goto` 和超过 `500` 行以上的巨无霸函数。。。

### `ngx_http_subrequest` 的局限 ###

`ngx_http_subrequest` 本质上利用了 [`ngx_http_proxy_module`](http://lxr.nginx.org/source/src/http/modules/ngx_http_proxy_module.c) 来实现 `http client` 的功能。这导致所有基于 `ngx_http_subrequest` 实现的功能必须融合到 `ngx_http_run_posted_requests` 所包含的循环之中。nginx本身的设计就是，每处理完一个事件后，将会检查有没有它对应的一个post事件（一对一），如果有则处理。

    void
    ngx_http_run_posted_requests(ngx_connection_t *c)
    {
        ngx_http_request_t         *r;
        ngx_http_posted_request_t  *pr;
    
        for ( ;; ) {
    
            if (c->destroyed) {
                return;
            }
    
            r = c->data;
            pr = r->main->posted_requests;
    
            if (pr == NULL) {
                return;
            }
    
            r->main->posted_requests = pr->next;
    
            r = pr->request;
    
            ngx_http_set_log_request(c->log, r);
    
            ngx_log_debug2(NGX_LOG_DEBUG_HTTP, c->log, 0,
                           "http posted request: \"%V?%V\"", &r->uri, &r->args);
    
            r->write_event_handler(r);
        }
    }

因此你很难用如下这种直观的方式生成多个子请求：

    for (i = 0; i < requests; ++i)
    {
        ngx_http_subrequest(...);
    }

### `parallel subrequests` 技术要点 ###
`hustngx` 采用了一种和 `ngx_http_subrequest` 完全不同的思路来实现 `parallel subrequests`。具体来讲， `hustngx` 实现了一套独立的 `http client` 来完成反向代理的功能，参考 [`ngx_http_fetch`](lib_hustngx/http_module.md)，利用这套实现可以摆脱 `ngx_http_subrequest` 的局限。

基于 `ngx_http_fetch` 的 `parallel subrequests` 编程范式如下：

* 在主循环中调用 `ngx_http_fetch` 生成多个子请求
* `r->main->count` 引用计数加一，将主请求挂住，同时主请求的处理函数返回 `NGX_DONE`
* 编写 `post_upstream` 的实现，处理子请求结果
* 合并子请求结果，向主请求返回该结果，释放主请求资源

具体的实现代码样例可参考如下的写法：

    // 定义上下文
    typedef struct
    {
        ngx_http_request_t * r;
        size_t backends;
        size_t requests;
        size_t finished;
        size_t err_count;
        // TODO: add your fields here
    } hustmq_ha_autost_ctx_t;

    // 此处必须显示调用 ngx_http_finalize_request
    static void __finialize(ngx_uint_t status, ngx_http_request_t *r)
    {
        ngx_int_t rc = ngx_http_send_response_imp(status, NULL, r);
        ngx_http_finalize_request(r, rc);
    }
    
    // 子请求的回调接口
    static ngx_int_t __post_upstream(ngx_http_request_t * r, void * data, ngx_int_t rc)
    {
        hustmq_ha_autost_ctx_t * ctx = data;
        // 每完成一个子请求，都将计数器加一
        ++ctx->finished;

        // 处理当前子请求的结果
        if (NGX_OK == rc)
        {
            if (NGX_HTTP_OK != r->headers_out.status)
            {
                ++ctx->err_count;
            }
        }

        // 判断所有的子请求是否完成
        if (ctx->finished >= ctx->requests)
        {
            // 合并子请求的结果，将结果返回给主请求
            __finialize((ctx->requests < ctx->backends || ctx->err_count > 0) ? NGX_HTTP_NOT_FOUND : NGX_HTTP_OK, ctx->r);
        }
        return NGX_OK;
    }
    
    // 生成多轮并行子请求的入口
    static ngx_int_t __parallel_subrequests(ngx_str_t * backend_uri, ngx_http_request_t * r)
    {
        ngx_http_hustmq_ha_main_conf_t * mcf = hustmq_ha_get_module_main_conf(r);
        if (!mcf)
        {
            return NGX_ERROR;
        }
        hustmq_ha_autost_ctx_t * ctx = ngx_palloc(r->pool, sizeof(hustmq_ha_autost_ctx_t));
        if (!ctx)
        {
            return NGX_ERROR;
        }
        memset(ctx, 0, sizeof(hustmq_ha_autost_ctx_t));
        ngx_http_set_addon_module_ctx(r, ctx);
        // TODO: you can initialize ctx here
    
        ctx->r = r;
        ctx->backends = ngx_http_get_backend_count();
        // 设置子请求总数
        ctx->requests = ctx->backends;
    
        static ngx_http_fetch_header_t headers[] = {
                { ngx_string("Connection"), ngx_string("Keep-Alive") },
                { ngx_string("Content-Type"), ngx_string("text/plain") }
        };
        static size_t headers_len = sizeof(headers) / sizeof(ngx_http_fetch_header_t);
        ngx_http_auth_basic_key_t auth = { mcf->username, mcf->password  };
    
        ngx_http_upstream_rr_peers_t * peers = ngx_http_get_backends();
        ngx_http_upstream_rr_peer_t * peer = peers->peer;
        while (peer)
        {
            ngx_http_fetch_args_t args = {
                NGX_HTTP_GET,
                { peer->sockaddr, peer->socklen, &peer->name, peer },
                *backend_uri,
                ngx_null_string,
                { headers, headers_len },
                ngx_null_string,
                { NULL, NULL },
                { __post_upstream, ctx }
            };
            // 生成子请求
            ngx_int_t rc = ngx_http_fetch(&args, &auth);
            if (NGX_OK != rc)
            {
                --ctx->requests;
            }
            peer = peer->next;
        }
        if (ctx->requests < 1)
        {
            return NGX_ERROR;
        }
        // 引用计数加一，挂住主请求
        ++r->main->count;
        r->write_event_handler = ngx_http_request_empty_handler;
        return NGX_OK;
    }
    
    // 主请求的入口函数
    ngx_int_t hustmq_ha_autost_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
    {
        if (NGX_ERROR == __parallel_subrequests(backend_uri, r))
        {
            ngx_http_send_response_imp(NGX_HTTP_NOT_FOUND, NULL, r);
        }
        // 此处需要返回 NGX_DONE，通知 nginx 当前请求已经处理完毕
        return NGX_DONE;
    }

[上一级](index.md)

[回首页](../index.md)