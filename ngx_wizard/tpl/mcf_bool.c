    int val = ngx_http_get_flag_slot(cf);
    if (NGX_ERROR == val)
    {
        return "ngx_http_$var_field error";
    }
    mcf->$var_field = val;