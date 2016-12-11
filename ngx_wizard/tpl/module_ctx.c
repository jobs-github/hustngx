static ngx_http_module_t $var_ctx = 
{
    NULL , // ngx_int_t (*preconfiguration)(ngx_conf_t *cf);
    NULL , // ngx_int_t (*postconfiguration)(ngx_conf_t *cf);
    $var_create ,
    $var_init ,
    NULL , // void * (*create_srv_conf)(ngx_conf_t *cf);
    NULL , // char * (*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);
    NULL , // void * (*create_loc_conf)(ngx_conf_t *cf);
    NULL   // char * (*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);
};
