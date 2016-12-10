    $var_rc = ngx_http_read_client_request_body(r, __post_body_handler);
    if (rc >= NGX_HTTP_SPECIAL_RESPONSE)
    {
        return rc;
    }
    return NGX_DONE;