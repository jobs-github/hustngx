#ifndef __ngx_shm_dict_test_utils_20160113121419_h__
#define __ngx_shm_dict_test_utils_20160113121419_h__

#include <ngx_http.h>
#include <ngx_http_utils_module.h>
#include "ngx_shm_dict.h"

typedef struct
{
    ngx_str_t key;
    ngx_str_t type;
} ngx_shm_dict_read_args_t;

typedef ngx_int_t (*hustdict_read_handler_t)(ngx_str_t * key, ngx_http_request_t * r);

typedef struct
{
    ngx_str_t type;
    hustdict_read_handler_t handler;
} ngx_shm_dict_test_read_item_t;

typedef struct
{
    ngx_str_t key;
    ngx_str_t val;
    ngx_str_t method;
    ngx_str_t type;
    uint64_t expire;
} ngx_shm_dict_write_args_t;

typedef ngx_int_t (*ngx_shm_dict_test_write_t)(ngx_shm_dict_write_args_t * args);

typedef struct
{
    ngx_str_t method;
    ngx_str_t type;
    ngx_shm_dict_test_write_t write;
} ngx_shm_dict_test_write_item_t;

typedef struct
{
    ngx_str_t key;
    ngx_str_t delta;
    ngx_str_t type;
    uint64_t expire;
    ngx_str_t result;
} ngx_shm_dict_mod_args_t;

typedef ngx_int_t (*ngx_shm_dict_test_mod_t)(ngx_pool_t * pool, ngx_shm_dict_mod_args_t * args);

typedef struct
{
    ngx_str_t type;
    ngx_shm_dict_test_mod_t handler;
} ngx_shm_dict_test_mod_item_t;

ngx_str_t ngx_shm_dict_test_from_int(ngx_pool_t * pool, int64_t val);
ngx_str_t ngx_shm_dict_test_from_uint(ngx_pool_t * pool, uint64_t val);
ngx_str_t ngx_shm_dict_test_from_double(ngx_pool_t * pool, double val);
ngx_shm_zone_t * ngx_shm_dict_test_get_zone();

ngx_int_t ngx_shm_dict_test_read_base(
    ngx_shm_dict_test_read_item_t dict[],
    size_t len,
    ngx_shm_dict_read_args_t * args,
    ngx_http_request_t *r);
ngx_int_t ngx_shm_dict_test_write_base(
    ngx_shm_dict_test_write_item_t dict[],
    size_t len,
    ngx_shm_dict_write_args_t * args);
ngx_int_t ngx_shm_dict_test_mod_base(
    ngx_shm_dict_test_mod_item_t dict[],
    size_t len,
    ngx_pool_t * pool,
    ngx_shm_dict_mod_args_t * args);

ngx_int_t ngx_shm_dict_test_get_read_args(ngx_http_request_t *r, ngx_shm_dict_read_args_t * args);
ngx_int_t ngx_shm_dict_test_get_write_args(ngx_bool_t need_val, ngx_http_request_t *r, ngx_shm_dict_write_args_t * args);
ngx_int_t ngx_shm_dict_test_get_mod_args(ngx_http_request_t *r, ngx_shm_dict_mod_args_t * args);
ngx_int_t ngx_shm_dict_test_get_key(ngx_http_request_t *r, ngx_str_t * key);
ngx_bool_t ngx_shm_dict_test_include_expires(ngx_http_request_t *r);

ngx_int_t ngx_shm_dict_test_init(ngx_conf_t * cf, ngx_str_t * name, size_t size, void * module);
ngx_int_t ngx_shm_dict_test_delete(ngx_str_t * key);
ngx_int_t ngx_shm_dict_test_keys(ngx_bool_t include_expires, size_t * keys);
ngx_int_t ngx_shm_dict_test_get_keys(ngx_bool_t include_expires, ngx_pool_t * pool, ngx_shm_dict_keys_t * keys);
ngx_int_t ngx_shm_dict_test_flush_all();
ngx_int_t ngx_shm_dict_test_flush_expired(int count);

#endif // __ngx_shm_dict_test_utils_20160113121419_h__
