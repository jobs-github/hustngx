#include "hustdict_handler.h"

// curl -i -X GET "http://localhost:8085/get?key=test_key&type=string"
ngx_int_t hustdict_get_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
{
    ngx_shm_dict_read_args_t args;
    if (NGX_OK != ngx_shm_dict_test_get_read_args(r, &args))
    {
        return NGX_ERROR;
    }
    return ngx_shm_dict_test_read(&args, r);
}
