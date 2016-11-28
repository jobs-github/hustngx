#ifndef __hustdict_handler_20160107150007_h__
#define __hustdict_handler_20160107150007_h__

#include "hustdict_utils.h"


ngx_int_t hustdict_set_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);
ngx_int_t hustdict_get_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);
ngx_int_t hustdict_del_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);
ngx_int_t hustdict_flush_all_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);
ngx_int_t hustdict_flush_expired_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);
ngx_int_t hustdict_keys_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);
ngx_int_t hustdict_get_keys_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);
ngx_int_t hustdict_incr_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);
ngx_int_t hustdict_decr_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);

#endif // __hustdict_handler_20160107150007_h__