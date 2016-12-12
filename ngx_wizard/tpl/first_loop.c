    $var_ctx_t * ctx = ngx_http_get_addon_module_ctx(r);
    if (!ctx)
    {
        return $var_first_handler(backend_uri, r);
    }