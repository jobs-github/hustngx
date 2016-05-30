#!/usr/bin/python
#author: jobs
#email: yao050421103@gmail.com
import sys
import os.path
import datetime
import string
import json

TYPE = 0
NAME = 1
FILTER = '@null_string_place_holder'
type_dict = {
    'json_int_t': 'integer',
    'double': 'double',
    'json_str_t': 'string'
    }
get_type = (lambda d: lambda k: d[k] if k in d else k)(type_dict)
json_filter = lambda f: (lambda f, l: os.path.splitext(f)[1] in l)(f, ['.json'])
headup = lambda s: ''.join([s[0].upper(), s[1:]])
merge = lambda l: string.join(filter(lambda item: FILTER != item, l), '\n')
make_end = lambda b: ';' if b else ''
spaces = lambda n: string.join([' ' for i in xrange(0, n)], '')
tabs = [spaces(0), spaces(4), spaces(8), spaces(12), spaces(16)]
tag = { 'ser': 'serialize', 'deser': 'deserialize' }

def manual(): 
    print """
    usage:
        python jsoncgen.py [lib] [path] filelist
            [lib]
                -j: use jansson as json library, 
                    otherwise use cjson as json library
                    NOTE: cjson will be used as default value
            [path]
                -f: use file in filelist
                -p: use dir in filelist, will be parsed recursive
                
    sample:
        python jsoncgen.py -f file1.json file2.json file3.json
        python jsoncgen.py -p ./dir1/ ./dir2/ ./dir3/
        python jsoncgen.py -j -f file1.json file2.json file3.json
        python jsoncgen.py -j -p ./dir1/ ./dir2/ ./dir3/
        """

def get_file_list(cur_dir, path_filter):
    def __get(cur_dir, path_filter, file_list):
        for root, dirs, files in os.walk(cur_dir): 
            for f in files:
                if path_filter(f):
                    file_list.append(os.path.join(root, f))
    file_list = []
    __get(cur_dir, path_filter, file_list)
    return file_list

def get_files(file_filter, op, urls):
    if '-f' == op:
        return filter(file_filter, urls)
    elif '-p' == op:
        file_set = set()
        for url in urls:
            for item in get_file_list(url, file_filter):
                if not item in file_set:
                    file_set.add(item)
        return list(file_set)
    return []
    
class MethodGenBase:
    def __init__(self, lib, ns):
        self.headup = lambda n: headup(get_type(n))
        self.lower = lambda n: get_type(n).lower()
        self.gen_base = lambda f: lambda b: lambda n, s: f % (ns, self.lower(n), s.lower(), n if n in type_dict and '' == s else self.headup(n), s, make_end(b))
        self.gen_imp_base = lambda arr_str, gen_dec, gen_imp: lambda o: merge([
            gen_dec(False)(o['name'], arr_str),
            '{',
            gen_imp(o),
            '}',
            ''
            ])
        self.gen_save_base = lambda arr_str, save_str, ret_str: lambda o: merge([
            '%sjson_t * json_val = %sserialize_%s%s(obj_val);' % (tabs[1], ns, self.lower(o['name']), arr_str.lower()),
            '%sif (!obj_val)' % tabs[1],
            '%s{' % tabs[1],
            '%sreturn %s;' % (tabs[2], ret_str),
            '%s}' % tabs[1],
            save_str,
            '%s%s_dispose_json_value(json_val);' % (tabs[1], lib),
            '%sreturn result;' % tabs[1]
            ])
        self.gen_load_base = lambda arr_str, load_str: lambda o: merge([
            '%sjson_t * json_val = %s;' % (tabs[1], load_str),
            '%sif (!json_val)' % tabs[1],
            '%s{' % tabs[1],
            '%sreturn false;' % tabs[2],
            '%s}' % tabs[1],
            '%sjson_bool_t result = %sdeserialize_%s%s(json_val, obj_val);' % (tabs[1], ns, self.lower(o['name']), arr_str.lower()),
            '%s%s_dispose_json_value(json_val);' % (tabs[1], lib),
            '%sreturn result;' % tabs[1]
            ])
        self.gen_pair_imp = lambda gen_dec, gen_item: self.gen_imp_base('', gen_dec, lambda o: merge([
            '%sif (!src || !des)' % tabs[1],
            '%s{' % tabs[1],
            '%sreturn false;' % tabs[2],
            '%s}' % tabs[1],
            '%sdo' % tabs[1],
            '%s{' % tabs[1],
            merge(map(gen_item, o['members'])),
            '',
            '%sreturn true;' % tabs[2],
            '%s} while(0);' % tabs[1],
            '',
            '%sreturn false;' % tabs[1]
            ]))

class SetGen(MethodGenBase):
    def __init__(self, lib, ns):
        MethodGenBase.__init__(self, lib, ns)
        self.gen_base = lambda f: lambda b: lambda n, s: f % (ns, self.lower(n), s.lower(), self.headup(n), s, self.headup(n), s, make_end(b))
        self.gen_dec = self.gen_base('json_bool_t %sset_%s%s(const %s%s * src, %s%s * des)%s')
        self.__gen_item = lambda m: '%sif (!%sset_%s(&src->%s, &des->%s)) break;' % (tabs[2], ns, self.lower(m[TYPE]), m[NAME], m[NAME])
        self.gen_imp = self.gen_pair_imp(self.gen_dec, self.__gen_item)
        self.gen_arr_imp = self.gen_imp_base('Array', self.gen_dec, lambda o: merge([
            '%sif(!src || !src->arr || !des)' % tabs[1],
            '%s{' % tabs[1],
            '%sreturn false;' % tabs[2],
            '%s}' % tabs[1],
            '%sdes->size = src->size;' % tabs[1],
            '%sif (des->size < 1)' % tabs[1],
            '%s{' % tabs[1],
            '%sdes->arr = NULL;' % tabs[2],
            '%sreturn true;' % tabs[2],
            '%s}' % tabs[1],
            '%sdes->arr = json_malloc(sizeof(%s) * des->size);' % (tabs[1], o['name']),
            '%smemset(des->arr, 0, sizeof(%s) * des->size);' % (tabs[1], o['name']),
            '%ssize_t index;' % tabs[1],
            '%sfor (index = 0; index < des->size; ++index)' % tabs[1],
            '%s{' % tabs[1],
            '%sif (!%sset_%s(src->arr + index, des->arr + index))' % (tabs[2], ns, self.lower(o['name'])),
            '%s{' % (tabs[2]),
            '%s%s_dispose_%sarray(des);' % (tabs[3], lib, self.lower(o['name'])),
            '%sreturn false;' % tabs[3],
            '%s}' % (tabs[2]),
            '%s}' % tabs[1],
            '%sreturn true;' % tabs[1],
            ]))

class EqGen(MethodGenBase):
    def __init__(self, lib, ns):
        MethodGenBase.__init__(self, lib, ns)
        self.gen_base = lambda f: lambda b: lambda n, s: f % (ns, self.lower(n), s.lower(), self.headup(n), s, self.headup(n), s, make_end(b))
        self.gen_dec = self.gen_base('json_bool_t %seq_%s%s(const %s%s * src, %s%s * des)%s')
        self.__gen_item = lambda m: '%sif (!%seq_%s(&src->%s, &des->%s)) break;' % (tabs[2], ns, self.lower(m[TYPE]), m[NAME], m[NAME])
        self.gen_imp = self.gen_pair_imp(self.gen_dec, self.__gen_item)
        self.gen_arr_imp = self.gen_imp_base('Array', self.gen_dec, lambda o: merge([
            '%sif(!src || !src->arr || !des || !des->arr || (src->size != des->size))' % tabs[1],
            '%s{' % tabs[1],
            '%sreturn false;' % tabs[2],
            '%s}' % tabs[1],
            '%ssize_t index;' % tabs[1],
            '%sfor (index = 0; index < src->size; ++index)' % tabs[1],
            '%s{' % tabs[1],
            '%sif (!%seq_%s(src->arr + index, des->arr + index))' % (tabs[2], ns, self.lower(o['name'])),
            '%s{' % (tabs[2]),
            '%sreturn false;' % tabs[3],
            '%s}' % (tabs[2]),
            '%s}' % tabs[1],
            '%sreturn true;' % tabs[1],
            ]))

class DisposeGen(MethodGenBase):
    def __init__(self, lib, ns):
        MethodGenBase.__init__(self, lib, ns)
        self.gen_dec = self.gen_base('void %sdispose_%s%s(%s%s * obj_val)%s')
        self.__gen_item = lambda m: '%s%sdispose_%s(&obj_val->%s);' % (tabs[2], ns, get_type(m[TYPE]).lower(), m[NAME])
        self.gen_imp = self.gen_imp_base('', self.gen_dec, lambda o: merge([
            '%sif (obj_val)' % tabs[1],
            '%s{' % tabs[1],
            merge(map(self.__gen_item, o['members'])),
            '%s}' % tabs[1],
            ]))
        self.gen_arr_imp = self.gen_imp_base('Array', self.gen_dec, lambda o: merge([
            '%sif (!obj_val || !obj_val->arr)' % tabs[1],
            '%s{' % tabs[1],
            '%sreturn;' % tabs[2],
            '%s}' % tabs[1],
            '%ssize_t index;' % tabs[1],
            '%sfor (index = 0; index < obj_val->size; ++index)' % tabs[1],
            '%s{' % tabs[1],
            '%s%sdispose_%s(&obj_val->arr[index]);' % (tabs[2], ns, self.lower(o['name'])),
            '%s}' % tabs[1],
            '%sjson_free(obj_val->arr);' % tabs[1],
            '%sobj_val->arr = NULL;' % tabs[1],
            '%sobj_val->size = 0;' % tabs[1]
            ]))

class SerializeGen(MethodGenBase):
    def __init__(self, lib, ns):
        MethodGenBase.__init__(self, lib, ns)
        self.gen_dec = self.gen_base('json_t * %sserialize_%s%s(const %s%s * obj_val)%s')
        self.__gen_ser_fields = lambda l: merge(map(lambda m: '%sif (!json_object_set_field(json_val, "%s", %sserialize_%s(&obj_val->%s))) break;' % (
            tabs[2], m[NAME], ns, self.lower(m[TYPE]), m[NAME]), l))
        self.gen_imp = self.gen_imp_base('', self.gen_dec, lambda o: merge([
            '%sif (!obj_val)' % tabs[1],
            '%s{' % tabs[1],
            '%sreturn NULL;' % (tabs[2]),
            '%s}' % tabs[1],
            '',
            '%sjson_t * json_val = json_object();' % tabs[1],
            '%sdo' % tabs[1],
            '%s{' % tabs[1],
            self.__gen_ser_fields(o['members']),
            '',
            '%sreturn json_val;' % tabs[2],
            '%s} while (0);' % tabs[1],
            '',
            '%s%s_dispose_json_value(json_val);' % (tabs[1], lib),
            '%sreturn NULL;' % tabs[1]
            ]) if 'members' in o else FILTER)
        self.gen_arr_imp = self.gen_imp_base('Array', self.gen_dec, lambda o: merge([
            '%sif (!obj_val || !obj_val->arr)' % tabs[1],
            '%s{' % tabs[1],
            '%sreturn NULL;' % tabs[2],
            '%s}' % tabs[1],
            '%sjson_t * json_val = json_array();' % tabs[1],
            '%ssize_t index = 0;' % tabs[1],
            '%sfor (index = 0; index < obj_val->size; ++index)' % tabs[1],
            '%s{' % tabs[1],
            '%sjson_t * item = %sserialize_%s(&obj_val->arr[index]);' % (tabs[2], ns, self.lower(o['name'])),
            '%sif (!item)' % tabs[2],
            '%s{' % tabs[2],
            '%s%s_dispose_json_value(json_val);' % (tabs[3], lib),
            '%sreturn NULL;' % tabs[3],
            '%s}' % tabs[2],
            '%sif (!json_array_append_item(json_val, item))' % tabs[2],
            '%s{' % tabs[2],
            '%s%s_dispose_json_value(item);' % (tabs[3], lib),
            '%sreturn NULL;' % tabs[3],
            '%s}' % tabs[2],
            '%s}' % tabs[1],
            '%sreturn json_val;' % tabs[1]
            ]))

class DeserializeGen(MethodGenBase):
    def __init__(self, lib, ns):
        MethodGenBase.__init__(self, lib, ns)
        self.gen_dec = self.gen_base('json_bool_t %sdeserialize_%s%s(const json_t * json_val, %s%s * obj_val)%s')
        self.__gen_deser_fields = lambda l: merge(map(
            lambda m: merge([
                '%stmp_json_val = json_object_get_field(json_val, "%s");' % (tabs[2], m[NAME]),
                '%sobj_val->json_has_%s = !!tmp_json_val;' % (tabs[2], m[NAME]),
                '%sif (tmp_json_val && !%sdeserialize_%s(tmp_json_val, &obj_val->%s)) break;' % (tabs[2], ns, self.lower(m[TYPE]), m[NAME])
                ]), l))
        self.gen_imp = self.gen_imp_base('', self.gen_dec, lambda o: merge([
            '%sif (!obj_val || !json_is_object(json_val))' % tabs[1],
            '%s{' % tabs[1],
            '%sreturn false;' % (tabs[2]),
            '%s}' % tabs[1],
            '',
            '%sjson_t * tmp_json_val = NULL;' % tabs[1],
            '%sdo' % tabs[1],
            '%s{' % tabs[1],
            self.__gen_deser_fields(o['members']),
            '',
            '%sreturn true;' % tabs[2],
            '%s} while (0);' % tabs[1],
            '',
            '%sreturn false;' % tabs[1]
            ]) if 'members' in o else FILTER)
        self.gen_arr_imp = self.gen_imp_base('Array', self.gen_dec, lambda o: merge([
            '%sif (!json_is_array(json_val) || !obj_val)' % tabs[1],
            '%s{' % tabs[1],
            '%sreturn false;' % tabs[2],
            '%s}' % tabs[1],
            '%sobj_val->size = json_array_size(json_val);' % tabs[1],
            '%sif (obj_val->size < 1)' % tabs[1],
            '%s{' % tabs[1],
            '%sobj_val->arr = NULL;' % tabs[2],
            '%sreturn true;' % tabs[2],
            '%s}' % tabs[1],
            '%sobj_val->arr = json_malloc(sizeof(%s) * obj_val->size);' % (tabs[1], o['name']),
            '%smemset(obj_val->arr, 0, sizeof(%s) * obj_val->size);' % (tabs[1], o['name']),
            '%ssize_t index;' % tabs[1],
            '%sfor (index = 0; index < obj_val->size; ++index)' % tabs[1],
            '%s{' % tabs[1],
            '%sjson_t * tmp_json_val = json_array_get_item(json_val, index);' % tabs[2],
            '%sif (!%sdeserialize_%s(tmp_json_val, &obj_val->arr[index]))' % (tabs[2], ns, self.lower(o['name'])),
            '%s{' % tabs[2],
            '%s%s_dispose_%sarray(obj_val);' % (tabs[3], lib, self.lower(o['name'])),
            '%sreturn false;' % tabs[3],
            '%s}' % tabs[2],
            '%s}' % tabs[1],
            '%sreturn true;' % tabs[1]
            ]))

class DumpGen(MethodGenBase):
    def __init__(self, lib, ns):
        MethodGenBase.__init__(self, lib, ns)
        self.gen_dec = self.gen_base('char * %sdump_%s%s(const %s%s * obj_val)%s')
        self.__gen_imp = lambda s: self.gen_imp_base(s, self.gen_dec, self.gen_save_base(
            s, '%schar * result = json_dump_to_str(json_val);' % tabs[1], 'NULL'))
        self.gen_imp = self.__gen_imp('')
        self.gen_arr_imp = self.__gen_imp('Array')

class LoadsGen(MethodGenBase):
    def __init__(self, lib, ns):
        MethodGenBase.__init__(self, lib, ns)
        self.gen_dec = self.gen_base('json_bool_t %sload_%s%s(const char * input, %s%s * obj_val)%s')
        self.__gen_imp = lambda s: self.gen_imp_base(s, self.gen_dec, self.gen_load_base(
            s, 'json_load_from_str(input)'))
        self.gen_imp = self.__gen_imp('')
        self.gen_arr_imp = self.__gen_imp('Array')

class SaveGen(MethodGenBase):
    def __init__(self, lib, ns):
        MethodGenBase.__init__(self, lib, ns)
        self.gen_dec = self.gen_base('json_bool_t %ssave_%s%s_to_file(const %s%s * obj_val, const char * path)%s')
        self.__gen_imp = lambda s: self.gen_imp_base(s, self.gen_dec, self.gen_save_base(
            s, '%sjson_bool_t result = json_dump_to_file(json_val, path);' % tabs[1], 'false'))
        self.gen_imp = self.__gen_imp('')
        self.gen_arr_imp = self.__gen_imp('Array')

class LoadGen(MethodGenBase):
    def __init__(self, lib, ns):
        MethodGenBase.__init__(self, lib, ns)
        self.gen_dec = self.gen_base('json_bool_t %sload_%s%s_from_file(const char * path, %s%s * obj_val)%s')
        self.__gen_imp = lambda s: self.gen_imp_base(s, self.gen_dec, self.gen_load_base(
            s, 'json_load_from_file(path)'))
        self.gen_imp = self.__gen_imp('')
        self.gen_arr_imp = self.__gen_imp('Array')
    
class MethodGen:
    def __init__(self, lib, ns):
        self.__set_gen = SetGen(lib, ns)
        self.__eq_gen = EqGen(lib, ns)
        self.__dispose_gen = DisposeGen(lib, ns)
        self.__serialize_gen = SerializeGen(lib, ns)
        self.__deserialize_gen = DeserializeGen(lib, ns)
        self.__dump_gen = DumpGen(lib, ns)
        self.__loads_gen =  LoadsGen(lib, ns)
        self.__save_gen =  SaveGen(lib, ns)
        self.__load_gen =  LoadGen(lib, ns)
        self.__dict = {
            'set': self.__set_gen,
            'eq': self.__eq_gen,
            'dispose': self.__dispose_gen,
            tag['ser']: self.__serialize_gen,
            tag['deser']: self.__deserialize_gen,
            'dump': self.__dump_gen,
            'loads': self.__loads_gen,
            'save': self.__save_gen,
            'load': self.__load_gen
            }
        self.__check_arr = lambda k, o: k in o and len(o[k]) > 0
        self.__gen_interface = lambda o: lambda m: self.__dict[m].gen_dec(
            True)(o['name'], '')
        self.__gen_implement = lambda o: lambda m: self.__dict[m].gen_imp(
            o)
        self.gen_interface = lambda struct: merge(map(self.__gen_interface(struct), struct['methods']))
        self.gen_arr_interface = lambda struct: merge(map(
            lambda m: self.__dict[m].gen_dec(True)(struct['name'], 'Array'),
            struct['array_methods']
            ));
        self.gen_implement = lambda struct: merge(map(self.__gen_implement(struct), struct['methods']))
        self.gen_arr_implement = lambda struct: merge(map(lambda m: self.__dict[m].gen_arr_imp(struct), struct['array_methods']))

class JsoncGen():
    def __init__(self, lib, ns):
        self.__method_gen = MethodGen(lib, ns)
        self.__header = [
            '/**',
            '******************************************************************************',
            '* NOTE : Generated by jsoncgen. It is NOT supposed to modify this file.',
            '*****************************************************************************/'
            ]
        self.__gen_includes = (lambda k, e: lambda o: merge([
            merge(map(lambda s: '#include %s' % s, o[k])) if k in o else FILTER,
            FILTER if k in o and e in o[k] else '#include "%s_serialization.h"' % lib
            ])) ('includes', '"%s_serialization_base.h"' % lib)
        self.__gen_base = lambda f: lambda l: merge(map(f, l))
        self.__check_field = lambda k, o: k in o and len(o[k]) > 0
        __enable_json = lambda struct: tag['deser'] in struct['methods']
        self.__gen_struct = lambda o: merge([
            'typedef struct',
            '{',
            merge(map(lambda l: '%s%s %s;' % (tabs[1], l[TYPE], l[NAME]), o['members'])
                + (map(lambda l: '%sjson_bool_t json_has_%s;' % (
                    tabs[1], l[NAME]), o['members']) if __enable_json(o) else [])
                ),
            '} %s;' % o['name'],
            ''
            ])
        self.__gen_arr = lambda o: merge([
            'typedef struct',
            '{',
            '%s%s * arr;' % (tabs[1], o['name']),
            '%ssize_t size;' % tabs[1],
            '} %sArray;' % headup(get_type(o['name'])),
            ''
            ])
        self.__gen_obj_dec = lambda f, k, o: f(o) if self.__check_field(k, o) else FILTER
        self.__double_check = lambda k1, k2, o: self.__check_field(k1, o) and self.__check_field(k2, o)
        self.__merge_gen = lambda s: merge(['', s]) if '' != s else s
        self.__gen_methods = lambda s1, s2: [
            s1,'',s2,''] if '' != s1 and '' != s2 else [
                s2,''] if '' == s1 and '' != s2 else [
                    s1, ''] if '' != s1 and '' == s2 else []
        self.__gen_declare = self.__gen_base(
            lambda o: merge([
                self.__gen_obj_dec(self.__gen_struct, 'members', o),
                self.__gen_obj_dec(self.__gen_arr, 'array_methods', o),
                (lambda s1, s2: merge(self.__gen_methods(s1, s2))) (
                    self.__method_gen.gen_interface(o) if self.__double_check('methods', 'members',o) else FILTER,
                    self.__gen_obj_dec(self.__method_gen.gen_arr_interface, 'array_methods', o)
                    )
                ]))
        self.__gen_methods_imp = self.__gen_base(
            lambda o: merge([
                self.__method_gen.gen_implement(o) if self.__double_check('methods', 'members',o) else FILTER,
                self.__gen_obj_dec(self.__method_gen.gen_arr_implement, 'array_methods', o)
                ]))
    def gen(self, path, obj):
        self.__gen_interface(path, obj)
        self.__gen_implement(path, obj)
    def __gen_interface(self, path, obj):
        tm = datetime.datetime.now().strftime('%Y%m%d%H%M%S')
        head_def = '__%s_%s_h__' % (os.path.basename(path).lower(), tm)
        self.__write(path + ".h", merge([
            merge(self.__header),
            '#ifndef %s' % head_def,
            '#define %s' % head_def,
            '',
            self.__gen_includes(obj),
            '',
            self.__gen_declare(obj['structs']),
            '',
            '#endif // %s' % head_def
            ]))
    def __gen_implement(self, path, obj):
        self.__write(path + ".c", merge([
            merge(self.__header),
            '#include "%s.h"' % os.path.basename(path),
            '',
            self.__gen_methods_imp(obj['structs'])
            ]))
    def __write(self, url, data):
        with open(url, 'w') as f:
            f.writelines(data)
        
def gen(file_list, lib, ns):
    generator = JsoncGen(lib, ns)
    for path_item in file_list:
        with open(path_item, 'r') as f:
            obj = json.load(f)
            generator.gen(path_item[:path_item.rfind('.')], obj)
    return True

def parse_shell(argv):
    size = len(argv)
    if size < 3:
        return False
    op = argv[2] if '-j' == argv[1] else argv[1]
    lib = 'jansson' if '-j' == argv[1] else 'cjson'
    file_list = get_files(json_filter, op, argv[3:] if '-j' == argv[1] else argv[2:])
    return gen(file_list, lib, '%s_' % lib) if len(file_list) > 0 else False

if __name__ == "__main__":
    if not parse_shell(sys.argv):
        manual()