#ifndef __ngx_shm_dict_20160104191736_h__
#define __ngx_shm_dict_20160104191736_h__

#include "ngx_shm_dict_code.h"

typedef struct
{
    ngx_str_t * arr;
    size_t size;
} ngx_shm_dict_keys_t;

ngx_shm_zone_t * ngx_shm_dict_init(ngx_conf_t * cf, ngx_str_t * name, size_t size, void * module);

int ngx_shm_dict_delete(ngx_shm_zone_t * zone, ngx_str_t * key);

typedef int (*ngx_shm_dict_tarverse_cb)(ngx_shm_dict_node_t * node, void * data);
int ngx_shm_dict_traverse(ngx_shm_zone_t * zone, ngx_shm_dict_tarverse_cb cb, void * data);

int ngx_shm_dict_get_total_keys(ngx_shm_zone_t * zone, uint8_t include_expires, size_t * total);
int ngx_shm_dict_get_keys(
    ngx_shm_zone_t * zone,
    uint8_t include_expires,
    ngx_pool_t * pool,
    ngx_shm_dict_keys_t * keys);

int ngx_shm_dict_flush_all(ngx_shm_zone_t * zone);
int ngx_shm_dict_flush_expired(ngx_shm_zone_t * zone, int attempts);

#endif // __ngx_shm_dict_20160104191736_h__
