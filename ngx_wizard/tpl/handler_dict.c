static ngx_http_request_item_t $var_dict[] =
{
$var_impl
};

static size_t $var_dict_len = sizeof($var_dict) / sizeof(ngx_http_request_item_t);

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
