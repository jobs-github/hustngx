Nginx project & configuration
--

### Compile options ###

If you need to disable compiler optimization and enable debugging information, please add the `--with-cc-opt` option when executing `configure`, for example:  

    ./configure --with-cc-opt="-g3 -O0"

### Link options ###

If you need to link a third-party library, please add the `--with-ld-opt` option when executing `configure`, for example:  

    ./configure --with-ld-opt="-lzlog -lpthread -lm -ldl -lcrypto"

### Installation options ###

If you need to customize the installation process of nginx, please open `nginx/auto/install` and find the following text:  

    install:	build $NGX_INSTALL_PERL_MODULES
        test -d '\$(DESTDIR)$NGX_PREFIX' || mkdir -p '\$(DESTDIR)$NGX_PREFIX'

You can directly change the installation of the script behind it.  

### Third party library integration ###

If you need to integrate a third-party library into nginx core source code, please open `nginx/auto/sources` and find the following code:  

    CORE_DEPS="src/core/nginx.h \
               ......


    CORE_SRCS="src/core/nginx.c \
               ......

Then add it to the end, for example:  

    CORE_DEPS="src/core/nginx.h \
               ......
               src/core/cJSON.h"


    CORE_SRCS="src/core/nginx.c \
               ...
               src/core/cJSON.c"

If the third-party library needs to be integrated into the nginx http module, please open `nginx/auto/modules` and find the following code:  

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

Please add the new module name at the end of `ngx_module_name`, the new module's `.h` file at the end of `ngx_module_deps`, the new module's `.c` file at the end of `ngx_module_srcs`, for example:  

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

### Nginx module integration ###

If you want to integrate your own modules into nginx source code, please open `nginx/auto/modules` and find the following code:  

    if :; then
        ngx_module_name=ngx_http_write_filter_module
        ngx_module_incs=
        ngx_module_deps=
        ngx_module_srcs=src/http/ngx_http_write_filter_module.c
        ngx_module_libs=
        ngx_module_link=YES

        . auto/module
    fi

Just follow this format. For example:  

    if :; then
        ngx_module_name=ngx_http_peer_selector_module
        ngx_module_incs=
        ngx_module_deps=
        ngx_module_srcs=src/http/modules/ngx_http_peer_selector_module.c
        ngx_module_libs=
        ngx_module_link=YES

        . auto/module
    fi
    
[Previous](index.md)

[Home](../index.md)