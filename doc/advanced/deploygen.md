deploygen
--

`deploygen` 是一个独立的代码部署工具，以 `json` 作为描述语言，生成 `hosts` 所定义的所有机器的 **一键部署脚本** 。

### 使用范例 ###

    usage:
        python deploygen.py [user] [cmd_file] [host_file] > [output]
    sample:
        python deploygen.py cmd.json hosts > deploy.sh
        python deploygen.py jobs cmd.json hosts > deploy.sh


### 配置范例 ###

`host_file` 文件的编写方法如下：

    192.168.1.101
	192.168.1.102
	192.168.1.103

每行可填写 IP 地址或者域名，各行之间通过回车分割。

以下是一个完整的 `json` 配置文件，包含 `deploygen` 所支持的所有字段：

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

### 字段结构 ###

[`key`](deploygen/key.md)  
[`value`](deploygen/value.md) => [`scp_cmds`](deploygen/scp_cmds.md)　`or`　[`ssh_cmds`](deploygen/ssh_cmds.md)  
　　　　　　[`scp_cmd`](deploygen/scp_cmd.md)  

### 字段约束 ###

* [`key`](deploygen/key.md) 如果定义为 `scp` ，则 [`value`](deploygen/value.md) 只能是 [`scp_cmds`](deploygen/scp_cmds.md)。

* [`key`](deploygen/key.md) 如果定义为 `ssh` ，则 [`value`](deploygen/value.md) 只能是 [`ssh_cmds`](deploygen/ssh_cmds.md)。

### 特殊标签 ###

* `@index_place_holder` : 具体使用方法可参考 [`ssh_cmds`](deploygen/ssh_cmds.md) 

[上一级](index.md)

[根目录](../index.md)