set
----------

**Interface:** `/set`

**Method:** `GET | POST`

**Argument:** 

*  **method**  (Required)   
Acceptable values:  
	* set  
	* add  
	* safe_set  
	* safe_add  
	* replace  
*  **key**  (Required) 
*  **type**  (Required)   
The value refers to [`value-types`](types.md)  
*  **val**  (Optional) 
*  **expire**  (Optional) 

**Comment:**

* When `type` is `binary`, `method` should be set as `POST`, and `val` should be posted by  `http body`.  
* When `type` is not `binary`, `method` could be set as `GET`, and `val` could by passed by arguments in `uri`.  
* The difference between `set` and `safe_set` is the former will swap data (delete the oldest data) while the latter will directly return error code without swapping data when the share memory is exhausted.  
* `expire` stands for expiration period, using second as unit.  

**Sample:**

    curl -i -X GET "http://localhost:8085/set?method=set&key=test_key&type=string&val=test_val"

[Previous](../hustdict.md)

[Home](../../index.md)