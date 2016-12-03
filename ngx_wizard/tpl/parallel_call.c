    if (NGX_ERROR == __parallel_subrequests(backend_uri, r))
    {
        ngx_http_send_response_imp(NGX_HTTP_NOT_FOUND, NULL, r);
    }
$var_end