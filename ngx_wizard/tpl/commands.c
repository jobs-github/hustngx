static ngx_command_t $var_commands[] = 
{
    {
        $var_md,
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
        $var_handler,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
$var_items
    ngx_null_command
};
