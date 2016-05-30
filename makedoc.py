#!/usr/bin/python
# email: yao050421103@gmail.com
import sys
import os.path
import string
import shutil
    
def make():
    src = 'doc/'
    des = 'releases/doc/'
    if os.path.exists(des):
        shutil.rmtree(des)
    os.system('generate-md --layout github --input %s --output %s' % (src, des))
    for file in ['development.vsd', 'maintenance.vsd']:
        os.remove('releases/doc/process/%s' % file)
    print 'make doc success!'

if __name__ == "__main__":
    make()