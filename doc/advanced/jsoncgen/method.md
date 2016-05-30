method
----------

**类型:** `array`

**值:** `["<method-str>", "<method-str>", ... , "<method-str>"]`

**属性:** 参考 [methods](methods.md)

**父节点:** [methods](methods.md)

描述结构体所支持的方法，其中 `<method-str>` 支持的方法包括：

-  `set` : 对象的赋值操作（深拷贝）

- `eq` : 判定两个对象是否相等

- `dispose` : 对象资源的清理（内存）

- `serialize` : 将自定义对象序列化成 `json` 对象

- `deserialize` : 将 `json` 对象反序列化成自定义对象

- `dump` : 将自定义对象编码成 `json` 字符串

- `loads` : 将 `json` 字符串解码成自定义对象

- `save` : 将自定义对象编码成 `json` 字符串并保存到文件中

- `load` : 从文件读取 `json` 字符串并解码成自定义对象

[上一级](../jsoncgen.md)

[根目录](../../index.md)