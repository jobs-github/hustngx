#include "ngx_http_mgr_module.h"
#include <time.h>
#include <stdint.h>

typedef struct
{
    ngx_str_t ngx_conf_path;
    ngx_str_t ngx_conf_tmp_path;
    ngx_str_t ngx_conf_del_path;
} ngx_http_conf_path_t;

static ngx_http_conf_path_t ngx_http_conf_path;

typedef struct
{
    const char * conf_path;
    const char * conf_tmp_path;
    const char * conf_del_path;
} ngx_conf_path_t;

typedef enum
{
    SET_NGX_CONF_SUCCESS = 1,
    INVALID_CONF_PATH = 2,
    INVALID_CONF_TMP_PATH = 3,
    INVALID_CONF_DEL_PATH = 4,
    WRITE_CONF_TMP_ERR = 5,
    CHECK_CONF_ERR = 6,
    RENAME_NGX_CONF_ERR = 7,
    RENAME_NGX_CONF_TMP_ERR = 8,
    ROLLBACK_OVERWRITE_ERR = 9
} setconf_result_t;

typedef struct
{
    setconf_result_t key;
    const char * value;
} setconf_result_pair_t;

static setconf_result_pair_t g_setconf_result_dict[] =
{
    { SET_NGX_CONF_SUCCESS,    "set_ngx_conf_success"    },
    { INVALID_CONF_PATH,       "invalid_conf_path"       },
    { INVALID_CONF_TMP_PATH,   "invalid_conf_tmp_path"   },
    { INVALID_CONF_DEL_PATH,   "invalid_conf_del_path"   },
    { WRITE_CONF_TMP_ERR,      "write_conf_tmp_err"      },
    { CHECK_CONF_ERR,          "check_conf_err"          },
    { RENAME_NGX_CONF_ERR,     "rename_ngx_conf_err"     },
    { RENAME_NGX_CONF_TMP_ERR, "rename_ngx_conf_tmp_err" },
    { ROLLBACK_OVERWRITE_ERR,  "rollback_overwrite_err"  }
};

static size_t g_setconf_result_dict_len = sizeof(g_setconf_result_dict) / sizeof(setconf_result_pair_t);

const char * __get_result_string(setconf_result_t key)
{
    size_t i = 0;
    for (i = 0; i < g_setconf_result_dict_len; ++i)
    {
        if (g_setconf_result_dict[i].key == key)
        {
            return g_setconf_result_dict[i].value;
        }
    }
    return "unknown_error";
}

static char * __get_check_conf_cmd(const char * path, ngx_pool_t * pool)
{
    enum { MAX_PATH = 256 };
    char exe[MAX_PATH + 1] = {0};
    int exe_len = readlink("/proc/self/exe", exe, MAX_PATH);
    if (exe_len < 0 || exe_len >= MAX_PATH)
    {
        return NULL;
    }
    enum { BUF_SIZE = 1024 };
    char buf[BUF_SIZE + 1] = {0};
    sprintf(buf, "%s -t -c %s 2>&1", exe, path);

    size_t size = strlen(buf);
    char * cmd = ngx_palloc(pool, size + 1);
    memcpy(cmd, buf, size);
    cmd[size] = '\0';

    return cmd;
}

static ngx_bool_t __check_conf(const char * cmd, ngx_pool_t * pool)
{
    char * output = ngx_http_execute(cmd, pool);
    if (!output)
    {
        return false;
    }
    static const char * success = "test is successful";
    if (!strstr(output, success))
    {
        return false;
    }
    return true;
}

static setconf_result_t __overwrite_conf(const char * data, const ngx_conf_path_t * path, ngx_pool_t * pool)
{
    // write "nginx.conf.tmp"
    if (!ngx_http_save_to_file(data, path->conf_tmp_path))
    {
        return WRITE_CONF_TMP_ERR;
    }

    // check "nginx.conf.tmp"
    if (!__check_conf(__get_check_conf_cmd(path->conf_tmp_path, pool), pool))
    {
        return CHECK_CONF_ERR;
    }

    // rename "nginx.conf" to "nginx.conf.del"
    if (0 != rename(path->conf_path, path->conf_del_path))
    {
        return RENAME_NGX_CONF_ERR;
    }

    // rename "nginx.conf.tmp" to "nginx.conf"
    if (0 != rename(path->conf_tmp_path, path->conf_path))
    {
        // rollback
        if (0 != rename(path->conf_del_path, path->conf_path))
        {
            return ROLLBACK_OVERWRITE_ERR;
        }
        return RENAME_NGX_CONF_TMP_ERR;
    }
    return SET_NGX_CONF_SUCCESS;
}

static setconf_result_t __setconf(const char * data, ngx_http_request_t * r)
{
    if (!ngx_http_conf_path.ngx_conf_path.data)
    {
        return INVALID_CONF_PATH;
    }

    if (!ngx_http_conf_path.ngx_conf_tmp_path.data)
    {
        return INVALID_CONF_TMP_PATH;
    }

    if (!ngx_http_conf_path.ngx_conf_del_path.data)
    {
        return INVALID_CONF_DEL_PATH;
    }

    ngx_conf_path_t path =
    {
        (const char *) ngx_http_conf_path.ngx_conf_path.data,
        (const char *) ngx_http_conf_path.ngx_conf_tmp_path.data,
        (const char *) ngx_http_conf_path.ngx_conf_del_path.data
    };

    return __overwrite_conf(data, &path, r->pool);
}

static void __post_handler(ngx_http_request_t * r)
{
    int status = NGX_HTTP_NOT_FOUND;
    ngx_str_t response = { 0, 0 };
    do
    {
        ngx_buf_t * buf = ngx_http_get_request_body(r);
        if (!buf)
        {
            break;
        }

        size_t buf_size = ngx_http_get_buf_size(buf);
        if (buf_size < 1)
        {
            break;
        }

        setconf_result_t result = __setconf((const char *)buf->pos, r);

        if (SET_NGX_CONF_SUCCESS == result)
        {
            status = NGX_HTTP_OK;
        }
        const char * value = __get_result_string(result);
        response.data = (u_char *) value;
        response.len = strlen(value);

    } while (0);

    ngx_int_t rc = ngx_http_send_response_imp(status, &response, r);
    ngx_http_finalize_request(r, rc);
}


static void __init_ngx_conf_tmp_path(const ngx_str_t * conf_file, const ngx_str_t * ext, ngx_pool_t * pool, ngx_str_t * path)
{
    size_t size = conf_file->len + ext->len;
    path->len = size;
    path->data = ngx_palloc(pool, size + 1);
    memcpy(path->data, conf_file->data, conf_file->len);
    memcpy(path->data + conf_file->len, ext->data, ext->len);
    path->data[size] = '\0';
}

void ngx_http_init_conf_path(ngx_conf_t * cf)
{
    ngx_http_conf_path.ngx_conf_path = cf->cycle->conf_file;

    ngx_str_t tmp = ngx_string(".tmp");
    __init_ngx_conf_tmp_path(&cf->cycle->conf_file, &tmp, cf->pool, &ngx_http_conf_path.ngx_conf_tmp_path);

    ngx_str_t del = ngx_string(".del");
    __init_ngx_conf_tmp_path(&cf->cycle->conf_file, &del, cf->pool, &ngx_http_conf_path.ngx_conf_del_path);
}

ngx_int_t ngx_http_setconf_handler(ngx_http_request_t *r)
{
    if (!(r->method & NGX_HTTP_POST) && !(r->method & NGX_HTTP_PUT))
    {
        return NGX_HTTP_NOT_ALLOWED;
    }

    if (!ngx_http_check_key(r))
    {
        return NGX_ERROR;
    }

    ngx_int_t rc = ngx_http_read_client_request_body(r, __post_handler);
    if (rc >= NGX_HTTP_SPECIAL_RESPONSE)
    {
        return rc;
    }
    return NGX_DONE;
}

ngx_int_t ngx_http_getconf_handler(ngx_http_request_t *r)
{
    if (!ngx_http_check_key(r))
    {
        return NGX_ERROR;
    }

    ngx_str_t conf_path = ngx_http_conf_path.ngx_conf_path;
    if (!conf_path.data)
    {
        return NGX_ERROR;
    }

    ngx_str_t response = ngx_http_load_from_file((const char *) conf_path.data, r->pool);
    if (!response.data)
    {
        return NGX_ERROR;
    }
    return ngx_http_send_response_imp(NGX_HTTP_OK, &response, r);
}

ngx_int_t ngx_http_reload_handler(ngx_http_request_t *r)
{
    if (!ngx_http_check_key(r))
    {
        return NGX_ERROR;
    }

    ngx_int_t rc = ngx_reload_configuration();
    return ngx_http_send_response_imp(NGX_OK == rc ? NGX_HTTP_OK : NGX_HTTP_NOT_FOUND, NULL, r);
}
