keys
----------

**Interface:** `/keys`

**Method:** `GET`

**Argument:** 

*  **all**  (Optional)   
Acceptable values: `true | false`.  
When set `true`, `hustdict` will return the number of all keys (including expired), otherwise, it will only return the number of keys not expired.   
If `all` is not included in arguments, `hustdict` will treat it as `false` by default.  

**Sample:**

    curl -i -X GET "http://localhost:8085/keys?all=true"

[Previous](../hustdict.md)

[Home](../../index.md)