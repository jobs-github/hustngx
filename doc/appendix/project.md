Nginx 工程配置
--

### 编译选项 ###

如果需要关闭编译器优化，打开调试信息，可在执行 `configure` 文件的时候加上 `--with-cc-opt` 选项，例如：

    ./configure --with-cc-opt="-g3 -O0"

### 链接选项 ###

如果需要链接第三方库，可在执行 `configure` 文件的时候加上 `--with-ld-opt` 选项，例如：

    ./configure --with-ld-opt="-lzlog -lpthread -lm -ldl -lcrypto"

### 安装选项 ###

如果需要在安装 nginx 时自定义文件拷贝的逻辑，请打开 `nginx/auto/install` ，找到如下行：

    install:	build $NGX_INSTALL_PERL_MODULES
        test -d '\$(DESTDIR)$NGX_PREFIX' || mkdir -p '\$(DESTDIR)$NGX_PREFIX'

直接更改后面的安装脚本即可。

### 第三方库集成 ###

如果你引入的第三方库需要集成到 nginx 核心源代码中，请打开 `nginx/auto/sources` ，找到如下代码段：

    CORE_DEPS="src/core/nginx.h \
               ......


    CORE_SRCS="src/core/nginx.c \
               ......

添加至末尾即可，例如：

    CORE_DEPS="src/core/nginx.h \
               ......
               src/core/cJSON.h"


    CORE_SRCS="src/core/nginx.c \
               ...
               src/core/cJSON.c"

如果第三方库需要集成到 nginx http 模块中，请打开 `nginx/auto/modules` ，找到如下代码段：

    ngx_module_type=HTTP

    if :; then
        ngx_module_name="ngx_http_module \
                         ngx_http_core_module \
                         ngx_http_log_module \
                         ngx_http_upstream_module"
        ngx_module_incs="src/http src/http/modules"
        ngx_module_deps="src/http/ngx_http.h \
                         src/http/ngx_http_request.h \
                         src/http/ngx_http_config.h \
                         src/http/ngx_http_core_module.h \
                         src/http/ngx_http_cache.h \
                         src/http/ngx_http_variables.h \
                         src/http/ngx_http_script.h \
                         src/http/ngx_http_upstream.h \
                         src/http/ngx_http_upstream_round_robin.h"
        ngx_module_srcs="src/http/ngx_http.c \
                         src/http/ngx_http_core_module.c \
                         src/http/ngx_http_special_response.c \
                         src/http/ngx_http_request.c \
                         src/http/ngx_http_parse.c \
                         src/http/modules/ngx_http_log_module.c \
                         src/http/ngx_http_request_body.c \
                         src/http/ngx_http_variables.c \
                         src/http/ngx_http_script.c \
                         src/http/ngx_http_upstream.c \
                         src/http/ngx_http_upstream_round_robin.c"
        ngx_module_libs=
        ngx_module_link=YES

        . auto/module
    fi

`ngx_module_name` 末尾加上新增的模块名，`ngx_module_deps` 末尾加上新增模块的 `.h` 文件，`ngx_module_srcs` 末尾加上新增模块的 `.c` 文件，例如：

    ngx_module_type=HTTP

    if :; then
        ngx_module_name="ngx_http_module \
                         ngx_http_core_module \
                         ngx_http_log_module \
                         ngx_http_upstream_module \
                         ngx_http_fetch_module"
        ngx_module_incs="src/http src/http/modules"
        ngx_module_deps="src/http/ngx_http.h \
                         src/http/ngx_http_request.h \
                         src/http/ngx_http_config.h \
                         src/http/ngx_http_core_module.h \
                         src/http/ngx_http_cache.h \
                         src/http/ngx_http_variables.h \
                         src/http/ngx_http_script.h \
                         src/http/ngx_http_upstream.h \
                         src/http/ngx_http_upstream_round_robin.h \
                         src/http/modules/ngx_http_fetch.h"
        ngx_module_srcs="src/http/ngx_http.c \
                         src/http/ngx_http_core_module.c \
                         src/http/ngx_http_special_response.c \
                         src/http/ngx_http_request.c \
                         src/http/ngx_http_parse.c \
                         src/http/modules/ngx_http_log_module.c \
                         src/http/ngx_http_request_body.c \
                         src/http/ngx_http_variables.c \
                         src/http/ngx_http_script.c \
                         src/http/ngx_http_upstream.c \
                         src/http/ngx_http_upstream_round_robin.c \
                         src/http/modules/ngx_http_fetch.c"
        ngx_module_libs=
        ngx_module_link=YES

        . auto/module
    fi

### nginx 模块集成 ###

如果你想将自己编写的模块集成到 nginx 源代码中，请打开 `nginx/auto/modules`，找到如下的代码段：

    if :; then
        ngx_module_name=ngx_http_write_filter_module
        ngx_module_incs=
        ngx_module_deps=
        ngx_module_srcs=src/http/ngx_http_write_filter_module.c
        ngx_module_libs=
        ngx_module_link=YES

        . auto/module
    fi

参考这个写法即可。例如：

    if :; then
        ngx_module_name=ngx_http_peer_selector_module
        ngx_module_incs=
        ngx_module_deps=
        ngx_module_srcs=src/http/modules/ngx_http_peer_selector_module.c
        ngx_module_libs=
        ngx_module_link=YES

        . auto/module
    fi
    
[上一级](index.md)

[根目录](../index.md)