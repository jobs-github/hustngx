`scp_cmd`
----------

**Type:** `object`

**Value:** `["local": <file-url>, "remote": <file-url>]`

**Attribute:** see [`scp_cmds`](scp_cmds.md)

**Parent:** [`scp_cmds`](scp_cmds.md)

Description on files used by `scp`. `local` means local file, `remote` means remote path.  

This command will be interpreted as action to copy from `<file-url>` set in `local` to path set in `remote`.  

**Now it only support a single file. If you need to copy folder, please tar it to a file.**  

[Previous](../deploygen.md)

[Home](../../index.md)