#include "hustdict_handler.h"

// curl -i -X GET "http://localhost:8085/incr?key=test_key&type=uint8&delta=1"
ngx_int_t hustdict_incr_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
{
    ngx_shm_dict_mod_args_t args;
    if (NGX_OK != ngx_shm_dict_test_get_mod_args(r, &args))
    {
        return NGX_ERROR;
    }
    if (NGX_OK != ngx_shm_dict_test_incr(r->pool, &args))
    {
        return NGX_ERROR;
    }
    return ngx_http_send_response_imp(NGX_HTTP_OK, &args.result, r);
}
