快速入门
--

解压 `hustngx.tar.gz`，进入 `hustngx` 目录，编写 `hello.json` 文件，内容如下：

    {
        "module": "hustmqha",
        "handlers":
        [
            {
                "uri": "test"
            }
        ]
    }

运行如下命令：

    python hustngx.py nginx-1.9.4.tar.gz hello.json

此时可以看到生成了 `hustmqha` 目录，进入该目录，可以看到如下文件（目录）：

    nginx            # nginx代码主目录
    deploygen.py     # 一键部署脚本生成器
    deploy_ngx.json  # deploygen.py 的命令配置文件
    hosts            # deploygen.py 的机器列表配置文件
    deploy.sh        # 一键部署脚本 (deploygen.py+deploy_ngx.json+hosts 生成)
    upgrade.sh       # nginx 平滑升级脚本


进入 `nginx/conf` 目录，编辑 `sample.json` 如下：

    {
	    "module": "hustmqha",
	    "worker_connections": 1048576,
	    "listen": 8080,
	    "local_cmds": ["/test"]
	}

生成 nginx 配置文件：

    python genconf.py sample.json

编译安装nginx:

    cd hustmqha/nginx/
    sh Config.sh && make install

安装完成之后，启动 nginx：

    cd /data/hustmqha/sbin
    ./nginx

输入如下测试命令：

    curl -i -X GET 'localhost:8080/test'

可以看到服务器返回如下内容：

    HTTP/1.1 200 OK
	Server: nginx/1.9.4
	Date: Tue, 03 Nov 2015 03:54:37 GMT
	Content-Type: text/plain
	Content-Length: 12
	Connection: keep-alive
	
	Hello World!

返回该结果说明服务器工作正常。

[根目录](../index.md)