upgrade
--

upgrade 是一个进行 nginx 平滑升级的脚本。

### 使用范例 ###

    usage:
        sh upgrade.sh [user]
    sample:
        sh upgrade.sh jobs

### nginx 平滑升级的原理 ###

nginx 内部定义了如下几个关键的宏：

    #define NGX_SHUTDOWN_SIGNAL      QUIT
    #define NGX_NOACCEPT_SIGNAL      WINCH
    #define NGX_CHANGEBIN_SIGNAL     USR2

这几个宏是实现平滑升级的关键。

首先需要获得指定用户的 nginx 主进程 pid，假定为 `$old_pid` ，然后发送 `USR2` 信号：

    kill -USR2 $old_pid

nginx 会将 `$old_pid` 对应的 pid 文件（通常为 `logs/nginx.pid`）重命名，并利用最新的可执行文件启动新的 `master` 进程；

之后向 nginx 发送 `WINCH` 信号：

    kill -WINCH $oldpid

nginx 在收到该信号之后会通知老的 `worker` 进程停止 `accept` 新的连接，并在处理完老的连接后退出进程。该过程由 `master` 进程通过如下代码实现：

    if (ngx_noaccept) {
        ......
        ngx_signal_worker_processes(cycle, 
            ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
    }

最后，向 nginx 发送 QUIT 信号，通知老的 `master` 进程退出：

    kill -QUIT $oldpid

[回上页](index.md)

[回首页](../index.md)