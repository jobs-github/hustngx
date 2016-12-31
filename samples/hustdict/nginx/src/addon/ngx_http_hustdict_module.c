#include "hustdict_handler.h"
#include "ngx_shm_dict_test.h"

static ngx_int_t ngx_http_hustdict_handler(ngx_http_request_t *r);
static char *ngx_http_hustdict(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_hustdict_init_module(ngx_cycle_t * cycle);
static ngx_int_t ngx_http_hustdict_init_process(ngx_cycle_t * cycle);
static void ngx_http_hustdict_exit_process(ngx_cycle_t * cycle);
static void ngx_http_hustdict_exit_master(ngx_cycle_t * cycle);
static char * ngx_http_shm_name(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static char * ngx_http_shm_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf);
static void * ngx_http_hustdict_create_main_conf(ngx_conf_t *cf);
static char * ngx_http_hustdict_init_main_conf(ngx_conf_t * cf, void * conf);
static ngx_int_t ngx_http_hustdict_postconfiguration(ngx_conf_t * cf);

static ngx_http_request_item_t hustdict_handler_dict[] =
{
    {
        ngx_string("/set"),
        ngx_null_string,
        hustdict_set_handler
    },
    {
        ngx_string("/get"),
        ngx_null_string,
        hustdict_get_handler
    },
    {
        ngx_string("/del"),
        ngx_null_string,
        hustdict_del_handler
    },
    {
        ngx_string("/flush_all"),
        ngx_null_string,
        hustdict_flush_all_handler
    },
    {
        ngx_string("/flush_expired"),
        ngx_null_string,
        hustdict_flush_expired_handler
    },
    {
        ngx_string("/keys"),
        ngx_null_string,
        hustdict_keys_handler
    },
    {
        ngx_string("/get_keys"),
        ngx_null_string,
        hustdict_get_keys_handler
    },
    {
        ngx_string("/incr"),
        ngx_null_string,
        hustdict_incr_handler
    },
    {
        ngx_string("/decr"),
        ngx_null_string,
        hustdict_decr_handler
    }
};

static size_t hustdict_handler_dict_len = sizeof(hustdict_handler_dict) / sizeof(ngx_http_request_item_t);

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

static ngx_command_t ngx_http_hustdict_commands[] = 
{
    {
        ngx_string("hustdict"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
        ngx_http_hustdict,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    APPEND_MCF_ITEM("shm_name", ngx_http_shm_name),
    APPEND_MCF_ITEM("shm_size", ngx_http_shm_size),
    ngx_null_command
};

static ngx_http_module_t ngx_http_hustdict_module_ctx = 
{
    NULL, // ngx_int_t (*preconfiguration)(ngx_conf_t *cf);
    ngx_http_hustdict_postconfiguration,
    ngx_http_hustdict_create_main_conf,
    ngx_http_hustdict_init_main_conf,
    NULL, // void * (*create_srv_conf)(ngx_conf_t *cf);
    NULL, // char * (*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);
    NULL, // void * (*create_loc_conf)(ngx_conf_t *cf);
    NULL  // char * (*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);
};

ngx_module_t ngx_http_hustdict_module = 
{
    NGX_MODULE_V1,
    &ngx_http_hustdict_module_ctx,
    ngx_http_hustdict_commands,
    NGX_HTTP_MODULE,
    NULL, // ngx_int_t (*init_master)(ngx_log_t *log);
    ngx_http_hustdict_init_module,
    ngx_http_hustdict_init_process,
    NULL, // ngx_int_t (*init_thread)(ngx_cycle_t *cycle);
    NULL, // void (*exit_thread)(ngx_cycle_t *cycle);
    ngx_http_hustdict_exit_process,
    ngx_http_hustdict_exit_master,
    NGX_MODULE_V1_PADDING
};

static char * ngx_http_shm_name(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdict_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdict_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_shm_name error";
    }
    ngx_str_t * arr = cf->args->elts;
    mcf->shm_name = ngx_http_make_str(&arr[1], cf->pool);
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char * ngx_http_shm_size(ngx_conf_t * cf, ngx_command_t * cmd, void * conf)
{
    ngx_http_hustdict_main_conf_t * mcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_hustdict_module);
    if (!mcf || 2 != cf->args->nelts)
    {
        return "ngx_http_shm_size error";
    }
    ngx_str_t * value = cf->args->elts;
    mcf->shm_size = ngx_atoi(value[1].data, value[1].len);
    if (NGX_ERROR == mcf->shm_size)
    {
        return "ngx_http_shm_size error";
    }
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}

static char *ngx_http_hustdict(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t * clcf = ngx_http_conf_get_module_loc_conf(
        cf, ngx_http_core_module);
    clcf->handler = ngx_http_hustdict_handler;
    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_hustdict_handler(ngx_http_request_t *r)
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

static ngx_int_t ngx_http_hustdict_init_module(ngx_cycle_t * cycle)
{
    // TODO: initialize in master process
    return NGX_OK;
}

static ngx_int_t ngx_http_hustdict_init_process(ngx_cycle_t * cycle)
{
    if (!__init_addon_handler_dict(hustdict_handler_dict, hustdict_handler_dict_len))
    {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "init addon_handler_dict error\n");
        return NGX_ERROR;
    }
    // TODO: initialize in worker process
    return NGX_OK;
}

static void ngx_http_hustdict_exit_process(ngx_cycle_t * cycle)
{
    __uninit_addon_handler_dict(hustdict_handler_dict, hustdict_handler_dict_len);
    // TODO: uninitialize in worker process
}

static void ngx_http_hustdict_exit_master(ngx_cycle_t * cycle)
{
    // TODO: uninitialize in master process
}

static void * ngx_http_hustdict_create_main_conf(ngx_conf_t *cf)
{
    return ngx_pcalloc(cf->pool, sizeof(ngx_http_hustdict_main_conf_t));
}

static char * ngx_http_hustdict_init_main_conf(ngx_conf_t * cf, void * conf)
{
    ngx_http_hustdict_main_conf_t * mcf = conf;
    if (!mcf)
    {
        return NGX_CONF_ERROR;
    }
    mcf->pool = cf->pool;
    mcf->log = cf->log;
    mcf->prefix = cf->cycle->prefix;
    // TODO: you can initialize mcf here
    if (NGX_OK != ngx_shm_dict_test_init(cf, &mcf->shm_name, (size_t) mcf->shm_size, &ngx_http_hustdict_module))
    {
        return NGX_CONF_ERROR;
    }
    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_hustdict_postconfiguration(ngx_conf_t * cf)
{
    return NGX_OK;
}

void * ngx_http_get_addon_module_ctx(ngx_http_request_t * r)
{
    if (!r)
    {
        return NULL;
    }
    return ngx_http_get_module_ctx(r, ngx_http_hustdict_module);
}

void ngx_http_set_addon_module_ctx(ngx_http_request_t * r, void * ctx)
{
    if (!r)
    {
        return;
    }
    ngx_http_set_ctx(r, ctx, ngx_http_hustdict_module);
}

void * hustdict_get_module_main_conf(ngx_http_request_t * r)
{
    if (!r)
    {
        return NULL;
    }
    return ngx_http_get_module_main_conf(r, ngx_http_hustdict_module);
}
