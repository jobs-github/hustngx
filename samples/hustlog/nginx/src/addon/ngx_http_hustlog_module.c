#include <c_dict.h>
#include "hustlog_handler.h"
#include "ngx_http_utils_module.h"
static ngx_int_t ngx_http_hustlog_handler(ngx_http_request_t *r);
static char *ngx_http_hustlog(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_hustlog_init_module(ngx_cycle_t * cycle);
static ngx_int_t ngx_http_hustlog_init_process(ngx_cycle_t * cycle);
static void ngx_http_hustlog_exit_process(ngx_cycle_t * cycle);
static void ngx_http_hustlog_exit_master(ngx_cycle_t * cycle);

static ngx_http_request_item_t hustlog_handler_dict[] =
{
	{
	    ngx_string("/hustlog/post"),
	    ngx_null_string,
        hustlog_post_handler
    }
};

static size_t hustlog_handler_dict_len = sizeof(hustlog_handler_dict) / sizeof(ngx_http_request_item_t);

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

static ngx_command_t ngx_http_hustlog_commands[] = 
{
    {
        ngx_string("hustlog"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
        ngx_http_hustlog,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },

    ngx_null_command
};

static ngx_http_module_t ngx_http_hustlog_module_ctx = 
{
    NULL, // ngx_int_t (*preconfiguration)(ngx_conf_t *cf);
    NULL, // ngx_int_t (*postconfiguration)(ngx_conf_t *cf);
	NULL,                                // void       *(*create_main_conf)(ngx_conf_t *cf);
	NULL,                                // char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);
    NULL, // void * (*create_srv_conf)(ngx_conf_t *cf);
    NULL, // char * (*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);
    NULL, // void * (*create_loc_conf)(ngx_conf_t *cf);
    NULL  // char * (*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);
};

ngx_module_t ngx_http_hustlog_module = 
{
    NGX_MODULE_V1,
    &ngx_http_hustlog_module_ctx,
    ngx_http_hustlog_commands,
    NGX_HTTP_MODULE,
    NULL, // ngx_int_t (*init_master)(ngx_log_t *log);
    ngx_http_hustlog_init_module,
    ngx_http_hustlog_init_process,
    NULL, // ngx_int_t (*init_thread)(ngx_cycle_t *cycle);
    NULL, // void (*exit_thread)(ngx_cycle_t *cycle);
    ngx_http_hustlog_exit_process,
    ngx_http_hustlog_exit_master,
    NGX_MODULE_V1_PADDING
};

static ngx_str_t g_zlog_conf_path = { 0, 0 };

void zlog_init_conf_path()
{
    ngx_str_t name = ngx_string("hustlog.conf");
    g_zlog_conf_path = ngx_http_get_conf_path((ngx_cycle_t *)ngx_cycle, &name);
}

int zlog_load_conf(zlog_handler handler)
{
    return handler((const char *)g_zlog_conf_path.data);
}

static char *ngx_http_hustlog(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t * clcf = ngx_http_conf_get_module_loc_conf(
        cf, ngx_http_core_module);
    clcf->handler = ngx_http_hustlog_handler;
    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_hustlog_handler(ngx_http_request_t *r)
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

static ngx_int_t ngx_http_hustlog_init_module(ngx_cycle_t * cycle)
{
    // TODO: initialize in master process
    return NGX_OK;
}

static ngx_int_t ngx_http_hustlog_init_process(ngx_cycle_t * cycle)
{
    if (!__init_addon_handler_dict(hustlog_handler_dict, hustlog_handler_dict_len))
    {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "init addon_handler_dict error\n");
        return NGX_ERROR;
    }
    // TODO: initialize in worker process
    return NGX_OK;
}

static void ngx_http_hustlog_exit_process(ngx_cycle_t * cycle)
{
    __uninit_addon_handler_dict(hustlog_handler_dict, hustlog_handler_dict_len);
    // TODO: uninitialize in worker process
}

static void ngx_http_hustlog_exit_master(ngx_cycle_t * cycle)
{
    // TODO: uninitialize in master process
}

void * ngx_http_get_addon_module_ctx(ngx_http_request_t * r)
{
    if (!r)
    {
        return NULL;
    }
    return ngx_http_get_module_ctx(r, ngx_http_hustlog_module);
}

void ngx_http_set_addon_module_ctx(ngx_http_request_t * r, void * ctx)
{
    if (!r)
    {
        return;
    }
    ngx_http_set_ctx(r, ctx, ngx_http_hustlog_module);
}
