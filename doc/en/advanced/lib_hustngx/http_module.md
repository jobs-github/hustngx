http module
--

### `ngx_http_basic_auth_module` ###
* `ngx_http_basic_auth_module.c`

Relevant background:  

The username and password are asymmetrically encrypted and stored in key file, which is used by built-in `ngx_http_auth_basic_module` of nginx, results to **encryption on username and password during per request**. Besides, when a request comes, the key file will be read. So **duplicated I/O operation** is another issue.  

According to the test, the QPS will **decrease by nearly an order of magnitude** duing to the issues above.  

`ngx_http_basic_auth_module` is just for such problem. Its key file uses **plain text** to store information to avoid performance problem on encryption. Besides, the contents will be read only in initialization of nginx, then buffered. As a result, duplicated I/O operation will not occur.  

If you require **high performance** `http basic authentication` service in nginx, this module could be used (in fact you do not have other choices). In that case, you will confront with **security problem**.  

**You need to evaluate the interest and risk by yourself, and carefully consider the situation before you decide to apply this module.**  

### `ngx_http_peer_selector_module` ###
* `ngx_http_addon_def.h`
* `ngx_http_peer_selector_module.c`

The function of this module is illustrated in [sequential_subrequests](../sequential_subrequests.md).  

`ngx_http_addon_def.h` defines the interface, while `hustngx` generates the implementation.  

### `ngx_http_utils_module` ###
* `ngx_http_addon_def.h`
* `ngx_http_utils_module.h`
* `ngx_http_utils_module.c`

One of the most important fundamental module of `lib_hustngx`, which supports SDK on http module development including basic data structures and functions to simplify programming on `sequential subrequests`.  

The interfaces on `sequential subrequests` are defined as below:  
	
	ngx_int_t ngx_http_run_subrequest(
	        ngx_http_request_t *r,
	        ngx_http_subrequest_ctx_t * ctx,
	        ngx_http_upstream_rr_peer_t * peer);
	ngx_int_t ngx_http_gen_subrequest(
	        ngx_str_t * backend_uri,
	        ngx_http_request_t *r,
	        ngx_http_upstream_rr_peer_t * peer,
	        ngx_http_subrequest_ctx_t * ctx,
	        ngx_http_post_subrequest_pt handler);
	ngx_int_t ngx_http_finish_subrequest(ngx_http_request_t * r);
	ngx_int_t ngx_http_post_subrequest_handler(ngx_http_request_t * r, void * data, ngx_int_t rc);	
	ngx_http_upstream_rr_peer_t * ngx_http_first_peer(ngx_http_upstream_rr_peer_t * peer);
	ngx_http_upstream_rr_peer_t * ngx_http_next_peer(ngx_http_upstream_rr_peer_t * peer);

Programming on `sequential subrequests` will be regular with them.  

### `ngx_http_fetch_module` ###
* `ngx_http_fetch_utils.h`
* `ngx_http_fetch_utils.c`
* `ngx_http_fetch_cache.h`
* `ngx_http_fetch_cache.c`
* `ngx_http_fetch_encode.h`
* `ngx_http_fetch_encode.c`
* `ngx_http_fetch_decode.h`
* `ngx_http_fetch_decode.c`
* `ngx_http_fetch_keepalive.h`
* `ngx_http_fetch_keepalive.c`
* `ngx_http_fetch_upstream_handler.h`
* `ngx_http_fetch_upstream_handler.c`
* `ngx_http_fetch_upstream.h`
* `ngx_http_fetch_upstream.c`
* `ngx_http_fetch.h`
* `ngx_http_fetch.c`
* `ngx_http_fetch_module.c`
* `ngx_http_fetch_test.h`
* `ngx_http_fetch_test.c`

This module implements `context-free` http client. You can construct a standalone instance of `ngx_http_request_t` WITHOUT any external instance of `ngx_http_request_t` to post subrequest to upstream machine.  

**It's meaningful for those who ever had trouble with programming on subrequest. ^_^**   

In general, you need to configure the interface on nginx reverse proxy to construct a subrequest driven by request of client.  

**As a result, to construct a subrequest, the context of main request is required.**  

However, sometimes you do not have main request to construct a subrequest. For instance, to periodically fetch data from upstream servers, you only need to start a timer task. `ngx_http_fetch_module` is designed for such case.  

Besides, with `ngx_http_fetch_module` you can code like this:  

    for (i = 0; i < size; ++i)
    {
        ngx_http_fetch(args_list[i], auth);
    }

The design of `ngx_http_fetch_module` is mainly borrowed from source of nginx, including:  

- `ngx_http_proxy_module.c`  
- `ngx_http_upstream_keepalive_module.c`
- `ngx_http_core_module.c`

It is easy to use `ngx_http_fetch_module` as you only need to know about the following interface:  

    ngx_int_t ngx_http_fetch(const ngx_http_fetch_args_t * args, const ngx_http_auth_basic_key_t * auth);

See more details from [`parallel subrequests`](../parallel_subrequests.md).  

[Previous](../lib_hustngx.md)

[Home](../../index.md)