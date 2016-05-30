#ifndef __ngx_http_mgr_module_20150910211248_h__
#define __ngx_http_mgr_module_20150910211248_h__

#include "ngx_http_utils_module.h"

void ngx_http_init_conf_path(ngx_conf_t * cf);

ngx_int_t ngx_http_reload_handler(ngx_http_request_t * r);
ngx_int_t ngx_http_getconf_handler(ngx_http_request_t * r);
ngx_int_t ngx_http_setconf_handler(ngx_http_request_t * r);

#endif // __ngx_http_mgr_module_20150910211248_h__
