typedef struct
{
    ngx_pool_t * pool;
    ngx_log_t * log;
    ngx_str_t prefix;
$var_items
} $var_mcf_t ;

void * $var_get_mcf (ngx_http_request_t * r);