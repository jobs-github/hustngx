incr
----------

**接口:** `/incr`

**方法:** `GET`

**参数:** 

*  **key** （必选）  
*  **type** （必选）  
可选值参考 [`value-types`](types.md) 中的 **数字类型** 。
*  **delta** （必选）
*  **expire** （可选）

**使用范例:**

    curl -i -X GET "http://localhost:8085/incr?key=test_key&type=uint8&delta=1"

[上一级](../hustdict.md)

[回首页](../../index.md)