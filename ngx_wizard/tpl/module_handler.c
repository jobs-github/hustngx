$var_declare
{
    ngx_http_request_item_t * it = ngx_http_get_request_item(
        $var_dict, $var_dict_len, &r->uri);
    if (!it)
    {
        return NGX_ERROR;
    }
    return it->handler(&it->backend_uri, r);
}
