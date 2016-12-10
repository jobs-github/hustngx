static char * $var_func ( $var_args )
{
    $var_mcf_t * mcf = $var_get_mcf ;
    if (!mcf || 2 != cf->args->nelts)
    {
        return "$var_func error";
    }
$var_impl
    // TODO: you can modify the value here
    return NGX_CONF_OK;
}
