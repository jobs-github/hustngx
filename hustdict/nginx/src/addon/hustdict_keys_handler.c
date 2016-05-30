#include "hustdict_handler.h"

// curl -i -X GET "http://localhost:8085/keys?all=true"
ngx_int_t hustdict_keys_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
{
    ngx_bool_t include_expires = ngx_shm_dict_test_include_expires(r);
    size_t keys = 0;
    if (NGX_OK != ngx_shm_dict_test_keys(include_expires, &keys))
    {
        return ngx_http_send_response_imp(NGX_HTTP_NOT_FOUND, NULL, r);
    }
    ngx_str_t response = ngx_shm_dict_test_from_uint(r->pool, (uint64_t) keys);
    return ngx_http_send_response_imp(NGX_HTTP_OK, &response, r);
}
