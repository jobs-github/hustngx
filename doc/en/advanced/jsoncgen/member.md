member
----------

**Type:** `array`

**Value:** `["<type-str>", "<name-str>"]`

**Attribute:** Refer to [members](members.md)

**Parent:** [members](members.md)

Define the member of struct. `<type-str>` could be set as scalar type as well as user-defined type.  

`jsoncgen` now supports the following scalar types:  

- `json_int_t`

- `double`

- `json_str_t`

If `<type-str>` is set as user-defined type, **the referenced type should be defined above all**, for instance:  

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

[Previous](../jsoncgen.md)

[Home](../../index.md)