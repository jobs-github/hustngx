`args`
----------

**类型:** `array`

**值:** `[<arg>, <arg>, ..., <arg>]`

**属性:** 可选

**父节点:** [handler](handler.md)

nginx 模块各个 `handler` 的 `uri` 参数列表。每一个子项都是 [arg](arg.md)。

如果配置了该项，`hustngx` 会生成参数列表的结构体，以及对应的 `decode` 函数实现。

例如：

    {
        "module": "hustmq_ha",
        ......
        "upstream": "backend",
        "handlers":
        [
            ......
            {
                "uri": "test",
                "args":
                [
                    ["int", "int_val", "-1"],
                    ["bool", "bool_val", "true"],
                    ["string", "str_val"],
                    ["size", "size_val", "1024"],
                    ["time", "time_val", "60"]
                ]
            },
            ......
        ]
    }

对应生成的 `hustmq_ha_test_handler.c` 会包含如下代码：

    typedef struct
    {
        ngx_int_t int_val;
        ngx_bool_t bool_val;
        ngx_str_t str_val;
        ssize_t size_val;
        ngx_int_t time_val;

        ngx_bool_t has_int_val;
        ngx_bool_t has_bool_val;
        ngx_bool_t has_str_val;
        ngx_bool_t has_size_val;
        ngx_bool_t has_time_val;
    } hustmq_ha_test_args_t;

    ngx_bool_t hustmq_ha_test_decode_args(ngx_http_request_t *r, hustmq_ha_test_args_t * args)
    {
        if (!r || !args)
        {
            return false;
        }
        memset(args, 0, sizeof(hustmq_ha_test_args_t));
        // reset
        args->int_val = -1;
        args->bool_val = true;
        args->size_val = 1024;
        args->time_val = 60;

        ngx_str_t val = {0,0};
        static const ngx_str_t INT_VAL = ngx_string("int_val");
        if (NGX_OK == ngx_http_arg(r, INT_VAL.data, INT_VAL.len, &val))
        {
            args->int_val = ngx_atoi(val.data, val.len);
            args->has_int_val = true;
        }
        static const ngx_str_t BOOL_VAL = ngx_string("bool_val");
        if (NGX_OK == ngx_http_arg(r, BOOL_VAL.data, BOOL_VAL.len, &val))
        {
            args->bool_val = ngx_http_decode_bool(&val);
            args->has_bool_val = true;
        }
        static const ngx_str_t STR_VAL = ngx_string("str_val");
        if (NGX_OK == ngx_http_arg(r, STR_VAL.data, STR_VAL.len, &val))
        {
            args->str_val = ngx_http_make_str(&val, r->pool);
            args->has_str_val = true;
        }
        static const ngx_str_t SIZE_VAL = ngx_string("size_val");
        if (NGX_OK == ngx_http_arg(r, SIZE_VAL.data, SIZE_VAL.len, &val))
        {
            args->size_val = ngx_parse_size(&val);
            args->has_size_val = true;
        }
        static const ngx_str_t TIME_VAL = ngx_string("time_val");
        if (NGX_OK == ngx_http_arg(r, TIME_VAL.data, TIME_VAL.len, &val))
        {
            args->time_val = ngx_parse_time(&val, 0);
            args->has_time_val = true;
        }
        return true;
    }

[回上页](../ngx_wizard.md)

[回首页](../../index.md)