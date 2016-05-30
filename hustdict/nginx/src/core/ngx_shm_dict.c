#include "ngx_shm_dict.h"
#include "ngx_shm_dict_reader.h"
#include "ngx_shm_dict_writer.h"

ngx_shm_zone_t* ngx_shm_dict_init(ngx_conf_t *cf, ngx_str_t * name, size_t size, void * module)
{
    ngx_shm_dict_ctx_t * ctx = ngx_pcalloc(cf->pool, sizeof(ngx_shm_dict_ctx_t));
    if (!ctx)
    {
        return NULL;
    }

    ngx_shm_zone_t * zone = ngx_shared_memory_add(cf, name, size, module);
    if (!zone)
    {
        ngx_pfree(cf->pool, ctx);
        ctx = NULL;
        return NULL;
    }

    ctx->name = *name;
    ctx->log = &cf->cycle->new_log;

    if (zone->data)
    {
        ctx = zone->data;
        return NULL;
    }

    zone->init = ngx_shm_dict_init_zone;
    zone->data = ctx;

    return zone;
}

int ngx_shm_dict_delete(ngx_shm_zone_t * zone, ngx_str_t * key)
{
    if (!zone || !key)
    {
        return NGX_ERROR;
    }
    return ngx_shm_dict_set_impl(zone, key, NULL, SHM_DICT_NULL, 0, 0, NGX_SHM_DICT_DELETE);
}

int ngx_shm_dict_traverse(ngx_shm_zone_t * zone, ngx_shm_dict_tarverse_cb cb, void * data)
{
    if (!zone)
    {
        return NGX_ERROR;
    }
    ngx_shm_dict_ctx_t *ctx = zone->data;
    ngx_shmtx_lock(&ctx->shpool->mutex);
    ngx_queue_t *q;
    for (q = ngx_queue_head(&ctx->sh->queue); q != ngx_queue_sentinel(&ctx->sh->queue); q = ngx_queue_next(q))
    {
        ngx_shm_dict_node_t * sd = ngx_queue_data(q, ngx_shm_dict_node_t, queue);
        if (NGX_OK != cb(sd, data))
        {
            ngx_shmtx_unlock(&ctx->shpool->mutex);
            return NGX_ERROR;
        }
    }
    ngx_shmtx_unlock(&ctx->shpool->mutex);
    return NGX_OK;
}

typedef struct
{
    uint8_t include_expires;
    uint64_t now;
    size_t total;
} ngx_shm_dict_get_total_keys_ctx_t;

static int __on_get_total_keys(ngx_shm_dict_node_t * node, void * data)
{
    ngx_shm_dict_get_total_keys_ctx_t * ctx = (ngx_shm_dict_get_total_keys_ctx_t *)data;
    if (ctx->include_expires || (0 == node->expires || node->expires > ctx->now))
    {
        ++ctx->total;
    }
    return NGX_OK;
}

int ngx_shm_dict_get_total_keys(ngx_shm_zone_t * zone, uint8_t include_expires, size_t * total)
{
    if (!zone || !total)
    {
        return NGX_ERROR;
    }
    ngx_shm_dict_get_total_keys_ctx_t ctx = { include_expires, 0, 0 };
    ngx_time_t * tp = ngx_timeofday();
    ctx.now = (uint64_t) tp->sec * 1000 + tp->msec;
    int rc = ngx_shm_dict_traverse(zone, __on_get_total_keys, &ctx);
    *total = ctx.total;
    return rc;
}

typedef struct
{
    uint8_t include_expires;
    uint64_t now;
    ngx_pool_t * pool;
    ngx_shm_dict_keys_t * keys;
} ngx_shm_dict_get_keys_ctx_t;

static int __on_get_keys(ngx_shm_dict_node_t * node, void * data)
{
    ngx_shm_dict_get_keys_ctx_t * ctx = (ngx_shm_dict_get_keys_ctx_t *)data;
    if (ctx->include_expires || (0 == node->expires || node->expires > ctx->now))
    {
        ngx_str_t src = { node->key_len, (u_char *)node->data };
        ngx_shm_dict_copy_str(&src, ctx->pool, ctx->keys->arr + ctx->keys->size);
        ++ctx->keys->size;
    }
    return NGX_OK;
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_get_keys
int ngx_shm_dict_get_keys(
    ngx_shm_zone_t * zone,
    uint8_t include_expires,
    ngx_pool_t * pool,
    ngx_shm_dict_keys_t * keys)
{
    if (!zone || !pool || !keys)
    {
        return NGX_ERROR;
    }
    ngx_shm_dict_get_total_keys_ctx_t ctx = { include_expires, 0, 0 };
    ngx_time_t * tp = ngx_timeofday();
    ctx.now = (uint64_t) tp->sec * 1000 + tp->msec;
    if (NGX_OK != ngx_shm_dict_traverse(zone, __on_get_total_keys, &ctx))
    {
        return NGX_ERROR;
    }
    keys->size = 0;
    keys->arr = ngx_palloc(pool, sizeof(ngx_str_t) * ctx.total);

    ngx_shm_dict_get_keys_ctx_t keys_ctx = { include_expires, ctx.now, pool, keys };
    return ngx_shm_dict_traverse(zone, __on_get_keys, &keys_ctx);
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_flush_all
int ngx_shm_dict_flush_all(ngx_shm_zone_t * zone)
{
    if (!zone)
    {
        return NGX_ERROR;
    }
    ngx_shm_dict_ctx_t * ctx = zone->data;
    ngx_shmtx_lock(&ctx->shpool->mutex);
    ngx_queue_t *q;
    for (q = ngx_queue_head(&ctx->sh->queue); q != ngx_queue_sentinel(&ctx->sh->queue); q = ngx_queue_next(q))
    {
        ngx_shm_dict_node_t * sd = ngx_queue_data(q, ngx_shm_dict_node_t, queue);
        sd->expires = 1;
    }
    ngx_shm_dict_expire(ctx, 0);
    ngx_shmtx_unlock(&ctx->shpool->mutex);
    return NGX_OK;
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_flush_expired
int ngx_shm_dict_flush_expired(ngx_shm_zone_t * zone, int attempts)
{
    if (!zone)
    {
        return NGX_ERROR;
    }
    ngx_shm_dict_ctx_t * ctx = zone->data;
    ngx_shmtx_lock(&ctx->shpool->mutex);
    if (ngx_queue_empty(&ctx->sh->queue))
    {
        return NGX_OK;
    }
    ngx_time_t * tp = ngx_timeofday();
    uint64_t now = (uint64_t) tp->sec * 1000 + tp->msec;
    ngx_queue_t * q = ngx_queue_last(&ctx->sh->queue);
    int freed = 0;
    while (q != ngx_queue_sentinel(&ctx->sh->queue))
    {
        ngx_queue_t * prev = ngx_queue_prev(q);
        ngx_shm_dict_node_t * sd = ngx_queue_data(q, ngx_shm_dict_node_t, queue);
        if (sd->expires != 0 && sd->expires <= now)
        {
            ngx_queue_remove(q);
            ngx_rbtree_node_t * node = (ngx_rbtree_node_t *) (
                (u_char *) sd - offsetof(ngx_rbtree_node_t, color));
            ngx_rbtree_delete(&ctx->sh->rbtree, node);
            ngx_slab_free_locked(ctx->shpool, node);
            freed++;
            if (attempts && freed == attempts)
            {
                break;
            }
        }
        q = prev;
    }
    ngx_shmtx_unlock(&ctx->shpool->mutex);
    return freed;
}
