# timer-wheel 自审与交接

日期：2026-09-19。仅涉及本章三个文件。应用 cpp-quant-writing、WRITING_STYLE 和 CONTENT_STANDARD。taxonomy 为 design / Algorithm Coding、System Design。主代理负责独立验收与共享 manifest，本报告记录作者自审及收到的复验结果。

## 内容与代码核查

- 正文 11 节；15 题，L1/L2/L3 各 5。问题覆盖取模、量化、完整一轮、取消、同期限顺序、长任务、双链摘除、补进度、回调隔离、真实复杂度、负载选择、集中到期、时钟接入、周期和停机。无公司来源或归属。
- canonical 是单线程逻辑模型，四桶、八槽；绝对 deadline 保留轮次信息，未把同桶误认为同时到期。只接受严格未来期限，已有或过去期限明确失败。
- 桶链为双链，句柄直接定位；空闲链与 active 桶链互斥。release 先修改桶链，再使用 next 归还；advance 保存原 next 后提取并释放当前槽，避免遍历丢项。
- 句柄绑定存活 wheel 的 owner/index/id；wheel 不可移动。id 在本实例内单调递增，抵达最大值前停止分配、不回绕；不同实例并不共享 id 域。池寿命外使用不在检测合同内。
- 取消和 stop 只处理 active 任务，不能撤回已返回 Batch。Event 为整数值副本；token 可由业务复用，不拥有对象，也不证明对象仍 live。无回调执行、并发访问或线程停止实验。
- advance_one 每次恰走一个逻辑 tick。真实接入的时钟、epoch、量化、补进度及 callback 队列未实现，正文明确解释不能直接跳过中间桶。
- 时间量化经独立审查修正：d=deadline−epoch、now_time=clock_now−epoch，先检查非负及可表示，再 ceil/floor；取整反例明确 epoch=0。
- 注册和取消元数据 O(1)，advance 扫描桶并产生事件；完整 Batch 初始化和可能返回复制带来 O(N) 成本。长任务每圈重扫，未冒充分层时间轮或常数时间到期处理。
- stop 清 active 并拒绝未来注册；不把 pending=0 当作业务已完成。相同期限无 FIFO 合同，oracle 按集合比较。
- 中文自审保留版本和条件，避免夸大精度、固定延迟承诺和聊天过程残留。表格内无未转义管道运算符。

## 编译和运行证据

WSL2 x86-64 / GCC 13.3。作者初版与新增全容量到期测试后的终版均运行以下配置并退出 0：

```text
-std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror
-std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie
```

输出一致：

```text
oracle: 24 ticks, 8 exact emissions, 3 simultaneous at tick 9
boundaries: stale/cross-wheel/closed rejected; tick limit does not wrap
logical ticks only; no wall-clock latency or callback execution measured
```

独立参考表直接列出八个 deadline/token，逐 tick 扫描，不使用轮算法的取模或链表；24 个 tick 全部核对数量、完整排序 token 集合、期限、输出 tick 和 pending。取消 token 4 后不再出现，复用槽加入 token 99；tick 9 三项同时到期。满、非未来、默认、跨 wheel、陈旧、重复和已输出句柄的失败路径均有断言。

终版新增八项全部设 tick 1 的测试，检查 count=8、pending=0、token 和为 36。此新增用例只作数量与求和检查，不能称为独立完整集合比对；完整集合比对由前述逐 tick oracle 提供。

从 UINT64_MAX−1 初始化 near_limit，真实执行到最大 tick 并输出最后任务，再推进失败，now 不回绕。id 最大值拒绝分支只做代码边界分析，未实际执行直至 uint64_t id 耗尽的注册过程。

主代理逐行预审、普通严格构建及 ASan/UBSan 均通过，后又审读并复跑新增的八任务同 tick 测试。正文和题目独立审查只要求精确 epoch 偏移与实例 id 域，已修正；实现未因此变化。

未使用 sleep，没有 wall-clock 或性能测量，未跑 TSan。断言必须启用；测试包含 assert 内操作。Sanitizer 无报告只对应实际有限执行，不证明所有输入或调度。

## 来源核查

2026-09-19 实际打开：

- Varghese/Lauck 1997 论文 PDF，核对算法家族、计时设施操作划分、绝对期限比较和链节点直接取消。论文全文成功打开，后来指定深页读取超时；未以未读页中的细节作结论。references 按当前 schema 使用 manual 类型，标题明确其论文身份。
- Netty 4.1.138.Final HashedWheelTimer 和 Timeout 官方 API：近似超时、tick 参数及具体取消语义。没有把 Java 实现线程或默认值套到本例。
- Linux kernel hrtimers 官方设计说明：文中含历史实现背景，只用于精度需求与结构选择的取舍，不宣称当前所有 Linux 时间轮都与此完全一致。
- Linux man-pages 6.19 timerfd_create：CLOCK_MONOTONIC / CLOCK_BOOTTIME 的挂起差异、epoll 接入、read 返回累计到期数。本示例没有运行这些系统调用。
- C++20 N4861 steady_clock：不倒退的时点语义，不据此推导线程调度保证。

首次尝试打开 kernel timers-howto 和 Netty 所链原论文地址失败，未列作已核查来源；通过检索找到 Columbia 托管的原论文全文并实际打开。来源均在正文 references 中保存链接与日期。

## 格式与交接

正文初次格式检查：schema 本章 errors=[]，11 个 H2、15 题各层 5；remark-gfm 解析四张表，列数均一致。Markdownlint 当时扫描 82 个 Markdown，无问题。最终 epoch/id 修订后再次校验，仍为 errors=[]、11 节、15 题、四张表列数一致；Prettier 正文与报告通过，Markdownlint 扫描 83 个 Markdown 无问题。

仅更改 content/topics/design/timer-wheel.md、examples/timer-wheel.cpp、reviews/timer-wheel-review.md。未改共享 manifest、roadmap、测试脚本或已冻结章节。正文、代码和本报告交付后等待主代理确认冻结。
