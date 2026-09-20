# perf / Flame Graph 自审交接

日期：2026-09-19。审查类型为作者自审，独立技术与页面验收由主代理完成。本章新增 `content/topics/performance/perf-flamegraph.md`、`examples/perf-flamegraph.cpp` 和本报告；未修改全局 manifest、roadmap 或测试。另按主代理要求修正已验收 UDP 交接中的一句题数笔误。

## 内容与来源

本章重新读取 cpp-quant-writing skill、WRITING_STYLE、CONTENT_STANDARD、CONTRIBUTING、schema 与 taxonomy。正文按 11 节契约组织，15 道 derived 题，L1/L2/L3 各 5 道，全部有参考答案和至少两个评分点，companies 为空。

12 条 references 均在本次用浏览工具实际打开。perf_event_open 用于计数布局、事件作用范围和复用时间；kernel perf-security 与 ring buffer 文档用于权限条件和采样记录路径；perf stat/record/report/script 手册用于采集、栈展开、自身与子调用归因及导出。Brendan Gregg 的 CPU Flame Graphs、Off-CPU Analysis 及原作者 FlameGraph 仓库用于图形语义和等待分析；GCC 优化选项用于 frame pointer 选项的保证边界。

主代理指出软件 task-clock 的更新路径不能从 exclude_kernel 属性直接推断为纯用户态时间。随后实际打开并核对 Linux v6.8 `kernel/events/core.c` 的 `task_clock_event_update/start/read`：累加上下文时钟差值，更新路径未按权限级拆分。正文标注该源码版本与本机 WSL 内核不同，仅用它解释不能作出的推论，没有把它伪称为本机精确源码。

## 工作负载与数值检查

输入为 4096 个 uint32_t，由固定表达式生成。算术阶段每元素 24 次混合，gather 阶段用 `(i * 73) & 4095` 索引。索引始终在输入范围内，salt 为 0..3，uint32_t 运算有意取模；每阶段和总 checksum 在最大 2048 轮下都不超过 uint64_t 上限。没有依赖 signed overflow 或 volatile 伪造优化障碍。

四个单轮结果通过独立 Python 整数代码计算，并在 C++ 中作为固定常量校验。实际运行的 Python 核心如下，循环不是从 C++ 的输出生成常量：

```python
a = [((i * 2654435761) & 0xffffffff) ^ 0xa5a5a5a5 for i in range(4096)]
out = []
for salt in range(4):
    s = 0
    for v in a:
        x = v ^ salt
        for j in range(24):
            x = ((x ^ (x >> 13)) * 1664525 + 1013904223) & 0xffffffff
        s += x
    g = sum(a[(i * 73) & 4095] ^ (salt + i) for i in range(4096))
    out.append((s, g, s + g))
print(out)
```

实际得到总值 `17396735839208`、`17684914805289`、`17535516519958`、`17574288582265`。默认 32 轮为 `561531645973760`，最大 2048 轮为 `35938025342320640`。主代理另行独立复算通过，该复算不计入本作者自测。

GNU noinline 仅为本例保留两个阶段的函数边界。`nm -C` 实际显示 arithmetic_phase、gather_phase 和 run_workload 的局部文本符号，另有 run_workload 的 cold clone；符号存在不证明采样链完整。没有将这个小工作集称为生产代表负载。

## 计数和失败路径

TaskClock 的 fd 创建成功后无复制/移动，析构关闭；ioctl/read 或工作校验抛出后栈展开释放资源。close 不重试。计数先禁用，预热在构造计数器前完成，start 执行 RESET/ENABLE，finish DISABLE 后读取 3 个 u64，检查完整长度。调用窗口包含少量控制代码和 checksum 比较，未称为仅工作函数内部的纯成本。

缩放先转 long double 再相乘，避免 uint64_t 中间乘积溢出。功能自测覆盖 running=0、running=enabled、50% 运行比例、大数及 running>enabled 拒绝；它只测试计算规则，没有制造真实 PMU 复用。单次软件事件实际返回 enabled=running，不作为硬件无复用的一般保证。

EACCES、EPERM、ENOSYS、ENOENT、ENODEV、EOPNOTSUPP、EINVAL 被作为本配置不可用时的可跳过路径，打印具体 errno。按主代理意见将函数命名改成 unavailable_configuration，并明确 EINVAL 可能是参数错误，不证明机器缺失能力。其他打开错误及 ioctl/read 异常使程序返回失败。没有修改内核权限设置。

本机实际成功计数，没有触发 SKIP，也没有注入 ioctl、read、close 失败。因此报告仅对这些路径给出代码审查结论，不声称完成系统调用故障注入。未实现采样 ring buffer、硬件计数或 off-CPU 追踪。

## 实际执行证据

环境：x86-64，WSL2 `6.18.33.2-microsoft-standard-WSL2`；GCC `13.3.0`，Ubuntu 编译器包 `13.3.0-6ubuntu2~24.04.1`；`perf_event_paranoid=2`。`command -v perf` 未找到路径，`perf --version` 输出 `command not found`。未安装工具或放宽权限，未运行正文中 perf record/script/FlameGraph 的条件性流程，没有 perf.data 或 SVG 产物。

实际命令由 PowerShell 调用 WSL bash 执行，下面保留 Linux 部分。编译产物放在独有临时目录，未跑共享全量 harness。

```bash
mkdir -p /tmp/cpp-quant-perf-review
src=/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/perf-flamegraph.cpp
g++ -std=c++20 -O2 -g -fno-omit-frame-pointer \
  -Wall -Wextra -Wpedantic -Werror "$src" -o /tmp/cpp-quant-perf-review/demo
/tmp/cpp-quant-perf-review/demo
/tmp/cpp-quant-perf-review/demo 1
/tmp/cpp-quant-perf-review/demo 2048
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  "$src" -o /tmp/cpp-quant-perf-review/demo-sanitize
/tmp/cpp-quant-perf-review/demo-sanitize
nm -C /tmp/cpp-quant-perf-review/demo | grep -E 'arithmetic_phase|gather_phase|run_workload'
```

上述正常构建的默认、1 和 2048 轮全部通过。另逐个执行参数 `0`、`2049`、`-1`、`1x`，均输出范围错误并非零退出。程序内测试还检查空串、前导空格及超出 unsigned 可表示范围的长数字。ASan/UBSan 默认运行通过，无报告。

修正 SKIP 消息、辅助函数命名和软件时钟注释之后，再次完成严格普通编译与默认执行、ASan/UBSan 编译与默认执行，均通过。最终普通执行记录 `raw=2575900 enabled_ns=2575900 running_ns=2575900`，sanitizer 执行为 `2945400`；这两个值只用于确认功能路径，不能跨构建计算性能差异。正文保留第一次普通运行 `2621000` 的真实观察值并说明其局限。

## 文字与题目自审

逐题核对计数/采样区别、宽度权重、非时间轴、off-CPU、符号与展开、复用缩放、样本偏差和因果验证。L3 包含分母变化、尾延迟、权限限制、事件组及诊断构建取舍，避免把热点定义换个说法重复出题。

正文明确外部 perf record 覆盖整个进程，与内部 enable/disable 区间不同；CPU 图不能填补稀有慢路径或等待数据。按主代理意见补上闭环压测适用前提：目标为独立外部到达流时闭环会改变需求，依赖上次响应的业务本来就可采用闭环模型。

应用写作 skill 通读后，保留必要技术纠错，去掉空泛强调和机械收尾。相关链接指向当前存在的 cache、NUMA、去虚拟化及 mutex/condition_variable 专题。格式与目标 schema 校验在冻结前完成，最终 hash 由交付消息提供；页面与全量验收由主代理记录。
