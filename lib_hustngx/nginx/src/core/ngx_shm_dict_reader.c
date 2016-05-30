#include "ngx_shm_dict_reader.h"

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_get
int ngx_shm_dict_get(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * data,
    uint8_t * value_type,
    uint32_t * exptime,
    uint32_t * user_flags)
{
    if (!zone || !key || !data || key->len == 0 || key->len > 65535)
    {
        return NGX_ERROR;
    }
    ngx_shm_dict_ctx_t * ctx = zone->data;
    uint32_t hash = ngx_shm_dict_crc32(key->data, key->len);
    ngx_shmtx_lock(&ctx->shpool->mutex);
    ngx_shm_dict_expire(ctx, 1);
    ngx_shm_dict_node_t * sd = NULL;
    ngx_int_t rc = ngx_shm_dict_lookup(zone, hash, key->data, key->len, &sd);
    if (rc == NGX_DECLINED || rc == NGX_DONE)
    {
        ngx_shmtx_unlock(&ctx->shpool->mutex);
        return NGX_ERROR;
    }
    data->data = sd->data + sd->key_len;
    data->len = (size_t) sd->value_len;
    if (value_type)
    {
        *value_type = sd->value_type;
    }
    if (user_flags)
    {
        *user_flags = sd->user_flags;
    }
    if (exptime)
    {
        if (sd->expires == 0)
        {
            *exptime = 0;
        }
        else
        {
            ngx_time_t * tp = ngx_timeofday();
            *exptime = (sd->expires - ((uint64_t) tp->sec * 1000 + tp->msec)) / 1000;
        }
    }
    ngx_shmtx_unlock(&ctx->shpool->mutex);
    return NGX_OK;
}

int ngx_shm_dict_get_base(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    uint8_t expect_vt,
    ngx_shm_dict_get_val_t get,
    void * ctx,
    void * val)
{
    uint8_t vt = SHM_DICT_NULL;
    ngx_str_t data = ngx_null_string;
    int rc = ngx_shm_dict_get(zone, key, &data, &vt, NULL, NULL);
    if (rc == NGX_OK)
    {
        if (vt != expect_vt)
        {
            rc = NGX_ERROR;
        }
        else
        {
            rc = get(&data, ctx, val);
        }
    }
    return rc;
}
