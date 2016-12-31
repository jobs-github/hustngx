
static ngx_http_addon_upstream_peers_t addon_upstream_peers = { 0, 0 };

$var_post
{
    static ngx_str_t backend = ngx_string("$var_upstream");
    ngx_bool_t rc = ngx_http_init_addon_backends(
        ngx_http_conf_get_module_main_conf(cf, ngx_http_upstream_module), 
        &backend, &addon_upstream_peers);
    if (!rc)
    {
        return NGX_ERROR;
    }
    return NGX_OK;
}

ngx_http_upstream_rr_peers_t * ngx_http_get_backends()
{
    return addon_upstream_peers.peer;
}

size_t ngx_http_get_backend_count()
{
    return addon_upstream_peers.count;
}
