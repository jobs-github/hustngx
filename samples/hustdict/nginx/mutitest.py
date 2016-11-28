#/usr/bin/python
import sys
import os
import time
import datetime
import json
import string
import requests
import random
import multiprocessing
import multiprocessing.dummy

HOST = 'http://localhost:8085'
AUTH= ('jobs', 'jobs')
gentm = lambda: datetime.datetime.now().strftime('[%Y-%m-%d %H:%M:%S] ')

def manual(): 
    print """
    usage:
        python mutitest [tasks]
    sample:
        python mutitest.py 2
        """
    
def log_err(str):
    print str
    with open('hustdict.log', 'a+') as f:
        f.writelines('%s%s\n' % (gentm(), str))

def gloop(arg, **kwarg):
    return MutiTester.loop(*arg, **kwarg)
def flush(sess, host):
    cmd = '%s/flush_all' % host
    r = sess.get(cmd, auth=AUTH)
    return r.status_code
def write(loop, func, host, method, key, type, delta):
    cmd = '%s/%s?key=%s&type=%s&delta=%s' % (host, method, key, type, delta)
    r = func(cmd, auth=AUTH)
    if 200 != r.status_code:
        log_err('loop %s {%s: %d}' % (loop, cmd, r.status_code))
class MutiTester:
    def __init__(self, number, host):
        self.__host = host
        self.__number = number * 2
        self.__key = 'co_test_key'
    def loop(self, ctx):
        sess = requests.Session()
        for i in xrange(65536):
            if 0 == i % 1000:
                print 'loop %d' % i
            write(i, sess.get, self.__host, ctx['method'], self.__key, ctx['type'], ctx['delta'])
    def __test_base(self, type, val, delta):
        sess = requests.Session()
        if 200 != flush(sess, self.__host):
            print 'flush fail'
            return
        cmd = '%s/set?method=set&key=%s&type=%s&val=%s' % (self.__host, self.__key, type, val)
        r = sess.get(cmd, auth=AUTH)
        if 200 != r.status_code:
            print '%s: %d' % (cmd, r.status_code)
            return
        task_ctxs = [{
            'method': 'incr' if 0 == i % 2 else 'decr',
            'type': type,
            'delta': delta
            } for i in range(self.__number)]
        pool = multiprocessing.Pool(processes=self.__number)
        pool.map(gloop, zip([self]*len(task_ctxs), task_ctxs))
        pool.close()
        pool.join()
        cmd = '%s/get?key=%s&type=%s' % (self.__host, self.__key, type)
        r = requests.get(cmd, auth=AUTH)
        if 200 == r.status_code:
            print r.content
    def run(self):
        self.__test_base('int64', '0', '1')
        self.__test_base('double', '1024.1024', '1.1024')

def test(argv):
    size = len(argv)
    if size < 2 or size > 3:
        return False
    number = int(argv[1])
    obj = MutiTester(number, HOST)
    obj.run()
    return True
                
if __name__ == "__main__":
    if not test(sys.argv):
        manual()