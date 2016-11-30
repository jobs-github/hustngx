#/usr/bin/python
import sys
import os
import time
import datetime
import string
import requests

def manual(): 
    print """
    usage:
        python autotest.py [host] [category] [level] [worker] [data]
    sample:
        python autotest.py localhost:8667
        python autotest.py localhost:8667 test_category 
        python autotest.py localhost:8667 test_category debug test_worker test_body
        """

def test(host, category, level, worker, body):
    sess = requests.session()
    cmd = 'http://%s/hustlog/post?category=%s&level=%s&worker=%s' % (host, category, level, worker)
    try:
        r = sess.post(cmd, body, headers={'content-type':'text/plain'}, timeout=5)
        if 200 != r.status_code:
            print '%s: %d' % (cmd, r.status_code)
        else:
            print 'ok'
    except requests.exceptions.RequestException as e:
        print 'error: %s: %s' % (cmd, str(e))
    return True

def run(argv):
    size = len(argv)
    if 2 == size:
        return test(argv[1], 'test_category', 'debug', 'test_worker', 'test_body')
    elif 3 == size:
        return test(argv[1], argv[2], 'debug', 'test_worker', 'test_body')
    elif 4 == size:
        return test(argv[1], argv[2], argv[3], 'test_worker', 'test_body')
    elif 5 == size:
        return test(argv[1], argv[2], argv[3], argv[4], 'test_body')
    elif 6 == size:
        return test(argv[1], argv[2], argv[3], argv[4], argv[5])
    return False

if __name__ == "__main__":
    if not run(sys.argv):
        manual()