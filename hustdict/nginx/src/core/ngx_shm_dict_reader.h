#ifndef __ngx_shm_dict_reader_20160106101249_h__
#define __ngx_shm_dict_reader_20160106101249_h__

#include "ngx_shm_dict_utils.h"

int ngx_shm_dict_get(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * data,
    uint8_t * value_type,
    uint32_t * exptime,
    uint32_t * user_flags);

typedef int (*ngx_shm_dict_get_val_t)(const ngx_str_t * data, void * ctx, void * val);

int ngx_shm_dict_get_base(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    uint8_t expect_vt,
    ngx_shm_dict_get_val_t get,
    void * ctx,
    void * val);

#endif // __ngx_shm_dict_reader_20160106101249_h__
