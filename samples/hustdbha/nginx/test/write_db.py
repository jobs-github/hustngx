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
    usage sample:
        python write_db.py localhost:8082
        """

AUTH = ('huststore', 'huststore')
MAX_REQ_PER_TASK = 1000

gen_hash_tb = lambda i : 'migrate_hash_tb_%d' % (i % 10)
gen_set_tb = lambda i : 'migrate_hash_set_%d' % (i % 10)
gen_zset_tb = lambda i : 'migrate_hash_zset_%d' % (i % 10)
gen_key = lambda i : 'migrate_key_%d' % i
gen_val = lambda i : 'migrate_val_%d' % i

def put_kv(sess, host, key, val):
    cmd = 'http://%s/put?key=%s' % (host, key)
    r = sess.post(cmd, val, headers = {'content-type':'text/plain'}, auth=AUTH)
    if 200 != r.status_code:
        print '%s: %d' % (cmd, r.status_code)
def put_hash(sess, host, tb, key, val):
    cmd = 'http://%s/hset?tb=%s&key=%s' % (host, tb, key)
    r = sess.post(cmd, val, headers = {'content-type':'text/plain'}, auth=AUTH)
    if 200 != r.status_code:
        print '%s: %d' % (cmd, r.status_code)
def put_set(sess, host, tb, key):
    cmd = 'http://%s/sadd?tb=%s' % (host, tb)
    r = sess.post(cmd, key, headers = {'content-type':'text/plain'}, auth=AUTH)
    if 200 != r.status_code:
        print '%s: %d' % (cmd, r.status_code)
def put_zset(sess, host, tb, key, score):
    cmd = 'http://%s/zadd?tb=%s&score=%d' % (host, tb, score)
    r = sess.post(cmd, key, headers = {'content-type':'text/plain'}, auth=AUTH)
    if 200 != r.status_code:
        print '%s: %d' % (cmd, r.status_code)

def write_loop(data):
    host = data['host']
    index = data['index']
    start = data['start']
    end = data['end']
    sess = requests.Session()
    for i in xrange(start, end): 
        key = gen_key(i)
        val = gen_val(i)
        put_kv(sess, host, key, val)
        put_hash(sess, host, gen_hash_tb(i), key, val)
        put_set(sess, host, gen_set_tb(i), key)
        put_zset(sess, host, gen_zset_tb(i), key, i % 99 + 1)
    print 'task-%d done' % index
def write_db(host, processes, blocks):
    pool = multiprocessing.Pool(processes=processes)
    pool.map(write_loop, [{
        'index': i,
        'start': MAX_REQ_PER_TASK * i,
        'end': MAX_REQ_PER_TASK * (i + 1),
        'host': host
    } for i in xrange(blocks)])
    pool.close()
    pool.join()
    return True

def run(argv):
    if 2 != len(argv):
        return False
    processes = (multiprocessing.cpu_count() / 2)
    return write_db(argv[1], processes, 100)

if __name__ == "__main__":
    if not run(sys.argv):
        manual()