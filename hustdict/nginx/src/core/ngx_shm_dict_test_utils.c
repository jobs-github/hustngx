#include "ngx_shm_dict_test_utils.h"

static ngx_str_t KEY = ngx_string("key");
static ngx_str_t TYPE = ngx_string("type");
static ngx_str_t EXPIRE = ngx_string("expire");
static ngx_str_t METHOD = ngx_string("method");
static ngx_str_t ALL = ngx_string("all");
static ngx_str_t TRUE = ngx_string("true");

static ngx_shm_zone_t * g_zone = NULL;

static ngx_bool_t __is_valid(u_char c)
{
    if (c >= '0' && c <= '9')
    {
        return true;
    }
    if (c >= 'a' && c <= 'z')
    {
        return true;
    }
    if (c >= 'A' && c <= 'Z')
    {
        return true;
    }
    if ('_' == c || ':' == c || '.' == c || '-' == c)
    {
        return true;
    }
    return false;
}

ngx_bool_t __check_key(ngx_str_t * key)
{
    if (!key || !key->data || key->len < 1 || key->len > 255)
    {
        return false;
    }

    size_t i = 0;
    for (i = 0; i < key->len; ++i)
    {
        if (!__is_valid(key->data[i]))
        {
            return false;
        }
    }
    return true;
}

ngx_str_t ngx_shm_dict_test_from_int(ngx_pool_t * pool, int64_t val)
{
    ngx_str_t str = { 0, 0 };
    str.data = ngx_palloc(pool, 21);
    sprintf((char *)str.data, "%ld", val);
    str.len = strlen((const char *)str.data);
    return str;
}

ngx_str_t ngx_shm_dict_test_from_uint(ngx_pool_t * pool, uint64_t val)
{
    ngx_str_t str = { 0, 0 };
    str.data = ngx_palloc(pool, 21);
    sprintf((char *)str.data, "%lu", val);
    str.len = strlen((const char *)str.data);
    return str;
}

ngx_str_t ngx_shm_dict_test_from_double(ngx_pool_t * pool, double val)
{
    ngx_str_t str = { 0, 0 };
    str.data = ngx_palloc(pool, 4096);
    sprintf((char *)str.data, "%lf", val);
    str.len = strlen((const char *)str.data);
    return str;
}

ngx_int_t ngx_shm_dict_test_init(ngx_conf_t * cf, ngx_str_t * name, size_t size, void * module)
{
    g_zone = ngx_shm_dict_init(cf, name, size, module);
    return !g_zone ? NGX_ERROR : NGX_OK;
}

ngx_shm_zone_t * ngx_shm_dict_test_get_zone()
{
    return g_zone;
}

ngx_bool_t __get_key(ngx_http_request_t *r, ngx_str_t * key)
{
    if (NGX_OK != ngx_http_arg(r, KEY.data, KEY.len, key))
    {
        return false;
    }
    return __check_key(key);
}

ngx_int_t ngx_shm_dict_test_get_read_args(ngx_http_request_t *r, ngx_shm_dict_read_args_t * args)
{
    if (!__get_key(r, &args->key))
    {
        return NGX_ERROR;
    }
    if (NGX_OK != ngx_http_arg(r, TYPE.data, TYPE.len, &args->type))
    {
        return NGX_ERROR;
    }
    return NGX_OK;
}

ngx_int_t __get_args(ngx_http_request_t *r, ngx_str_t * key, ngx_str_t * type, uint64_t * expire)
{
    if (!__get_key(r, key))
    {
        return NGX_ERROR;
    }
    if (NGX_OK != ngx_http_arg(r, TYPE.data, TYPE.len, type))
    {
        return NGX_ERROR;
    }
    ngx_str_t exp;
    if (NGX_OK == ngx_http_arg(r, EXPIRE.data, EXPIRE.len, &exp))
    {
        *expire = ngx_parse_time(&exp, 1);
    }
    else
    {
        *expire = 0;
    }
    return NGX_OK;
}

ngx_int_t ngx_shm_dict_test_get_write_args(ngx_bool_t need_val, ngx_http_request_t *r, ngx_shm_dict_write_args_t * args)
{
    if (need_val)
    {
        char * val = ngx_http_get_param_val(&r->args, "val", r->pool);
        if (!val)
        {
            return NGX_ERROR;
        }
        args->val.data = (u_char *)val;
        args->val.len = strlen(val);
    }
    if (NGX_OK != ngx_http_arg(r, METHOD.data, METHOD.len, &args->method))
    {
        return NGX_ERROR;
    }
    return __get_args(r, &args->key, &args->type, &args->expire);
}

ngx_int_t ngx_shm_dict_test_get_mod_args(ngx_http_request_t *r, ngx_shm_dict_mod_args_t * args)
{
    char * val = ngx_http_get_param_val(&r->args, "delta", r->pool);
    if (!val)
    {
        return NGX_ERROR;
    }
    args->delta.data = (u_char *)val;
    args->delta.len = strlen(val);

    return __get_args(r, &args->key, &args->type, &args->expire);
}

ngx_int_t ngx_shm_dict_test_get_key(ngx_http_request_t *r, ngx_str_t * key)
{
    if (!__get_key(r, key))
    {
        return NGX_ERROR;
    }
    return NGX_OK;
}

ngx_bool_t ngx_shm_dict_test_include_expires(ngx_http_request_t *r)
{
    ngx_str_t all;
    if (NGX_OK != ngx_http_arg(r, ALL.data, ALL.len, &all))
    {
        return false;
    }
    return 0 == ngx_strncasecmp(all.data, TRUE.data, TRUE.len);
}

ngx_int_t ngx_shm_dict_test_read_base(
    ngx_shm_dict_test_read_item_t dict[],
    size_t len,
    ngx_shm_dict_read_args_t * args,
    ngx_http_request_t *r)
{
    size_t i = 0;
    for (i = 0; i < len; ++i)
    {
        if (0 == ngx_strncasecmp(args->type.data, dict[i].type.data, dict[i].type.len))
        {
            return dict[i].handler(&args->key, r);
        }
    }
    return NGX_ERROR;
}

ngx_int_t ngx_shm_dict_test_write_base(
    ngx_shm_dict_test_write_item_t dict[],
    size_t len,
    ngx_shm_dict_write_args_t * args)
{
    size_t i = 0;
    for (i = 0; i < len; ++i)
    {
        if (0 == ngx_strncasecmp(args->type.data, dict[i].type.data, dict[i].type.len)
            && 0 == ngx_strncasecmp(args->method.data, dict[i].method.data, dict[i].method.len))
        {
            return dict[i].write(args);
        }
    }
    return NGX_ERROR;
}

ngx_int_t ngx_shm_dict_test_mod_base(
    ngx_shm_dict_test_mod_item_t dict[],
    size_t len,
    ngx_pool_t * pool,
    ngx_shm_dict_mod_args_t * args)
{
    size_t i = 0;
    for (i = 0; i < len; ++i)
    {
        if (0 == ngx_strncasecmp(args->type.data, dict[i].type.data, dict[i].type.len))
        {
            return dict[i].handler(pool, args);
        }
    }
    return NGX_ERROR;
}

ngx_int_t ngx_shm_dict_test_delete(ngx_str_t * key)
{
    return ngx_shm_dict_delete(g_zone, key);
}

ngx_int_t ngx_shm_dict_test_keys(ngx_bool_t include_expires, size_t * keys)
{
    return ngx_shm_dict_get_total_keys(g_zone, include_expires, keys);
}

ngx_int_t ngx_shm_dict_test_get_keys(ngx_bool_t include_expires, ngx_pool_t * pool, ngx_shm_dict_keys_t * keys)
{
    return ngx_shm_dict_get_keys(g_zone, include_expires, pool, keys);
}

ngx_int_t ngx_shm_dict_test_flush_all()
{
    return ngx_shm_dict_flush_all(g_zone);
}

ngx_int_t ngx_shm_dict_test_flush_expired(int count)
{
    return ngx_shm_dict_flush_expired(g_zone, count);
}
