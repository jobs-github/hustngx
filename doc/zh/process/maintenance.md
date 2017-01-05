Nginx Module Maintenance Process
--

### 工序流程图 ###
![architect](maintenance.png)

### 工序流程要点说明 ###

- 参考 [Nginx Module Development Process](development.md) 的要点说明。

### 代码同步 ###

假定现有的工程叫做 `hustmqha`，目录位于 `/data/src/hustmqha` 下，实现阶段中， **同步生成的新代码至项目** 通常可用如下方法完成（以 `hustngx` 生成的代码为例）：

1. 更新 `ngx_wizard` [配置文件](../advanced/ngx_wizard.md)。
2. 用 `hustngx` 以及更新过的配置文件生成 **全新的工程目录** ，假定为 `/data/tools/hustngx/hustmqha` 。
3. 用文本比对工具（例如 `Beyond Compare`）对比 `/data/tools/hustngx/hustmqha` 以及 `/data/src/hustmqha` 的差异，并将 `/data/tools/hustngx/hustmqha` 新增的部分 **逐行同步** 至 `/data/src/hustmqha` 。
4. 如果需要增加新的命令处理接口，重复如上步骤。

[回上页](index.md)

[回首页](../index.md)