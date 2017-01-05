简介
--

hustngx 是一套用于 nginx 模块开发的自动化代码生成框架。

### 参考 ###

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

### 目标 ###

如果你：

- **具备基本的 nginx 模块开发的知识**

- 需要一套 nginx 模块开发的实用的基础函数库

- 需要一套量产 nginx 模块的自动化工具

- 不想被 nginx 模块开发中 subrequest 的各种坑所折磨

- 不想被对象的序列化/反序列化的各种重复 coding 所折磨


那么 hustngx 就是为你量身打造的工具链，在它的帮助下，**你的生产力将如虎添翼**。

### 组件 ###

hustngx包含如下组件：

- **lib_hustngx**: nginx 模块开发基础函数库
- **patch**: nginx 源代码补丁文件
- **third_party**: nginx 第三方模块
- **ngx_wizard**: nginx 模块代码生成工具
- **deploygen**: nginx 一键部署脚本生成工具
- **sync**: 文件同步工具
- **upgrade**: nginx 平滑升级工具
- **hustngx**: 对以上所有工具的整合(从 lib_hustngx 到 upgrade)
- **jsoncgen**: 基于json的对象代码生成器

其中 ngx_wizard, jsoncgen，deploygen，sync, upgrade 均可作为独立的工具使用。

### 限制 ###

**当前 hustngx 支持的 nginx 版本为 1.10.0 stable version**

[回首页](../index.md)