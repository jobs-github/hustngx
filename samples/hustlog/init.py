#!/usr/bin/python
# email: yao050421103@gmail.com
import sys
import os
import string
import shutil

def init_hustlog(argv):
    cwd = os.path.dirname(os.path.abspath(argv[0]))
    root = os.path.dirname(os.path.dirname(cwd))
    os.system('cd ../../ && python hustngx.py nginx-1.10.0.tar.gz samples/schema/hustlog.json')
    items = [
        'auto/install',
        'auto/modules',
        'auto/sources',
        'conf/hustlog.conf',
        'conf/hustlog.conf.template',
        'conf/nginx.conf',
        'src/addon',
        'src/core/nginx.c',
        'src/core/ngx_config.h',
        'src/http/ngx_http_upstream_round_robin.c',
        'src/http/ngx_http_upstream_round_robin.c',
        'src/os/unix/ngx_process_cycle.c',
        'test',
        'Config.sh'
        ]
    src_dir = os.path.join(root, 'samples/hustlog/nginx')
    dst_dir = os.path.join(root, 'samples/schema/hustlog/nginx')
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
    init_hustlog(sys.argv)