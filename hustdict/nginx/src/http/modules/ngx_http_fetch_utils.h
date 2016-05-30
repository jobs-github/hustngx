#ifndef __ngx_http_fetch_utils_20151225203510_h__
#define __ngx_http_fetch_utils_20151225203510_h__

#include <ngx_http.h>
#include <ngx_http_addon_def.h>

typedef struct
{
    struct sockaddr * sockaddr;
    socklen_t socklen;
    ngx_str_t * name;

    // optional field, if the peer is not in nginx upstream conf, leave it NULL
    ngx_http_upstream_rr_peer_t * peer;
} ngx_http_fetch_addr_t;

typedef struct
{
    ngx_str_t key;
    ngx_str_t value;
} ngx_http_fetch_header_t;

typedef struct
{
    ngx_str_t user;
    ngx_str_t password;
} ngx_http_auth_basic_key_t;

typedef struct
{
    ngx_http_fetch_header_t * arr;
    size_t size;
} ngx_http_fetch_headers_t;

typedef struct
{
    int http_method;
    ngx_http_fetch_addr_t addr;
    ngx_str_t uri;
    ngx_str_t args;
    ngx_http_fetch_headers_t headers;
    ngx_str_t body;
    ngx_http_post_subrequest_pt post_request_handler;
    void * data;
} ngx_http_fetch_args_t;

typedef void (*ngx_http_fetch_reuse_connection_t) (ngx_connection_t * c);

typedef struct
{
    ngx_http_status_t status;
    ngx_http_fetch_reuse_connection_t reuse;

    ngx_hash_t * headers_in_hash;
    const ngx_http_auth_basic_key_t * auth;
    ngx_http_fetch_headers_t headers;
    ngx_http_fetch_addr_t addr;
    ngx_buf_t * body;
    ngx_http_post_subrequest_pt post_request_handler;
    void * data;
} ngx_http_fetch_ctx_t;

void * ngx_http_fetch_get_module_ctx(ngx_http_request_t * r);
void ngx_http_fetch_set_module_ctx(ngx_http_request_t * r, void * ctx);

ngx_str_t ngx_http_fetch_get_method(int method);
size_t ngx_http_fetch_get_header_len(const ngx_http_fetch_header_t * header);
void ngx_http_fetch_encode_header(const ngx_http_fetch_header_t * header, ngx_buf_t * b);

ngx_buf_t * ngx_http_fetch_create_buf(const ngx_str_t * data, ngx_pool_t * pool);
ngx_int_t ngx_http_fetch_copy_str(const ngx_str_t * src, ngx_pool_t * pool, ngx_str_t * des);

#endif // __ngx_http_fetch_utils_20151225203510_h__
