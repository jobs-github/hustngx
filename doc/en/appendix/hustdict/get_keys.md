get_keys
----------

**Interface:** `/get_keys`

**Method:** `GET`

**Argument:** 

*  **all**  (Optional)   
Acceptable values: `true | false`.  
When set `true`, all keys (including expired) will be fetched, otherwise, only those not expired will be fetched.   
If `all` is not included in arguments, `hustdict` will treat it as `false` by default.  

**Sample:**

    curl -i -X GET "http://localhost:8085/get_keys?all=true"

[Previous](../hustdict.md)

[Home](../../index.md)