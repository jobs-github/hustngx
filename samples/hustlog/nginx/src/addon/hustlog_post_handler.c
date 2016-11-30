#include "hustlog_handler.h"
#include "ngx_http_utils_module.h"

typedef struct
{
	const char * level;
	hustlog_write_t writer;
} hustlog_level_writer_t;

typedef struct
{
	const char * category;
	const char * level;
	const char * worker;
	const char * data;
} hustlog_item_t;

static hustlog_level_writer_t hustlog_level_writer_dict[] =
{
	{ "fatal",  hustlog_write_fatal  },
	{ "error",  hustlog_write_error  },
	{ "warn",   hustlog_write_warn   },
	{ "notice", hustlog_write_notice },
	{ "info",   hustlog_write_info   },
	{ "debug",  hustlog_write_debug  }
};

static size_t hustlog_level_writer_dict_len = sizeof(hustlog_level_writer_dict) / sizeof(hustlog_level_writer_t);

hustlog_level_writer_t * get_hustlog_level_writer_item(const char * level)
{
	size_t i = 0;
	for (i = 0; i < hustlog_level_writer_dict_len; ++i)
	{
		if (0 == strcmp(level, hustlog_level_writer_dict[i].level))
		{
			return hustlog_level_writer_dict + i;
		}
	}
	return NULL;
}

static bool __write_log_item(const hustlog_item_t * item)
{
	if (!item)
	{
		return false;
	}
	hustlog_level_writer_t * writer = get_hustlog_level_writer_item(item->level);
	if (!writer)
	{
		return false;
	}
	zlog_category_t * category = zlog_get_category(item->category);
	if (!category)
	{
		return false;
	}
	zlog_put_mdc("worker", item->worker);
	writer->writer(category, item->data);
	return true;
}

static ngx_bool_t __post_body_cb(ngx_http_request_t * r, ngx_buf_t * buf, size_t buf_size)
{
    char * category = ngx_http_get_param_val(&r->args, "category", r->pool);
    if (!category)
    {
        return false;
    }

    size_t len = strlen(category);
    if (len > MAX_HUSTLOG_CATEGORY_LEN)
    {
        return false;
    }

    char * level = ngx_http_get_param_val(&r->args, "level", r->pool);
    if (!level)
    {
        return false;
    }

    char * worker = ngx_http_get_param_val(&r->args, "worker", r->pool);
    if (!worker)
    {
        return false;
    }
    len = strlen(worker);
    if (len > MAX_HUSTLOG_WORKER_SIZE)
    {
        return false;
    }
    hustlog_item_t item = { category, level, worker, (const char *)buf->pos };
    return __write_log_item(&item);
}

static void __post_body_handler(ngx_http_request_t * r)
{
    ngx_http_post_body_handler(r, __post_body_cb);
}

ngx_int_t hustlog_post_handler(ngx_str_t * backend_uri, ngx_http_request_t *r)
{
    if (!(r->method & NGX_HTTP_POST) && !(r->method & NGX_HTTP_PUT))
	{
		return NGX_HTTP_NOT_ALLOWED;
	}
	ngx_int_t rc = ngx_http_read_client_request_body(r, __post_body_handler);
	if ( rc >= NGX_HTTP_SPECIAL_RESPONSE )
	{
		return rc;
	}
	return NGX_DONE;
}
