upgrade
--

`upgrade` is a tool for smooth upgrading on nginx.  

### Usage ###

    usage:
        sh upgrade.sh [user]
    sample:
        sh upgrade.sh jobs

### Principle of smooth upgrading on nginx ###

The nginx source code defines the following key macros:  

    #define NGX_SHUTDOWN_SIGNAL      QUIT
    #define NGX_NOACCEPT_SIGNAL      WINCH
    #define NGX_CHANGEBIN_SIGNAL     USR2

It is the key to perform smooth upgrading.  

Firstly, we need to get the pid of master process (assuming it to be `$old_pid`), then send signal `USR2`:  

    kill -USR2 $old_pid

nginx will rename the pid file (in general, `logs/nginx.pid`) corresponding to `$old_pid`, and start master process using the latest executable file.  

Secondly, send signal `WINCH`:  

    kill -WINCH $oldpid

After receiving the signal, nginx will notify the old worker processes to stop accepting new connections. When all old connections have been done, the old worker processes will quit. Such process is performed by the code below (in master process):  

    if (ngx_noaccept) {
        ......
        ngx_signal_worker_processes(cycle, 
            ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
    }

Finally, send signal `QUIT` to notify old master process to quit:  

    kill -QUIT $oldpid

[Previous](index.md)

[Home](../index.md)