`upstream`
----------

**Type:** `object`

**Value:** `{ "<key-str>": <val>, "<key-str>": <val>, ... , "<key-str>": <val> }`

**Attribute:** Optional

**Parent:** [handler](handler.md)

Configuration on upstream for handler in nginx module. It will be used to generated code template on subrequest.  

There are two patterns on subrequest: [`parallel subrequests`](../parallel_subrequests.md) and [`sequential subrequests`](../sequential_subrequests.md).  
The former has better performance while the latter has simpler configuration.  

**If this field is ignored, code template on `upstream` will not be generated**.  

`<key-str>` could be set as below:  

* [`backend_uri`](backend_uri.md)

* [`parallel_subrequests`](parallel_subrequests.md)  

* [`sequential_subrequests`](sequential_subrequests.md)

[Previous](../ngx_wizard.md)

[Home](../../index.md)