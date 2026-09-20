# 分支预测自审交接

日期：2026-09-19。类型为作者自审，独立技术与页面验收由主代理负责。仅新增本章正文、`examples/branch-prediction.cpp` 与本文件；未修改共享 manifest、roadmap 或 harness。

## 内容契约与资料

沿用本次已完整读取的 cpp-quant-writing skill，并在本章再次读取其工作流程、WRITING_STYLE 和 CONTENT_STANDARD。正文 11 节，15 道 derived 题，L1/L2/L3 各 5 道，所有题目有答案和评分点、companies 为空。

9 条 references 均实际打开：GCC 13.3 优化与插桩选项；Clang likely 属性与 PGO 指南；LLVM 向量化；C++20 N4861 的 likelihood、条件运算符和内建逻辑与；Intel 的 Hardware Features and Behavior Related to Speculative Execution。引用的 GCC 版本对应本地编译器，其他实现资料未被表述为 C++ 保证。

调查时也打开了 Intel 优化手册索引，但大 PDF 请求因大小限制失败；另一个 SDM 下载入口重定向失败。这些下载内容未作为已读依据，也没有加入 references。Intel 的可读技术文章直接支持条件方向和间接目标预测区别，以及错推测的恢复说明。没有补造通用预测器内部容量或固定周期惩罚。

## 程序自审

输入大小固定 16384，uint16_t 值 0..255 各 64 次。生成循环在 256 终止，远低于 uint16_t 上限。xorshift32 的左移、异或有定义明确的无符号语义，初始种子非零；置换循环只在 size>1 执行，取模除数非零，交换索引位于当前前缀中。代码只承诺固定算法的可复现置换，不承诺均匀随机样本。

三种实现只读取有效 span 范围。mask 从 uint64_t 的 0 减布尔值形成 0 或全 1，避免 signed overflow；源值转成 uint64_t 后进行按位与。最大本例输入总和远低于 uint64_t 上限。阈值 uint32_t 能表达大于所有 uint16_t 值的界限，不会把 65536 截断成零。

oracle 使用等差和与重复次数，独立于逐元素 if/select/mask 路径。所有阈值 0..256 均核对有序与打乱输入；直方图检查每值重复次数恰为 64，并验证两份置换重复运行一致且不同于原顺序。边界包括空输入、单元素 65535、阈值 65535/65536、0/1/127/128/255 混合和最大 uint32_t 阈值。

静态期望值额外核对总和 `2088960`、阈值 128 的 `1568768`、阈值 255 的 `16320`。vector 管理所有权，异常由 main 捕获并非零退出；没有手动分配、外部流量、线程或无限重试。分配失败未注入，仅审查 RAII 路径。

## 实际构建与运行

平台为 x86-64 WSL2，内核 `6.18.33.2-microsoft-standard-WSL2`，GCC `13.3.0`。未使用 `-march=native`，未安装 clang 或运行 Clang 编译。作者实际执行以下 Linux 命令；由 PowerShell 调用 WSL bash，产物位于独有临时目录。

```bash
mkdir -p /tmp/cpp-quant-branch-review
src=/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/branch-prediction.cpp
g++ -std=c++20 -O2 -g -Wall -Wextra -Wpedantic -Werror \
  "$src" -o /tmp/cpp-quant-branch-review/demo
/tmp/cpp-quant-branch-review/demo
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  "$src" -o /tmp/cpp-quant-branch-review/demo-sanitize
/tmp/cpp-quant-branch-review/demo-sanitize
g++ -std=c++20 -O3 -Wall -Wextra -Wpedantic -Werror \
  "$src" -o /tmp/cpp-quant-branch-review/demo-O3
/tmp/cpp-quant-branch-review/demo-O3
```

O2、O3 和 ASan/UBSan 均通过；sanitizer 无报告。程序输出：

```text
values=16384 thresholds=257 implementations=3
threshold=128 sum=1568768 histogram=equal correctness=OK
No timing or branch-miss result is measured. Inspect optimized assembly.
```

一次单独调用 O3 编译时临时目录已不存在，链接器报 cannot open output file；随后在同次命令中先 mkdir 再编译和运行，通过。该失败来自输出目录，不是源码编译诊断。没有据此修改程序。

## 汇编实际观察

实际执行了三套汇编生成命令，并通过 Python 按具名函数提取读取相应汇编体。

```bash
g++ -std=c++20 -O2 -S -masm=intel "$src" \
  -o /tmp/cpp-quant-branch-review/demo-O2.s
g++ -std=c++20 -O3 -S -masm=intel "$src" \
  -o /tmp/cpp-quant-branch-review/demo-O3.s
g++ -std=c++20 -O2 -fno-if-conversion -fno-if-conversion2 \
  -fno-tree-vectorize -S -masm=intel "$src" \
  -o /tmp/cpp-quant-branch-review/diagnostic.s
```

O2 的 sum_if 和 sum_select 均为 `cmp esi, edx` 后 `cmovnb rcx, rax`；mask 版本通过 `cmovb` 把不满足条件的源值置零。循环回边的 jne 位于输入指针和结束指针比较之后。不能从其存在推导数据阈值条件仍有跳转。

O3 三个主循环出现 SSE 比较、掩码和整数累加，包含 `pcmpgtd`、`pand`/`pandn`、`paddq`；尾部为标量 cmov 路径，并有长度/回边控制跳转。正文没有称整个函数绝对无分支。

诊断选项关闭 if-conversion 和树向量化后，sum_if 在数据比较后出现 `jb` 跳过累加。本次只生成和读取这份诊断汇编，没有运行诊断二进制、计时或采集 PMU。普通 O3 可执行版本另行运行通过，不能把它与诊断选项混淆。

主代理另外独立严格编译及 ASan/UBSan 执行通过，并独立生成和读取 O2 汇编，复核上述 cmov 与回边区别。这是主代理反馈的独立证据，不冒充作者本人另一次测试。

## 测量边界与中文自审

本章仅做功能校验与编译产物观察。没有耗时、branch-miss、IPC 或性能倍率。本机 perf CLI 缺失已在上一章调查，本章不从工具缺失推断所有 PMU 不可用，也没有声称已实际尝试本章硬件事件。相关流程和指标写作后续验证方案。

逐题检查了语义与指令层区别、比例与序列区别、方向与目标区别、likely 的规范边界及 PGO 代表性。L3 保留业务重排约束、端到端测量、诊断构建范围、训练与验证分离，以及冷路径内存安全。没有通过微基准规则删掉错误处理。

按 cpp-quant-writing 通读，删去无依据的性能评价，保留必要条件、具体输入与指令观察。代码中的无符号术语没有在润色时换成模糊描述，未加入公司题目来源。格式、schema 和 lint 在交付前核对，最终 hash 在交付消息报告；主代理负责全量与页面验收。
