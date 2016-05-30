#include "ngx_shm_dict_writer.h"


// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_set_helper
// warning: please do NOT modify this function!!! (so many fuck goto's!!!)
int ngx_shm_dict_set_impl(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags,
    int flags)
{
    int i, n;
    uint32_t hash;
    ngx_int_t rc;
    ngx_shm_dict_ctx_t * ctx;
    ngx_shm_dict_node_t * sd;
    u_char * p;
    ngx_rbtree_node_t * node;
    ngx_time_t * tp;

    ctx = zone->data;
    if (key->len == 0 || key->len > 65535)
    {
        return NGX_ERROR;
    }

    hash = ngx_shm_dict_crc32(key->data, key->len);
    ngx_shmtx_lock(&ctx->shpool->mutex);
#if 1
    ngx_shm_dict_expire(ctx, 1);
#endif
    rc = ngx_shm_dict_lookup(zone, hash, key->data, key->len, &sd);
    if (flags & NGX_SHM_DICT_DELETE)
    {
        if (rc == NGX_DECLINED || rc == NGX_DONE)
        {
            ngx_shmtx_unlock(&ctx->shpool->mutex);
            // not exists
            return NGX_ERROR;
        }
    }
    if (flags & NGX_SHM_DICT_REPLACE)
    {
        if (rc == NGX_DECLINED || rc == NGX_DONE)
        {
            ngx_shmtx_unlock(&ctx->shpool->mutex);
            // not exists
            return NGX_ERROR;
        }
        // rc == NGX_OK
        goto replace;
    }
    if (flags & NGX_SHM_DICT_ADD)
    {
        if (rc == NGX_OK)
        {
            ngx_shmtx_unlock(&ctx->shpool->mutex);
            // exists
            return NGX_ERROR;
        }
        if (rc == NGX_DONE)
        {
            // exists but expired
            goto replace;
        }
        // rc == NGX_DECLINED
        goto insert;
    }
    if (rc == NGX_OK || rc == NGX_DONE)
    {
        if (value_type == SHM_DICT_NULL)
        {
            goto remove;
        }
replace:
        if (value->data && value->len == (size_t) sd->value_len)
        {
            // shmap set: found old entry and value size matched, reusing it
            ngx_queue_remove(&sd->queue);
            ngx_queue_insert_head(&ctx->sh->queue, &sd->queue);
            sd->key_len = key->len;
            if (exptime > 0)
            {
                tp = ngx_timeofday();
                sd->expires = (uint64_t) tp->sec * 1000 + tp->msec + exptime * 1000;
            }
            else
            {
                sd->expires = 0;
            }
            sd->user_flags = user_flags;
            sd->value_len = (uint32_t) value->len;
            sd->value_type = value_type;
            p = ngx_copy(sd->data, key->data, key->len);
            ngx_memcpy(p, value->data, value->len);
            ngx_shmtx_unlock(&ctx->shpool->mutex);
            return NGX_OK;
        }

        // shmap set: found old entry but value size NOT matched, removing it first
remove:
        ngx_queue_remove(&sd->queue);
        node = (ngx_rbtree_node_t *)((u_char *) sd - offsetof(ngx_rbtree_node_t, color));
        ngx_rbtree_delete(&ctx->sh->rbtree, node);
        ngx_slab_free_locked(ctx->shpool, node);
        if (value_type == SHM_DICT_NULL)
        {
            ngx_shmtx_unlock(&ctx->shpool->mutex);
            return NGX_OK;
        }
    }
insert:
    // rc == NGX_DECLINED or value size unmatch
    if (value == NULL || value->data == NULL)
    {
        ngx_shmtx_unlock(&ctx->shpool->mutex);
        // shmap add failed! value is null!
        return NGX_ERROR;
    }
    n = offsetof(ngx_rbtree_node_t, color)
        + offsetof(ngx_shm_dict_node_t, data)
        + key->len
        + value->len;
    // shmap set: creating a new entry
    node = ngx_slab_alloc_locked(ctx->shpool, n);
    if (node == NULL)
    {
        if (flags & NGX_SHM_DICT_SAFE_STORE)
        {
            ngx_shmtx_unlock(&ctx->shpool->mutex);
            // shmap add failed! no memory!
            return NGX_ERROR;
        }

        // shmap set: overriding non-expired items due to memory shortage for entry
        for (i = 0; i < 30; i++)
        {
            if (ngx_shm_dict_expire(ctx, 0) == 0)
            {
                break;
            }
            node = ngx_slab_alloc_locked(ctx->shpool, n);
            if (node != NULL)
            {
                goto allocated;
            }
        }
        ngx_shmtx_unlock(&ctx->shpool->mutex);
        // shmap add failed! no memory!
        return NGX_ERROR;
    }
allocated:
    sd = (ngx_shm_dict_node_t *) &node->color;
    node->key = hash;
    sd->key_len = key->len;
    if (exptime > 0)
    {
        tp = ngx_timeofday();
        sd->expires = (uint64_t) tp->sec * 1000 + tp->msec + exptime * 1000;
    }
    else
    {
        sd->expires = 0;
    }
    sd->user_flags = user_flags;
    sd->value_len = (uint32_t) value->len;
    sd->value_type = value_type;
    p = ngx_copy(sd->data, key->data, key->len);
    ngx_memcpy(p, value->data, value->len);
    ngx_rbtree_insert(&ctx->sh->rbtree, node);
    ngx_queue_insert_head(&ctx->sh->queue, &sd->queue);
    ngx_shmtx_unlock(&ctx->shpool->mutex);
    return NGX_OK;
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_add
int ngx_shm_dict_add(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags)
{
    if (!zone || !key || !value)
    {
        return NGX_ERROR;
    }
    return ngx_shm_dict_set_impl(zone, key, value, value_type, exptime, user_flags, NGX_SHM_DICT_ADD);
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_safe_add
int ngx_shm_dict_safe_add(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags)
{
    if (!zone || !key || !value)
    {
        return NGX_ERROR;
    }
    return ngx_shm_dict_set_impl(zone, key, value, value_type, exptime, user_flags, NGX_SHM_DICT_ADD|NGX_SHM_DICT_SAFE_STORE);
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_replace
int ngx_shm_dict_replace(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags)
{
    if (!zone || !key || !value)
    {
        return NGX_ERROR;
    }
    return ngx_shm_dict_set_impl(zone, key, value, value_type, exptime, user_flags, NGX_SHM_DICT_REPLACE);
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_set
int ngx_shm_dict_set(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags)
{
    if (!zone || !key || !value)
    {
        return NGX_ERROR;
    }
    return ngx_shm_dict_set_impl(zone, key, value, value_type, exptime, user_flags, 0);
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_safe_set
int ngx_shm_dict_safe_set(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags)
{
    if (!zone || !key || !value)
    {
        return NGX_ERROR;
    }
    return ngx_shm_dict_set_impl(zone, key, value, value_type, exptime, user_flags, NGX_SHM_DICT_SAFE_STORE);
}
