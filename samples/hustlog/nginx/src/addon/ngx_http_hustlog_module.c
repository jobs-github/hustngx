#include "hustlog_handler.h"
#include "ngx_http_utils_module.h"

static ngx_http_request_item_t hustlog_handler_dict[] =
{
	{
	    ngx_string("/hustlog/post"),
	    ngx_null_string,
        hustlog_post_handler
    }
};

static size_t hustlog_handler_dict_len = sizeof(hustlog_handler_dict) / sizeof(ngx_http_request_item_t);

static ngx_int_t ngx_http_hustlog_handler(ngx_http_request_t *r);
static char *ngx_http_hustlog(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

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
	NULL,                                // ngx_int_t   (*preconfiguration)(ngx_conf_t *cf);
	NULL,                                // ngx_int_t   (*postconfiguration)(ngx_conf_t *cf);
	NULL,                                // void       *(*create_main_conf)(ngx_conf_t *cf);
	NULL,                                // char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);
	NULL,                                // void       *(*create_srv_conf)(ngx_conf_t *cf);
	NULL,                                // char       *(*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);
	NULL,                                // void       *(*create_loc_conf)(ngx_conf_t *cf);
	NULL                                 // char       *(*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);
};

ngx_module_t ngx_http_hustlog_module =
{
	NGX_MODULE_V1,
	&ngx_http_hustlog_module_ctx,
	ngx_http_hustlog_commands,
	NGX_HTTP_MODULE,
	NULL,                            // ngx_int_t           (*init_master)(ngx_log_t *log);
	NULL,                            // ngx_int_t           (*init_module)(ngx_cycle_t *cycle);
	NULL,                            // ngx_int_t           (*init_process)(ngx_cycle_t *cycle);
	NULL,                            // ngx_int_t           (*init_thread)(ngx_cycle_t *cycle);
	NULL,                            // void                (*exit_thread)(ngx_cycle_t *cycle);
	NULL,                            // void                (*exit_process)(ngx_cycle_t *cycle);
	NULL,                            // void                (*exit_master)(ngx_cycle_t *cycle);
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

static char * ngx_http_hustlog(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t * clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
	clcf->handler = ngx_http_hustlog_handler;
	return NGX_CONF_OK;
}

static ngx_int_t ngx_http_hustlog_handler(ngx_http_request_t *r)
{
    ngx_http_request_item_t * it = ngx_http_get_request_item(hustlog_handler_dict, hustlog_handler_dict_len, &r->uri);
	if (!it)
	{
		return NGX_ERROR;
	}
	return it->handler(&it->backend_uri, r);
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
