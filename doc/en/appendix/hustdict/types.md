Types of value
----------

`hustdict` supports the following types:  

* numeric types:  
	* int8
	* uint8
	* int16
	* uint16
	* int32
	* uint32
	* int64
	* uint64
	* double
* string type  
	* string
* binary type  
	* binary

`string` is interpreted as `ngx_str_t`, while `binary` is interpreted as the following type:  

    typedef struct
	{
	    size_t len;
	    void * data;
	} ngx_binary_t;

[Previous](../hustdict.md)

[Home](../../index.md)