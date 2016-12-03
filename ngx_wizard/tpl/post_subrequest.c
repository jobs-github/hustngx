static ngx_int_t __post_subrequest_handler(
    ngx_http_request_t * r, void * data, ngx_int_t rc)
{
    $var_ctx_t * ctx = ngx_http_get_addon_module_ctx(r->parent);
    if (ctx && NGX_HTTP_OK == r->headers_out.status)
    {
        ctx->base.response.len = ngx_http_get_buf_size(
            &r->upstream->buffer);
        ctx->base.response.data = r->upstream->buffer.pos;
        // TODO: you can process the response from backend server here
    }
    return ngx_http_finish_subrequest(r);
}
