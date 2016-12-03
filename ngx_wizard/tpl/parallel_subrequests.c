static void __finialize(ngx_uint_t status, ngx_http_request_t *r)
{
    ngx_int_t rc = ngx_http_send_response_imp(status, NULL, r);
    ngx_http_finalize_request(r, rc);
}

static ngx_int_t __post_upstream(ngx_http_request_t * r, void * data, ngx_int_t rc)
{
    $var_ctx_t * ctx = data;
    ++ctx->finished;
    if (NGX_OK == rc)
    {
        if (NGX_HTTP_OK != r->headers_out.status)
        {
            ++ctx->err_count;
        }
    }
    if (ctx->finished >= ctx->requests)
    {
        __finialize((ctx->requests < ctx->backends || ctx->err_count > 0) ? NGX_HTTP_NOT_FOUND : NGX_HTTP_OK, ctx->r);
    }
    return NGX_OK;
}

static ngx_int_t __parallel_subrequests(ngx_str_t * backend_uri, ngx_http_request_t * r)
{
    // this is just a sample implement!
    $var_mcf_t * mcf = $var_get_mcf
    if (!mcf)
    {
        return NGX_ERROR;
    }
$var_create_ctx
    ctx->r = r;
    ctx->backends = ngx_http_get_backend_count();
    ctx->requests = ctx->backends;

    static ngx_http_fetch_header_t headers[] = {
            { ngx_string("Connection"), ngx_string("Keep-Alive") },
            { ngx_string("Content-Type"), ngx_string("text/plain") }
    };
    static size_t headers_len = sizeof(headers) / sizeof(ngx_http_fetch_header_t);
    ngx_http_auth_basic_key_t auth = { mcf->username, mcf->password  }; // "username" and "password" should to be in "nginx.conf"

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
    ++r->main->count;
    r->write_event_handler = ngx_http_request_empty_handler;
    return NGX_OK;
}
