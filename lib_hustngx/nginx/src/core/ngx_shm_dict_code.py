#!/usr/bin/python
#author: jobs
#email: yao050421103@gmail.com
import sys
import os.path
import datetime
import string
import json

FILTER = '@null_string_place_holder'
merge = lambda l: string.join(filter(lambda item: FILTER != item, l), '\n')

double_mod = lambda op: merge([
    '        ngx_shm_dict_ctx_t * ctx = zone->data;',
    '        ngx_shmtx_lock(&ctx->shpool->mutex);',
    '        double * val = (double *) data.data;',
    '        *result = (*val %s= delta);' % op,
    '        ngx_shmtx_unlock(&ctx->shpool->mutex);'
    ])
    
ngx_shm_dict_types = [
    { 'key': 'int8', 'ctype': 'int8_t', 'macro': 'SHM_DICT_INT8', 'atomic': True },
    { 'key': 'uint8', 'ctype': 'uint8_t', 'macro': 'SHM_DICT_UINT8', 'atomic': True },
    { 'key': 'int16', 'ctype': 'int16_t', 'macro': 'SHM_DICT_INT16', 'atomic': True },
    { 'key': 'uint16', 'ctype': 'uint16_t', 'macro': 'SHM_DICT_UINT16', 'atomic': True },
    { 'key': 'int32', 'ctype': 'int32_t', 'macro': 'SHM_DICT_INT32', 'atomic': True },
    { 'key': 'uint32', 'ctype': 'uint32_t', 'macro': 'SHM_DICT_UINT32', 'atomic': True },
    { 'key': 'int64', 'ctype': 'int64_t', 'macro': 'SHM_DICT_INT64', 'atomic': True },
    { 'key': 'uint64', 'ctype': 'uint64_t', 'macro': 'SHM_DICT_UINT64', 'atomic': True },
    {
        'key': 'double', 
        'ctype': 'double', 
        'macro': 'SHM_DICT_DOUBLE', 
        'atomic': False,
        'incr': double_mod('+'),
        'decr': double_mod('-'),
    },
    { 
        'key': 'string', 
        'ctype': 'ngx_str_t', 
        'macro': 'SHM_DICT_STRING', 
        'read': {},
        'from': {},
        'to':
        {
            'dec': 'static int ngx_shm_dict_to_string(const ngx_str_t * src, ngx_pool_t * pool, ngx_str_t * dst)',
            'imp': '    return ngx_shm_dict_copy_str(src, pool, dst);'
        },
        'get_val': '    return ngx_shm_dict_to_string(data, (ngx_pool_t *)ctx, (ngx_str_t *)val);'
    },
    { 
        'key': 'binary', 
        'ctype': 'ngx_binary_t', 
        'macro': 'SHM_DICT_BINARY',
        'read': {},
        'from': {},
        'to':
        {
            'dec': 'static int ngx_shm_dict_to_binary(const ngx_str_t * src, ngx_pool_t * pool, ngx_binary_t * dst)',
            'imp': '    return ngx_shm_dict_copy_bin(src, pool, dst);'
        },
        'get_val': '    return ngx_shm_dict_to_binary(data, (ngx_pool_t *)ctx, (ngx_binary_t *)val);'
    }
    ]
gen_read_dec = lambda type: (
    'int ngx_shm_dict_get_%s(ngx_shm_zone_t * zone, ngx_str_t * key, ngx_pool_t * pool, %s * val)'
    ) % (type['key'], type['ctype'])
gen_read_imp = lambda type: (
    '    return pool ? ngx_shm_dict_get_base(zone, key, %s, ngx_shm_dict_get_%s_val, pool, val) : NGX_ERROR;'
    ) % (type['macro'], type['key'])
ngx_shm_dict_read = {
    'key': 'get', 
    'dec': lambda type: 'int ngx_shm_dict_get_%s(ngx_shm_zone_t * zone, ngx_str_t * key, %s * val)' % (
        type['key'], type['ctype']),
    'imp': lambda dec, type: merge([
        dec,
        '{',
        gen_read_imp(type) if 'read' in type else (
            '    return ngx_shm_dict_get_base(zone, key, %s, ngx_shm_dict_get_%s_val, NULL, val);'
            ) % (type['macro'], type['key']),
        '}',
        ''
        ])
    }
ngx_shm_dict_write = lambda key: {
    'key': key, 
    'dec': lambda type: 'int ngx_shm_dict_%s_%s(ngx_shm_zone_t * zone, ngx_str_t * key, %s val, uint32_t expire)' % (
        key, type['key'], type['ctype']),
    'imp': lambda dec, type: merge([
        dec,
        '{',
        '    ngx_str_t data;',
        '    ngx_shm_dict_from_%s(&val, &data);' % type['key'],
        '    return ngx_shm_dict_%s(zone, key, &data, %s, expire, 0);' % (key, type['macro']),
        '}',
        ''
        ])
    }
ngx_shm_dict_write_methods = ['add', 'safe_add', 'replace', 'set', 'safe_set']

atomic_mod = lambda func: lambda type: merge([
    '        %s * val = (%s *) data.data;' % (type['ctype'], type['ctype']),
    '        *result = %s(val, delta);' % func
    ])
gen_atomic_mod = { 'incr': atomic_mod('__sync_add_and_fetch'), 'decr': atomic_mod('__sync_sub_and_fetch') }

gen_atomic_dec = lambda key: lambda type: (
    'int ngx_shm_dict_%s_%s(ngx_shm_zone_t * zone, ngx_str_t * key, %s delta, uint32_t expire, %s * result)'
    ) % (key, type['key'], type['ctype'], type['ctype'])
gen_atomic_imp = lambda key: lambda type: merge([
    gen_atomic_dec(key)(type),
    '{',
    '    if (!zone || !key || !result)',
    '    {',
    '        return NGX_ERROR;',
    '    }',
    '    ngx_str_t data = ngx_null_string;',
    '    uint8_t value_type = %s;' % type['macro'],
    '    ngx_int_t rc = ngx_shm_dict_get(zone, key, &data, &value_type, NULL, NULL);',
    '    if (rc == NGX_OK)',
    '    {',
    '        if (value_type != %s)' % type['macro'],
    '        {',
    '            return NGX_ERROR;',
    '        }',
    gen_atomic_mod[key](type) if type['atomic'] else type[key],
    '    }',
    '    else',
    '    {',
    '        ngx_shm_dict_from_%s(&delta, &data);' % type['key'],
    '        rc = ngx_shm_dict_set(zone, key, &data, value_type, expire, 0);',
    '        if (rc == NGX_OK)',
    '        {',
    '            *result = delta;',
    '        }',
    '    }',
    '    return rc;',
    '}',
    ''
    ])
        
def gen(path, types, read, write, write_methods):
    __header = [
        '/**',
        '********************************************************************************',
        '* NOTE : Generated by ngx_shm_dict_code. It is NOT supposed to modify this file.',
        '*******************************************************************************/'
        ]
    __get_read_dec = lambda method, type: gen_read_dec(type) if 'read' in type else method['dec'](type)
    __get_write_dec = lambda method, type: method['dec'](type)
    def __write(url, data):
        with open(url, 'w') as f:
            f.writelines(data)
    def __gen_interface(path, types, num_types, read, write, write_methods):
        __includes = [
            '#include "ngx_shm_dict_utils.h"'
            ]
        tm = datetime.datetime.now().strftime('%Y%m%d%H%M%S')
        head_def = '__%s_%s_h__' % (os.path.basename(path).lower(), tm)
        
        __write(path + ".h", merge([
            merge(__header),
            '#ifndef %s' % head_def,
            '#define %s' % head_def,
            '',
            merge(__includes),
            '',
            merge(map(lambda type: '%s;' % __get_read_dec(read, type), types)),
            '',
            merge(map(lambda method: merge(
                map(lambda type: '%s;' % __get_write_dec(write(method), type), types)
                ) + '\n', write_methods)),
            merge(map(lambda type: '%s;' % gen_atomic_dec('incr')(type), num_types)),
            '',
            merge(map(lambda type: '%s;' % gen_atomic_dec('decr')(type), num_types)),
            '',
            '#endif // %s' % head_def
            ]))
    def __gen_implement(path, types, num_types, read, write, write_methods):
        __from = lambda type: merge([
            'static void ngx_shm_dict_from_%s(const %s * src, ngx_str_t * dst)' % (type['key'], type['ctype']),
            '{',
            merge([
                '    dst->len = src->len;',
                '    dst->data = (u_char *) src->data;'
                ]) if 'from' in type else merge([
                '    dst->len = sizeof(%s);' % type['ctype'],
                '    dst->data = (u_char *)src;'
                ]),
            '}',
            ''
            ])
        __to = lambda type: merge([
            type['to']['dec'] if 'to' in type and 'dec' in type['to'] else (
                'static int ngx_shm_dict_to_%s(const ngx_str_t * src, %s * dst)') % (type['key'], type['ctype']),
            '{',
            type['to']['imp'] if 'to' in type and 'imp' in type['to'] else merge([
                '    *dst = *((%s *) src->data);' % type['ctype'],
                '    return NGX_OK;'
                ]),
            '}',
            ''
            ])
        __get_val = lambda type: merge([
            'static int ngx_shm_dict_get_%s_val(const ngx_str_t * data, void * ctx, void * val)' % type['key'],
            '{',
            type['get_val'] if 'get_val' in type else (
                '    return ngx_shm_dict_to_%s(data, (%s *)val);') % (type['key'], type['ctype']),
            '}',
            ''
            ])
        __generators = [__from, __to, __get_val]
        __write(path + ".c", merge([
            merge(__header),
            '#include "%s.h"' % os.path.basename(path),
            '#include "ngx_shm_dict_reader.h"',
            '#include "ngx_shm_dict_writer.h"',
            '',
            merge(map(lambda generator: merge(map(lambda type: generator(type), types)), __generators)),
            merge(map(lambda type: read['imp'](
                __get_read_dec(read, type), type), types)),
            merge(map(lambda method: merge(
                map(lambda type: method['imp'](__get_write_dec(method, type), type), types)
                ), map(lambda method: write(method), write_methods))),
            merge(map(lambda type: gen_atomic_imp('incr')(type), num_types)),
            merge(map(lambda type: gen_atomic_imp('decr')(type), num_types))
            ]))
    num_types = filter(lambda type: 'atomic' in type, types)
    __gen_interface(path, types, num_types, read, write, write_methods)
    __gen_implement(path, types, num_types, read, write, write_methods)

def get_path(argv):
    __path = '.' if 2 != len(argv) else argv[1]
    return os.path.join(os.path.abspath(__path), 'ngx_shm_dict_code')

if __name__ == "__main__":
    gen(get_path(sys.argv), 
        ngx_shm_dict_types, ngx_shm_dict_read, ngx_shm_dict_write, ngx_shm_dict_write_methods)