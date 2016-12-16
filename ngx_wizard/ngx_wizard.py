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

use_upstream = lambda handler: 'upstream' in handler and None != handler['upstream']
get_uri = lambda handler: handler['uri'].replace('/', '_')
substitute_tpls = lambda tpls, vars: join_lines([tpl.substitute(vars) for tpl in tpls], '')
get_ctx = lambda md, handler: '%s_%s_ctx_t' % (md, get_uri(handler))

def load_templates():
    cwd = os.path.split(os.path.realpath(__file__))[0]
    tpl_path = os.path.join(cwd, 'tpl')
    with open(os.path.join(tpl_path, 'tpls.json')) as f:
        cf = json.load(f)
        templates = {}
        for item in cf['tpls']:
            with open(os.path.join(tpl_path, item)) as f:
                key = os.path.splitext(item)[0]
                val = string.Template(f.read())
                templates[key] = val
        for item in cf['tpl_lines']:
            with open(os.path.join(tpl_path, item)) as f:
                key = os.path.splitext(item)[0]
                templates[key] = [string.Template(line) for line in f]
        return templates

tpls = load_templates()
type_dict = json.loads(tpls['type_dict'].template)
fmts = json.loads(tpls['fmt'].template)
consts = json.loads(tpls['consts'].template)

def gen_config(addon, md, handlers):
    __gen_handler = lambda md, uri: ('    $ngx_addon_dir/%s_%s_handler.c\\') % (md, uri)
    write_file('%s/config' % addon, merge([
        'ngx_addon_name=ngx_http_%s_module' % md,
        'HTTP_MODULES="$HTTP_MODULES ngx_http_%s_module"' % md,
        'NGX_ADDON_SRCS="$NGX_ADDON_SRCS \\',
        '    $ngx_addon_dir/%s_utils.c\\' % md,
        merge([__gen_handler(md, uri) for uri in [get_uri(handler) for handler in handlers]]),
        '    $ngx_addon_dir/ngx_http_%s_module.c"' % md
        ]))
def gen_tm():
    return datetime.datetime.now().strftime('%Y%m%d%H%M%S')
def gen_head_frame(md, submd, str):
    return tpls['header'].substitute({'var_head_def': '__%s_%s_%s_h__' % (md, submd, gen_tm()), 'var_str': str})
def gen_utils(addon, md, has_shm_dict, has_peer_sel, has_http_fetch, mcf):
    __get_type = lambda t: type_dict[t] if t in type_dict else t
    __gen_mcf = lambda md, mcf: tpls['main_conf'].substitute({
        'var_items': merge(['    %s %s;' % (__get_type(item[TYPE]), item[NAME]) for item in mcf]) if len(mcf) > 0 else FILTER,
        'var_mcf_t': consts['mcf_fmt'] % md,
        'var_get_mcf': consts['get_mcf_fmt'] % md
        })
    write_file('%s/%s_utils.h' % (addon, md), gen_head_frame(md, 'utils', tpls['utils'].substitute({
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
            ) % (md, get_uri(handler), consts['request_args']), handlers))
        ])))
def gen_create_ctx(md, handler): 
    return tpls['create_ctx'].substitute({'var_ctx_t': get_ctx(md, handler)})
def gen_parallel_call(use_parallel, handler, end):
    return tpls['parallel_call'].substitute({'var_end': end}) if use_parallel(handler) else FILTER
def gen_parallel_imp(use_parallel, md, handler):
    return tpls['parallel_subrequests'].substitute({
        'var_ctx_t': get_ctx(md, handler),
        'var_mcf_t': consts['mcf_fmt'] % md,
        'var_get_mcf': consts['get_mcf_fmt'] % md,
        'var_create_ctx': gen_create_ctx(md, handler),
        }) if use_parallel(handler) else FILTER

def gen_handler_imp(addon, md, handler):
    __check_kv = lambda key, val, handler: (key in handler) and (val == handler[key])
    __request_body_action = lambda action: lambda handler: __check_kv('action_for_request_body', action, handler)
    __read_request_body = __request_body_action('read')
    __discard_request_body = __request_body_action('discard')
    __upstream_has_key = lambda key: lambda handler: use_upstream(handler) and (
        key in handler['upstream'] and handler['upstream'][key])
    __use_parallel = __upstream_has_key('parallel_subrequests')
    __use_sequential = __upstream_has_key('sequential_subrequests')
    __gen_ctx = lambda md, handler: tpls['parallel_ctx'].substitute({
            'var_ctx_t': get_ctx(md, handler)
        }) if __use_parallel(handler) else substitute_tpls(tpls['upstream_ctx'], {
            'var_peer': '    %s;' % consts['peer_def'] if __use_sequential(handler) else FILTER,
            'var_ctx_t': get_ctx(md, handler)
        }) if use_upstream(handler) else FILTER
    __gen_check_parameter = lambda: tpls['check'].substitute({'var_args': consts['request_args']})
    __gen_post_subrequest_handler = lambda md, handler: FILTER if (__use_parallel(handler) or not use_upstream(handler)
        ) else tpls['post_subrequest'].substitute({'var_ctx_t': get_ctx(md, handler)})
    __gen_sr_peer = lambda handler: 'ctx->peer' if __use_sequential(handler) else 'peer'
    __gen_sr = lambda prefix, backend_uri, handler: merge([
        merge([
            '    // TODO: initialize the peer here',
            '    %s = NULL;' % consts['peer_def']
            ]) if not __use_sequential(handler) else FILTER,
        '    %sngx_http_gen_subrequest(%s, r, %s,' % (prefix, backend_uri, __gen_sr_peer(handler)),
        '        &ctx->base, __post_subrequest_handler);'
        ])
    __gen_post_body_impl = lambda md, handler: tpls['parallel_post_body'].substitute({
            'var_parallel_call': gen_parallel_call(__use_parallel, handler, '    // TODO')
        }) if __use_parallel(handler) else tpls['sequential_post_body'].substitute({
            'var_ctx_t': get_ctx(md, handler),
            'var_sr': __gen_sr('', 'ctx->base.backend_uri', handler)
        }) if use_upstream(handler) else '    ngx_http_post_body_handler(r, __post_body_cb);'
    __gen_post_body_handler = lambda md, handler: merge([
        FILTER if use_upstream(handler) else tpls['post_body_cb'].template,
        tpls['post_body_handler'].substitute({'var_impl': __gen_post_body_impl(md, handler)})
        ]) if __read_request_body(handler) else FILTER
    __gen_methods_filter = lambda handler: tpls['http_not_allowed'].substitute({
        'var_cond': string.join(['!(r->method & NGX_HTTP_%s)' % method.upper() for method in handler['methods']], ' && ')
        }) if 'methods' in handler and len(handler['methods']) > 0 else FILTER
    __gen_discard_body = lambda handler: tpls['discard_body'].template if __discard_request_body(handler) else FILTER
    __gen_init_peer = lambda handler: tpls['init_peer'].template if __use_sequential(handler) else FILTER
    __gen_init_ctx_base = lambda handler: '    ctx->base.backend_uri = backend_uri;' if __read_request_body(handler) else FILTER
    __gen_init_ctx = lambda handler: '    ctx->peer = ngx_http_first_peer(peers->peer);\n' if __use_sequential(handler) else FILTER
    __gen_read_body = lambda handler: tpls['read_body'].substitute({'var_rc': ('rc' if __discard_request_body(handler) else 'ngx_int_t rc')})
    __gen_first_handler = lambda md, handler: FILTER if __use_parallel(handler) else merge([
        'static ngx_int_t __first_%s_handler(%s)' % (get_uri(handler), consts['request_args']),
        '{',
        __gen_methods_filter(handler),
        __gen_discard_body(handler),
        tpls['call_check'].template,
        __gen_init_peer(handler),
        gen_create_ctx(md, handler),
        __gen_init_ctx_base(handler),
        __gen_init_ctx(handler),
        __gen_read_body(handler) if __read_request_body(handler) else __gen_sr(
            'return ', 'backend_uri', handler),
        '}',
        ''
        ]) if use_upstream(handler) else FILTER
    __gen_first_loop = lambda md, handler: tpls['first_loop'].substitute({
        'var_ctx_t': get_ctx(md, handler), 
        'var_first_handler': '__first_%s_handler' % get_uri(handler)
        })
    __gen_next_loop = lambda handler: tpls['next_loop'].substitute({
        'var_sr_peer': __gen_sr_peer(handler)}) if __use_sequential(handler) else FILTER
    __gen_request_handler = lambda md, handler: merge([
        'ngx_int_t %s_%s_handler(%s)' % (md, get_uri(handler), consts['request_args']),
        '{',
        merge([
            __gen_methods_filter(handler),
            __gen_discard_body(handler),
            tpls['call_check'].template,
            merge(['    %s' % consts['set_ctx'], __gen_read_body(handler)]) if __read_request_body(
                handler) else gen_parallel_call(__use_parallel, handler, '    return NGX_DONE;'),
            ]) if __use_parallel(handler) else merge([
            __gen_first_loop(md, handler),
            __gen_next_loop(handler),
            tpls['final_loop'].template
            ]) if use_upstream(handler) else merge([
            __gen_methods_filter(handler),
            __gen_discard_body(handler),
            tpls['call_check'].template,
            __gen_read_body(handler) if __read_request_body(
                handler) else tpls['default_handler'].template
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
    __gen_frame = lambda md, field, impl: tpls['mcf_frame'].substitute({
        'var_func': 'ngx_http_%s' % field,
        'var_args': consts['conf_args'],
        'var_mcf_t': consts['mcf_fmt'] % md,
        'var_get_mcf': '%s(cf, ngx_http_%s_module)' % (consts['get_mcf'], md),
        'var_impl': impl
        })
    __gen_int_base = lambda parse_str: lambda field: tpls['mcf_int'].substitute({'var_field': field, 'var_value': parse_str})
    __gen_default = lambda field: tpls['mcf_default'].substitute({'var_field': field})
    __dict = { 
        'int': __gen_int_base('ngx_atoi(value[1].data, value[1].len)'), 
        'size': __gen_int_base('ngx_parse_size(&value[1])'),
        'time': __gen_int_base('ngx_parse_time(&value[1], 0)'),
        'bool': lambda field: tpls['mcf_bool'].substitute({'var_field': field}),
        'string': lambda field: tpls['mcf_string'].substitute({'var_field': field})
        }
    __get_func = lambda key: __dict[key] if key in __dict else __gen_default
    return merge([__gen_frame(md, item[NAME], __get_func(item[TYPE])(item[NAME])) for item in mcf]) if len(mcf) > 0 else FILTER

def gen_module_vars(md, mcf, handlers):
    __gen_handler_dict = lambda md, handlers: tpls['handler_dict'].substitute({
        'var_dict': '%s_handler_dict' % md,
        'var_dict_len': '%s_handler_dict_len' % md,
        'var_impl': string.join([tpls['handler_item'].substitute({
            'var_uri': 'ngx_string("/%s")' % handler['uri'],
            'var_upstream': ('ngx_string("%s")' % handler['upstream']['backend_uri']) if use_upstream(handler) else 'ngx_null_string',
            'var_handler': '%s_%s_handler' % (md, get_uri(handler))
            }) for handler in handlers], ',\n')
        })
    __gen_mcf_cmds = lambda mcf: merge(['    APPEND_MCF_ITEM("%s", ngx_http_%s),' % (item[NAME], item[NAME]) for item in mcf])
    __gen_commands = lambda md, mcf: tpls['commands'].substitute({
        'var_commands': 'ngx_http_%s_commands' % md,
        'var_md': 'ngx_string("%s")' % md,
        'var_handler': 'ngx_http_%s' % md,
        'var_items': __gen_mcf_cmds(mcf) if len(mcf) > 0 else ''
        })
    __gen_module_ctx = lambda md, mcf: tpls['module_ctx'].substitute({
        'var_ctx': 'ngx_http_%s_module_ctx' % md,
        'var_create': 'ngx_http_%s_create_main_conf' % md,
        'var_init': 'ngx_http_%s_init_main_conf' % md
        })
    __gen_module = lambda md: tpls['module'].substitute({
        'var_md': 'ngx_http_%s_module' % md,
        'var_md_ctx': 'ngx_http_%s_module_ctx' % md,
        'var_commands': 'ngx_http_%s_commands' % md,
        'var_init_module': 'ngx_http_%s_init_module' % md,
        'var_init_process': 'ngx_http_%s_init_process' % md,
        'var_exit_process': 'ngx_http_%s_exit_process' % md,
        'var_exit_master': 'ngx_http_%s_exit_master' % md
        })
    return merge([
        __gen_handler_dict(md, handlers),
        __gen_commands(md, mcf),
        __gen_module_ctx(md, mcf),
        __gen_module(md)
        ])

def gen_module_dec(md, mcf):
    __gen_includes = lambda md: '#include "%s_handler.h"\n' % md
    __gen_declare = lambda md, mcf: merge([
        fmts['module_handler'] % (md, ';'),
        fmts['module_conf'] % (md, ';'),
        fmts['init_module'] % (md, ';'),
        fmts['init_process'] % (md, ';'),
        fmts['exit_process'] % (md, ';'),
        fmts['exit_master'] % (md, ';')
        ])
    __gen_mcf_dec = lambda md, mcf: merge([
        merge(map(lambda item: (
            'static char * ngx_http_%s(%s);'
            ) % (item[NAME], consts['conf_args']), mcf)
        ) if len(mcf) > 0 else FILTER,
        fmts['create_main_conf'] % (md, ';'),
        fmts['init_main_conf'] % (md, ';'),
        ''
        ])
    return merge([
        __gen_includes(md),
        __gen_declare(md, mcf),
        __gen_mcf_dec(md, mcf)
        ])

def gen_module_imp(md, mcf):
    __gen_module_conf = lambda md: tpls['module_conf'].substitute({
        'var_declare': fmts['module_conf'] % (md, ''),
        'var_handler': 'ngx_http_%s_handler' % md
        })
    __gen_module_handler = lambda md: tpls['module_handler'].substitute({
        'var_declare': fmts['module_handler'] % (md, ''),
        'var_dict': '%s_handler_dict' % md,
        'var_dict_len': '%s_handler_dict_len' % md
        })
    __gen_init_module = lambda md: tpls['init_process'].substitute({
        'var_declare': fmts['init_module'] % (md, ''),
        'var_type': 'master'
        })
    __gen_init_process = lambda md: tpls['init_process'].substitute({
        'var_declare': fmts['init_process'] % (md, ''),
        'var_type': 'worker'
        })
    __gen_exit_process = lambda md: tpls['exit_process'].substitute({
        'var_declare': fmts['exit_process'] % (md, ''),
        'var_type': 'worker'
        })
    __gen_exit_master = lambda md: tpls['exit_process'].substitute({
        'var_declare': fmts['exit_master'] % (md, ''),
        'var_type': 'master'
        })
    __gen_main_conf = lambda md: tpls['mcf'].substitute({
        'var_create': fmts['create_main_conf'] % (md, ''),
        'var_init': fmts['init_main_conf'] % (md, ''),
        'var_mcf_t': consts['mcf_fmt'] % md
        })
    __gen_return = lambda val: tpls['return'].substitute({'var_val': val})
    __gen_module_tail = lambda md: tpls['module_tail'].substitute({
        'var_null_return': __gen_return(' NULL'),
        'var_no_return': __gen_return(''),
        'var_md': 'ngx_http_%s_module' % md,
        'var_get_mcf': consts['get_mcf_fmt'] % md
        })
    return merge([
        __gen_module_conf(md),
        __gen_module_handler(md),
        __gen_init_module(md),
        __gen_init_process(md),
        __gen_exit_process(md),
        __gen_exit_master(md),
        __gen_main_conf(md),
        __gen_module_tail(md)
        ])
    
def gen_module(addon, md, mcf, handlers):
    write_file('%s/ngx_http_%s_module.c' % (addon, md), merge([
        gen_module_dec(md, mcf),
        gen_module_vars(md, mcf, handlers),
        gen_main_conf(md, mcf),
        gen_module_imp(md, mcf)
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
    return True
        
def get_json_data(path):
    json_filter = lambda f: (lambda f, l: os.path.splitext(f)[1] in l)(f, ['.json'])
    if not json_filter(path):
        return None
    f = open(path, 'r')
    data = json.load(f)
    f.close()
    return data
    
def gen_addon(uri):
    obj = get_json_data(uri)
    if not obj:
        return False
    root = os.path.dirname(os.path.abspath(uri))
    mdpath = os.path.join(root, obj['module'])
    addon = os.path.join(mdpath, 'addon')
    return gen_code(mdpath, addon, obj)

def parse_shell(argv):
    return gen_addon(argv[1]) if 2 == len(argv) else False
if __name__ == "__main__":
    if not parse_shell(sys.argv):
        manual()