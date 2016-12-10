    ngx_http_upstream_rr_peers_t * peers = ngx_http_get_backends();
    if (!peers || !peers->peer)
    {
        return NGX_ERROR;
    }
