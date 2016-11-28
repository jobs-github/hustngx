#/usr/bin/python
import sys
import os
import os.path
import time
import datetime
import json
import string
import requests
import random

HOST = 'http://localhost:8085'
AUTH = ('jobs', 'jobs')

numerics = [
    ['int8', lambda i: 2**7-i], 
    ['uint8', lambda i: 2**8-i], 
    ['int16', lambda i: 2**15-i],
    ['uint16', lambda i: 2**16-i], 
    ['int32', lambda i: 2**31-i], 
    ['uint32', lambda i: 2**32-i], 
    ['int64', lambda i: 2**63-i], 
    ['uint64', lambda i: 2**64-i], 
    ['double', lambda i: (1.79769e+308) / (10**i)]
    ]
TYPE = 0
VAL = 1

methods = [
    ['set', lambda i, type: 'test_key_set_%s_%d' % (type, i)], 
    ['add', lambda i, type: 'test_key_add_%s_%d' % (type, i)], 
    ['safe_set', lambda i, type: 'test_key_safe_set_%s_%d' % (type, i)], 
    ['safe_add', lambda i, type: 'test_key_safe_add_%s_%d' % (type, i)],
    ['replace', lambda i, type: 'test_key_safe_set_%s_%d' % (type, i)]
    ]
METHOD = 0
KEY = 1
            
def manual(): 
    print """
    usage:
        python autotest [action] [file]
        [action]
            set
            add
            replace
            del
            flush_all
            flush_expired
            incr
            decr
            get
            keys
            get_keys
            loop
            bin
    example:
        python autotest set
        python autotest get
        python autotest del
        python autotest keys
        python autotest loop test.bin
        """

def fetch(func, cmd, nobody):
    r = func(cmd, auth=AUTH)
    print ('pass' if nobody else r.content) if 200 == r.status_code else '%s: %d' % (cmd, r.status_code)
def flush(sess, host):
    cmd = '%s/flush_all' % host
    r = sess.get(cmd, auth=AUTH)
    return r.status_code
def test_num():
    def __run_set(sess, host, method, type, key, val):
        cmd = '%s/set?method=%s&key=%s&type=%s&val=%s' % (
            host, method, key, type, str(val))
        r = sess.get(cmd, auth=AUTH)
        if 200 != r.status_code:
            print '%s: %d' % (cmd, r.status_code)
        return r.status_code
    def __run_get(sess, host, type, key, val):
        cmd = '%s/get?key=%s&type=%s' % (host, key, type)
        r = sess.get(cmd, auth=AUTH)
        if 200 == r.status_code:
            val_out = str(float(r.content)) if 'double' == type else r.content
            if val_out != str(val):
                print 'src: %s, dst: %s' % (str(val), r.content)
        else:
            print '%s: %d' % (cmd, r.status_code)
        return r.status_code
    def __run(loop, sess, host):
        print '[loop-%d][test_num]' % loop
        flush(sess, host)
        for method in methods:
            print '[loop-%d][test_num][method : %s]' % (loop, method[METHOD])
            for num in numerics:
                type = num[TYPE]
                for i in xrange(1, 128):
                    key = method[KEY](i, type)
                    val = num[VAL](i)
                    val_str = str(val)
                    rc = __run_set(sess, host, method[METHOD], type, key, val)
                    if 200 != rc:
                        break
                    rc = __run_get(sess, host, type, key, val)
                    if 200 != rc:
                        break;
    return __run
def test_str():
    def __run_set(sess, host, method, key, val):
        cmd = '%s/set?method=%s&key=%s&type=string&val=%s' % (
            host, method, key, val)
        r = sess.get(cmd, auth=AUTH)
        if 200 != r.status_code:
            print '%s: %d' % (cmd, r.status_code)
        return r.status_code
    def __run_get(sess, host, key, val):
        cmd = '%s/get?key=%s&type=string' % (host, key)
        r = sess.get(cmd, auth=AUTH)
        if 200 == r.status_code:
            if r.content != val:
                print 'src: %s, dst: %s' % (val, r.content)
        else:
            print '%s: %d' % (cmd, r.status_code)
        return r.status_code
    def __run(loop, sess, host):
        print '[loop-%d][test_str]' % loop
        flush(sess, host)
        for method in methods:
            print '[loop-%d][test_str][method : %s]' % (loop, method[METHOD])
            for i in xrange(1, 65536):
                if 0 == i % 1024:
                    print '[loop-%d][test_str][method : %s][subloop %d]' % (loop, method[METHOD], i)
                key = method[KEY](i, 'string')
                val = 'test_string_%d' % i
                rc = __run_set(sess, host, method[METHOD], key, val)
                if 200 != rc:
                    break
                rc = __run_get(sess, host, key, val)
                if 200 != rc:
                    break;
    return __run
def test_bin():
    def __run_set(sess, host, method, key, val):
        cmd = '%s/set?method=%s&key=%s&type=binary' % (
            host, method, key)
        r = sess.post(cmd, val, headers = {'content-type':'application/octet-stream'}, auth=AUTH)
        if 200 != r.status_code:
            print '%s: %d' % (cmd, r.status_code)
        return r.status_code
    def __run_get(sess, host, key, val):
        cmd = '%s/get?key=%s&type=binary' % (host, key)
        r = sess.get(cmd, auth=AUTH)
        if 200 == r.status_code:
            if r.content != val:
                print 'src: %s, dst: %s' % (val, r.content)
        else:
            print '%s: %d' % (cmd, r.status_code)
        return r.status_code
    def __test(sess, host, bin_dir):
        size = os.path.getsize(bin_dir)
        method = methods[0]
        with open(bin_dir, 'rb') as f:
            data = f.read(size)
            key = method[KEY](0, 'binary')
            val = data
            rc = __run_set(sess, host, method[METHOD], key, val)
            if 200 == rc:
                rc = __run_get(sess, host, key, val)
                if 200 == rc:
                    print 'pass'
    def __run(loop, sess, host, bin_dir):
        print '[loop-%d][test_bin]' % loop
        size = os.path.getsize(bin_dir)
        if size < 1:
            return
        flush(sess, host)
        for method in methods:
            print '[loop-%d][test_bin][method : %s]' % (loop, method[METHOD])
            with open(bin_dir, 'rb') as f:
                offset = 0
                count = 0
                for line in f:
                    block = len(line)
                    if 0 == count % 1000:
                        print '[loop-%d][test_bin][method : %s][offset: %d, block: %d]' % (loop, method[METHOD], offset, block)
                    key = method[KEY](offset, 'binary')
                    val = line
                    offset += block
                    count += 1
                    rc = __run_set(sess, host, method[METHOD], key, val)
                    if 200 != rc:
                        break
                    rc = __run_get(sess, host, key, val)
                    if 200 != rc:
                        break
    return { 'loop': __run, 'test': __test } 
class Tester:
    def __init__(self, log_dir, host, bin_dir):
        self.__sess = requests.Session()
        self.__log_dir = log_dir
        self.__bin_dir = bin_dir
        self.__host = host
        self.__blen = 1024 * 1024
        self.__host = host
        self.__test_num = test_num()
        self.__test_str = test_str()
        self.__test_bin = test_bin()
        self.dict = {
            'set': self.__set,
            'add': self.__add,
            'replace': self.__replace,
            'get': self.__get,
            'del': self.__del,
            'flush_all': self.__flush_all,
            'flush_expired': self.__flush_expired,
            'keys': self.__keys,
            'get_keys': self.__get_keys,
            'incr': self.__incr,
            'decr': self.__decr,
            'loop': self.__loop,
            'bin': self.__bin,
            }
    def __set(self):
        cmd = '%s/set?method=set&key=test_key&type=int8&val=1' % self.__host
        fetch(self.__sess.get, cmd, True)
    def __add(self):
        cmd = '%s/set?method=add&key=test_key_add&type=int8&val=255' % self.__host
        fetch(self.__sess.get, cmd, True)
    def __replace(self):
        cmd = '%s/set?method=replace&key=test_key&type=int8&val=254' % self.__host
        fetch(self.__sess.get, cmd, True)
    def __get(self):
        cmd = '%s/get?key=test_key&type=int8' % self.__host
        fetch(self.__sess.get, cmd, False)
    def __del(self):
        cmd = '%s/del?key=test_key' % self.__host
        fetch(self.__sess.get, cmd, True)
    def __flush_all(self):
        cmd = '%s/flush_all' % self.__host
        fetch(self.__sess.get, cmd, True)
    def __flush_expired(self):
        cmd = '%s/flush_expired?count=3' % self.__host
        fetch(self.__sess.get, cmd, True)
    def __keys(self):
        cmd = '%s/keys?all=true' % self.__host
        fetch(self.__sess.get, cmd, False)
    def __get_keys(self):
        cmd = '%s/get_keys?all=true' % self.__host
        fetch(self.__sess.get, cmd, False)
    def __incr(self):
        cmd = '%s/incr?key=test_key&type=int8&delta=1' % self.__host
        fetch(self.__sess.get, cmd, False)
    def __decr(self):
        cmd = '%s/decr?key=test_key&type=int8&delta=1' % self.__host
        fetch(self.__sess.get, cmd, False)
    def __loop(self):
        for i in xrange(1, 10000):
            self.__test_num(i, self.__sess, self.__host)
            self.__test_str(i, self.__sess, self.__host)
            self.__test_bin['loop'](i, self.__sess, self.__host, self.__bin_dir)
    def __bin(self):
        self.__test_bin['test'](self.__sess, self.__host, self.__bin_dir)
def test(argv):
    log_dir = os.path.join(os.path.abspath('.'), 'hustdict.log')
    size = len(argv)
    if size < 2 or size > 3:
        return False
    cmd = argv[1]
    bin_dir = argv[2] if 3 == size else 'test.bin'
    obj = Tester(log_dir, HOST, bin_dir)
    if cmd in obj.dict:
        obj.dict[cmd]()
        return True
    return False
                
if __name__ == "__main__":
    if not test(sys.argv):
        manual()