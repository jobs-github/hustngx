    ngx_str_t * arr = cf->args->elts;
    mcf->$var_field = ngx_http_make_str(&arr[1], cf->pool);