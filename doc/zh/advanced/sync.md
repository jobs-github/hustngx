sync
--

`sync` 是一个独立的文件和代码同步工具。

### 使用范例 ###

    usage:
        python sync.py [conf]
        python sync.py [src_dir] [des_dir]
    sample:
        python sync.py sync.json
        python sync.py addon/ ../../nginx/src/addon/

`sync` 根据输入参数的个数执行不同的行为：

- 如果输入参数是 2 个，则读取配置文件的内容进行同步

- 如果输入参数是 3 个，则将 `src_dir` 文件夹的内容同步至 `des_dir` （平级同步）

### 配置范例 ###

以下是一个完整的 `json` 配置文件，包含 `sync` 所支持的所有字段：

    [
	    {
	        "src":
	        {
	            "prefix": "../lib_hustngx/nginx/src/core/",
	            "files":
	            [
	                "c_dict.c",
	                "c_dict.h"
	            ]
	        },
	        "des":
	        {
	            "prefix": "../../",
	            "files":
	            [
	                "hustdb/ha/",
	                "hustmq/ha/",
	                "hustlog/"
	            ],
	            "suffix": "nginx/src/core/"
	        }
	    },
	    {
	        "src":
	        {
                "prefix": "../",
	            "files": 
	            [
	                "lib_hustngx/",
                    "patch/",
                    "third_party/"
	            ],
                "suffix": "nginx/"
	        },
	        "des":
	        {
	            "files":
	            [
	                "../hustmqha/nginx/"
	            ]
	        }
	    }
	]

### 字段结构 ###

[`sync_files`](sync/sync_files.md)  
　　[`sync_file`](sync/sync_file.md)  
　　　　[`path`](sync/path.md)  
　　　　　　[`prefix`](sync/prefix.md)  
　　　　　　[`files`](sync/files.md)  
　　　　　　[`suffix`](sync/suffix.md)  

[回上页](index.md)

[回首页](../index.md)