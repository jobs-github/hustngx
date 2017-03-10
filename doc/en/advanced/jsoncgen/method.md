method
----------

**Type:** `array`

**Value:** `["<method-str>", "<method-str>", ... , "<method-str>"]`

**Attribute:** Refer to [methods](methods.md)

**Parent:** [methods](methods.md)

Define the supported methods. `<method-str>` can be values as below:  

-  `set` : assignment operator of object (deepcopy)

- `eq` : to determine whether the value is equal

- `dispose` : to dispose resource of object (memory)

- `serialize` : to encode user-defined object to `json` object

- `deserialize` : to decode user-defined object from `json` object

- `dump` : to encode user-defined object to `json` string

- `loads` : to decode user-defined object from `json` string

- `save` : to encode user-defined object to `json` string and save it to file

- `load` : to load `json` string from file and decode it to user-defined object 

[Previous](../jsoncgen.md)

[Home](../../index.md)