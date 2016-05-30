jsoncgen
--

`jsoncgen` 是一个独立而且完整的代码生成工具，以 `json` 作为描述语言，支持基于 [`cjson`](https://github.com/kbranigan/cJSON) 或者 [`jansson`](https://github.com/akheron/jansson) 作为 `json` 的底层解析库，可以批量生成该工具所支持的C语言结构体的函数实现代码。

目前 `hustngx` 使用 `cjson` 作为 `json` 序列化基础库的实现。

### 使用范例 ###

    usage:
        python jsoncgen.py [lib] [path] filelist
            [lib]
                -j: use jansson as json library, 
                    otherwise use cjson as json library
                    NOTE: cjson will be used as default value
            [path]
                -f: use file in filelist
                -p: use dir in filelist, will be parsed recursive
                
    sample:
        python jsoncgen.py -f file1.json file2.json file3.json
        python jsoncgen.py -p ./dir1/ ./dir2/ ./dir3/
        python jsoncgen.py -j -f file1.json file2.json file3.json
        python jsoncgen.py -j -p ./dir1/ ./dir2/ ./dir3/

### 配置范例 ###

以下是一个完整的配置文件，包含 `jsoncgen` 所支持的所有字段：

    {
        "includes": ["\"cjson_serialization_base.h\""],
	    "structs": 
        [
	        {
	            "name": "SampleStruct",
	            "members":
                [
	                ["json_int_t", "int_val"],
	                ["double", "double_val"],
	                ["json_str_t", "str_val"]
	            ],
	            "methods" : 
                [
                    "set", "eq", "dispose", 
                    "serialize", "deserialize", 
                    "dump", "loads", "save", "load"
                ],
	            "array_methods":
                [
                    "set", "eq", "dispose", 
                    "serialize", "deserialize", 
                    "dump", "loads", "save", "load"
                ]
	        }
	    ]
	}

### 字段结构 ###

[`includes`](jsoncgen/includes.md)  
[`structs`](jsoncgen/structs.md)  
　　[`struct`](jsoncgen/struct.md)  
　　　　[`name`](jsoncgen/name.md)  
　　　　[`members`](jsoncgen/members.md)  
　　　　　　[`member`](jsoncgen/member.md)  
　　　　[`methods`](jsoncgen/methods.md)  
　　　　　　[`method`](jsoncgen/method.md)  
　　　　[`array_methods`](jsoncgen/array_methods.md)  
　　　　　　[`array_method`](jsoncgen/array_method.md)  

### 字段约束 ###

* [`members`](jsoncgen/members.md) 和 [`array_methods`](jsoncgen/array_methods.md) 至少要定义一个，不可以两个都忽略。

	* 建议定义 [`members`](jsoncgen/members.md)，把 [`array_methods`](jsoncgen/array_methods.md) 作为可选字段。

	* 特殊情况下，可以只定义 [`array_methods`](jsoncgen/array_methods.md) ，用于生成基本数据类型的 **数组** 相关的函数实现，可参考 [`cjson_serialization`](../lib_hustngx/core_module.md) ， **这种做法你不会用到，因为这只适用于基本类型的数组代码生成，不适用于自定义类型** 。

* [`methods`](jsoncgen/methods.md) 和 [`array_methods`](jsoncgen/array_methods.md) 至少要定义一个，不可以两个都忽略。

	* 建议定义 [`methods`](jsoncgen/methods.md)，把 [`array_methods`](jsoncgen/array_methods.md) 作为可选字段。

	* 如果同时定义了两者，则 [`array_methods`](jsoncgen/array_methods.md) 所包含的字段必须全部在 [`methods`](jsoncgen/methods.md) 中定义。

[上一级](index.md)

[根目录](../index.md)