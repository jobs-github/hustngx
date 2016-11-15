#!/usr/bin/python
# email: yao050421103@gmail.com
import sys
import os
import string
import shutil

def init_hustmqha(argv):
    cwd = os.path.dirname(os.path.abspath(argv[0]))
    root = os.path.dirname(cwd)
    os.system('cd .. && python hustngx.py nginx-1.9.10.tar.gz sample/hustmqha.json')
    items = [
        'auto/sources',
        'conf/genhtpasswd.sh',
        'conf/htpasswd',
        'conf/htpasswd.py',
        'conf/nginx.conf', 
        'conf/nginx.json', 
        'src/addon',
        'test',
        'Config.sh'
        ]
    src_dir = os.path.join(root, 'hustmqha/nginx')
    dst_dir = os.path.join(root, 'sample/hustmq_ha/nginx')
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
    init_hustmqha(sys.argv)