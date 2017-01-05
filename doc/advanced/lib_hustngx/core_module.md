核心扩展模块
--

### `lib_c_dict` ###

* `c_dict.h`
* `c_dict.c`
	
利用 `hash table` 实现的字典，可以用于自定义 `key-value` 集合，并提供基本的 `set`, `get`, `remove`, `iterator` 接口。例如：

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

基于 `cjson` 实现的一套对象序列化的基础库，结合 `jsoncgen` 可以发挥强大的生产力。可以参考 `cjson_serialization.json` 的内容如下：

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

你只需要利用 `json` 定义对象的描述文件（ `cjson_serialization.json` ），然后使用 `jsoncgen` 即可生成对应的实现代码：

    python jsoncgen.py -f cjson_serialization.json

有关 `jsoncgen` 的详细用法，可以参考 [后续章节](../jsoncgen.md)。

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

基于 `ngx_rbtree_t` 实现的共享内存字典。核心实现来源于 [`lua-nginx-module`](https://github.com/openresty/lua-nginx-module) （`src/ngx_http_lua_shdict.c`）。

nginx 本身对于共享内存进行了封装，提供了如下操作共享内存的基本接口：

	ngx_shm_zone_t * ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name, size_t size, void *tag);
	void ngx_slab_init(ngx_slab_pool_t *pool);
	void * ngx_slab_alloc(ngx_slab_pool_t *pool, size_t size);
	void ngx_slab_free(ngx_slab_pool_t *pool, void *p);

但即便有这些接口，编写基于共享内存的 nginx 模块依然需要大量的代码，其中不少是模式化的工作，具体可以参考：  

* `ngx_http_limit_req_module.c`
* `ngx_http_limit_conn_module.c`

`lib_ngx_shm_dict` 将这些模式化的工作进行了抽象，封装成统一的 `key-value` 操作接口，使得编写基于共享内存的 nginx 模块变得简单。
  
`lib_ngx_shm_dict` 支持的基本类型包括：  

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

`ngx_shm_dict_code.py` 用于生成 `ngx_shm_dict_code.h` 以及 `ngx_shm_dict_code.c`，提供所有基本类型的读写接口。以 `int8_t` 为例，生成的接口定义如下：

    int ngx_shm_dict_get_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t * val);
    int ngx_shm_dict_add_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t val, uint32_t expire);
    int ngx_shm_dict_safe_add_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t val, uint32_t expire);
    int ngx_shm_dict_replace_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t val, uint32_t expire);
    int ngx_shm_dict_set_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t val, uint32_t expire);
    int ngx_shm_dict_safe_set_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t val, uint32_t expire);
    int ngx_shm_dict_incr_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t delta, uint32_t expire, int8_t * result);
    int ngx_shm_dict_decr_int8(ngx_shm_zone_t * zone, ngx_str_t * key, int8_t delta, uint32_t expire, int8_t * result);
其他数据类型的接口定义类似。

`ngx_shm_dict.h` 提供了对于共享内存字典的初始化以及批量操作的接口：

    ngx_shm_zone_t * ngx_shm_dict_init(ngx_conf_t * cf, ngx_str_t * name, size_t size, void * module);
    int ngx_shm_dict_delete(ngx_shm_zone_t * zone, ngx_str_t * key);
    int ngx_shm_dict_traverse(ngx_shm_zone_t * zone, ngx_shm_dict_tarverse_cb cb, void * data);
    int ngx_shm_dict_get_total_keys(ngx_shm_zone_t * zone, uint8_t include_expires, size_t * total);
    int ngx_shm_dict_get_keys(ngx_shm_zone_t * zone, uint8_t include_expires, ngx_pool_t * pool, ngx_shm_dict_keys_t * keys);
    int ngx_shm_dict_flush_all(ngx_shm_zone_t * zone);
    int ngx_shm_dict_flush_expired(ngx_shm_zone_t * zone, int attempts);
`ngx_shm_dict_test.py` 用于生成 `ngx_shm_dict_test.h` 以及 `ngx_shm_dict_test.c` ，提供测试接口：

    ngx_int_t ngx_shm_dict_test_read(ngx_shm_dict_read_args_t * args, ngx_http_request_t * r);
    ngx_int_t ngx_shm_dict_test_write(ngx_shm_dict_write_args_t * args);
    ngx_int_t ngx_shm_dict_test_incr(ngx_pool_t * pool, ngx_shm_dict_mod_args_t * args);
    ngx_int_t ngx_shm_dict_test_decr(ngx_pool_t * pool, ngx_shm_dict_mod_args_t * args);
关于 `ngx_shm_dict` 的详细使用方式，可以参考 [`hustdict`](../../appendix/hustdict.md)。
   
[上一级](../lib_hustngx.md)

[回首页](../../index.md)