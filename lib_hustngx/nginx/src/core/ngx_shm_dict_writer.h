#ifndef __ngx_shm_dict_writer_20160106101305_h__
#define __ngx_shm_dict_writer_20160106101305_h__

#include "ngx_shm_dict_utils.h"

#define NGX_SHM_DICT_ADD         0x0001
#define NGX_SHM_DICT_REPLACE     0x0002
#define NGX_SHM_DICT_SAFE_STORE  0x0004
#define NGX_SHM_DICT_DELETE      0x0008

int ngx_shm_dict_set_impl(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags,
    int flags);

// will delete the oldest data when no space
int ngx_shm_dict_add(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags);
// will return NGX_ERROR when no space
int ngx_shm_dict_safe_add(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags);
int ngx_shm_dict_replace(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags);
int ngx_shm_dict_set(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags);
int ngx_shm_dict_safe_set(
    ngx_shm_zone_t * zone,
    ngx_str_t * key,
    ngx_str_t * value,
    uint8_t value_type,
    uint32_t exptime,
    uint32_t user_flags);

#endif // __ngx_shm_dict_writer_20160106101305_h__
