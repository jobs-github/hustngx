#include "hustdict_handler.h"

ngx_str_t __encode_keys(const ngx_shm_dict_keys_t * keys, ngx_pool_t * pool)
{
    size_t len = 0;
    size_t i = 0;
    for (i = 0; i < keys->size; ++i)
    {
        len += (keys->arr[i].len + 1);
    }
    char split = '\n';
    ngx_str_t data = { 0, 0 };
    data.data = ngx_palloc(pool, len + 1);
    for (i = 0; i < keys->size; ++i)
    {
        memcpy(data.data + data.len, keys->arr[i].data, keys->arr[i].len);
        data.len += keys->arr[i].len;
        memcpy(data.data + data.len, &split, 1);
        data.len += 1;
    }
    return data;
}

// curl -i -X GET "http://localhost:8085/get_keys?all=true"
ngx_int_t hustdict_get_keys_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
{
    ngx_bool_t include_expires = ngx_shm_dict_test_include_expires(r);
    ngx_shm_dict_keys_t keys;
    if (NGX_OK != ngx_shm_dict_test_get_keys(include_expires, r->pool, &keys))
    {
        return ngx_http_send_response_imp(NGX_HTTP_NOT_FOUND, NULL, r);
    }
    ngx_str_t response = __encode_keys(&keys, r->pool);
    return ngx_http_send_response_imp(NGX_HTTP_OK, &response, r);
}
