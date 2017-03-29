`core module`
--

### `lib_c_dict` ###

* `c_dict.h`
* `c_dict.c`
	
Dictionary implemented based on `hash table` for user-defined `key-value` collections. It defines interfaces including `set`, `get`, `remove`, `iterator`. For example:  

    typedef struct
	{
	    hustmq_ha_queue_base_t base;
		char queue[HUSTMQ_HA_QUEUE_SIZE + 1];
		const char * host;
	} hustmq_ha_queue_item_t;
	
	typedef struct
	{
	    c_dict_t(hustmq_ha_queue_item_t *) host_dict; // host as key, hustmq_ha_queue_item_t * as value
	} hustmq_ha_queue_value_t;

    hustmq_ha_queue_item_t * hustmq_ha_host_dict_get(hustmq_ha_queue_value_t * dict, const char * key)
	{
	    if (!dict)
	    {
	        return 0;
	    }
	    hustmq_ha_queue_item_t ** val = c_dict_get(&dict->host_dict, key);
	    return val ? *val : 0;
	}

    static bool __is_invalid(hustmq_ha_queue_value_t * queue_val)
	{
	    const char * key;
	    c_dict_iter_t iter = c_dict_iter(&queue_val->host_dict);
	    while ((key = c_dict_next(&queue_val->host_dict, &iter)))
	    {
	        hustmq_ha_queue_item_t * queue_item = hustmq_ha_host_dict_get(queue_val, key);
	        if (!queue_item->base.valid)
	        {
	            return true;
	        }
	    }
	    return false;
	}    

### `lib_cjson_serialization` ###

* `cjson_serialization.h`
* `cjson_serialization.c`
* `cjson_serialization.json`
* `cjson_serialization_base.h`
* `cjson_serialization_base.c`

Object serialization library based on `cjson`. It will bring in great productivity when works together with `jsoncgen`. You can write schema file like `cjson_serialization.json`:  

    {
	    "includes": ["\"cjson_serialization_base.h\""],
	    "structs": 
        [
	        {
	            "name": "json_int_t",
	            "array_methods":
                [
                    "set", "eq", "dispose", 
                    "serialize", "deserialize", 
                    "dump", "loads", "save", "load"
                ]
	        },
	        {
	            "name": "json_str_t",
	            "array_methods": 
                [
                    "set", "eq", "dispose", 
                    "serialize", "deserialize", 
                    "dump", "loads", "save", "load"
                ]
	        },
	        {
	            "name": "double",
	            "array_methods": 
                [
                    "set", "eq", "dispose", 
                    "serialize", "deserialize", 
                    "dump", "loads", "save", "load"
                ]
	        }
	    ]
	}

Then you can use `jsoncgen` to generate codes:  

    python jsoncgen.py -f cjson_serialization.json

See [here](../jsoncgen.md) for more details on `jsoncgen`.  

### `lib_ngx_shm_dict` ###

* `ngx_shm_dict_utils.h`
* `ngx_shm_dict_utils.c`
* `ngx_shm_dict_reader.h`
* `ngx_shm_dict_reader.c`
* `ngx_shm_dict_writer.h`
* `ngx_shm_dict_writer.c`
* `ngx_shm_dict_code.py`
* `ngx_shm_dict_code.h`
* `ngx_shm_dict_code.c`
* `ngx_shm_dict.h`
* `ngx_shm_dict.c`
* `ngx_shm_dict_test_utils.h`
* `ngx_shm_dict_test_utils.c`
* `ngx_shm_dict_test.py`
* `ngx_shm_dict_test.h`
* `ngx_shm_dict_test.c`

Share memory dictionary based on `ngx_rbtree_t` borrowed from [`lua-nginx-module`](https://github.com/openresty/lua-nginx-module)  (`src/ngx_http_lua_shdict.c`).  

nginx encapsulates share memory as following interfaces:  

	ngx_shm_zone_t * ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name, size_t size, void *tag);
	void ngx_slab_init(ngx_slab_pool_t *pool);
	void * ngx_slab_alloc(ngx_slab_pool_t *pool, size_t size);
	void ngx_slab_free(ngx_slab_pool_t *pool, void *p);

You can use them to implement nginx module based on share memory. However, you still need to write much code using the same patterns. See examples from here:  

* `ngx_http_limit_req_module.c`
* `ngx_http_limit_conn_module.c`

So the patterns are abstracted as universal `key-value` interfaces in `lib_ngx_shm_dict` for easier programming on nginx module using share memory.  
  
`lib_ngx_shm_dict` supports built-in types as below:  

- `int8_t`  
- `uint8_t`  
- `int16_t`  
- `uint16_t`  
- `int32_t`  
- `uint32_t`  
- `int64_t`  
- `uint64_t`  
- `double`  
- `ngx_str_t`  
- `ngx_binary_t`  

`ngx_shm_dict_code.py` generates `ngx_shm_dict_code.h` and `ngx_shm_dict_code.c` to provide read & write interfaces for built-in types. For instance, the interfaces on `int8_t` are generated as below:  

    int ngx_shm_dict_get_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t * val);
    int ngx_shm_dict_add_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t val, uint32_t expire);
    int ngx_shm_dict_safe_add_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t val, uint32_t expire);
    int ngx_shm_dict_replace_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t val, uint32_t expire);
    int ngx_shm_dict_set_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t val, uint32_t expire);
    int ngx_shm_dict_safe_set_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t val, uint32_t expire);
    int ngx_shm_dict_incr_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t delta, uint32_t expire, int8_t * result);
    int ngx_shm_dict_decr_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t delta, uint32_t expire, int8_t * result);

Other built-in types are similar.  

`ngx_shm_dict.h` defines the interfaces on initialization and bulk operations for share memory dictionary:  

    ngx_shm_zone_t * ngx_shm_dict_init(ngx_conf_t * cf, ngx_str_t * name, size_t size, void * module);
    int ngx_shm_dict_delete(ngx_shm_zone_t * zone, ngx_str_t * key);
    int ngx_shm_dict_traverse(ngx_shm_zone_t * zone, ngx_shm_dict_tarverse_cb cb, void * data);
    int ngx_shm_dict_get_total_keys(ngx_shm_zone_t * zone, uint8_t include_expires, size_t * total);
    int ngx_shm_dict_get_keys(ngx_shm_zone_t * zone, uint8_t include_expires, ngx_pool_t * pool, ngx_shm_dict_keys_t * keys);
    int ngx_shm_dict_flush_all(ngx_shm_zone_t * zone);
    int ngx_shm_dict_flush_expired(ngx_shm_zone_t * zone, int attempts);
`ngx_shm_dict_test.py` generates `ngx_shm_dict_test.h` and `ngx_shm_dict_test.c` for testing:  

    ngx_int_t ngx_shm_dict_test_read(ngx_shm_dict_read_args_t * args, ngx_http_request_t * r);
    ngx_int_t ngx_shm_dict_test_write(ngx_shm_dict_write_args_t * args);
    ngx_int_t ngx_shm_dict_test_incr(ngx_pool_t * pool, ngx_shm_dict_mod_args_t * args);
    ngx_int_t ngx_shm_dict_test_decr(ngx_pool_t * pool, ngx_shm_dict_mod_args_t * args);

See more details on `ngx_shm_dict` from [`hustdict`](../../appendix/hustdict.md).  
   
[Previous](../lib_hustngx.md)

[Home](../../index.md)