    ngx_str_t * backend_uri = ngx_http_get_addon_module_ctx(r);
    --r->main->count;
$var_parallel_call