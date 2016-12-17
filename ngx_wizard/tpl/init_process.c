$var_declare
{
    if (!__init_addon_handler_dict($var_dict, $var_dict_len))
    {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "init addon_handler_dict error\n");
        return NGX_ERROR;
    }
    // TODO: initialize in worker process
    return NGX_OK;
}
