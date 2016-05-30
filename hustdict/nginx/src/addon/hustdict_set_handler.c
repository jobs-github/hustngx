#include "hustdict_handler.h"
#include <stdbool.h>

static ngx_bool_t __post_body_cb(
    ngx_http_request_t * r, ngx_buf_t * buf, size_t buf_size)
{
    ngx_shm_dict_write_args_t args;
    if (NGX_OK != ngx_shm_dict_test_get_write_args(false, r, &args))
    {
        return false;
    }
    if (!buf || !buf->pos || buf_size < 1)
    {
        return false;
    }
    ngx_str_t val = { buf_size, buf->pos };
    args.val = val;
    ngx_int_t rc = ngx_shm_dict_test_write(&args);
    return NGX_OK == rc;
}

static void __post_body_handler(ngx_http_request_t * r)
{
    ngx_http_post_body_handler(r, __post_body_cb);
}

// curl -i -X GET "http://localhost:8085/set?method=set&key=test_key&type=string&val=test_val"
ngx_int_t hustdict_set_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
{
    if ((r->method & NGX_HTTP_POST)) // binary
    {
        ngx_int_t rc = ngx_http_read_client_request_body(r, __post_body_handler);
        if (rc >= NGX_HTTP_SPECIAL_RESPONSE)
        {
            return rc;
        }
        return NGX_DONE;
    }

    ngx_shm_dict_write_args_t args;
    if (NGX_OK != ngx_shm_dict_test_get_write_args(true, r, &args))
    {
        return NGX_ERROR;
    }
    ngx_int_t rc = ngx_shm_dict_test_write(&args);
    return ngx_http_send_response_imp(NGX_OK == rc ? NGX_HTTP_OK : NGX_HTTP_NOT_FOUND, NULL, r);
}
