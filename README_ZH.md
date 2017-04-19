[English](README.md)

# hustngx - 为懒人打造的nginx模块开发利器 #
![logo](res/logo.png)

## hustngx是什么？ ##

hustngx 是一套用于 nginx 模块开发的自动化代码生成框架。

如果你：

- 不了解 nginx 代码目录结构
- 不了解 nginx 配置的基本结构
- 不了解 nginx 的构建方法
- 不了解如何调试 nginx
- 不了解什么是 nginx 模块
- 不了解什么是 upstream, proxy, subrequest

那么，在使用 hustngx 之前，建议学习如下内容：

> [《深入理解Nginx》阅读与实践（一）：Nginx安装配置与HelloWorld](http://www.cnblogs.com/wuyuegb2312/p/3226771.html)

> [《深入理解Nginx》阅读与实践（二）：配置项的使用](http://www.cnblogs.com/wuyuegb2312/p/3256136.html)

> [《深入理解Nginx》阅读与实践（三）：使用upstream和subrequest访问第三方服务](http://www.cnblogs.com/wuyuegb2312/p/3269507.html)

**如果没有以上的积累作为基本储备，你将**：

- **很难理解 hustngx 所总结的各种范式**

- **即使你用它帮你生成了很多代码，你还是不知道如何下手**

对于 nginx 配置中的常见问题，可以参考如下内容：

> [nginx 配置之 upstream](http://nginx.org/en/docs/http/ngx_http_upstream_module.html)

> [nginx 配置之 proxy](http://nginx.org/en/docs/http/ngx_http_proxy_module.html)

此外，友情推荐如下参考资料：

> [Emiller's Advanced Topics In Nginx Module Development](http://www.evanmiller.org/nginx-modules-guide-advanced.html)

如果你：

- **具备基本的 nginx 模块开发的知识**

- 需要一套 nginx 模块开发的实用的基础函数库

- 需要一套 **量产** nginx 模块的自动化工具

- 不想被 nginx 模块开发中 subrequest 的各种坑所折磨

那么 hustngx 就是为你量身打造的工具链，在它的帮助下，**你的生产力将如虎添翼**。

![hustngx](res/hustngx.png)

## hustngx有什么？ ##

![modules](res/modules_zh.png)

## 限制 ##

**当前 hustngx 支持的 nginx 版本为 1.12.0 stable version**

## hustngx怎么用？ ##

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

    python hustngx.py nginx-1.12.0.tar.gz hello.json

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
	Server: nginx/1.12.0
	Date: Tue, 18 Apr 2017 03:54:37 GMT
	Content-Type: text/plain
	Content-Length: 12
	Connection: keep-alive
	
	Hello World!

返回该结果说明服务器工作正常。

## 文档 ##

更多细节请参考[这里](doc/zh/index.md)。

## License ##

`hustngx` is licensed under [New BSD License](https://opensource.org/licenses/BSD-3-Clause), a very flexible license to use.

## 作者 ##

* 程卓 (jobs, yao050421103@163.com)  

## 更多 ##

- 高性能分布式存储服务 - [huststore](https://github.com/Qihoo360/huststore)