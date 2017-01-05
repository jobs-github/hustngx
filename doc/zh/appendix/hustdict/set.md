set
----------

**接口:** `/set`

**方法:** `GET | POST`

**参数:** 

*  **method** （必选）  
可选值列表：
	* set
	* add
	* safe_set
	* safe_add
	* replace
*  **key** （必选）
*  **type** （必选）  
可选值参考：[`value-types`](types.md)
*  **val** （可选）
*  **expire** （可选）

**备注:**

* 当 `type` 为 `binary` 时，方法必须为 `POST` , 同时 `val` 必须通过 `http body` 来传递。
* 当 `type` 不为 `binary` 时，方法可为 `GET`，同时 `val` 通过参数传递。
* `set` 和 `safe_set` 的差别在于，前者在共享内存被用尽时，会进行置换（将最老的数据删掉），而后者则不会进行置换，直接返回错误码。
* `expire` 表示过期时间，单位为秒

**使用范例:**

    curl -i -X GET "http://localhost:8085/set?method=set&key=test_key&type=string&val=test_val"

[回上页](../hustdict.md)

[回首页](../../index.md)