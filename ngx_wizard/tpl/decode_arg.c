    static const ngx_str_t $var_key = ngx_string("$var_field");
    if (NGX_OK == ngx_http_arg(r, $var_key.data, $var_key.len, &val))
    {
        args->$var_field = $var_val;
    }