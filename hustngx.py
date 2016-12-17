#!/usr/bin/python
# email: yao050421103@gmail.com
import sys
import os.path
import datetime
import string
import json
import tarfile
import shutil

FILTER = '@null_string_place_holder'
join_lines = lambda lines, dim: string.join(filter(lambda item: -1 == item.find(FILTER), lines), dim)
merge = lambda lines: join_lines(lines, '\n')

gen_tm = lambda: datetime.datetime.now().strftime('%Y%m%d%H%M%S')
get_bin_uri = lambda name: os.path.join(name, name)

def manual(): 
    print """
    usage:
        python hustngx.py [nginx] [conf]
    sample:
        python hustngx.py nginx-1.10.0.tar.gz ngx_wizard/ngx_wizard.json
        """

def get_json_data(path):
    json_filter = lambda f: (lambda f, l: os.path.splitext(f)[1] in l)(f, ['.json'])
    if not json_filter(path):
        return None
    f = open(path, 'r')
    data = json.load(f)
    f.close()
    return data
def write_file(path, data):
    with open(path, 'w') as f:
        f.writelines(data)
def copy_file(src, des):
    try:
        shutil.copy(src, des)
        return True
    except (IOError, os.error) as e:
        return False
        
def untar_ngx(nginx_tar_gz, ngx_dir, mdpath):
    if not copy_file(nginx_tar_gz, mdpath):
        return False
    tar_name = os.path.join(mdpath, os.path.basename(nginx_tar_gz))
    t = tarfile.open(tar_name)
    t.extractall(path = mdpath)
    t.close()
    tmp_ngx_dir = os.path.join(mdpath, os.path.splitext(
        os.path.splitext(os.path.basename(nginx_tar_gz))[0])[0])
    if os.path.exists(ngx_dir):
        shutil.rmtree(ngx_dir)
    os.rename(tmp_ngx_dir, os.path.join(mdpath, 'nginx'))
    os.remove(tar_name)
    return True
def gen_nginx_json(ngx_dir, md):
    __health_check = merge([
        '        "health_check":',
        '        [',
        '            "check interval=5000 rise=1 fall=3 timeout=5000 type=http",',
        '            "check_http_send \\"GET /status.html HTTP/1.1\\\\r\\\\n\\\\r\\\\n\\"",',
        '            "check_http_expect_alive http_2xx"',
        '        ],'
        ])
    write_file(
        os.path.join(ngx_dir, 'conf/nginx.json'), merge([
        '{',
        '    "module": "%s",' % md,
        '    "worker_connections": 1048576,',
        '    "listen": 8080,',
        '    "keepalive_timeout": 540,',
        '    "keepalive": 32768,',
        '    "http_basic_auth_file": "/data/%s/conf/htpasswd",' % md,
        '    "auth_filter": ["test"],',
        '    "local_cmds": ["getconf", "test"],',
        '    "main_conf": ',
        '    [',
        '        ["test_num", 1],',
        '        ["test_size", "1m"],',
        '        ["test_time", "1ms"],',
        '        ["test_flag", "on"],',
        '        ["test_str", "test"]',
        '    ],',
        '    "proxy":',
        '    {',
        __health_check,
        '        "auth": "am9iczpqb2Jz",',
        '        "proxy_connect_timeout": "2s",',
        '        "proxy_send_timeout": "60s",',
        '        "proxy_read_timeout": "60s",',
        '        "proxy_buffer_size": "64m",',
        '        "backends": ["backend:8087"],',
        '        "proxy_cmds": ["/backend/test"]',
        '    }',
        '}'
        ]))
def sync(ngx_dir, md_path):
    json_data = [
        {
            "src":
            {
                "files":
                [
                    "lib_hustngx",
                    "patch",
                    "third_party"
                ],
                "suffix": "/nginx"
            },
            "des": 
            {
                "files": 
                [
                    ngx_dir
                ]
            }
        },
        {
            "src":
            {
                "files":
                [
                    "upgrade.sh",
                    "deploygen/cmds/deploy_ngx.json"
                ]
            },
            "des": 
            {
                "files": 
                [
                    md_path
                ]
            }
        }
    ]
    file = 'sync_%s.json' % gen_tm()
    with open(file, 'w') as f:
        json.dump(json_data, f)
    bin_uri = get_bin_uri("sync")
    cmd = ('python %s.py %s') % (bin_uri, file)
    os.system(cmd)
    os.remove(file)
def gen_config(ngx_dir, md):
    write_file(os.path.join(
        ngx_dir, 'Config.sh'), (
        './configure --with-cc-opt="-g3 -O0" --with-ld-opt="-lm" --prefix=/data/%s --add-module=src/addon'
        ) % md)
def gen_htpasswd(ngx_dir):
    write_file(os.path.join(
        ngx_dir, 'conf/htpasswd'), 'jobs:jobs\n')
def gen_deploy(md_path):
    bin_uri = get_bin_uri("deploygen")
    copy_file("%s.py" % bin_uri, md_path)
    hosts_path = os.path.join(md_path, 'hosts')
    write_file(hosts_path, '127.0.0.1')
    output = os.path.join(md_path, 'deploy.sh')
    json_path = os.path.join(md_path, 'deploy_ngx.json')
    
    tool_path = os.path.join(md_path, "deploygen")
    cmd = 'python %s.py search %s %s' % (tool_path, json_path, hosts_path)
    write_file(output, os.popen(cmd))
def apply_hustngx(ngx_dir, addon, md, md_path):
    sync(ngx_dir, md_path)
    shutil.move(addon, os.path.join(ngx_dir, 'src/'))
    gen_nginx_json(ngx_dir, md)
    gen_htpasswd(ngx_dir)
    gen_deploy(md_path)
    gen_config(ngx_dir, md)
def gen_addon(uri):
    obj = get_json_data(uri)
    if not obj:
        return None
    root = os.path.dirname(os.path.abspath(uri))
    mdpath = os.path.join(root, obj['module'])
    addon = os.path.join(mdpath, 'addon')
    bin_uri = get_bin_uri("ngx_wizard")
    cmd = 'python %s.py %s' % (bin_uri, uri)
    os.system(cmd)
    return { 'md': obj['module'], 'mdpath': mdpath, 'addon': addon }
def gen_hustngx(nginx_tar_gz, uri):
    obj = gen_addon(uri)
    if not obj:
        return False
    ngx_dir = os.path.join(obj['mdpath'], 'nginx')
    nginx_tar_gz = os.path.abspath(nginx_tar_gz)
    if not untar_ngx(nginx_tar_gz, ngx_dir, obj['mdpath']):
        return False
    apply_hustngx(ngx_dir, obj['addon'], obj['md'], obj['mdpath'])
    return True
                
parse_shell = lambda argv: gen_hustngx(argv[1], argv[2]) if 3 == len(argv) else False

if __name__ == "__main__":
    if not parse_shell(sys.argv):
        manual()