jsoncgen
--

`jsoncgen` is a standalone code generator for json serialization/deserialization, using `json` as schema to generate json encoder/decoder for struct in C. It supports  [`cjson`](https://github.com/kbranigan/cJSON) and [`jansson`](https://github.com/akheron/jansson) as fundamental library.  

Now `hustngx` uses `cjson` as default library.  

### Usage ###

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

### Configuration ###

A complete example of configuration file:  

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

### Structure ###

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

### Limitation ###

* [`members`](jsoncgen/members.md) and [`array_methods`](jsoncgen/array_methods.md) could not be ignored both, please include at least one.  

	* [`members`](jsoncgen/members.md) is suggested to be included, while [`array_methods`](jsoncgen/array_methods.md) is optional tag.  

	* In special case, we can define [`array_methods`](jsoncgen/array_methods.md) only to generate the implementation on **array** of scalar types. Please refer to [`cjson_serialization`](lib_hustngx/core_module.md). **You will never use the magic way as it's only for scalar types, not for user-defined types**.  

* [`methods`](jsoncgen/methods.md) and [`array_methods`](jsoncgen/array_methods.md) could not be ignored both, please include at least one.  

	* [`methods`](jsoncgen/methods.md)is suggested to be included, while [`array_methods`](jsoncgen/array_methods.md) is optional tag.  

	* If both of them are included, the fields defined in [`array_methods`](jsoncgen/array_methods.md) must be defined in [`methods`](jsoncgen/methods.md) as well.  

[Previous](index.md)

[Home](../index.md)