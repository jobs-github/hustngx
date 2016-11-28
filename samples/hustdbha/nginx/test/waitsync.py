#/usr/bin/python
# author: jobs
# email: yao050421103@163.com
import sys
import os
import datetime
import time
import json
import string

gentm = lambda: datetime.datetime.now().strftime('[%Y-%m-%d %H:%M:%S] ')

def manual(): 
    print """
    usage:
        python waitsync.py [script] [uri] [conns] [reqs]

        [conns]
            number of cocurrent connections
        
        [reqs]
            number of requests per connection
        
    sample:
        python waitsync.py perf.py localhost:8082 10 1000
        """

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

def format_time(tm):
    hours = int(tm / 3600)
    tm = tm % 3600
    minutes = int(tm / 60)
    tm = tm % 60
    seconds = int(tm)
    return '%d:%d:%d' % (hours, minutes, seconds)

def write_log(uri, data):
    with open(uri, 'a+') as f:
        f.writelines('%s%s\n' % (gentm(), data))

def waitsync(cmd, log_uri):
    cost = 0
    __start = time.time()
    loop = 0
    while 1:
        _s = time.time()
        with os.popen(cmd) as f:
            data = f.read()
            _e = time.time()
            cost = _e - _s
            if len(data) != 0:
                write_log(log_uri, 'loop: %s\n\ncmd: %s\n\n%s\n' % (str(loop), cmd, data))
                loop = loop + 1
                time.sleep(cost if cost > 1 else 1)
                continue
            else:
                break
    __end = time.time()
    write_log(log_uri, '[sync complete]\n\nloop: %s\n\ncmd: %s\n\nstart:%s\nend:%s\n' % (str(loop), cmd, str(__start), str(__end)))
    return (__start, __end, cost)

def writedb(cmd, log_uri):
    write_log(log_uri, '[start write]\n\ncmd: %s' % cmd)
    __start = time.time()
    with os.popen(cmd) as f:
        data = f.read()
        if len(data) != 0:
            write_log(log_uri, '[abort] cmd: %s\n\n%s\n' % (cmd, data))
            return (0, False)
    __end = time.time()
    cost = __end - __start
    return (cost, True)

def run_script(action, script, uri, conns, reqs, log_uri, result_uri):
    write_cmd = 'python %s %s %s %s %s -d -e' % (script, action, uri, conns, reqs)
    (write_cost, rc) = writedb(write_cmd, log_uri)
    if not rc:
        return False
    sync_cmd = 'python %s %s %s %s %s -d -v' % (script, action, uri, conns, reqs)
    (start, end, delta) = waitsync(sync_cmd, log_uri)
    with open(result_uri, 'a+') as f:
        f.write('%s\n' % json.dumps({
            'write': { 'cmd': write_cmd, 'cost': format_time(write_cost) },
            'sync': { 'cmd': sync_cmd, 'cost': format_time(end - start), 'start': start, 'end': end, 'delta': delta }
            }))
    return True
def run(argv):
    cwd = os.path.dirname(os.path.abspath(argv[0]))
    args = len(argv)
    if 5 != args:
        return False
    (script, uri, conns, reqs) = (os.path.abspath(argv[1]), argv[2], argv[3], argv[4])
    daemonize()
    log_uri = os.path.join(cwd, 'waitsync.log')
    result_uri = os.path.join(cwd, 'waitsync.result')
    if not run_script('write', script, uri, conns, reqs, log_uri, result_uri):
        return True
    return run_script('clean', script, uri, conns, reqs, log_uri, result_uri)

if __name__ == "__main__":
    if not run(sys.argv):
        manual()