member
----------

**类型:** `array`

**值:** `["<type-str>", "<name-str>"]`

**属性:** 参考 [members](members.md)

**父节点:** [members](members.md)

描述结构体的成员，其中 `<type-str>` 可以是基本类型，也可以是自定义类型。

`jsoncgen` 目前支持的基本类型包括：

- `json_int_t`

- `double`

- `json_str_t`

如果 `<type-str>` 是自定义类型，则 **被引用的类型请务必放在前面定义** ，例如：

    {
	    "structs": 
        [
	        {
	            "name": "BillGates",
	            "members":
                [
	                ["json_int_t", "int_val"]
	            ],
	            "methods" : ["serialize", "deserialize"]
	        },
            {
	            "name": "Jobs",
	            "members":
                [
	                ["BillGates", "bill_gates"]
	            ],
	            "methods" : ["serialize", "deserialize"]
	        }
	    ]
	}

[回上页](../jsoncgen.md)

[回首页](../../index.md)