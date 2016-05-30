#!/usr/bin/python
# email: yao050421103@gmail.com
import sys
import platform
import os.path
import datetime
import string
import json
import shutil
import tarfile

merge = lambda l: string.join(l, '\n')

IS_WIN32 = ("Windows" == platform.system())
DIM = '\\' if IS_WIN32 else '/'

get_bin_uri = lambda name: "%s%s%s" % (name, DIM, name)
use_py = lambda bin_uri: os.path.exists("%s.py" % bin_uri)

SYNC_URI = get_bin_uri("sync")
sync_cmd = lambda src, des: ('python %s.py %s %s' if use_py(SYNC_URI) else '%s %s %s') % (SYNC_URI, src, des)

items = [
    { 'name': 'sync', 'path': 'sync/' },
    { 'name': 'deploygen', 'path': 'deploygen/' },
    { 'name': 'ngx_wizard', 'path': 'ngx_wizard/' },
    { 'name': 'jsoncgen', 'path': 'jsoncgen/' }
    ]

def __mkdir(dir):
    if os.path.exists(dir):
        shutil.rmtree(dir)
    os.mkdir(dir)
    
def post_make(root, sys_str):
    def __copy_dir(src, des):
        os.system(sync_cmd(src, des))
    def __copy_file(src, des):
        try:
            shutil.copy(src, des)
            return True
        except (IOError, os.error) as e:
            return False
    def __tar(src, des):
        tar = tarfile.open(des, "w:gz")
        for root, dir, files in os.walk(src):
            for file in files:
                tar.add(os.path.join(root, file))
        tar.close()
    for dir in ["lib_hustngx", "patch", "third_party"]:
        print 'copy %s...' % dir
        des = os.path.join(root, dir)
        __mkdir(des)
        __copy_dir(dir, des)
        
    __mkdir(os.path.join(root, 'deploygen/cmds'))
        
    for file in ["sample.json", "upgrade.sh", 'deploygen/cmds/deploy_ngx.json']:
        __copy_file(file, os.path.join(root, file))
    
    tm = datetime.datetime.now().strftime('%Y%m%d%H')
    tar_name = 'hustngx_%s_%s.tar.gz' % (sys_str, tm)
    print 'tar...'
    __tar('hustngx', tar_name)
    
    if not os.path.exists('releases'):
        os.mkdir('releases')
    shutil.move(tar_name, 'releases/%s' % tar_name)
    
    shutil.rmtree(root)
    print 'deploy success!'
    
def make_linux(py_ver):
    def __build(py_ver, name, suffix):
        os.popen('cython --embed -o %s.c %s.py' % (name, name))
        os.popen('gcc -Os `python%s-config --cflags` -o %s%s %s.c `python%s-config --ldflags`' % (py_ver, name, suffix, name, py_ver))
        os.popen('rm -f %s.c' % name)
    
    root = './hustngx/'
    __mkdir(root)

    print 'build hustngx...'
    __build(py_ver, 'hustngx', '.bin')
    shutil.move('hustngx.bin', os.path.join(root, 'hustngx'))
    
    for item in items:
        print 'build %s...' % item['name']
        __build(py_ver, '%s%s' % (item['path'], item['name']), '')
        des = os.path.join(root, item['path'])
        __mkdir(des)
        src = os.path.join(item['path'], item['name'])
        shutil.move(src, os.path.join(des, item['name']))
    
    post_make(root, 'linux')
    
def make_win32():
    def __gen_setup_py(name, includes):
        __gen_include_item = lambda item: '"%s", "%s.*"' % (item, item)
        __gen_includes = lambda includes: '[%s]' % (
            __gen_include_item('encodings')) if len(includes) < 1 else (
                '[\n%s\n%s]') % (reduce(lambda s1, s2: '%s,\n%s' % (s1, s2), map(
                    lambda item: '%s%s' % (
                        tabs[3], __gen_include_item(item)), ['encodings'] + includes
                    )), tabs[2])
        return merge([
            'from distutils.core import setup',
            'import py2exe',
            '',
            '__console = [\'%s.py\']' % name,
            '__options = {',
            '    "py2exe": {',
            '        "includes": %s,' % __gen_includes(includes),
            '        "dll_excludes": ["w9xpopen.exe"],',
            '        "compressed": 1,',
            '        "ascii": 1,',
            '        "bundle_files": 1',
            '    }',
            '}',
            '',
            'setup(',
            '    version = "1.0",',
            '    description = "%s",' % name,
            '    name = "jobs",',
            '    zipfile = None,',
            '    console=__console,',
            '    options = __options,',
            ')'
            ])
    def __gen_setup_bat():
        return merge([
            'python setup.py install',
            'python setup.py py2exe'
            ])
    def __build(cur_path, path, name):
        setup_py = os.path.join(path, 'setup.py')
        with open(setup_py, 'w') as f:
            f.writelines(__gen_setup_py(name, []))
        setup_bat = os.path.join(path, 'setup.bat')
        with open(setup_bat, 'w') as f:
            f.writelines(__gen_setup_bat())
        os.system('cd %s && setup.bat && cd %s' % (path, cur_path))
        os.remove(setup_py)
        os.remove(setup_bat)
            
    cur_path = os.path.abspath('.')
    
    root = 'hustngx/'
    __mkdir(root)
    
    print 'build hustngx...'
    __build(cur_path, '.', 'hustngx')
    shutil.move('dist/hustngx.exe', os.path.join(root, 'hustngx.exe'))
    shutil.rmtree('build')
    os.rmdir('dist')
    
    for item in items:
        print 'build %s...' % item['name']
        __build(cur_path, item['path'], item['name'])
        des = os.path.join(root, item['path'])
        __mkdir(des)
        src = os.path.join(item['path'], 'dist/%s.exe' % item['name'])
        shutil.move(src, os.path.join(des, '%s.exe' % item['name']))
        shutil.rmtree(os.path.join(item['path'], 'build'))
        os.rmdir(os.path.abspath(os.path.join(item['path'], 'dist')))
        
    post_make(root, 'win32')
    
def parse_shell(argv):
    if "Windows" == platform.system():
        make_win32()
    else:
        make_linux('2.6' if len(argv) < 2 else argv[1])

if __name__ == "__main__":
    parse_shell(sys.argv)