#!/usr/bin/python
# email: yao050421103@gmail.com
import sys
import platform
import os.path
import re
import datetime
import string
import json

TYPE = 0
NAME = 1

FILTER = '@null_string_place_holder'
join_lines = lambda lines, dim: string.join(filter(lambda item: -1 == item.find(FILTER), lines), dim)
merge = lambda lines: join_lines(lines, '\n')

def manual(): 
    print """
    usage:
        python ngx_wizard.py [conf]
    sample:
        python ngx_wizard.py ngx_wizard.json
        """
def write_file(path, data):
    with open(path, 'w') as f:
        f.writelines(data)
def load_file(url):
    with open(url) as f:
        return f.read()
def load_from_tpl(url, vars):
    return string.Template(load_file(url)).substitute(vars)
def load_from_tpl_lines(url, vars):
    with open(url) as f:
        return join_lines([string.Template(line).substitute(vars) for line in f], '')

req_params = 'ngx_str_t * backend_uri, ngx_http_request_t *r'
conf_params = 'ngx_conf_t * cf, ngx_command_t * cmd, void * conf'
use_upstream = lambda handler: 'upstream' in handler and None != handler['upstream']
get_uri = lambda handler: handler['uri'].replace('/', '_')

# init tpl
cwd = os.path.split(os.path.realpath(__file__))[0]
type_dict = json.loads(load_file(os.path.join(cwd, 'tpl/type_dict.json')))
read_tpl = lambda url: load_file(os.path.join(cwd, url))
read_from_tpl = lambda url, vars: load_from_tpl(os.path.join(cwd, url), vars)
read_from_tpl_lines = lambda url, vars: load_from_tpl_lines(os.path.join(cwd, url), vars)

def gen_config(addon, md, handlers):
    __gen_handler = lambda md: lambda cmd: (
        '    $ngx_addon_dir/%s_%s_handler.c\\'
        ) % (md, cmd)
    write_file('%s/config' % addon, merge([
        'ngx_addon_name=ngx_http_%s_module' % md,
        'HTTP_MODULES="$HTTP_MODULES ngx_http_%s_module"' % md,
        'NGX_ADDON_SRCS="$NGX_ADDON_SRCS \\',
        '    $ngx_addon_dir/%s_utils.c\\' % md,
        merge(map(__gen_handler(md), map(lambda handler: get_uri(handler), handlers))),
        '    $ngx_addon_dir/ngx_http_%s_module.c"' % md
        ]))
def gen_tm():
    return datetime.datetime.now().strftime('%Y%m%d%H%M%S')
def gen_head_frame(md, submd, str):
    return read_from_tpl('tpl/header.h', {'var_head_def': '__%s_%s_%s_h__' % (md, submd, gen_tm()), 'var_str': str})
def gen_utils(addon, md, has_shm_dict, has_peer_sel, has_http_fetch, mcf):
    __get_type = lambda t: type_dict[t] if t in type_dict else t
    __gen_mcf = lambda md, mcf: read_from_tpl('tpl/main_conf.h', {
        'var_items': merge(['    %s %s;' % (__get_type(item[TYPE]), item[NAME]) for item in mcf]) if len(mcf) > 0 else FILTER,
        'var_mcf_t': 'ngx_http_%s_main_conf_t' % md,
        'var_get_mcf': '%s_get_module_main_conf' % md
        })
    write_file('%s/%s_utils.h' % (addon, md), gen_head_frame(md, 'utils', read_from_tpl('tpl/utils.h', {
        'var_includes': merge([
            '#include <ngx_shm_dict.h>' if has_shm_dict else FILTER,
            '#include <ngx_http_peer_selector.h>' if has_peer_sel else FILTER,
            '#include <ngx_http_fetch.h>' if has_http_fetch else FILTER
            ]),
        'var_mcf': __gen_mcf(md, mcf)
        })))
    write_file('%s/%s_utils.c' % (addon, md), merge([
        '#include "%s_utils.h"' % md
        ]))
def gen_handler_dec(addon, md, handlers):
    write_file('%s/%s_handler.h' % (addon, md), gen_head_frame(md, 'handler', merge([
        '#include "%s_utils.h"' % md,
        '',
        merge(map(lambda handler: (
            'ngx_int_t %s_%s_handler(%s);'
            ) % (md, get_uri(handler), req_params), handlers))
        ])))
def gen_create_ctx(md, handler): 
    return read_from_tpl('tpl/create_ctx.c', {'var_ctx_t': '%s_%s_ctx_t' % (md, get_uri(handler))})
def gen_parallel_call(use_parallel, handler, end):
    return read_from_tpl('tpl/parallel_call.c', {'var_end': end}) if use_parallel(handler) else FILTER
def gen_parallel_imp(use_parallel, md, handler):
    # "upstream"
    #     "parallel_subrequests"
    return read_from_tpl('tpl/parallel_subrequests.c', {
        'var_ctx_t': '%s_%s_ctx_t' % (md, get_uri(handler)),
        'var_mcf_t': 'ngx_http_%s_main_conf_t' % md,
        'var_get_mcf': '%s_get_module_main_conf(r);' % md,
        'var_create_ctx': gen_create_ctx(md, handler),
        }) if use_parallel(handler) else FILTER

def gen_handler_imp(addon, md, handler):
    # "action_for_request_body": ["read", "discard", "default"]
    # "methods": ["GET", "HEAD", "POST", "PUT", "DELETE"]
    # "upstream"
    #     "sequential_subrequests"
    #     "parallel_subrequests"
    __request_body_action = lambda action: lambda handler: (
        'action_for_request_body' in handler) and (
            action == handler['action_for_request_body'])
    __read_request_body = __request_body_action('read')
    __discard_request_body = __request_body_action('discard')
    __upstream_has_key = lambda key: lambda handler: use_upstream(handler) and (
        key in handler['upstream'] and handler['upstream'][key])
    __use_parallel = __upstream_has_key('parallel_subrequests')
    __use_sequential = __upstream_has_key('sequential_subrequests')

    __gen_ctx = lambda md, handler: read_from_tpl(
            'tpl/parallel_ctx.h', {'var_ctx_t': '%s_%s_ctx_t' % (md, get_uri(handler))}
        ) if __use_parallel(handler) else read_from_tpl_lines('tpl/upstream_ctx.h', {
            'var_peer': '    ngx_http_upstream_rr_peer_t * peer;' if __use_sequential(handler) else FILTER,
            'var_ctx_t': '%s_%s_ctx_t' % (md, get_uri(handler))
        }) if use_upstream(handler) else FILTER
    __gen_check_parameter = lambda: read_from_tpl('tpl/check.c', {'var_args': req_params})
    __gen_post_subrequest_handler = lambda md, handler: FILTER if __use_parallel(handler) else read_from_tpl(
        'tpl/post_subrequest.c', {'var_ctx_t': '%s_%s_ctx_t' % (md, get_uri(handler))})
    __gen_sr_peer = lambda handler: 'ctx->peer' if __use_sequential(handler) else 'peer'
    __gen_sr = lambda prefix, backend_uri, handler: merge([
        merge([
            '    // TODO: initialize the peer here',
            '    ngx_http_upstream_rr_peer_t * peer = NULL;'
            ]) if not __use_sequential(handler) else FILTER,
        '    %sngx_http_gen_subrequest(%s, r, %s,' % (prefix, backend_uri, __gen_sr_peer(handler)),
        '        &ctx->base, __post_subrequest_handler);'
        ])
    # "action_for_request_body": "read"
    # "upstream" (optional)
    #     "sequential_subrequests"
    #     "parallel_subrequests"
    __gen_post_body_impl = lambda md, handler: read_from_tpl(
        'tpl/parallel_post_body.c', {
            'var_parallel_call': gen_parallel_call(__use_parallel, handler, '    // TODO')
        }) if __use_parallel(handler) else read_from_tpl(
        'tpl/sequential_post_body.c', {
            'var_ctx_t': '%s_%s_ctx_t' % (md, get_uri(handler)),
            'var_sr': __gen_sr('', 'ctx->base.backend_uri', handler)
        }) if use_upstream(handler) else '    ngx_http_post_body_handler(r, __post_body_cb);'
    __gen_post_body_handler = lambda md, handler: merge([
        FILTER if use_upstream(handler) else read_tpl('tpl/post_body_cb.c'),
        read_from_tpl('tpl/post_body_handler.c', {'var_impl': __gen_post_body_impl(md, handler)})
        ]) if __read_request_body(handler) else FILTER
    # 'methods'
    __gen_methods_filter = lambda handler: read_from_tpl('tpl/http_not_allowed.c', {
        'var_cond': string.join(['!(r->method & NGX_HTTP_%s)' % method.upper() for method in handler['methods']], ' && ')
        }) if 'methods' in handler and len(handler['methods']) > 0 else FILTER
    # "action_for_request_body": "discard"
    __gen_discard_body = lambda handler: read_tpl('tpl/discard_body.c') if __discard_request_body(handler) else FILTER
    __call_check = read_tpl('tpl/call_check.c')
    # "upstream"
    #     "sequential_subrequests"
    #         "subrequests"
    __gen_init_peer = lambda handler: read_tpl('tpl/init_peer.c') if __use_sequential(handler) else FILTER
    __gen_init_ctx_base = lambda handler: '    ctx->base.backend_uri = backend_uri;' if __read_request_body(handler) else FILTER
    __gen_init_ctx = lambda handler: '    ctx->peer = ngx_http_first_peer(peers->peer);\n' if __use_sequential(handler) else FILTER
    __gen_read_body = lambda handler: read_from_tpl('tpl/read_body.c', {
        'var_rc': ('rc' if __discard_request_body(handler) else 'ngx_int_t rc')})
    # 'action_for_request_body'
    # 'methods'
    # 'upstream'
    __gen_first_handler = lambda md, handler: FILTER if __use_parallel(handler) else merge([
        'static ngx_int_t __first_%s_handler(%s)' % (get_uri(handler), req_params),
        '{',
        __gen_methods_filter(handler),
        __gen_discard_body(handler),
        __call_check,
        __gen_init_peer(handler),
        gen_create_ctx(md, handler),
        __gen_init_ctx_base(handler),
        __gen_init_ctx(handler),
        __gen_read_body(handler) if __read_request_body(handler) else __gen_sr(
            'return ', 'backend_uri', handler),
        '}',
        ''
        ]) if use_upstream(handler) else FILTER
    __gen_handler_tail = lambda: merge([
        '    // TODO: you can implement the business here',
        '',
        '    ngx_str_t response = ngx_string("Hello World!");',
        '    r->headers_out.status = NGX_HTTP_OK;',
        '    return ngx_http_send_response_imp(r->headers_out.status, &response, r);'
        ])
    __gen_first_loop = lambda md, handler: merge([
        '    %s_%s_ctx_t * ctx = ngx_http_get_addon_module_ctx(r);' % (md, get_uri(handler)),
        '    if (!ctx)',
        '    {',
        '        return __first_%s_handler(backend_uri, r);' % get_uri(handler),
        '    }',
        ])
    __gen_next_loop = lambda handler: merge([
        '    if (NGX_HTTP_OK != r->headers_out.status)',
        '    {',
        '        ctx->peer = ngx_http_next_peer(ctx->peer);',
        '        return (ctx->peer) ? ngx_http_run_subrequest(r, &ctx->base, %s)' % __gen_sr_peer(handler),
        '            : ngx_http_send_response_imp(NGX_HTTP_NOT_FOUND, NULL, r);',
        '    }'
        ]) if __use_sequential(handler) else FILTER
    __gen_final_loop = lambda: merge([
        '    // TODO: you decide the return value',
        '    return ngx_http_send_response_imp(NGX_HTTP_OK, &ctx->base.response, r);'
        ])
    __gen_request_handler = lambda md, handler: merge([
        'ngx_int_t %s_%s_handler(%s)' % (md, get_uri(handler), req_params),
        '{',
        merge([
            __gen_methods_filter(handler),
            __gen_discard_body(handler),
            __call_check,
            merge([
                '    ngx_http_set_addon_module_ctx(r, backend_uri);',
                __gen_read_body(handler)
                ]) if __read_request_body(
                handler) else gen_parallel_call(__use_parallel, handler, '    return NGX_DONE;'),
            ]) if __use_parallel(handler) else merge([
            __gen_first_loop(md, handler),
            __gen_next_loop(handler),
            __gen_final_loop()
            ]) if use_upstream(handler) else merge([
            __gen_methods_filter(handler),
            __gen_discard_body(handler),
            __call_check,
            __gen_read_body(handler) if __read_request_body(
                handler) else __gen_handler_tail()
            ]),
        '}'
        ])
    write_file('%s/%s_%s_handler.c' % (addon, md, get_uri(handler)), merge([
        '#include "%s_handler.h"' % md,
        '',
        __gen_ctx(md, handler),
        __gen_check_parameter(),
        gen_parallel_imp(__use_parallel, md, handler),
        __gen_post_subrequest_handler(md, handler),
        __gen_post_body_handler(md, handler),
        __gen_first_handler(md, handler),
        __gen_request_handler(md, handler)
        ]))

def gen_handlers_imp(addon, md, handlers):
    for handler in handlers:
        gen_handler_imp(addon, md, handler)

def gen_main_conf(md, mcf):
    __get_mcf_func = 'ngx_http_conf_get_module_main_conf'
    __gen_frame = lambda md, field, impl: merge([
        'static char * ngx_http_%s(%s)' % (field, conf_params),
        '{',
        '    ngx_http_%s_main_conf_t * mcf = %s(cf, ngx_http_%s_module);' % (md, __get_mcf_func, md),
        '    if (!mcf || 2 != cf->args->nelts)',
        '    {',
        '        return "ngx_http_%s error";' % field,
        '    }',
        impl,
        '    // TODO: you can modify the value here',
        '    return NGX_CONF_OK;',
        '}',
        ''
        ])
    __gen_int_base = lambda parse_str: lambda field: merge([
        '    ngx_str_t * value = cf->args->elts;',
        '    mcf->%s = %s;' % (field, parse_str),
        '    if (NGX_ERROR == mcf->%s)' % field,
        '    {',
        '        return "ngx_http_%s error";' % field,
        '    }'
        ])
    __gen_int = __gen_int_base('ngx_atoi(value[1].data, value[1].len)')
    __gen_size = __gen_int_base('ngx_parse_size(&value[1])')
    __gen_time = __gen_int_base('ngx_parse_time(&value[1], 0)')
    __gen_bool = lambda field: merge([
        '    int val = ngx_http_get_flag_slot(cf);',
        '    if (NGX_ERROR == val)',
        '    {',
        '        return "ngx_http_%s error";' % field,
        '    }',
        '    mcf->%s = val;' % field
        ])
    __gen_str = lambda field: merge([
        '    ngx_str_t * arr = cf->args->elts;',
        '    mcf->%s = ngx_http_make_str(&arr[1], cf->pool);' % field
        ])
    __gen_unknown = lambda field: merge([
        '    ngx_str_t * arr = cf->args->elts;',
        '    ngx_str_t * val = &arr[1]; // mcf->%s' % field
        ])
    __dict = { 
        'int': __gen_int, 
        'size': __gen_size,
        'time': __gen_time,
        'bool': __gen_bool, 
        'string': __gen_str 
        }
    __get_func = lambda key: __dict[key] if key in __dict else __gen_unknown
    
    return merge(map(lambda item: __gen_frame(
        md, item[NAME], __get_func(item[TYPE])(item[NAME])
        ), mcf)) if len(mcf) > 0 else FILTER

def gen_module_dict():
    __module_handler = lambda md, str: (
        'static ngx_int_t ngx_http_%s_handler(ngx_http_request_t *r)%s') % (md, str)
    __module_conf = lambda md, str: (
        'static char *ngx_http_%s(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)%s'
        ) % (md, str)
    __init_module = lambda md, str: (
        'static ngx_int_t ngx_http_%s_init_module(ngx_cycle_t * cycle)%s'
        ) % (md, str)
    __init_process = lambda md, str: (
        'static ngx_int_t ngx_http_%s_init_process(ngx_cycle_t * cycle)%s'
        ) % (md, str)
    __exit_process = lambda md, str: (
        'static void ngx_http_%s_exit_process(ngx_cycle_t * cycle)%s'
        ) % (md, str)
    __exit_master = lambda md, str: (
        'static void ngx_http_%s_exit_master(ngx_cycle_t * cycle)%s'
        ) % (md, str)
    __create_main_conf = lambda md, str: (
        'static void * ngx_http_%s_create_main_conf(ngx_conf_t *cf)%s') % (md, str)
    __init_main_conf = lambda md, str: (
        'char * ngx_http_%s_init_main_conf(ngx_conf_t * cf, void * conf)%s') % (md, str)
    
    return {
        'module_handler': __module_handler,
        'module_conf': __module_conf,
        'init_module': __init_module,
        'init_process': __init_process,
        'exit_process': __exit_process,
        'exit_master': __exit_master,
        'create_main_conf': __create_main_conf,
        'init_main_conf': __init_main_conf,
        }

def gen_module_vars(md, mcf, handlers):
    __item_len = 'sizeof(ngx_http_request_item_t)'
    __gen_handler_dict = lambda md, handlers: merge([
        'static ngx_http_request_item_t %s_handler_dict[] =' % md,
        '{',
        reduce(lambda s1, s2: '%s,\n%s' % (s1, s2), map(lambda handler: merge([
        '    {',
        '        ngx_string("/%s"),' % handler['uri'],
        '        %s,' % ((
            'ngx_string("%s")' % handler['upstream']['backend_uri']
            ) if use_upstream(handler) else 'ngx_null_string'),
        '        %s_%s_handler' % (md, get_uri(handler)),
        '    }'
        ]), handlers)),
        '};',
        '',
        'static size_t %s_handler_dict_len = sizeof(%s_handler_dict) / %s;' % (
            md, md, __item_len),
        ''
        ])
    __gen_mcf_cmds = lambda mcf: merge(map(
        lambda item: '    APPEND_MCF_ITEM("%s", ngx_http_%s),' % (item[NAME], item[NAME]), mcf))
    __gen_commands = lambda md, mcf: merge([
        'static ngx_command_t ngx_http_%s_commands[] =' % md,
        '{',
        '    {',
        '        ngx_string("%s"),' % md,
        '        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,',
        '        ngx_http_%s,' % md,
        '        NGX_HTTP_LOC_CONF_OFFSET,',
        '        0,',
        '        NULL',
        '    },',
        __gen_mcf_cmds(mcf) if len(mcf) > 0 else FILTER,
        '    ngx_null_command',
        '};',
        ''
        ])
    __gen_module_ctx = lambda md, mcf: merge([
        'static ngx_http_module_t ngx_http_%s_module_ctx =' % md,
        '{',
        '    NULL, // ngx_int_t (*preconfiguration)(ngx_conf_t *cf);',
        '    NULL, // ngx_int_t (*postconfiguration)(ngx_conf_t *cf);',
        '    ngx_http_%s_create_main_conf,' % md,
        '    ngx_http_%s_init_main_conf,' % md,
        '    NULL, // void * (*create_srv_conf)(ngx_conf_t *cf);',
        '    NULL, // char * (*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);',
        '    NULL, // void * (*create_loc_conf)(ngx_conf_t *cf);',
        '    NULL // char * (*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);',
        '};',
        ''
        ])
    __gen_module = lambda md: merge([
        'ngx_module_t ngx_http_%s_module =' % md,
        '{',
        '    NGX_MODULE_V1,',
        '    &ngx_http_%s_module_ctx,' % md,
        '    ngx_http_%s_commands,' % md,
        '    NGX_HTTP_MODULE,',
        '    NULL, // ngx_int_t (*init_master)(ngx_log_t *log);',
        '    ngx_http_%s_init_module,' % md,
        '    ngx_http_%s_init_process,' % md,
        '    NULL, // ngx_int_t (*init_thread)(ngx_cycle_t *cycle);',
        '    NULL, // void (*exit_thread)(ngx_cycle_t *cycle);',
        '    ngx_http_%s_exit_process,' % md,
        '    ngx_http_%s_exit_master,' % md,
        '    NGX_MODULE_V1_PADDING',
        '};',
        ''
        ])
    return merge([
        __gen_handler_dict(md, handlers),
        __gen_commands(md, mcf),
        __gen_module_ctx(md, mcf),
        __gen_module(md)
        ])

def gen_module_dec(md, mcf, module_dict):
    __gen_includes = lambda md: merge([
        '#include "%s_handler.h"' % md,
        ''
        ])
    __gen_declare = lambda md, mcf: merge([
        module_dict['module_handler'](md, ';'),
        module_dict['module_conf'](md, ';'),
        module_dict['init_module'](md, ';'),
        module_dict['init_process'](md, ';'),
        module_dict['exit_process'](md, ';'),
        module_dict['exit_master'](md, ';')
        ])
    __gen_mcf_dec = lambda md, mcf: merge([
        merge(map(lambda item: (
            'static char * ngx_http_%s(%s);'
            ) % (item[NAME], conf_params), mcf)
        ) if len(mcf) > 0 else FILTER,
        module_dict['create_main_conf'](md, ';'),
        module_dict['init_main_conf'](md, ';')
        ])
    return merge([
        __gen_includes(md),
        __gen_declare(md, mcf),
        __gen_mcf_dec(md, mcf),
        ''
        ])

def gen_module_imp(md, mcf, module_dict):
    __gen_module_conf = lambda md: merge([
        module_dict['module_conf'](md, ''),
        '{',
        '    ngx_http_core_loc_conf_t * clcf = ngx_http_conf_get_module_loc_conf(',
        '        cf, ngx_http_core_module);',
        '    clcf->handler = ngx_http_%s_handler;' % md,
        '    return NGX_CONF_OK;',
        '}',
        ''
        ])
    __gen_module_handler = lambda md: merge([
        module_dict['module_handler'](md, ''),
        '{',
        '    ngx_http_request_item_t * it = ngx_http_get_request_item(',
        '        %s_handler_dict, %s_handler_dict_len, &r->uri);' % (md, md),
        '    if (!it)',
        '    {',
        '        return NGX_ERROR;',
        '    }',
        '    return it->handler(&it->backend_uri, r);',
        '}',
        ''
        ])
    __gen_init_module = lambda md: merge([
        module_dict['init_module'](md, ''),
        '{',
        '    // TODO: initialize in master process',
        '    return NGX_OK;',
        '}',
        ''
        ])
    __gen_init_process = lambda md: merge([
        module_dict['init_process'](md, ''),
        '{',
        '    // TODO: initialize in worker process',
        '    return NGX_OK;',
        '}',
        ''
        ])
    __gen_exit_process = lambda md: merge([
        module_dict['exit_process'](md, ''),
        '{',
        '    // TODO: uninitialize in worker process',
        '}',
        ''
        ])
    __gen_exit_master = lambda md: merge([
        module_dict['exit_master'](md, ''),
        '{',
        '    // TODO: uninitialize in master process',
        '}',
        ''
        ])
    __gen_main_conf = lambda md: merge([
        module_dict['create_main_conf'](md, ''),
        '{',
        '    return ngx_pcalloc(cf->pool, sizeof(ngx_http_%s_main_conf_t));' % md,
        '}',
        '',
        module_dict['init_main_conf'](md, ''),
        '{',
        '    ngx_http_%s_main_conf_t * mcf = conf;' % md,
        '    if (!mcf)',
        '    {',
        '        return NGX_CONF_ERROR;',
        '    }',
        '    mcf->pool = cf->pool;',
        '    mcf->log = cf->log;',
        '    mcf->prefix = cf->cycle->prefix;',
        '    // TODO: you can initialize mcf here',
        '    return NGX_CONF_OK;',
        '}',
        ''
        ])
    __gen_return = lambda val: merge([
        '    if (!r)',
        '    {',
        '        return%s;' % val,
        '    }',
        ])
    __gen_ctx = lambda md: merge([
        'void * ngx_http_get_addon_module_ctx(ngx_http_request_t * r)',
        '{',
        __gen_return(' NULL'),
        '    return ngx_http_get_module_ctx(r, ngx_http_%s_module);' % md,
        '}',
        '',
        'void ngx_http_set_addon_module_ctx(ngx_http_request_t * r, void * ctx)',
        '{',
        __gen_return(''),
        '    ngx_http_set_ctx(r, ctx, ngx_http_%s_module);' % md,
        '}',
        ''
        ])
    __gen_get_mcf = lambda md: merge([
        'void * %s_get_module_main_conf(ngx_http_request_t * r)' % md,
        '{',
        __gen_return(' NULL'),
        '    return ngx_http_get_module_main_conf(r, ngx_http_%s_module);' % md,
        '}',
        ''
        ])
    return merge([
        __gen_module_conf(md),
        __gen_module_handler(md),
        __gen_init_module(md),
        __gen_init_process(md),
        __gen_exit_process(md),
        __gen_exit_master(md),
        __gen_main_conf(md),
        __gen_ctx(md),
        __gen_get_mcf(md)
        ])
    
def gen_module(addon, md, mcf, handlers):
    __module_dict = gen_module_dict()
    write_file('%s/ngx_http_%s_module.c' % (addon, md), merge([
        gen_module_dec(md, mcf, __module_dict),
        gen_module_vars(md, mcf, handlers),
        gen_main_conf(md, mcf),
        gen_module_imp(md, mcf, __module_dict)
        ]))
    
def gen_code(mdpath, addon, obj):
    __has_md = lambda k: lambda o: 'includes' in o and k in o['includes']
    __get_mcf = lambda o: o['main_conf'] if 'main_conf' in o else []
    if not os.path.exists(mdpath):
        os.mkdir(mdpath)
    if not os.path.exists(addon):
        os.mkdir(addon)
    md = obj['module']
    has_peer_sel = __has_md('ngx_http_peer_selector')(obj)
    has_shm_dict = __has_md('ngx_shm_dict')(obj)
    has_http_fetch = __has_md('ngx_http_fetch')(obj)
    mcf = __get_mcf(obj)
    handlers = obj['handlers']
    gen_config(addon, md, handlers)
    gen_utils(addon, md, has_shm_dict, has_peer_sel, has_http_fetch, mcf)
    gen_handler_dec(addon, md, handlers)
    gen_handlers_imp(addon, md, handlers)
    gen_module(addon, md, mcf, handlers)
        
def get_json_data(path):
    json_filter = lambda f: (lambda f, l: os.path.splitext(f)[1] in l)(f, ['.json'])
    if not json_filter(path):
        return None
    f = open(path, 'r')
    data = json.load(f)
    f.close()
    return data
    
def gen_addon(path):
    obj = get_json_data(path)
    if not obj:
        return False
    DIM = '\\' if ("Windows" == platform.system()) else '/'
    __md_path = lambda path, obj: '%s%s%s' % (os.path.dirname(os.path.abspath(path)), DIM, obj['module'])
    __addon_path = lambda mdpath: '%s%saddon' % (mdpath, DIM)
    mdpath = __md_path(path, obj)
    addon = __addon_path(mdpath)
    gen_code(mdpath, addon, obj)
    return True

def parse_shell(argv):
    if 2 == len(argv):
        gen_addon(argv[1])
    else:
        manual()

if __name__ == "__main__":
    parse_shell(sys.argv)