值类型列表
----------

`hustdict` 支持的值类型如下：  

* 数字类型：
	* int8
	* uint8
	* int16
	* uint16
	* int32
	* uint32
	* int64
	* uint64
	* double
* 字符串
	* string
* 二进制
	* binary

其中，`string` 对应的数据结构为 `ngx_str_t` , `binary` 对应的数据结构定义如下：

    typedef struct
	{
	    size_t len;
	    void * data;
	} ngx_binary_t;

[上一级](../hustdict.md)

[根目录](../../index.md)