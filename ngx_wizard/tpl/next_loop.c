    if (NGX_HTTP_OK != r->headers_out.status)
    {
        ctx->peer = ngx_http_next_peer(ctx->peer);
        return (ctx->peer) ? ngx_http_run_subrequest(r, &ctx->base, $var_sr_peer)
            : ngx_http_send_response_imp(NGX_HTTP_NOT_FOUND, NULL, r);
    }