sync
--

`sync` is a standalone tool to keep files and code in sync

### Usage ###

    usage:
        python sync.py [conf]
        python sync.py [src_dir] [des_dir]
    sample:
        python sync.py sync.json
        python sync.py addon/ ../../nginx/src/addon/

`sync` will perform different actions according to the number of input arguments:  

- If input 2 arguments, it will perform synchronization by configuration  

- If input 3 arguments, it will perform synchronization by configuration from `src_dir` to `des_dir` (the same level)  

### Configuration ###

A complete example of configuration file:  

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

### Structure ###

[`sync_files`](sync/sync_files.md)  
　　[`sync_file`](sync/sync_file.md)  
　　　　[`path`](sync/path.md)  
　　　　　　[`prefix`](sync/prefix.md)  
　　　　　　[`files`](sync/files.md)  
　　　　　　[`suffix`](sync/suffix.md)  

[Previous](index.md)

[Home](../index.md)