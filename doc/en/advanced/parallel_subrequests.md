parallel subrequests
--

In this chapter, we will introduce the details on `parallel subrequests`. Before reading this chapter, please make sure you've understood the contents in [`sequential subrequests`](sequential_subrequests.md).  

### Present state on `parallel subrequests` ###
It is difficult to implement `parallel subrequests` correctly by [`ngx_http_subrequest`](http://lxr.nginx.org/source/src/http/ngx_http_core_module.c) in `nginx` . In fact, you can get some information on this topic
from [Emiller's Advanced Topics In Nginx Module Development](http://www.evanmiller.org/nginx-modules-guide-advanced.html):  

> It's also possible to issue several subrequests at once without waiting for previous subrequests to finish. This technique is, in fact, too advanced even for Emiller's Advanced Topics in Nginx Module Development. See the SSI module for an example.

In other words: it is nearly impossible to describe the technology in this book as it is so complicated, you'd better see the source code of [`SSI module`](http://lxr.nginx.org/source/src/http/modules/ngx_http_ssi_filter_module.c), about 3k lines of code...

I ever attempted to understand this file for the principle of `parallel subrequests`. However, I gave up in the end as it was implemented with so many `goto`s and monster functions (each function included more than 500 lines of code)...

### Limitation on `ngx_http_subrequest` ###

`ngx_http_subrequest` implements the function of `http client` through [`ngx_http_proxy_module`](http://lxr.nginx.org/source/src/http/modules/ngx_http_proxy_module.c). So any implement based on `ngx_http_subrequest` should work well with the loop in `ngx_http_run_posted_requests`. The design of nginx is that, when a event is processed, it will check whether a posted event is in subrequest, otherwise it will not execute the event handler.  

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

As a result, it is hard to generate multiple subrequests by normal way:  

    for (i = 0; i < requests; ++i)
    {
        ngx_http_subrequest(...);
    }

### Technical essential on `parallel subrequests` ###

Compared with `ngx_http_subrequest`, `hustngx` uses an absolutely different way to implement `parallel subrequests`. In details, `hustngx` implements a standalone `http client` named [`ngx_http_fetch`](lib_hustngx/http_module.md) which could work as reverse proxy not restricted by `ngx_http_subrequest` .  

You can implement `parallel subrequests` based on `ngx_http_fetch` as following:  

* Generate multiple subrequests by calling `ngx_http_fetch` in nginx main loop  
* Increase `r->main->count` by one to hold on main request, then return `NGX_DONE` in the handler of main request  
* Implement callback `post_upstream` to process the result of subrequest  
* Merge results of subrequests, which could be responsed by main request, then dispose resources allocated in main request  

This is a example:  

    // Define the context of request
    typedef struct
    {
        ngx_http_request_t * r;
        size_t backends;
        size_t requests;
        size_t finished;
        size_t err_count;
        // TODO: add your fields here
    } hustmq_ha_autost_ctx_t;

    // You need to call ngx_http_finalize_request explicitly here
    static void __finialize(ngx_uint_t status, ngx_http_request_t *r)
    {
        ngx_int_t rc = ngx_http_send_response_imp(status, NULL, r);
        ngx_http_finalize_request(r, rc);
    }
    
    // Callback of subrequest
    static ngx_int_t __post_upstream(ngx_http_request_t * r, void * data, ngx_int_t rc)
    {
        hustmq_ha_autost_ctx_t * ctx = data;
        // When a subrequest is done, increase the counter by one
        ++ctx->finished;

        // Process the result of current subrequest
        if (NGX_OK == rc)
        {
            if (NGX_HTTP_OK != r->headers_out.status)
            {
                ++ctx->err_count;
            }
        }

        // Check whether all subrequests are completed
        if (ctx->finished >= ctx->requests)
        {
            // Merge the results of subrequests, and return the merged result to main request
            __finialize((ctx->requests < ctx->backends || ctx->err_count > 0) ? NGX_HTTP_NOT_FOUND : NGX_HTTP_OK, ctx->r);
        }
        return NGX_OK;
    }
    
    // The entry where parallel subrequests are generated
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
        // Set the total count of subrequests
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
            // Generate subrequest
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
        // Increase the reference count by one to hold on the main request
        ++r->main->count;
        r->write_event_handler = ngx_http_request_empty_handler;
        return NGX_OK;
    }
    
    // The entry of main request
    ngx_int_t hustmq_ha_autost_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
    {
        if (NGX_ERROR == __parallel_subrequests(backend_uri, r))
        {
            ngx_http_send_response_imp(NGX_HTTP_NOT_FOUND, NULL, r);
        }
        // You need to return NGX_DONE to notify nginx that current request has been done
        return NGX_DONE;
    }

[Previous](index.md)

[Home](../index.md)