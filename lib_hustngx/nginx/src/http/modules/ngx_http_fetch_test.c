#include "ngx_http_fetch_test.h"
#include "ngx_http_fetch.h"
#include "ngx_http_fetch_cache.h"

static ngx_event_t g_ev_event;

static void __write_response(const ngx_buf_t * buf, const char * path)
{
    if (!buf->start)
    {
        return;
    }
    FILE * fp = fopen(path, "w");
    fwrite(buf->start, 1, (size_t) (buf->end - buf->start), fp);
    fclose(fp);
}

static ngx_int_t __post_request(ngx_http_request_t *r, void *data, ngx_int_t rc)
{
    if (NGX_HTTP_OK == r->headers_out.status)
    {
        __write_response(&r->upstream->buffer, "/tmp/ngx_http_fetch_ok.log");
    }
    else
    {
        __write_response(&r->upstream->buffer, "/tmp/ngx_http_fetch_err.log");
    }
    return NGX_OK;
}

static void __test_base(
    ngx_http_upstream_rr_peer_t * peer,
    ngx_http_auth_basic_key_t * auth,
    ngx_http_fetch_header_t headers[],
    size_t len)
{
    ngx_http_fetch_args_t args1 = {
        NGX_HTTP_GET,
        { peer->sockaddr, peer->socklen, &peer->name, peer },
        ngx_string("/status.html"),
        ngx_null_string,
        { headers, len },
        ngx_null_string,
        { NULL, NULL },
        { __post_request, NULL }
    };
    ngx_http_fetch_args_t args2 = {
        NGX_HTTP_POST,
        { peer->sockaddr, peer->socklen, &peer->name, peer },
        ngx_string("/hustdb/sadd"),
        ngx_string("tb=shustdbhatb&key=hustdbhakey"),
        { headers, len },
        ngx_string("hustdbhavalue"),
        { NULL, NULL },
        { __post_request, NULL }
    };
    ngx_http_fetch_args_t args3 = {
        NGX_HTTP_PUT,
        { peer->sockaddr, peer->socklen, &peer->name, peer },
        ngx_string("/hustdb/hset"),
        ngx_string("tb=hhustdbhatb&key=hustdbhakey&val=hustdbhavalue"),
        { headers, len },
        ngx_null_string,
        { NULL, NULL },
        { __post_request, NULL }
    };

    ngx_http_fetch_args_t * args_list[] = { &args1, &args2, &args3 };
    size_t size = sizeof(args_list) / sizeof(ngx_http_fetch_args_t *);

    size_t i = 0;
    for (i = 0; i < size; ++i)
    {
        ngx_http_fetch(args_list[i], auth);
    }
}

static void __test_peers(
    ngx_http_upstream_rr_peers_t * peers,
    ngx_http_auth_basic_key_t * auth,
    ngx_http_fetch_header_t headers[],
    size_t len)
{
    ngx_http_upstream_rr_peer_t * peer = peers->peer;
    while (peer)
    {
        ngx_http_fetch_args_t args = {
            NGX_HTTP_GET,
            { peer->sockaddr, peer->socklen, &peer->name, peer },
            ngx_string("/hustdb/stat_all"),
            ngx_null_string,
            { headers, len },
            ngx_null_string,
            { NULL, NULL },
            { __post_request, NULL }
        };
        ngx_http_fetch(&args, auth);
        peer = peer->next;
    }
}

static void __active_ev(ngx_event_t * ev)
{
    ngx_http_auth_basic_key_t auth = { ngx_string("hustdbxc"), ngx_string("qihoohustxrbczdb") };
    ngx_http_fetch_header_t headers[] = {
        { ngx_string("Connection"), ngx_string("Keep-Alive") },
        { ngx_string("Content-Type"), ngx_string("text/plain") }
    };
    size_t len = sizeof(headers) / sizeof(ngx_http_fetch_header_t);

    ngx_http_upstream_rr_peers_t * peers = ngx_http_get_backends();
    __test_peers(peers, &auth, headers, len);
    __test_base(peers->peer, &auth, headers, len);

    ngx_add_timer(&g_ev_event, 1000);
}

void ngx_http_fetch_init_test(ngx_log_t * log)
{
    ngx_memzero(&g_ev_event, sizeof(ngx_event_t));
    g_ev_event.handler = __active_ev;
    g_ev_event.log = log;
}

void ngx_http_fetch_invoke_test()
{
    ngx_add_timer(&g_ev_event, 1);
}
