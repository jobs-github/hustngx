http 扩展模块
--

### `ngx_http_basic_auth_module` ###
* `ngx_http_basic_auth_module.c`

相关背景：

nginx 内置的 `ngx_http_auth_basic_module` 由于使用了非对称加密存储的密钥文件（ `htpasswd` ），导致 **每一次请求都需要对用户信息做加密操作** 。此外其内部实现对于密钥文件采用了实时读取的方式，因此 **每一次请求都会导致重复的 `I/O` 操作** 。

经测试，以上两个问题会使得 `QPS` 下降 **接近一个数量级** 。

`ngx_http_basic_auth_module` 正是为了解决此问题。该模块对应的密钥文件使用 **明文** 存储，因此可以绕过加密操作所带来的性能问题。此外对于 `htpasswd` 的内容全部在 nginx 初始化的时候读取完毕，并进行缓存。因此可以避免请求过程中的重复 `I/O` 操作。

如果你需要 nginx 提供 **高性能** 的 `http basic authentication` 服务，则可以使用该模块（事实上目前你也没有其他选择），但这将面临 **安全性的问题** 。

**你需要自行评估使用该模块的利益和风险，酌情使用。**

### `ngx_http_peer_selector_module` ###
* `ngx_http_addon_def.h`
* `ngx_http_peer_selector.h`
* `ngx_http_peer_selector_module.c`

[includes](../ngx_wizard/includes.md) 一节中描述过该模块的功能。

如果想发挥 `subrequest` 的最大效用，必须获得 `backend` 节点选择的控制权，因此 **大多数情况下，建议启用此模块** 。

如果你想将控制权临时交还给 nginx 内核，可以参考 [`use_round_robin`](../ngx_wizard/use_round_robin.md) 的做法。

`ngx_http_addon_def.h` 属于接口文件，其实现将由 `hustngx` 自动生成。

### `ngx_http_utils_module` ###
* `ngx_http_addon_def.h`
* `ngx_http_utils_module.h`
* `ngx_http_utils_module.c`

这是 `lib_hustngx` 非常重要的一个基础模块，定义了一整套用于 http 模块开发的基础数据结构以及函数库，简化了 `sequential subrequests` 的编写方式。

以下是部分和 `sequential subrequests` 相关的接口定义：
	
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

通过这些接口可以使得 `sequential subrequests` 的实现变得模式化。

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

该模块提供上下文无关的 http 访问接口（`context-free`），你可以在不依赖于任何外部 `ngx_http_request_t` 的条件下构造一个完全独立的 `ngx_http_request_t` 对象，并向上游主机发起 http 请求（`upstream`）。

**所有编写过 `subrequest` 代码的朋友，想必都了解这一点（`context-free`）意味着什么 ^_^**

通常情况下，如果你想构造一个 `subrequest` ，你必须配置 nginx 反向代理的接口，并且要由客户端来驱动该子请求的产生。 **在这种设计下，一个子请求的生成必须依赖于一个父请求的上下文。**

但是在特殊情况下，你所构造的子请求并不需要依赖于父请求（例如，周期性地向上游主机抓取数据，这通常开一个定时任务即可）， `ngx_http_fetch_module` 正是为解决此类问题而出现。

此外， `ngx_http_fetch_module` 提供的接口使得编写如下形式的代码成为可能：

    for (i = 0; i < size; ++i)
    {
        ngx_http_fetch(args_list[i], auth);
    }

`ngx_http_fetch_module` 的设计大量参考了 nginx 源代码，包括：

- `ngx_http_proxy_module.c`  
- `ngx_http_upstream_keepalive_module.c`
- `ngx_http_core_module.c`

`ngx_http_fetch_module` 的使用非常简单，只需要了解如下接口：

    ngx_int_t ngx_http_fetch(const ngx_http_fetch_args_t * args, const ngx_http_auth_basic_key_t * auth);

具体的写法可以参考 [`parallel subrequests`](../parallel_subrequests.md) 。

[上一级](../lib_hustngx.md)

[根目录](../../index.md)