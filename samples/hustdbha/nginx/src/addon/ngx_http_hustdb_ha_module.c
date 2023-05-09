#include <dlfcn.h>
#include <pthread.h>
#include <c_dict.h>
#include <ngx_http_fetch.h>
#include "hustdb_ha_table_def.h"
#include "hustdb_ha_handler.h"

static ngx_http_hustdb_ha_main_conf_t * g_mcf = NULL;

static ngx_int_t ngx_http_hustdb_ha_handler(ngx_http_request_t *r);
static char *ngx_http_hustdb_ha(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_hustdb_ha_init_module(ngx_cycle_t * cycle);
static ngx_int_t ngx_http_hustdb_ha_init_process(ngx_cycle_t * cycle);
static void ngx_http_hustdb_ha_exit_process(ngx_cycle_t * cycle);
static void ngx_http_hustdb_ha_exit_master(ngx_cycle_t * cycle);
static char * ngx_http_debug_sync(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_zlog_mdc(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_hustdbtable_file(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_hustdb_ha_shm_name(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_hustdb_ha_shm_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_public_pem(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_identifier_cache_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_identifier_timeout(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_fetch_req_pool_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_keepalive_cache_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_connection_cache_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_fetch_connect_timeout(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_fetch_send_timeout(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_fetch_read_timeout(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_fetch_buffer_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_sync_port(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_sync_status_uri(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_sync_user(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_sync_passwd(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_binlog_uri(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static void * ngx_http_hustdb_ha_create_main_conf(ngx_conf_t *cf);
static char * ngx_http_hustdb_ha_init_main_conf(ngx_conf_t * cf, void * conf);
static ngx_int_t ngx_http_hustdb_ha_postconfiguration(ngx_conf_t * cf);

static ngx_http_request_item_t hustdb_ha_handler_dict[] =
{
    {
        ngx_string("/version"),
        ngx_null_string,
        hustdb_ha_version_handler
    },
    {
        ngx_string("/get"),
        ngx_string("/hustdb/get"),
        hustdb_ha_get_handler
    },
    {
        ngx_string("/get2"),
        ngx_string("/hustdb/get"),
        hustdb_ha_get2_handler
    },
    {
        ngx_string("/exist"),
        ngx_string("/hustdb/exist"),
        hustdb_ha_exist_handler
    },
    {
        ngx_string("/put"),
        ngx_string("/hustdb/put"),
        hustdb_ha_put_handler
    },
    {
        ngx_string("/del"),
        ngx_string("/hustdb/del"),
        hustdb_ha_del_handler
    },
    {
        ngx_string("/keys"),
        ngx_string("/hustdb/keys"),
        hustdb_ha_keys_handler
    },
    {
        ngx_string("/hset"),
        ngx_string("/hustdb/hset"),
        hustdb_ha_hset_handler
    },
    {
        ngx_string("/hincrby"),
        ngx_string("/hustdb/hincrby"),
        hustdb_ha_hincrby_handler
    },
    {
        ngx_string("/hget"),
        ngx_string("/hustdb/hget"),
        hustdb_ha_hget_handler
    },
    {
        ngx_string("/hget2"),
        ngx_string("/hustdb/hget"),
        hustdb_ha_hget2_handler
    },
    {
        ngx_string("/hdel"),
        ngx_string("/hustdb/hdel"),
        hustdb_ha_hdel_handler
    },
    {
        ngx_string("/hexist"),
        ngx_string("/hustdb/hexist"),
        hustdb_ha_hexist_handler
    },
    {
        ngx_string("/hkeys"),
        ngx_string("/hustdb/hkeys"),
        hustdb_ha_hkeys_handler
    },
    {
        ngx_string("/sadd"),
        ngx_string("/hustdb/sadd"),
        hustdb_ha_sadd_handler
    },
    {
        ngx_string("/srem"),
        ngx_string("/hustdb/srem"),
        hustdb_ha_srem_handler
    },
    {
        ngx_string("/sismember"),
        ngx_string("/hustdb/sismember"),
        hustdb_ha_sismember_handler
    },
    {
        ngx_string("/sismember2"),
        ngx_string("/hustdb/sismember"),
        hustdb_ha_sismember2_handler
    },
    {
        ngx_string("/smembers"),
        ngx_string("/hustdb/smembers"),
        hustdb_ha_smembers_handler
    },
    {
        ngx_string("/file_count"),
        ngx_string("/hustdb/file_count"),
        hustdb_ha_file_count_handler
    },
    {
        ngx_string("/stat"),
        ngx_string("/hustdb/stat"),
        hustdb_ha_stat_handler
    },
    {
        ngx_string("/stat_all"),
        ngx_string("/hustdb/stat_all"),
        hustdb_ha_stat_all_handler
    },
    {
        ngx_string("/peer_count"),
        ngx_null_string,
        hustdb_ha_peer_count_handler
    },
    {
        ngx_string("/sync_status"),
        ngx_null_string,
        hustdb_ha_sync_status_handler
    },
    {
        ngx_string("/sync_alive"),
        ngx_null_string,
        hustdb_ha_sync_alive_handler
    },
    {
        ngx_string("/get_table"),
        ngx_null_string,
        hustdb_ha_get_table_handler
    },
    {
        ngx_string("/set_table"),
        ngx_null_string,
        hustdb_ha_set_table_handler
    },
    {
        ngx_string("/zismember"),
        ngx_string("/hustdb/zismember"),
        hustdb_ha_zismember_handler
    },
    {
        ngx_string("/zscore"),
        ngx_string("/hustdb/zscore"),
        hustdb_ha_zscore_handler
    },
    {
        ngx_string("/zscore2"),
        ngx_string("/hustdb/zscore"),
        hustdb_ha_zscore2_handler
    },
    {
        ngx_string("/zadd"),
        ngx_string("/hustdb/zadd"),
        hustdb_ha_zadd_handler
    },
    {
        ngx_string("/zrem"),
        ngx_string("/hustdb/zrem"),
        hustdb_ha_zrem_handler
    },
    {
        ngx_string("/zrangebyrank"),
        ngx_string("/hustdb/zrangebyrank"),
        hustdb_ha_zrangebyrank_handler
    },
    {
        ngx_string("/zrangebyscore"),
        ngx_string("/hustdb/zrangebyscore"),
        hustdb_ha_zrangebyscore_handler
    },
    {
        ngx_string("/cache/exist"),
        ngx_string("/hustcache/exist"),
        hustdb_ha_cache_exist_handler
    },
    {
        ngx_string("/cache/get"),
        ngx_string("/hustcache/get"),
        hustdb_ha_cache_get_handler
    },
    {
        ngx_string("/cache/ttl"),
        ngx_string("/hustcache/ttl"),
        hustdb_ha_cache_ttl_handler
    },
    {
        ngx_string("/cache/put"),
        ngx_string("/hustcache/put"),
        hustdb_ha_cache_put_handler
    },
    {
        ngx_string("/cache/append"),
        ngx_string("/hustcache/append"),
        hustdb_ha_cache_append_handler
    },
    {
        ngx_string("/cache/del"),
        ngx_string("/hustcache/del"),
        hustdb_ha_cache_del_handler
    },
    {
        ngx_string("/cache/expire"),
        ngx_string("/hustcache/expire"),
        hustdb_ha_cache_expire_handler
    },
    {
        ngx_string("/cache/persist"),
        ngx_string("/hustcache/persist"),
        hustdb_ha_cache_persist_handler
    },
    {
        ngx_string("/cache/hexist"),
        ngx_string("/hustcache/hexist"),
        hustdb_ha_cache_hexist_handler
    },
    {
        ngx_string("/cache/hget"),
        ngx_string("/hustcache/hget"),
        hustdb_ha_cache_hget_handler
    },
    {
        ngx_string("/cache/hset"),
        ngx_string("/hustcache/hset"),
        hustdb_ha_cache_hset_handler
    },
    {
        ngx_string("/cache/hdel"),
        ngx_string("/hustcache/hdel"),
        hustdb_ha_cache_hdel_handler
    },
    {
        ngx_string("/cache/hincrby"),
        ngx_string("/hustcache/hincrby"),
        hustdb_ha_cache_hincrby_handler
    },
    {
        ngx_string("/cache/hincrbyfloat"),
        ngx_string("/hustcache/hincrbyfloat"),
        hustdb_ha_cache_hincrbyfloat_handler
    }
};

static size_t hustdb_ha_handler_dict_len = sizeof(hustdb_ha_handler_dict) / sizeof(ngx_http_request_item_t);

typedef c_dict_t(ngx_http_request_item_t *) addon_handler_dict_t;
static addon_handler_dict_t * addon_handler_dict = NULL;

static ngx_bool_t __init_addon_handler_dict(ngx_http_request_item_t dict[], size_t size)
{
    if (addon_handler_dict)
    {
        return true;
    }
    addon_handler_dict = malloc(sizeof(addon_handler_dict_t));
    if (!addon_handler_dict)
    {
        return false;
    }
    c_dict_init(addon_handler_dict);
    size_t i = 0;
    for (i = 0; i < size; ++i)
    {
        ngx_http_request_item_t ** it = c_dict_get(addon_handler_dict, (const char *) dict[i].uri.data);
        if (it && *it)
        {
            return false;
        }
        c_dict_set(addon_handler_dict, (const char *) dict[i].uri.data, &dict[i]);
    }
    return true;
}

static void __uninit_addon_handler_dict()
{
    if (!addon_handler_dict)
    {
        return;
    }
    c_dict_deinit(addon_handler_dict);
    if (addon_handler_dict)
    {
        free(addon_handler_dict);
        addon_handler_dict = NULL;
    }
}

static ngx_command_t ngx_http_hustdb_ha_commands[] = 
{
    {
        ngx_string("hustdb_ha"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
        ngx_http_hustdb_ha,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    APPEND_MCF_ITEM("debug_sync", ngx_http_debug_sync),
    APPEND_MCF_ITEM("zlog_mdc", ngx_http_zlog_mdc),
    APPEND_MCF_ITEM("hustdbtable_file", ngx_http_hustdbtable_file),
    APPEND_MCF_ITEM("hustdb_ha_shm_name", ngx_http_hustdb_ha_shm_name),
    APPEND_MCF_ITEM("hustdb_ha_shm_size", ngx_http_hustdb_ha_shm_size),
    APPEND_MCF_ITEM("public_pem", ngx_http_public_pem),
    APPEND_MCF_ITEM("identifier_cache_size", ngx_http_identifier_cache_size),
    APPEND_MCF_ITEM("identifier_timeout", ngx_http_identifier_timeout),
    APPEND_MCF_ITEM("fetch_req_pool_size", ngx_http_fetch_req_pool_size),
    APPEND_MCF_ITEM("keepalive_cache_size", ngx_http_keepalive_cache_size),
    APPEND_MCF_ITEM("connection_cache_size", ngx_http_connection_cache_size),
    APPEND_MCF_ITEM("fetch_connect_timeout", ngx_http_fetch_connect_timeout),
    APPEND_MCF_ITEM("fetch_send_timeout", ngx_http_fetch_send_timeout),
    APPEND_MCF_ITEM("fetch_read_timeout", ngx_http_fetch_read_timeout),
    APPEND_MCF_ITEM("fetch_buffer_size", ngx_http_fetch_buffer_size),
    APPEND_MCF_ITEM("sync_port", ngx_http_sync_port),
    APPEND_MCF_ITEM("sync_status_uri", ngx_http_sync_status_uri),
    APPEND_MCF_ITEM("sync_user", ngx_http_sync_user),
    APPEND_MCF_ITEM("sync_passwd", ngx_http_sync_passwd),
    APPEND_MCF_ITEM("binlog_uri", ngx_http_binlog_uri),
    ngx_null_command
};

static ngx_http_module_t ngx_http_hustdb_ha_module_ctx = 
{
    NULL, // ngx_int_t (*preconfiguration)(ngx_conf_t *cf);
    ngx_http_hustdb_ha_postconfiguration,
    ngx_http_hustdb_ha_create_main_conf,
    ngx_http_hustdb_ha_init_main_conf,
    NULL, // void * (*create_srv_conf)(ngx_conf_t *cf);
    NULL, // char * (*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);
    NULL, // void * (*create_loc_conf)(ngx_conf_t *cf);
    NULL  // char * (*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);
};

ngx_module_t ngx_http_hustdb_ha_module = 
{
    NGX_MODULE_V1,
    &ngx_http_hustdb_ha_module_ctx,
    ngx_http_hustdb_ha_commands,
    NGX_HTTP_MODULE,
    NULL, // ngx_int_t (*init_master)(ngx_log_t *log);
    ngx_http_hustdb_ha_init_module,
    ngx_http_hustdb_ha_init_process,
    NULL, // ngx_int_t (*init_thread)(ngx_cycle_t *cycle);
    NULL, // void (*exit_thread)(ngx_cycle_t *cycle);
    ngx_http_hustdb_ha_exit_process,
    ngx_http_hustdb_ha_exit_master,
    NGX_MODULE_V1_PADDING
};

static char * ngx_http_debug_sync(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        mcf->debug_sync = false;
        return NGX_CONF_OK;
    }
    int val = ngx_http_get_flag_slot(cf);
    if (NGX_ERROR == val)
    {
        return "ngx_http_debug_sync error";
    }
    mcf->debug_sync = val;
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_zlog_mdc(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_zlog_mdc error";
    }
    ngx_str_t * arr = cf->args->elts;
    mcf->zlog_mdc = ngx_http_make_str(&arr[1], cf->pool);
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_hustdbtable_file(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_hustdbtable_file error";
    }
    ngx_str_t * arr = cf->args->elts;
    mcf->hustdbtable_file = ngx_http_make_str(&arr[1], cf->pool);
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_hustdb_ha_shm_name(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_hustdb_ha_shm_name error";
    }
    ngx_str_t * arr = cf->args->elts;
    mcf->hustdb_ha_shm_name = ngx_http_make_str(&arr[1], cf->pool);
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_hustdb_ha_shm_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_hustdb_ha_shm_size error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->hustdb_ha_shm_size = ngx_parse_size(&value[1]);
    if (NGX_ERROR == mcf->hustdb_ha_shm_size)
    {
        return "ngx_http_hustdb_ha_shm_size error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_public_pem(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_public_pem error";
    }
    ngx_str_t * arr = cf->args->elts;
    mcf->public_pem = ngx_http_make_str(&arr[1], cf->pool);
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_identifier_cache_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_identifier_cache_size error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->identifier_cache_size = ngx_atoi(value[1].data, value[1].len);
    if (NGX_ERROR == mcf->identifier_cache_size)
    {
        return "ngx_http_identifier_cache_size error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_identifier_timeout(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_identifier_timeout error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->identifier_timeout = ngx_parse_time(&value[1], 0);
    if (NGX_ERROR == mcf->identifier_timeout)
    {
        return "ngx_http_identifier_timeout error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_fetch_req_pool_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_fetch_req_pool_size error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->fetch_req_pool_size = ngx_parse_size(&value[1]);
    if (NGX_ERROR == mcf->fetch_req_pool_size)
    {
        return "ngx_http_fetch_req_pool_size error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_keepalive_cache_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_keepalive_cache_size error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->keepalive_cache_size = ngx_atoi(value[1].data, value[1].len);
    if (NGX_ERROR == mcf->keepalive_cache_size)
    {
        return "ngx_http_keepalive_cache_size error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_connection_cache_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_connection_cache_size error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->connection_cache_size = ngx_atoi(value[1].data, value[1].len);
    if (NGX_ERROR == mcf->connection_cache_size)
    {
        return "ngx_http_connection_cache_size error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_fetch_connect_timeout(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_fetch_connect_timeout error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->fetch_connect_timeout = ngx_parse_time(&value[1], 0);
    if (NGX_ERROR == mcf->fetch_connect_timeout)
    {
        return "ngx_http_fetch_connect_timeout error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_fetch_send_timeout(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_fetch_send_timeout error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->fetch_send_timeout = ngx_parse_time(&value[1], 0);
    if (NGX_ERROR == mcf->fetch_send_timeout)
    {
        return "ngx_http_fetch_send_timeout error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_fetch_read_timeout(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_fetch_read_timeout error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->fetch_read_timeout = ngx_parse_time(&value[1], 0);
    if (NGX_ERROR == mcf->fetch_read_timeout)
    {
        return "ngx_http_fetch_read_timeout error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_fetch_buffer_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_fetch_buffer_size error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->fetch_buffer_size = ngx_parse_size(&value[1]);
    if (NGX_ERROR == mcf->fetch_buffer_size)
    {
        return "ngx_http_fetch_buffer_size error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_sync_port(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_sync_port error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->sync_port = ngx_atoi(value[1].data, value[1].len);
    if (NGX_ERROR == mcf->sync_port)
    {
        return "ngx_http_sync_port error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_sync_status_uri(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_sync_status_uri error";
    }
    ngx_str_t * arr = cf->args->elts;
    mcf->sync_status_uri = ngx_http_make_str(&arr[1], cf->pool);
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_sync_user(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_sync_user error";
    }
    ngx_str_t * arr = cf->args->elts;
    mcf->sync_user = ngx_http_make_str(&arr[1], cf->pool);
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_sync_passwd(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_sync_passwd error";
    }
    ngx_str_t * arr = cf->args->elts;
    mcf->sync_passwd = ngx_http_make_str(&arr[1], cf->pool);
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_binlog_uri(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdb_ha_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_binlog_uri error";
    }
    ngx_str_t * arr = cf->args->elts;
    mcf->binlog_uri = ngx_http_make_str(&arr[1], cf->pool);
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char *ngx_http_hustdb_ha(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t * clcf = ngx_http_conf_get_module_loc_conf(
        cf, ngx_http_core_module);
    clcf->handler = ngx_http_hustdb_ha_handler;
    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_hustdb_ha_handler(ngx_http_request_t *r)
{
    if (!r->uri.data)
    {
        return NGX_ERROR;
    }
    u_char tmp = r->uri.data[r->uri.len];
    r->uri.data[r->uri.len] = '\0';
    ngx_http_request_item_t ** it = c_dict_get(addon_handler_dict, (const char *)r->uri.data);
    r->uri.data[r->uri.len] = tmp;
    
    if (!it || !*it)
    {
        return NGX_ERROR;
    }
    return (*it)->handler(&(*it)->backend_uri, r);
}

static ngx_int_t ngx_http_hustdb_ha_init_module(ngx_cycle_t * cycle)
{
    // TODO: initialize in master process
    return NGX_OK;
}

static ngx_int_t ngx_http_hustdb_ha_init_process(ngx_cycle_t * cycle)
{
    if (!__init_addon_handler_dict(hustdb_ha_handler_dict, hustdb_ha_handler_dict_len))
    {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "init addon_handler_dict error\n");
        return NGX_ERROR;
    }
    // TODO: initialize in worker process
    return NGX_OK;
}

static void ngx_http_hustdb_ha_exit_process(ngx_cycle_t * cycle)
{
    __uninit_addon_handler_dict();
    // TODO: uninitialize in worker process
}

static void ngx_http_hustdb_ha_exit_master(ngx_cycle_t * cycle)
{
    // TODO: uninitialize in master process
}

static void * ngx_http_hustdb_ha_create_main_conf(ngx_conf_t *cf)
{
    return ngx_pcalloc(cf->pool, sizeof(ngx_http_hustdb_ha_main_conf_t));
}

static ngx_int_t ngx_http_addon_init_shm_ctx(ngx_slab_pool_t * shpool, void * sh)
{
    hustdb_ha_shctx_t * ctx = (hustdb_ha_shctx_t *) sh;
    ctx->table_locked = false;
    if (!hustdb_ha_init_identifier_cache(g_mcf))
    {
        return NGX_ERROR;
    }
    return NGX_OK;
}

static ngx_bool_t __init_fetch(ngx_conf_t * cf, ngx_http_hustdb_ha_main_conf_t * mcf)
{
    static ngx_str_t ARGS = ngx_string("backend_count=");
    mcf->sync_status_args = hustdb_ha_strcat(&ARGS, (int) ngx_http_get_backend_count(), cf->pool);

    static ngx_str_t PREFIX = ngx_string("127.0.0.1:");
    ngx_str_t uri = hustdb_ha_strcat(&PREFIX, mcf->sync_port, cf->pool);
    if (!uri.data)
    {
        return false;
    }

    ngx_url_array_t urls = { &uri, 1 };

    ngx_http_upstream_rr_peers_t  * peers = ngx_http_init_upstream_rr_peers(&urls, cf);
    if (!peers)
    {
        return false;
    }
    mcf->sync_peer = peers->peer;
    ngx_http_fetch_essential_conf_t ecf = { mcf->fetch_req_pool_size, mcf->keepalive_cache_size, mcf->connection_cache_size, cf, peers };
    ngx_http_fetch_upstream_conf_t ucf = {
        mcf->fetch_connect_timeout,
        mcf->fetch_send_timeout,
        mcf->fetch_read_timeout,
        mcf->fetch_buffer_size,
        { 0, 0 },
        0
    };
    if (NGX_OK != ngx_http_fetch_init_conf(&ecf, &ucf, NULL))
    {
        return false;
    }
    return true;
}

static ngx_bool_t __init_addon(ngx_conf_t * cf, ngx_http_hustdb_ha_main_conf_t * mcf);

static char * ngx_http_hustdb_ha_init_main_conf(ngx_conf_t * cf, void * conf)
{
    ngx_http_hustdb_ha_main_conf_t * mcf = conf;
    if (!mcf)
    {
        return NGX_CONF_ERROR;
    }
    mcf->pool = cf->pool;
    mcf->log = cf->log;
    mcf->prefix = cf->cycle->prefix;
    // TODO: you can initialize mcf here
    g_mcf = mcf;

    return NGX_CONF_OK;
}

static ngx_bool_t __init_addon(ngx_conf_t * cf, ngx_http_hustdb_ha_main_conf_t * mcf)
{
    mcf->zone = ngx_http_addon_init_shm(cf, &mcf->hustdb_ha_shm_name, mcf->hustdb_ha_shm_size,
        sizeof(hustdb_ha_shctx_t), ngx_http_addon_init_shm_ctx, &ngx_http_hustdb_ha_module);

    hustdb_ha_init_peer_count(cf->pool);
    if (!hustdb_ha_init_peer_dict())
    {
        return false;
    }

    if (!hustdb_ha_init_peer_array(cf->pool))
    {
        return false;
    }

    if (!hustdb_ha_init_log_dirs(&mcf->prefix, mcf->pool))
    {
        return false;
    }

    if (!__init_fetch(cf, mcf))
    {
        return false;
    }

    mcf->public_pem_full_path = ngx_http_get_conf_path(cf->cycle, &mcf->public_pem);

    ngx_str_t table_path = ngx_http_get_conf_path(cf->cycle, &mcf->hustdbtable_file);
    if (!table_path.data)
    {
        return false;
    }
    if (!hustdb_ha_init_table_str(&table_path, cf->pool))
    {
        return false;
    }
    HustDbHaTable table;
    if (!cjson_load_hustdbhatable_from_file((const char *)table_path.data, &table))
    {
        return false;
    }
    if (!table.json_has_table)
    {
        return false;
    }
    if (!hustdb_ha_build_table(&table, cf->pool))
    {
        return false;
    }
    cjson_dispose_hustdbhatable(&table);

    hustdb_ha_init_table_path(table_path, cf->pool);

    return true;
}

static ngx_http_addon_upstream_peers_t addon_upstream_peers = { 0, 0 };

static ngx_int_t ngx_http_hustdb_ha_postconfiguration(ngx_conf_t * cf)
{
    static ngx_str_t backend = ngx_string("backend");
    ngx_bool_t rc = ngx_http_init_addon_backends(
        ngx_http_conf_get_module_main_conf(cf, ngx_http_upstream_module), 
        &backend, &addon_upstream_peers);
    if (!rc)
    {
        return NGX_ERROR;
    }
    if (!__init_addon(cf, g_mcf))
    {
        return NGX_ERROR;
    }
    return NGX_OK;
}

ngx_http_upstream_rr_peers_t * ngx_http_get_backends()
{
    return addon_upstream_peers.peer;
}

size_t ngx_http_get_backend_count()
{
    return addon_upstream_peers.count;
}

void * ngx_http_get_addon_module_ctx(ngx_http_request_t * r)
{
    if (!r)
    {
        return NULL;
    }
    return ngx_http_get_module_ctx(r, ngx_http_hustdb_ha_module);
}

void ngx_http_set_addon_module_ctx(ngx_http_request_t * r, void * ctx)
{
    if (!r)
    {
        return;
    }
    ngx_http_set_ctx(r, ctx, ngx_http_hustdb_ha_module);
}

void * hustdb_ha_get_module_main_conf(ngx_http_request_t * r)
{
    if (!r)
    {
        return NULL;
    }
    return ngx_http_get_module_main_conf(r, ngx_http_hustdb_ha_module);
}
