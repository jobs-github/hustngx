$var_create
{
    return ngx_pcalloc(cf->pool, sizeof($var_mcf_t));
}

$var_init
{
    $var_mcf_t * mcf = conf;
    if (!mcf)
    {
        return NGX_CONF_ERROR;
    }
    mcf->pool = cf->pool;
    mcf->log = cf->log;
    mcf->prefix = cf->cycle->prefix;
    // TODO: you can initialize mcf here
    return NGX_CONF_OK;
}
