#include "ngx_shm_dict_utils.h"

uint32_t ngx_shm_dict_crc32(u_char * p, size_t len)
{
    if (len == sizeof(ngx_int_t))
    {
        uint32_t* pi = (uint32_t*) p;
        return *pi;
    }
    else
    {
        return ngx_crc32_short(p, len);
    }
}

ngx_int_t ngx_shm_dict_copy_str(const ngx_str_t * src, ngx_pool_t * pool, ngx_str_t * dst)
{
    if (!src || !src->data || src->len < 1 || !pool || !dst)
    {
        return NGX_ERROR;
    }
    dst->data = ngx_palloc(pool, src->len + 1);
    if (!dst->data)
    {
        return NGX_ERROR;
    }
    memcpy(dst->data, src->data, src->len);
    dst->data[src->len] = '\0';
    dst->len = src->len;
    return NGX_OK;
}

ngx_int_t ngx_shm_dict_copy_bin(const ngx_str_t * src, ngx_pool_t * pool, ngx_binary_t * dst)
{
    if (!src || !src->data || src->len < 1 || !pool || !dst)
    {
        return NGX_ERROR;
    }
    if (!dst->data)
    {
        dst->data = ngx_palloc(pool, src->len);
        if (!dst->data)
        {
            return NGX_ERROR;
        }
    }
    memcpy(dst->data, src->data, src->len);
    dst->len = src->len;
    return NGX_OK;
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_init_zone
ngx_int_t ngx_shm_dict_init_zone(ngx_shm_zone_t * shm_zone, void *data)
{
    ngx_shm_dict_ctx_t * octx = data;
    ngx_shm_dict_ctx_t * ctx = shm_zone->data;

    if (octx)
    {
        ctx->sh = octx->sh;
        ctx->shpool = octx->shpool;
        return NGX_OK;
    }

    ctx->shpool = (ngx_slab_pool_t *) shm_zone->shm.addr;
    if (shm_zone->shm.exists)
    {
        ctx->sh = ctx->shpool->data;
        return NGX_OK;
    }

    ctx->sh = ngx_slab_alloc(ctx->shpool, sizeof(ngx_shm_dict_shctx_t));
    if (ctx->sh == NULL)
    {
        return NGX_ERROR;
    }

    ctx->shpool->data = ctx->sh;
    ngx_rbtree_init(&ctx->sh->rbtree, &ctx->sh->sentinel, ngx_shm_dict_rbtree_insert_value);
    ngx_queue_init(&ctx->sh->queue);

    size_t len = sizeof(" in ngx_shm_dict zone \"\"") + shm_zone->shm.name.len;
    ctx->shpool->log_ctx = ngx_slab_alloc(ctx->shpool, len);
    if (!ctx->shpool->log_ctx)
    {
        return NGX_ERROR;
    }
    return NGX_OK;
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_expire
int ngx_shm_dict_expire(ngx_shm_dict_ctx_t *ctx, ngx_uint_t n)
{
    ngx_time_t * tp = ngx_timeofday();
    uint64_t now = (uint64_t) tp->sec * 1000 + tp->msec;
    // n == 1 deletes one or two expired entries
    // n == 0 deletes oldest entry by force and one or two zero rate entries
    int freed = 0;
    while (n < 3)
    {
        if (ngx_queue_empty(&ctx->sh->queue))
        {
            return freed;
        }
        ngx_queue_t * q = ngx_queue_last(&ctx->sh->queue);
        ngx_shm_dict_node_t * sd = ngx_queue_data(q, ngx_shm_dict_node_t, queue);
        if (n++ != 0)
        {
            if (sd->expires == 0)
            {
                return freed;
            }
            int64_t ms = sd->expires - now;
            if (ms > 0)
            {
                return freed;
            }
        }
        ngx_queue_remove(q);
        ngx_rbtree_node_t * node = (ngx_rbtree_node_t *) (
            (u_char *) sd - offsetof(ngx_rbtree_node_t, color));
        ngx_rbtree_delete(&ctx->sh->rbtree, node);
        ngx_slab_free_locked(ctx->shpool, node);
        freed++;
    }
    return freed;
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_rbtree_insert_value
void ngx_shm_dict_rbtree_insert_value(
    ngx_rbtree_node_t * temp,
    ngx_rbtree_node_t * node,
    ngx_rbtree_node_t * sentinel)
{
    ngx_rbtree_node_t ** p;
    ngx_shm_dict_node_t * sdn, * sdnt;

    for (;;)
    {
        if (node->key < temp->key)
        {
            p = &temp->left;
        }
        else if (node->key > temp->key)
        {
            p = &temp->right;
        }
        else
        {
            sdn = (ngx_shm_dict_node_t *) &node->color;
            sdnt = (ngx_shm_dict_node_t *) &temp->color;
            p = ngx_memn2cmp(sdn->data, sdnt->data, sdn->key_len, sdnt->key_len)
                < 0 ? &temp->left : &temp->right;
        }
        if (*p == sentinel)
        {
            break;
        }
        temp = *p;
    }
    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ngx_rbt_red(node);
}

// reference: lua-nginx-module:ngx_http_lua_shdict.c:ngx_http_lua_shdict_lookup
ngx_int_t ngx_shm_dict_lookup(
    ngx_shm_zone_t *shm_zone,
    ngx_uint_t hash,
    u_char *kdata,
    size_t klen,
    ngx_shm_dict_node_t **sdp)
{
    ngx_shm_dict_ctx_t * ctx = shm_zone->data;
    ngx_rbtree_node_t * node = ctx->sh->rbtree.root;
    ngx_rbtree_node_t * sentinel = ctx->sh->rbtree.sentinel;
    while (node != sentinel)
    {
        if (hash < node->key)
        {
            node = node->left;
            continue;
        }
        if (hash > node->key)
        {
            node = node->right;
            continue;
        }
        ngx_shm_dict_node_t * sd = (ngx_shm_dict_node_t *) &node->color;
        ngx_int_t rc = ngx_memn2cmp(kdata, sd->data, klen, (size_t) sd->key_len);
        if (0 == rc)
        {
            ngx_queue_remove(&sd->queue);
            ngx_queue_insert_head(&ctx->sh->queue, &sd->queue);
            *sdp = sd;
            if (sd->expires != 0)
            {
                ngx_time_t * tp = ngx_timeofday();
                uint64_t now = (uint64_t) tp->sec * 1000 + tp->msec;
                int64_t ms = sd->expires - now;
                if (ms < 0)
                {
                    return NGX_DONE;
                }
            }
            return NGX_OK;
        }
        node = (rc < 0) ? node->left : node->right;
    }
    *sdp = NULL;
    return NGX_DECLINED;
}
