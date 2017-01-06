typedef struct
{
$var_fields

$var_flags
} $var_arg_t;

ngx_bool_t $var_decoder(ngx_http_request_t *r, $var_arg_t * args)
{
    if (!r || !args)
    {
        return false;
    }
    memset(args, 0, sizeof($var_arg_t));
    // reset
$var_reset
    ngx_str_t val = {0,0};
$var_impl
    return true;
}
