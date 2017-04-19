#!/usr/bin/python
# email: yao050421103@gmail.com
import sys
import os
import string
import shutil

def init_hustdbha(argv):
    cwd = os.path.dirname(os.path.abspath(argv[0]))
    root = os.path.dirname(os.path.dirname(cwd))
    os.system('cd ../../ && python hustngx.py nginx-1.12.0.tar.gz samples/schema/hustdbha.json')
    items = [
        'auto/install',
        'auto/sources',
        'conf/gen_table.py',
        'conf/genhtpasswd.sh',
        'conf/htpasswd',
        'conf/htpasswd.py',
        'conf/hustdbtable.json',
        'conf/hustdbtable.json.decr',
        'conf/hustdbtable.json.incr',
        'conf/nginx.conf', 
        'conf/nginx.json', 
        'conf/private.pem', 
        'conf/public.pem', 
        'conf/sample.hustdbtable.json', 
        'conf/sample.hustdbtable.json.decr', 
        'conf/sample.hustdbtable.json.incr', 
        'conf/zlog.conf', 
        'src/addon',
        'src/core/nginx.c',
        'src/core/ngx_config.h',
        'src/os/unix/ngx_process_cycle.c',
        'test',
        'Config.sh'
        ]
    src_dir = os.path.join(root, 'samples/hustdbha/nginx')
    dst_dir = os.path.join(root, 'samples/schema/hustdb_ha/nginx')
    for item in items:
        src = os.path.join(src_dir, item)
        dst = os.path.join(dst_dir, item)
        if os.path.isdir(src):
            if os.path.exists(dst):
                shutil.rmtree(dst)
            shutil.copytree(src, dst)
        else:
            shutil.copy(src, dst)
    return True

if __name__ == "__main__":
    init_hustdbha(sys.argv)