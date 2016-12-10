    // TODO: you can implement the business here

    ngx_str_t response = ngx_string("Hello World!");
    r->headers_out.status = NGX_HTTP_OK;
    return ngx_http_send_response_imp(r->headers_out.status, &response, r);