void * ngx_http_get_addon_module_ctx(ngx_http_request_t * r)
{
$var_null_return
    return ngx_http_get_module_ctx(r, $var_md);
}

void ngx_http_set_addon_module_ctx(ngx_http_request_t * r, void * ctx)
{
$var_no_return
    ngx_http_set_ctx(r, ctx, $var_md);
}

void * $var_get_mcf(ngx_http_request_t * r)
{
$var_null_return
    return ngx_http_get_module_main_conf(r, $var_md);
}
