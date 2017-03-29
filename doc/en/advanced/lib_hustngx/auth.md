`auth`
----------

**Type:** `string`

**Value:** `<base64-encoded-user-str>`

**Example:** `"auth": "aHVzdGRieGM6cWlob29odXN0eHJiY3pkYg=="`

**Parent:** [proxy](proxy.md)

This field could be set as base64 encoded user-str for nginx if the `backend` requires `http basic authentication`.  

For example, if the username & password for accessing backend is following:  

    jobs:p@ssword

The value of `auth` will be:  

    am9iczpwQHNzd29yZA==

[Previous](genconf.md)

[Home](../../index.md)