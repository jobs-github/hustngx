    $var_ctx_t * ctx = ngx_palloc(r->pool, sizeof($var_ctx_t));
    if (!ctx)
    {
        return NGX_ERROR;
    }
    memset(ctx, 0, sizeof($var_ctx_t));
    ngx_http_set_addon_module_ctx(r, ctx);
    // TODO: you can initialize ctx here
