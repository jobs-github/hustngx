#include "hustdict_handler.h"

// curl -i -X GET "http://localhost:8085/flush_all"
ngx_int_t hustdict_flush_all_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
{
    ngx_int_t rc = ngx_shm_dict_test_flush_all();
    return ngx_http_send_response_imp((NGX_OK == rc) ? NGX_HTTP_OK : NGX_HTTP_NOT_FOUND, NULL, r);
}
