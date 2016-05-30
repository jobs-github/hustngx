Nginx 工程配置
--

### 编译选项 ###

    [file]
    nginx/auto/cc/gcc
    
    [key]
    ......
    # optimizations

    #NGX_GCC_OPT="-O2"
    #NGX_GCC_OPT="-Os"
    NGX_GCC_OPT="-O"
    ......
    # debug
    CFLAGS="$CFLAGS -g"
    ......

如果需要关闭编译器优化，打开调试信息，可修改此文件。

### 链接选项 ###

    [file]
    nginx/auto/cc/conf
    
    [key]
    ......
    LINK="\$(CC)"
    ......

如果需要链接第三方库，可以修改此文件。

### 安装选项 ###

    [file]
    nginx/auto/install

    [key]
    ......
    install:	$NGX_OBJS${ngx_dirsep}nginx${ngx_binext}
    ......

如果需要在安装 nginx 时自定义文件拷贝的逻辑，可以修改此文件。

### 第三方库集成 ###

    [file]
    nginx/auto/sources

    [key]
    ......
    CORE_DEPS="src/core/nginx.h \
    ......
    CORE_SRCS="src/core/nginx.c \
    ......
    HTTP_DEPS="src/http/ngx_http.h \
    ......
    HTTP_SRCS="src/http/ngx_http.c \

如果你引入的第三方库需要集成到 nginx 源代码中，可以修改此文件添加依赖关系。

### nginx 模块集成 ###

如果你想将自己编写的模块集成到 nginx 源代码中，你需要关注如下几个文件：

- `nginx/auto/options`

- `nginx/auto/sources`

- `nginx/auto/modules`

`nginx/auto/options` 用于定义是否添加模块的编译：

    [file]
    nginx/auto/options

    [key]
    ......
    HTTP_AUTH_BASIC=YES
    ......

`nginx/auto/sources` 用于定义模块的名称以及源代码文件路径，例如：

    [file]
    nginx/auto/sources
    [key]
    ......
    HTTP_AUTH_BASIC_MODULE=ngx_http_auth_basic_module
    HTTP_AUTH_BASIC_SRCS=src/http/modules/ngx_http_auth_basic_module.c
    ......

`nginx/auto/modules` 用于整合以上两个文件所增加的模块，例如：

    [file]
    nginx/auto/modules

    [key]
    ......
    if [ $HTTP_AUTH_BASIC = YES ]; then
        USE_MD5=YES
        USE_SHA1=YES
        have=NGX_CRYPT . auto/have
        HTTP_MODULES="$HTTP_MODULES $HTTP_AUTH_BASIC_MODULE"
        HTTP_SRCS="$HTTP_SRCS $HTTP_AUTH_BASIC_SRCS"
        CORE_LIBS="$CORE_LIBS $CRYPT_LIB"
    fi
    ......

其中这两句是关键：

    HTTP_MODULES="$HTTP_MODULES $HTTP_AUTH_BASIC_MODULE"
    HTTP_SRCS="$HTTP_SRCS $HTTP_AUTH_BASIC_SRCS"
    
[上一级](index.md)

[根目录](../index.md)