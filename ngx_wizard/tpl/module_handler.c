$var_declare
{
    if (!r->uri.data)
    {
        return NGX_ERROR;
    }
    u_char tmp = r->uri.data[r->uri.len];
    r->uri.data[r->uri.len] = '\0';
    ngx_http_request_item_t ** it = c_dict_get(addon_handler_dict, (const char *)r->uri.data);
    r->uri.data[r->uri.len] = tmp;
    
    if (!it || !*it)
    {
        return NGX_ERROR;
    }
    return (*it)->handler(&(*it)->backend_uri, r);
}
