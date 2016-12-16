    ngx_str_t * value = cf->args->elts;
    mcf->$var_field = $var_value;
    if (NGX_ERROR == mcf->$var_field)
    {
        return "ngx_http_$var_field error";
    }