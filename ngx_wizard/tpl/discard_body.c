    ngx_int_t rc = ngx_http_discard_request_body(r);
    if (NGX_OK != rc)
    {
        return rc;
    }
