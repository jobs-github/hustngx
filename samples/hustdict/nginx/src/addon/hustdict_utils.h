#ifndef __hustdict_utils_20160107150007_h__
#define __hustdict_utils_20160107150007_h__

#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_http_addon_def.h>
#include <ngx_http_utils_module.h>
#include "ngx_shm_dict_test.h"

typedef struct
{
    ngx_pool_t * pool;
    ngx_log_t * log;
    ngx_str_t prefix;
    ngx_str_t shm_name;
    ngx_int_t shm_size;
} ngx_http_hustdict_main_conf_t;

void * hustdict_get_module_main_conf(ngx_http_request_t * r);

#endif // __hustdict_utils_20160107150007_h__
