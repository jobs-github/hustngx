keys
----------

**接口:** `/keys`

**方法:** `GET`

**参数:** 

*  **all** （可选）  
可选值参考：`true | false`  。  
当值为 `true` 时，会获取所有的 `key` 的数量（包含已过期的）；否则只获取未过期的 `key` 的数量。  
如果参数中不包含 `all` 字段，默认按 `false` 处理。

**使用范例:**

    curl -i -X GET "http://localhost:8085/keys?all=true"

[回上页](../hustdict.md)

[回首页](../../index.md)