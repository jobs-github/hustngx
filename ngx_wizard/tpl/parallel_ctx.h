typedef struct
{
    ngx_http_request_t * r;
    size_t backends;
    size_t requests;
    size_t finished;
    size_t err_count;
    // TODO: add your fields here
} $var_ctx_t;
