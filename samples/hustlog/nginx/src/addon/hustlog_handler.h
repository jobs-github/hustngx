#ifndef __hustlog_handler_20150720144219_h__
#define __hustlog_handler_20150720144219_h__

#include "hustlog_utils.h"

ngx_int_t hustlog_reload_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);
ngx_int_t hustlog_getconf_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);
ngx_int_t hustlog_setconf_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);

ngx_int_t hustlog_post_handler(ngx_str_t * backend_uri, ngx_http_request_t *r);

#endif // __hustlog_handler_20150720144219_h__
