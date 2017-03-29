`auth_filter`
----------

**Type:** `array`

**Value:** `["<localcmd>", "<localcmd>", ... , "<localcmd>"]`

**Example:** `"auth_filter": ["test"]`

**Parent:** None

Defines the commands not require `http basic authentication` in `local_cmds`.  

For example, you can add location `test` into `auth_filter` to access nginx without authentication. The generated contents in `nginx.conf` is like this:  

    location /test {
        hustmqha;
        #http_basic_auth_file /data/hustmqha/conf/htpasswd;
    }

As what you see, `http_basic_auth_file` is commented.  

[Previous](genconf.md)

[Home](../../index.md)