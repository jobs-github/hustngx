deploygen
--

`deploygen` is a standalone tool for deployment, using `json` as schema to generate **one-key deployment script** for machines defined in `hosts`.  

### Usage ###

    usage:
        python deploygen.py [user] [cmd_file] [host_file] > [output]
    sample:
        python deploygen.py cmd.json hosts > deploy.sh
        python deploygen.py jobs cmd.json hosts > deploy.sh


### Configuration ###

You can write `host_file` like this:  

    192.168.1.101
	192.168.1.102
	192.168.1.103

Fill IP or domain for each line, and split them by newline.  

A complete example of configuration file:  

    [
	    {
	        "key": "ssh",
	        "value":
	        [
	            "cd /data/",
	            "test -d tmp || mkdir -p tmp",
	            "cd /var/www/html/",
	            "test -f status.html || echo \"ok\" > status.html"
	        ]
	    },
	    {
	        "key": "scp",
	        "value":
	        [
	            { "local": "nginx.tar.gz", "remote": "/data/tmp/nginx.tar.gz" },
	            { "local": "upgrade.sh", "remote": "/data/tmp/" }
	        ]
	    },
	    {
	        "key": "ssh",
	        "value":
	        [
	            "cd /data/tmp/",
	            "rm -rf nginx",
	            "tar -zxf nginx.tar.gz -C .",
	            "cd /data/tmp/nginx/",
	            "sh Config.sh",
	            "make -j",
	            "make install",
	            "cd /data/tmp/",
	            "rm -rf nginx",
	            "rm -f nginx.tar.gz"
	        ]
	    }
	]

### Structure ###

[`key`](deploygen/key.md)  
[`value`](deploygen/value.md) => [`scp_cmds`](deploygen/scp_cmds.md)　`or`　[`ssh_cmds`](deploygen/ssh_cmds.md)  
　　　　　　[`scp_cmd`](deploygen/scp_cmd.md)  

### Limitation ###

* If [`key`](deploygen/key.md) is set as `scp` , then [`value`](deploygen/value.md) could only be set as [`scp_cmds`](deploygen/scp_cmds.md).  

* If [`key`](deploygen/key.md) is set as `ssh` , then [`value`](deploygen/value.md) could only be set as [`ssh_cmds`](deploygen/ssh_cmds.md). 

### Special tags ###

* `@index_place_holder` : please refer to [`ssh_cmds`](deploygen/ssh_cmds.md) 

[Previous](index.md)

[Home](../index.md)