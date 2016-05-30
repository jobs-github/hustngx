#!/usr/bin/python
# email: yao050421103@gmail.com
import sys
import os.path
import string
import json
import shutil
import platform

def manual(): 
    print """
    usage:
        python sync.py [conf]
        python sync.py [src_dir] [des_dir]
    sample:
        python sync.py sync.json
        python sync.py addon/ ../../nginx/src/addon/
        """
dim = '\\' if "Windows" == platform.system() else '/'
is_dim = lambda s: '\\' == s or '/' == s
first = lambda s: s[0] if len(s) > 0 else ''
last = lambda s: s[len(s) - 1] if len(s) > 0 else ''
move_last = lambda s: s[:(len(s) - 1)]
move_first = lambda s: s[1:]
def get_json_data(path):
    json_filter = lambda f: (lambda f, l: os.path.splitext(f)[1] in l)(f, ['.json'])
    if not json_filter(path):
        return None
    f = open(path, 'r')
    data = json.load(f)
    f.close()
    return data

def update_dir(s):
    while is_dim(last(s)):
        s = move_last(s)
    return s

def strip_dim(s):
    while is_dim(first(s)):
        s = move_first(s)
    return update_dir(s)
    
def copy_file(src, des):
    try:
        shutil.copy(src, des)
        return True
    except (IOError, os.error) as e:
        return False
    
def copy_dir(src, des):
    def __copy_dir(src, des):
        if not os.path.exists(src):
            print "%s not exist!" % (src)
            return
        if not os.path.exists(des):
            os.mkdir(des)
        for dir in os.listdir(src):
            srcname = os.path.join(src, dir)
            if os.path.isdir(srcname):
                __copy_dir(srcname, os.path.join(des, dir))
            else:
                copy_file(srcname, des)
        
    if not os.path.isdir(src):
        print '%s must be dir!' % src
        return False
    if not os.path.exists(des):
        os.mkdir(des)
    elif not os.path.isdir(des):
        print '%s must be dir!' % des
        return False
    src = update_dir(src)
    des = update_dir(des)

    __copy_dir(src, des)
    return True

def sync(data):
    __update_prefix = lambda s: '%s%s' % (strip_dim(s), dim)
    __update_suffix = lambda s: '%s%s' % (dim, strip_dim(s))
    __get_prefix = lambda o: __update_prefix(o['prefix']) if 'prefix' in o else ''
    __get_suffix = lambda o: __update_suffix(o['suffix']) if 'suffix' in o else ''
    for item in data:
        src = item['src']
        src_prefix = __get_prefix(src)
        src_suffix = __get_suffix(src)
        for src_file in src['files']:
            src_path = '%s%s%s' % (src_prefix, src_file, src_suffix)
            cp = copy_dir if os.path.isdir(src_path) else copy_file
            des = item['des']
            des_prefix = __get_prefix(des)
            des_suffix = __get_suffix(des)
            for des_file in des['files']:
                des_path = '%s%s%s' % (des_prefix, des_file, des_suffix)
                if not cp(src_path, des_path):
                    print 'copy fail { "src": "%s", "des": "%s" }' % (src_path, des_path)

def parse_shell(argv):
    size = len(argv)
    if 2 == size:
        data = get_json_data(argv[1])
        if not data:
            return False
        sync(data)
        return True
    elif 3 == size:
        copy_dir(argv[1], argv[2])
        return True
    return False

if __name__ == "__main__":
    if not parse_shell(sys.argv):
        manual()