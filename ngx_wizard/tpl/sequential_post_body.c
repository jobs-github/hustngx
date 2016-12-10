    $var_ctx_t * ctx = ngx_http_get_addon_module_ctx(r);
    // TODO: you can update ctx->base.args here
    --r->main->count;
$var_sr