#include "hustdict_handler.h"

// curl -i -X GET "http://localhost:8085/del?key=test_key"
ngx_int_t hustdict_del_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
{
    ngx_str_t key;
    if (NGX_OK != ngx_shm_dict_test_get_key(r, &key))
    {
        return NGX_ERROR;
    }
    ngx_int_t rc = ngx_shm_dict_test_delete(&key);
    return ngx_http_send_response_imp(NGX_OK == rc ? NGX_HTTP_OK : NGX_HTTP_NOT_FOUND, NULL, r);
}
