`main_conf`
----------

**Type:** `array`

**Value:** `[ ["<key>", <value>], ["<key>", <value>], ... , ["<key>", <value>] ]`

**Example:** `"main_conf": [ ["test_num", 1], ["test_flag", "on"] ]`

**Parent:** None

To configure fields in `main_conf`.  

The supported names of `<key>` could be found [here](../ngx_wizard/main_conf.md). 

`int` is equivalent to `ngx_int_t` in nginx, example:  

    {
        "main_conf":
        [
            ["keepalive_cache_size", 128]
        ]
    }

`size` is equivalent to `ssize_t` in nginx, example:  

    {
        "main_conf":
        [
            ["fetch_buffer_size", "64m"]
        ]
    }

`time` is equivalent to `ngx_int_t` in nginx, example:  

    {
        "main_conf":
        [
            ["long_polling_timeout", "180s"],
            ["autost_interval", "200ms"]
        ]
    }

`bool` is equivalent to `ngx_bool_t` in nginx, example:  

    {
        "main_conf":
        [
            ["status_cache", "off"]
        ]
    }

`string` is equivalent to `ngx_str_t` in nginx, example:  

    {
        "main_conf":
        [
            ["autost_uri", "/hustmq/stat_all"]
        ]
    }

[Previous](genconf.md)

[Home](../../index.md)