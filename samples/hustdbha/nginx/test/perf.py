#/usr/bin/python
# author: jobs
# email: yao050421103@163.com
import sys
import os
import time
import datetime
import json
import string
import requests
import shutil
from time import sleep
import multiprocessing
import multiprocessing.dummy

def manual(): 
    print """
    usage:
        python perf.py [action] [uri] [conns] [reqs] [mode] [option]

        [action]
            write: write data
            clean: clean data
            progress: show the realtime progress of test

        [conns]
            number of cocurrent connections
        
        [reqs]
            number of requests per connection

        [mode]
            -d: run as debug mode
            -s: run as service mode (daemon)
        
        [option]
            -e: execute [action]
            -v: verify the result of [action]
            
        NOTE: if [action] is "progress", only [conns] is required.
        
    sample:
        python perf.py write localhost:8082 10 1000 -d -e
        python perf.py write localhost:8082 10 1000 -d -v
        python perf.py clean localhost:8082 10 1000 -d -e
        python perf.py clean localhost:8082 10 1000 -d -v
        
        python perf.py write localhost:8082 10 1000 -s -e
        python perf.py write localhost:8082 10 1000 -s -v
        python perf.py clean localhost:8082 10 1000 -s -e
        python perf.py clean localhost:8082 10 1000 -s -v
        
        python perf.py progress 10
        """

AUTH = ('huststore', 'huststore')
MAX_REQ_PER_TASK = 5000

gen_hash_tb = lambda i : 'perf_hash_tb_%d' % (i % 10)
gen_set_tb = lambda i : 'perf_hash_set_%d' % (i % 10)
gen_zset_tb = lambda i : 'perf_hash_zset_%d' % (i % 10)
gen_key = lambda i : 'perf_key_%d' % i
gen_val = lambda i : 'perf_val_%d' % i
get_status_uri = lambda cwd, i: os.path.join(cwd, 'task_%d.status' % i)

def daemonize():
    pid = os.fork()
    if pid > 0:
        sys.exit(0) # exit first parent
    # decouple from parent environment
    os.chdir("/")
    os.setsid()
    os.umask(0)
    pid = os.fork()
    if pid > 0:
        print "Daemon PID %d" % pid
        sys.exit(0) # exit from second parent, print eventual PID before
    for f in sys.stdout, sys.stderr:
        f.flush()
    si = file('/dev/null', 'r')
    so = file('/dev/null', 'a+')
    se = file('/dev/null', 'a+', 0)
    os.dup2(si.fileno(), sys.stdin.fileno())
    os.dup2(so.fileno(), sys.stdout.fileno())
    os.dup2(se.fileno(), sys.stderr.fileno())

def http_post(sess, cmd, body):
    return sess.post(cmd, body, headers = {'content-type':'text/plain'}, auth=AUTH)
def http_get(sess, cmd):
    return sess.get(cmd, auth=AUTH)

def log(debug, uri, data):
    if debug:
        print data
        return
    with open(uri, 'a+') as f:
        f.write('%s\n' % data)

def log_result(debug, err_uri, r, cmd):
    if 200 != r.status_code:
        log(debug, err_uri, '%s: %d' % (cmd, r.status_code))
        return False
    return True

def post_base(debug, err_uri, sess, cmd, body):
    return log_result(debug, err_uri, http_post(sess, cmd, body), cmd)
def get_base(debug, err_uri, sess, cmd):
    return log_result(debug, err_uri, http_get(sess, cmd), cmd)

def put(debug, err_uri, sess, host, key, val):
    return post_base(debug, err_uri, sess, 'http://%s/put?key=%s' % (host, key), val)
def hset(debug, err_uri, sess, host, tb, key, val):
    return post_base(debug, err_uri, sess, 'http://%s/hset?tb=%s&key=%s' % (host, tb, key), val)
def sadd(debug, err_uri, sess, host, tb, key):
    return post_base(debug, err_uri, sess, 'http://%s/sadd?tb=%s' % (host, tb), key)
def zadd(debug, err_uri, sess, host, tb, key, score):
    return post_base(debug, err_uri, sess, 'http://%s/zadd?tb=%s&score=%d' % (host, tb, score), key)

def delete(debug, err_uri, sess, host, key, val):
    return get_base(debug, err_uri, sess, 'http://%s/del?key=%s' % (host, key))
def hdel(debug, err_uri, sess, host, tb, key, val):
    return get_base(debug, err_uri, sess, 'http://%s/hdel?tb=%s&key=%s' % (host, tb, key))
def srem(debug, err_uri, sess, host, tb, key):
    return post_base(debug, err_uri, sess, 'http://%s/srem?tb=%s' % (host, tb), key)
def zrem(debug, err_uri, sess, host, tb, key, score):
    return post_base(debug, err_uri, sess, 'http://%s/zrem?tb=%s' % (host, tb), key)

def get2_complete(debug, err_uri, r, cmd, val):
    info = {}
    if 'version1' in r.headers:
        info['Version1'] = r.headers['version1']
        info['Version2'] = r.headers['version2']
    if 200 == r.status_code:
        if r.content != val:
            info['code'] = r.status_code
            info['Expected'] = val
            info['Val'] = r.content
    elif 409 == r.status_code:
        info['code'] = r.status_code
        if 'val-offset' in r.headers:
            off = int(r.headers['val-offset'])
            info['Val-Offset'] = off
            info['Value1'] = r.content[:off]
            info['Value2'] = r.content[off:]
    else:
        info['code'] = r.status_code
    if 0 != len(info):
        log(debug, err_uri, '%s: %s' % (cmd, json.dumps(info)))
        return False
    return True
def exist2_complete(debug, err_uri, r, cmd):
    info = {}
    if 'version1' in r.headers:
        info['Version1'] = r.headers['version1']
        info['Version2'] = r.headers['version2']
    if 200 != r.status_code:
        info['code'] = r.status_code
    if 0 != len(info):
        log(debug, err_uri, '%s: %s' % (cmd, json.dumps(info)))
        return False
    return True
def get2(debug, err_uri, sess, host, key, val):
    cmd = 'http://%s/get2?key=%s' % (host, key)
    return get2_complete(debug, err_uri, http_get(sess, cmd), cmd, val)
def hget2(debug, err_uri, sess, host, tb, key, val):
    cmd = 'http://%s/hget2?tb=%s&key=%s' % (host, tb, key)
    return get2_complete(debug, err_uri, http_get(sess, cmd), cmd, val)
def sismember2(debug, err_uri, sess, host, tb, key):
    cmd = 'http://%s/sismember2?tb=%s' % (host, tb)
    return exist2_complete(debug, err_uri, http_post(sess, cmd, key), cmd)
def zscore2(debug, err_uri, sess, host, tb, key, score):
    cmd = 'http://%s/zscore2?tb=%s' % (host, tb)
    return get2_complete(debug, err_uri, http_post(sess, cmd, key), cmd, str(score))
        
def exist_base(debug, err_uri, sess, cmd):
    r = http_get(sess, cmd)
    if 200 == r.status_code:
        log(debug, err_uri, '%s: still exist' % cmd)
        return False
    return True
def ismember_base(debug, err_uri, sess, cmd, body):
    r = http_post(sess, cmd, body)
    if 200 == r.status_code:
        log(debug, err_uri, '%s: still exist, body: %s' % (cmd, body))
        return False
    return True

def exist(debug, err_uri, sess, host, key, val):
    return exist_base(debug, err_uri, sess, 'http://%s/exist?key=%s' % (host, key))
def hexist(debug, err_uri, sess, host, tb, key, val):
    return exist_base(debug, err_uri, sess, 'http://%s/hexist?tb=%s&key=%s' % (host, tb, key))
def sismember(debug, err_uri, sess, host, tb, key):
    return ismember_base(debug, err_uri, sess, 'http://%s/sismember?tb=%s' % (host, tb), key)
def zismember(debug, err_uri, sess, host, tb, key, score):
    return ismember_base(debug, err_uri, sess, 'http://%s/zismember?tb=%s' % (host, tb), key)

def write_status(status_uri, data):
    count = data['count'] 
    offset = data['offset']
    cost = data['cost']
    block = data['block']
    remaining_time = cost * (count - offset) / block
    progress = 100 if offset >= (count - 1) else int(offset * 100 / count)
    with open(status_uri, 'w') as f:
        json.dump({
            'start_pos': data['start_pos'],
            'offset': offset,
            'costed': data['costed'],
            'count': count,
            'progress': progress,
            'remaining_time': remaining_time,
            'speed': (block / cost) if 0 != cost else 0
            }, f, indent=4)
    return True

def access_db(debug, sess, host, i, functors, err_uri):
    key = gen_key(i)
    val = gen_val(i)
    if not functors['kv'](debug, err_uri, sess, host, key, val):
        return False
    if not functors['hash'](debug, err_uri, sess, host, gen_hash_tb(i), key, val):
        return False
    if not functors['set'](debug, err_uri, sess, host, gen_set_tb(i), key):
        return False
    if not functors['zset'](debug, err_uri, sess, host, gen_zset_tb(i), key, i % 99 + 1):
        return False
    return True

def perf_loop(data):
    debug = data['debug']
    host = data['host']
    index = data['index']
    start = data['start']
    end = data['end']
    cwd = data['cwd']
    functors = data['functors']
    keyword = data['key']
    sess = requests.Session()
    
    status_uri = get_status_uri(cwd, index)
    err_uri = os.path.join(cwd, 'task_%d.%s.err' % (index, keyword))
    
    costed = 0
    count = end - start
    offset = 0
    for i in xrange(start, end): 
        __start = time.time()
        if not access_db(debug, sess, host, i, functors, err_uri):
            break
        __end = time.time()
        offset = offset + 1
        cost = __end - __start
        costed = costed + cost
        write_status(status_uri, {
            'start_pos': start,
            'count': count,
            'offset': offset,
            'block': 1,
            'costed': costed,
            'cost': cost
            })
        if 0 == (offset % MAX_REQ_PER_TASK):
            sess.close()
            sess = requests.Session()

def perf_test(debug, cwd, host, processes, conns, reqs):
    def test(key, functors):
        def execute():
            if not debug:
                daemonize()
            pool = multiprocessing.Pool(processes=processes)
            pool.map(perf_loop, [{
                'index': i,
                'start': reqs * i,
                'end': reqs * (i + 1),
                'cwd': cwd,
                'debug': debug,
                'host': host,
                'functors': functors,
                'key': key
            } for i in xrange(conns)])
            pool.close()
            pool.join()
            return True
        return execute
    return test

def progress(cwd, tasks):
    KEYS = ['progress', 'costed', 'remaining_time', 'offset', 'count']
    def is_valid(progress, keys):
        if not progress:
            return False
        for key in keys:
            if not key in progress:
                return False
        return True
    def load_progress(file):
        with open(file, 'r') as f:
            try:
                return json.load(f)
            except:
                return None
        return None
    def format_time(tm):
        hours = int(tm / 3600)
        tm = tm % 3600
        minutes = int(tm / 60)
        tm = tm % 60
        seconds = int(tm)
        return '%d:%d:%d' % (hours, minutes, seconds)
    def load_stat_list(cwd, tasks):
        stat_list = []
        for i in xrange(tasks):
            status_uri = get_status_uri(cwd, i)
            if os.path.exists(status_uri):
                stat = load_progress(status_uri)
                if not is_valid(stat, KEYS):
                    return None
                stat_list.append(stat)
        return stat_list
    def merge_stat_list(stat_list):
        progress = None
        for stat in stat_list:
            if not progress:
                progress = stat
            else:
                if stat['progress'] < progress['progress']:
                    progress['progress'] = stat['progress']
                if stat['costed'] > progress['costed']:
                    progress['costed'] = stat['costed']
                if stat['remaining_time'] > progress['remaining_time']:
                    progress['remaining_time'] = stat['remaining_time']
                progress['offset'] += stat['offset']
                progress['count'] += stat['count']
                progress['speed'] += stat['speed']
        return progress
    def get_process(cwd, tasks):
        stat_list = load_stat_list(cwd, tasks)
        if None == stat_list:
            return (False, None)
        progress = merge_stat_list(stat_list)
        if not progress:
            return (False, None)
        return (True, progress)
    def summary():
        stat_list = load_stat_list(cwd, tasks)
        progress = merge_stat_list(stat_list)
        print 'costed_time: %s' % format_time(progress['costed'])
        print 'count: %d' % progress['count']
        print 'speed: %d (loops / second)' % (progress['speed'])
    def execute():
        while 1:
            (rc, progress) = get_process(cwd, tasks)
            if not rc:
                time.sleep(0.1)
                continue
            os.system('clear')
            print 'progress: %d%%' % progress['progress']
            print 'costed_time: %s' % format_time(progress['costed'])
            print 'remaining_time: %s' % format_time(progress['remaining_time'])
            print 'offset: %d' % progress['offset']
            print 'count: %d' % progress['count']
            print 'speed: %d (loops / second)' % (progress['speed'])
            if progress['progress'] >= 100:
                os.system('clear')
                break
            time.sleep(1)
        summary()
        return True
    return execute

def get_test_func(argv):
    cwd = os.path.dirname(os.path.abspath(argv[0]))
    args = len(argv)
    if 3 == args:
        (action, conns) = (argv[1], int(argv[2]))
        if 'progress' == action:
            return progress(cwd, conns)
    if 7 != args:
        return None
    (action, uri, conns, reqs, mode, option) = (argv[1], argv[2], int(argv[3]), int(argv[4]), argv[5], argv[6])
    if '-d' == mode:
        debug = True
    elif '-s' == mode:
        debug = False
    else:
        return None
    processes = (multiprocessing.cpu_count() / 2)
    test_func = perf_test(debug, cwd, uri, processes, conns, reqs)
    if 'write' == action:
        if '-e' == option:
            return test_func('write',  {'kv': put,    'hash': hset,   'set': sadd,       'zset': zadd})
        if '-v' == option:
            return test_func('vwrite', {'kv': get2,   'hash': hget2,  'set': sismember2, 'zset': zscore2})
    elif 'clean' == action:
        if '-e' == option:
            return test_func('clean',  {'kv': delete, 'hash': hdel,   'set': srem,       'zset': zrem})
        if '-v' == option:
            return test_func('vclean', {'kv': exist,  'hash': hexist, 'set': sismember,  'zset': zismember})
    return None

def run(argv):
    test_func = get_test_func(argv)
    if not test_func:
        return False
    return test_func()

if __name__ == "__main__":
    if not run(sys.argv):
        manual()