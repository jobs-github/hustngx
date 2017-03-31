`sync_file`
----------

**Type:** `object`

**Value:** `["src": <path>, "des": <path>]`

**Attribute:** Required

**Parent:** [sync_files](sync_files.md)

Defines files need to be synchronized. Tag `src` represents information of source file, while `des` represents information of destination file. Both of them use [path](path.md) as value.  

`sync` will copy file from [path](path.md) in `src` to [path](path.md) in `des`.  

[Previous](../sync.md)

[Home](../../index.md)