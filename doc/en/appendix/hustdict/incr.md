incr
----------

**Interface:** `/incr`

**Method:** `GET`

**Argument:** 

*  **key**  (Required)   
*  **type**  (Required)   
The value refers to the **numeric type** in [`value-types`](types.md)  
*  **delta**  (Required) 
*  **expire**  (Optional) 

**Sample:**

    curl -i -X GET "http://localhost:8085/incr?key=test_key&type=uint8&delta=1"

[Previous](../hustdict.md)

[Home](../../index.md)