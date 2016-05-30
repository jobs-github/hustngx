#ifndef __ngx_shm_dict_utils_20160105110337_h__
#define __ngx_shm_dict_utils_20160105110337_h__

#include <ngx_config.h>
#include <ngx_core.h>
#include <stdint.h>

#define SHM_DICT_NULL    0
#define SHM_DICT_INT8    1
#define SHM_DICT_UINT8   2
#define SHM_DICT_INT16   3
#define SHM_DICT_UINT16  4
#define SHM_DICT_INT32   5
#define SHM_DICT_UINT32  6
#define SHM_DICT_INT64   7
#define SHM_DICT_UINT64  8
#define SHM_DICT_DOUBLE  9
#define SHM_DICT_STRING  10
#define SHM_DICT_BINARY  11 // user defined struct / raw binary data

typedef struct
{
    size_t len;
    void * data;
} ngx_binary_t;

#pragma pack(push)
#pragma pack(4)
typedef struct
{
    u_char color;
    u_char dummy;
    u_short key_len;
    ngx_queue_t queue;
    uint64_t expires;
    uint32_t value_len;
    uint32_t user_flags;
    uint8_t value_type;
    u_char data[1];
} ngx_shm_dict_node_t;
#pragma pack(pop)

typedef struct
{
    ngx_rbtree_t rbtree;
    ngx_rbtree_node_t sentinel;
    ngx_queue_t queue;
} ngx_shm_dict_shctx_t;

typedef struct
{
    ngx_shm_dict_shctx_t * sh;
    ngx_slab_pool_t * shpool;
    ngx_str_t name;
    ngx_log_t * log;
} ngx_shm_dict_ctx_t;

uint32_t ngx_shm_dict_crc32(u_char * p, size_t len);
ngx_int_t ngx_shm_dict_copy_str(const ngx_str_t * src, ngx_pool_t * pool, ngx_str_t * dst);
ngx_int_t ngx_shm_dict_copy_bin(const ngx_str_t * src, ngx_pool_t * pool, ngx_binary_t * dst);

ngx_int_t ngx_shm_dict_init_zone(ngx_shm_zone_t * shm_zone, void *data);
int ngx_shm_dict_expire(ngx_shm_dict_ctx_t *ctx, ngx_uint_t n);
void ngx_shm_dict_rbtree_insert_value(
    ngx_rbtree_node_t * temp,
    ngx_rbtree_node_t * node,
    ngx_rbtree_node_t * sentinel);
ngx_int_t ngx_shm_dict_lookup(
    ngx_shm_zone_t *shm_zone,
    ngx_uint_t hash,
    u_char *kdata,
    size_t klen,
    ngx_shm_dict_node_t **sdp);

#endif // __ngx_shm_dict_utils_20160105110337_h__
