ngx_module_t $var_md = 
{
    NGX_MODULE_V1 ,
    &$var_md_ctx ,
    $var_commands ,
    NGX_HTTP_MODULE,
    NULL , // ngx_int_t (*init_master)(ngx_log_t *log);
    $var_init_module ,
    $var_init_process ,
    NULL , // ngx_int_t (*init_thread)(ngx_cycle_t *cycle);
    NULL , // void (*exit_thread)(ngx_cycle_t *cycle);
    $var_exit_process ,
    $var_exit_master ,
    NGX_MODULE_V1_PADDING
};
