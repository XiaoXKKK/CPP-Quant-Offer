---
{
  'schemaVersion': 1,
  'id': 'perf-flamegraph',
  'title': 'perf / Flame Graph：从采样归因到优化验证',
  'description': '区分计数、CPU 采样与等待时间，检查符号和调用栈质量，解释火焰图宽度、复用缩放与测量偏差，并用固定输入工作负载验证分析条件。',
  'category': 'performance',
  'areas': ['Performance Engineering', 'Low-Latency Programming'],
  'tags': ['perf', 'flamegraph', 'profiling', 'sampling', 'benchmark'],
  'difficulty': 'L2',
  'roles':
    [
      'C++ Developer',
      'Quant Developer',
      'Low-Latency C++ Developer',
      'Trading Infrastructure Engineer',
    ],
  'companyTypes': ['高频交易', '量化私募', 'Trading Firm'],
  'status': 'published',
  'updated': '2026-09-19',
  'reviewed': '2026-09-19',
  'standard': 'C++20',
  'estimatedMinutes': 55,
  'prerequisites': ['优化构建与调试符号', 'CPU 时间与经过时间', 'Linux 文件描述符'],
  'related':
    [
      'cpu-cache-false-sharing',
      'numa-first-touch',
      'vtable-devirtualization',
      'mutex-condition-variable',
    ],
  'demo':
    {
      'file': 'examples/perf-flamegraph.cpp',
      'platform': 'linux',
      'exercise': '在可用 perf 的 Linux 环境固定同一二进制与 2048 轮输入，分别用 frame pointer 和 DWARF 获取用户态调用栈，比较 unknown/截断比例、采样数与采集开销。记录失败原因；没有真实样本时不要补画火焰图。',
    },
  'references':
    [
      {
        'title': 'Linux v6.8 task-clock implementation',
        'url': 'https://raw.githubusercontent.com/torvalds/linux/v6.8/kernel/events/core.c',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux perf_event_open(2)',
        'url': 'https://man7.org/linux/man-pages/man2/perf_event_open.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux kernel: Perf events and tool security',
        'url': 'https://docs.kernel.org/admin-guide/perf-security.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux kernel: Perf ring buffer',
        'url': 'https://docs.kernel.org/userspace-api/perf_ring_buffer.html',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux perf-record(1)',
        'url': 'https://man7.org/linux/man-pages/man1/perf-record.1.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux perf-stat(1)',
        'url': 'https://man7.org/linux/man-pages/man1/perf-stat.1.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux perf-report(1)',
        'url': 'https://man7.org/linux/man-pages/man1/perf-report.1.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux perf-script(1)',
        'url': 'https://man7.org/linux/man-pages/man1/perf-script.1.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Brendan Gregg: CPU Flame Graphs',
        'url': 'https://www.brendangregg.com/FlameGraphs/cpuflamegraphs.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Brendan Gregg: Off-CPU Analysis',
        'url': 'https://www.brendangregg.com/offcpuanalysis.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'FlameGraph original implementation and workflow',
        'url': 'https://github.com/brendangregg/FlameGraph',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC optimization options',
        'url': 'https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'perf-flamegraph-q01',
        'level': 'L1',
        'prompt': 'perf stat 与 perf record 分别能回答什么问题？',
        'answer': 'stat 汇总指定事件在测量区间的计数，例如任务 CPU 时间、指令或 cycles；record 保存采样及相关元数据，可用 report 或 script 按符号、调用路径归因。总计数不自带热点调用栈，采样分布也不能替代完整的请求延迟测量。两者都要明确事件、对象和区间。',
        'rubric': ['区分计数与采样记录', '说明事件、对象和时间区间'],
        'source': { 'kind': 'derived', 'rationale': '由计数读取与采样记录的数据路径差异推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q02',
        'level': 'L1',
        'prompt': 'CPU 火焰图中的横向宽度和纵向高度表示什么？',
        'answer': '常见等权折叠流程中，宽度对应包含该调用路径的样本数量；采用其他权重时应按图中定义的权重解释。高度表示栈层级。横轴通常按栈名聚合排列，不保留执行时间顺序；宽块也不直接表示单次调用耗时或调用次数。',
        'rubric': ['宽度解释包含权重前提', '指出层级与非时间轴'],
        'source': { 'kind': 'derived', 'rationale': '由火焰图折叠栈聚合方式和布局规则推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q03',
        'level': 'L1',
        'prompt': '一个线程等待锁很久，为什么 CPU 火焰图可能看不到这段等待？',
        'answer': '按 CPU 运行时间采样时，线程睡眠期间没有持续执行，不能期待等待时长以相同比例出现在 CPU 栈上。需要结合调度切换或阻塞事件及持续时间分析 off-CPU 状态。若锁通过自旋等待，线程仍在 CPU 上运行，自旋代码可能成为 CPU 热点。',
        'rubric': ['区别睡眠与自旋', '提出带时长的 off-CPU 分析'],
        'source': { 'kind': 'derived', 'rationale': '由阻塞与自旋时的调度状态和采样覆盖推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q04',
        'level': 'L1',
        'prompt': '为什么发布优化构建仍应保存对应的符号和二进制？',
        'answer': '采样最初主要记录地址，后处理需要正确的映射、二进制和符号信息才能定位函数或源码。不同构建的地址布局和内联决策会变化，不能用另一次构建的符号随意解释。保留 build ID、编译参数及对应调试文件，才能核对报告是否来自目标版本。',
        'rubric': ['说明地址解析依赖对应构建', '记录 build ID 与调试信息'],
        'source': { 'kind': 'derived', 'rationale': '由采样地址到符号和源码的解析依赖推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q05',
        'level': 'L1',
        'prompt': 'demo 的 checksum 正确，能够证明哪些结果？',
        'answer': '它证明本次有限输入上的工作结果符合已计算常量，帮助发现优化或改写破坏计算。它不能证明所有输入正确，也不能证明运行速度、热点位置或业务收益。代码打印的 task-clock 是当前有限区间的一次计数，需要独立控制实验条件才能讨论性能差异。',
        'rubric': ['限定为有限输入的功能校验', '不将功能通过等同性能结论'],
        'source': { 'kind': 'derived', 'rationale': '由固定校验值实验和性能测量的不同目标推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q06',
        'level': 'L2',
        'prompt': 'time_running 小于 time_enabled 时，如何读取计数并说明局限？',
        'answer': '先保留原始 value、time_enabled 与 time_running；running 非零时可估计 value 乘 enabled 再除 running，乘法前转为合适的宽数值类型以免整数溢出。running 为零时没有可缩放的有效运行计数。缩放假设已运行区间能代表未运行区间，阶段性负载可能破坏这一前提。',
        'rubric': ['正确公式及零分母处理', '说明复用缩放只是估计'],
        'source': { 'kind': 'derived', 'rationale': '由硬件计数器复用与计数读取字段语义推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q07',
        'level': 'L2',
        'prompt': '有函数名但调用栈断裂，应该如何选择和检查展开方式？',
        'answer': '符号解析与栈展开是不同步骤。frame pointer 模式依赖经过的调用链保留适用的帧链；仅给主程序加编译选项不能修复所有库。DWARF 模式保存部分用户栈并利用 CFI 展开，需要工具支持和匹配的展开信息，栈转储不足也会截断。比较 unknown、栈深度和采集开销后再使用报告。',
        'rubric': ['区分符号与栈展开', '说明 FP 与 DWARF 的依赖和失败条件'],
        'source': { 'kind': 'derived', 'rationale': '由 perf 两种调用栈采集方式的前提推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q08',
        'level': 'L2',
        'prompt': '采样频率翻倍后热点排序变化，如何排查？',
        'answer': '先确认输入、二进制、事件和测量阶段一致，再检查样本数、丢样、内核限频和采集开销。短任务及周期负载可能使样本不足或出现相位偏差，高频采样还可能改变被测执行。保留原始数据并重复独立运行，不能只挑一张排序符合预期的图。',
        'rubric': ['检查样本质量与测量扰动', '通过独立重复区分噪声和稳定现象'],
        'source': { 'kind': 'derived', 'rationale': '由采样频率与样本代表性之间的关系推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q09',
        'level': 'L2',
        'prompt': '函数的 children 占比很高但 self 很低，应该检查什么？',
        'answer': '高 children 表明大量样本经由这个调用者进入下层函数；self 低说明采样指令较少直接落在它自身。应展开具体子路径，区分计算、分配或库调用。调用者可能适合通过减少调用次数优化，但不能根据包含子调用的占比直接断言其函数体执行昂贵。',
        'rubric': ['区分包含子调用与自身采样', '沿具体子路径定位工作与调用次数'],
        'source': { 'kind': 'derived', 'rationale': '由 perf report 自身和子调用归因方式推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q10',
        'level': 'L2',
        'prompt': '采集到 cache-misses 样本，能否把所有样本归为同一种内存瓶颈？',
        'answer': '不能。通用事件映射取决于处理器和 PMU，采样地址也可能受 skid 影响。需要核对事件定义、支持程度、数据来源及相关指令，再结合工作集、缓存层级和 CPU 利用率形成假设。某地址出现 miss 并不能单独证明它主导了端到端延迟。',
        'rubric': ['核对 PMU 事件与采样归因精度', '结合负载和端到端指标验证'],
        'source': { 'kind': 'derived', 'rationale': '由通用 PMU 事件的硬件差异与采样位置偏差推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q11',
        'level': 'L3',
        'prompt': '热点占比从 40% 降到 20%，如何判断优化是否有效？',
        'answer': '先比较同等工作量下的绝对 CPU 时间、吞吐和延迟分布，再确认工作没有转移到其他线程或等待路径。百分比的分母会变化，新增其他开销也能使旧热点占比下降。保持输入及资源条件，重复基线与改动版本，并检验预期机制对应的计数或调用次数是否变化。',
        'rubric': ['识别百分比分母变化', '提出绝对指标和因果对照'],
        'source': { 'kind': 'derived', 'rationale': '由归一化热点比例与实际优化目标的差异推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q12',
        'level': 'L3',
        'prompt': '行情处理平均 CPU 很低但 p99.9 很高，应如何组织排查？',
        'answer': '先按请求或消息记录排队、处理和交付边界，确认尾部是否来自负载突发、调度等待或恢复路径。CPU 图覆盖的是选定运行区间，罕见慢路径可能样本不足；补充 off-CPU 或针对慢请求的追踪，并控制采集开销。若目标是模拟独立外部到达流，应保留该到达过程，避免闭环发请求漏掉排队压力；闭环压测仍适用于本身依赖上次响应的业务模型。',
        'rubric': ['按业务时间边界定位尾延迟', '处理稀有样本和 coordinated omission'],
        'source': { 'kind': 'derived', 'rationale': '由交易处理尾延迟与 CPU 采样覆盖范围推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q13',
        'level': 'L3',
        'prompt': '线上采集因权限被拒绝，怎样保留可复查的结果？',
        'answer': '记录内核、perf 版本、事件、作用对象、命令和具体错误，并区分工具缺失、权限、PMU 支持与资源不足。本章不修改安全配置。可以在已授权且条件匹配的环境复现，或使用已有监控缩小范围；没有真实栈样本就明确缺少火焰图证据，不能用人工构造数据填补。',
        'rubric': ['区分失败类别并保留环境证据', '没有数据时不伪造分析产物'],
        'source': { 'kind': 'derived', 'rationale': '由受限环境中的性能采集边界与证据要求推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q14',
        'level': 'L3',
        'prompt': '指令和 cycles 分别独立采集后，直接计算 IPC 有什么风险？',
        'answer': '两次采集可能覆盖不同阶段、调度和复用区间，结果相除未必代表同一工作片段。可用能够同时调度的事件组记录，并检查每个事件的运行比例和硬件支持。组也可能因计数器限制无法运行；即便 IPC 可信，也要结合业务吞吐和延迟解释其变化。',
        'rubric': ['说明分子分母需要一致覆盖', '检查事件组可调度性与业务指标'],
        'source': { 'kind': 'derived', 'rationale': '由 PMU 事件组调度和比例指标测量条件推导' },
        'companies': [],
      },
      {
        'id': 'perf-flamegraph-q15',
        'level': 'L3',
        'prompt': '为了得到完整调用栈而关闭所有优化，为什么可能误导判断？',
        'answer': '关闭优化会改变内联、向量化、寄存器使用和调用结构，使热点分布偏离实际发布程序。应优先在代表性的优化构建上保留调试与展开信息，核对符号质量。若为解释机制使用额外 noinline 或诊断构建，应把结果标为该构建的观察，并在真实目标构建验证收益。',
        'rubric': ['说明优化改变被测代码', '区分诊断构建与目标构建结论'],
        'source': { 'kind': 'derived', 'rationale': '由编译优化对调用图和执行成本的改变推导' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

perf 可以汇总硬件或软件事件，也可以采样执行位置和调用栈。火焰图把相同调用路径聚合起来，便于查看样本集中在哪里；常见 CPU 图的宽度代表样本权重，横轴不表示执行顺序。看到热点后，还要核对事件、栈质量、采样覆盖与绝对耗时，再通过受控改动验证它是否限制业务吞吐或延迟。睡眠等待需要额外的 off-CPU 证据。

## 核心概念

计数回答“这一段发生了多少”，采样回答“抽到的事件落在什么位置”。`perf stat` 通常读取汇总结果；`perf record` 保存采样数据，`perf report` 和 `perf script` 再做展示或导出。一个进程的指令总数不会自动告诉我们每条调用路径占了多少。[perf-stat(1)](https://man7.org/linux/man-pages/man1/perf-stat.1.html)、[perf-record(1)](https://man7.org/linux/man-pages/man1/perf-record.1.html)

| 观察方式                   | 可以支持的判断              | 需要补充的边界                 |
| -------------------------- | --------------------------- | ------------------------------ |
| task-clock 计数            | 选定任务运行的 CPU 时间量级 | 不是请求从到达到完成的经过时间 |
| cycles 或 cpu-clock 栈采样 | 指定事件下的执行路径分布    | 事件不同，权重与解释也不同     |
| 调度切换与持续时间         | 哪些栈对应阻塞或等待区间    | 需区分睡眠与已就绪但尚未运行   |
| 请求时间戳                 | 排队和各处理段的延迟        | 要保留到达过程及测量边界       |

on-CPU 表示线程正在 CPU 上运行；off-CPU 包括不在运行的时间。阻塞锁通常使线程睡眠，自旋锁仍消耗 CPU。采样 CPU 栈不能完整解释睡眠等待时长。阻塞栈、唤醒事件与调度延迟还可能分别指向等待者和造成等待的线程。[Off-CPU Analysis](https://www.brendangregg.com/offcpuanalysis.html)

## 原理深入

采样记录包含指令地址，启用调用栈采集后还包含调用链或展开所需数据。把每条栈折叠为 `入口;调用者;叶函数 权重`，相同路径的权重相加，便得到火焰图输入。具体导出工具可能按样本条数或其他权重聚合，比较两张图之前必须确认这一点。[FlameGraph 工作流](https://github.com/brendangregg/FlameGraph)

一个矩形的宽度包括经由它进入子调用的权重。顶部没有子块覆盖的部分对应这条路径上直接落在该函数的样本；同名函数若出现在不同调用路径，可能出现多个块。普通火焰图按调用关系聚合与排列，左右相邻不意味着先后执行，默认配色也不能用来读取耗时高低。需要时间顺序时应使用保留时间维度的视图。[CPU Flame Graphs](https://www.brendangregg.com/FlameGraphs/cpuflamegraphs.html)

`perf report` 的 self 与 children 分别帮助查看自身和包含子调用的归因。一个入口函数 children 很大，可能只是绝大多数工作都由它发起。需要下钻到实际路径，再考虑减少调用、批量处理或改变实现。父子占比有重叠，不能沿栈相加得到新的总耗时。[perf-report(1)](https://man7.org/linux/man-pages/man1/perf-report.1.html)

采样比例还依赖分母。某函数从 40% 变成 20%，可能是它变快，也可能是别处新增开销。若要验证“减少分配改善解析吞吐”，同时检查同等消息量的分配次数、CPU 时间和完成速率，保持消息分布与线程放置一致。图上的归因提供待检验的原因，优化前后的对照才检验这个原因是否成立。

## 数据结构/系统内部实现

`perf_event_open` 用 `perf_event_attr` 指定事件和范围，返回 fd。计数模式通过 `read` 取值；采样模式把记录写入 mmap ring buffer，由消费者读取。环形缓冲区装不下记录时可能丢样，内核文档说明了缓冲区的生产与消费协议。采样频率、记录大小与读取速度都会影响这条路径。[Perf ring buffer](https://docs.kernel.org/userspace-api/perf_ring_buffer.html)

本例只打开当前线程的 `PERF_COUNT_SW_TASK_CLOCK`，`pid=0, cpu=-1`，不继承其他线程，请求 exclude_kernel/hv 过滤，初始禁用并设置 close-on-exec。预热与功能自测在 enable 之前完成。计数区间仍包含 enable 返回、finish 控制路径及每轮 checksum 比较，因此不会把结果标成某个函数的纯执行时间。过滤属性也不能直接保证软件时钟是严格的用户态时间：Linux v6.8 的 task_clock_event_update 累加上下文时钟差值，没有在这个更新路径按用户态与内核态拆分。本机内核版本不同，本章仅将输出解释为该软件事件的计数，不把它换算成纯用户态工作耗时。[Linux v6.8 task-clock 实现](https://raw.githubusercontent.com/torvalds/linux/v6.8/kernel/events/core.c)

计数器复用时保留 `value`、`time_enabled` 和 `time_running`。当 running 非零，可用 `value × enabled / running` 估计完整区间计数；先转成浮点等合适类型，避免整数乘法溢出。这个估计假设实际运行片段具有代表性，阶段负载可能使误差很大。running 为零时不做除法。事件组能让成员覆盖同一组已计数执行，但组太大或事件约束冲突也可能无法调度。[perf_event_open(2)](https://man7.org/linux/man-pages/man2/perf_event_open.2.html)

函数名解析和栈展开需要分别检查。符号文件把地址关联到函数或源码；frame pointer 展开沿帧链回溯，DWARF 模式则保存用户栈片段并使用展开信息。栈片段过短、库缺少相应信息、工具未包含展开支持，都可能得到不完整链。内联函数没有独立的物理调用帧，工具可以利用调试信息恢复部分内联位置；尾调用优化也会改变可见调用关系。[perf-record(1)](https://man7.org/linux/man-pages/man1/perf-record.1.html)

`-fno-omit-frame-pointer` 有助于 FP 方式，但 GCC 文档明确它不保证所有函数都保留 frame pointer。给主程序加选项也不会重新编译已有共享库。示例用 GNU `noinline` 属性保留两个工作阶段的边界，服务于教学观察；生产归因仍应使用具有代表性的优化构建。[GCC 优化选项](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)

## C++ runnable demo

```cpp include=examples/perf-flamegraph.cpp

```

Linux 下使用 GCC 或支持对应 GNU 属性的编译器。默认 32 轮，上限 2048；固定输入为 4096 个 `uint32_t`。`arithmetic_phase` 做 24 步无符号混合，`gather_phase` 按固定排列读取同一输入，每轮 salt 在 0 到 3 之间循环。32 位运算有意按模回绕，64 位校验和在本例上界内不会溢出。四个单轮常量通过独立 Python 整数计算并显式截断到 32 位得到，每轮都进行验证。

```bash
g++ -std=c++20 -O2 -g -fno-omit-frame-pointer \
  -Wall -Wextra -Wpedantic -Werror examples/perf-flamegraph.cpp -o /tmp/perf-flamegraph
/tmp/perf-flamegraph
/tmp/perf-flamegraph 2048
```

默认输出末行为 `rounds=32 checksum=561531645973760 correctness=OK`；2048 轮的 checksum 为 `35938025342320640`。计数成功时会输出原始 task-clock、enabled/running 以及缩放估计；列入可跳过的配置错误时输出带 errno 的 `SKIP task-clock configuration`，功能负载仍运行。其中 EINVAL 也可能来自参数或组合错误，只表示本次配置未成功，仍需排查，不能断言机器不支持。其他打开错误以及 ioctl/read 失败使程序明确失败。fd 由析构关闭，异常路径也会释放；没有无限重试系统调用。

自测覆盖参数 1 和 2048、空串、零、负数、超范围与尾随字符；缩放覆盖零 running、正常比例、无复用、大数乘法和非法时间顺序。示例没有采样 ring buffer、硬件 PMU 事件或 off-CPU 追踪。`SKIP` 分支在本次宿主未实际触发，不能据此声称已完成权限拒绝的故障注入。

2026-09-19 的实际环境为 x86-64 WSL2，内核 `6.18.33.2-microsoft-standard-WSL2`，GCC `13.3.0`，`perf_event_paranoid=2`。`perf --version` 返回 command not found；直接 `perf_event_open` 的 task-clock 计数成功。普通默认运行记录过 `raw=2621000, enabled_ns=2621000, running_ns=2621000`，这只是功能路径的一次观察。没有取得调用栈样本，没有生成火焰图，也没有从这个值推导性能优势。

以下是已有 perf 和 FlameGraph 工具的环境可复核的流程，本次未执行。先确认工具版本、事件支持及用户态采样权限，再运行；若命令失败或没有有效样本，应停止解释结果。`$FG` 指向已准备好的原作者 FlameGraph 目录，并记录该目录的 Git revision。

```bash
perf --version
perf list
perf stat -e task-clock:u -- /tmp/perf-flamegraph 2048
perf record -o perf.data -e cpu-clock:u -F 499 --call-graph dwarf,8192 \
  -- /tmp/perf-flamegraph 2048
perf report --stdio -i perf.data
perf script -i perf.data > perf.stacks
"$FG/stackcollapse-perf.pl" perf.stacks > perf.folded
"$FG/flamegraph.pl" perf.folded > perf.svg
```

先检查 record/report 的丢样、栈质量和实际样本数，再阅读 SVG。上述短负载可能样本不够，不能承诺 `-F 499` 能得到固定数量；它只是请求频率。外部 record 覆盖整个进程，包括初始化、预热和输出，与内部计数区间不同。比较 FP 与 DWARF 时分别保存文件，不能把两次覆盖和开销差异忽略掉。`perf script` 负责从真实 perf.data 导出记录；折叠和渲染步骤不补充原始数据中缺失的栈。[perf-script(1)](https://man7.org/linux/man-pages/man1/perf-script.1.html)

## 高频追问

### 为什么不用 Debug 构建找热点？

无优化构建可能保留实际发布程序中已消失的临时对象、调用和内存访问，也可能失去向量化。优先使用代表性的优化参数，加上所需调试和展开信息。若为解释一个机制使用了特殊编译选项，应同时检查目标构建的行为；不能把诊断版本的热点排序直接搬过去。

### cpu-clock、cycles 与 cache-misses 能混在一张图里吗？

它们的统计含义不同。CPU 时间采样、硬件周期和缓存事件不能简单相加成同一个时间分布。选择多个事件时需要在导出和聚合时保留事件身份，分别解释。通用缓存事件的 PMU 映射存在处理器差异，硬件中断的采样位置还可能受 skid 影响，应按平台支持选择精确采样能力并核对事件定义。

### 没有权限时应如何继续？

记录失败的命令、事件、作用范围和 errno，区分 CLI 缺失、权限拒绝、事件不支持和资源不足。`perf_event_paranoid` 只是权限条件之一，能力、容器策略等也可能影响调用。本章不修改安全配置，也不通过 sudo 放宽限制；在已有授权的可比环境补采集，再记录环境差异。[Perf 安全文档](https://docs.kernel.org/admin-guide/perf-security.html)

## 容易答错的点

- “宽度就是一次调用耗时。” 聚合会把多次执行的样本合并；没有调用次数和边界，无法推出单次耗时。
- “最宽的函数就是业务瓶颈。” 它可能是必须执行的高频工作，也可能与当前尾延迟目标无关，需要绝对指标和对照实验。
- “只要 `-g` 就有完整调用栈。” 调试符号和展开条件不同，完整链还受库、优化、栈转储及工具支持影响。
- “提高频率一定更准确。” 样本可能增加，也可能引入更多扰动、限频或丢样；要检查质量和重复结果。
- “scaled count 是实际观测总数。” 复用缩放补的是估计，必须保留运行比例，不能把估计值冒充未复用的原始计数。

## 性能分析

固定输入 demo 的计算量随 rounds 线性增长，两个阶段各遍历 4096 项，混合步数固定；输入存储为 16 KiB。这个工作集不代表生产行情解析、订单状态更新或跨 NUMA 内存访问。普通校验和 sanitizer 通过只支持功能与有限执行检查，sanitizer 构建的计数不能拿来评估发布性能。

若要比较优化版本，保存硬件型号和拓扑、内核与 perf 版本、二进制 build ID、编译参数、事件配置、CPU/内存放置及工作负载。先预热，再交错重复基线与改动版本，保留各次原始计数和运行比例。当前章节未做这类性能对照，未固定 CPU 或收集完整分布，因此不报告加速比。

采样还应检查丢失记录、unknown 占比、栈截断与内核限频。固定周期负载可能与采样相位相关；稀有尾部路径可能几乎没有样本。减少事件、降低频率或扩大缓冲区各有作用范围，不能只通过增加 SVG 的细节掩盖数据缺口。比较图时同时看有效样本数和绝对 CPU/经过时间，保留原始 perf.data，方便重新选择归因维度。

业务延迟需单独测量。对请求处理记录 p50/p99/p99.9 和吞吐，明确排队是否包含在延迟内。若目标是模拟独立的外部到达流，压测总等上一条完成才发下一条会改变需求并掩盖过载排队。闭环压测适用于相应的交互或依赖响应的业务模型，应按目标选择；CPU 热点图无法替代到达模型的核对。

## Quant/Low-Latency 场景

行情处理线程可能把 CPU 用在解码、价格层更新或状态恢复。先用采样缩小到调用路径，再用同一消息轨迹验证，例如减少分配是否降低每条消息 CPU 时间、是否同时改变恢复期间的尾延迟。若批量处理减少函数调用，却延后了较早到达消息的发布，需要将批次等待纳入延迟边界。

另一个常见情况是处理线程平时空闲，突发时 p99.9 升高。CPU 图可能只显示正常处理逻辑，此时要对照入队、开始处理和发出结果的时间，查看排队、调度等待或锁持有者。off-CPU 堆栈描述等待发生的位置，锁持有者为何迟迟不释放还需要相应线程的执行证据，不能把等待者栈当作完整原因。

线上采集本身会消耗资源。先限定线程、事件和持续时间，观察采集开启后吞吐与延迟是否变化；保存原始记录并记录缺失。不能把一次诊断中看到的占比当作所有交易时段都稳定成立的分布。

## 相关专题

- [CPU cache / false sharing](cpu-cache-false-sharing.md)：从共享数据布局形成可测量的缓存争用假设。
- [NUMA 与 first-touch](numa-first-touch.md)：解释线程与内存放置如何改变实验条件。
- [虚调用与去虚拟化](../cpp/vtable-devirtualization.md)：理解优化构建的调用结构为何变化。
- [mutex 与 condition_variable](../concurrency/mutex-condition-variable.md)：把自旋、阻塞和唤醒路径与 CPU/off-CPU 观察对应起来。

## 分层面试题

题库共 15 道，L1/L2/L3 各 5 道。先练习解释计数和火焰图，再核对复用、栈展开与样本质量，最后用受控实验说明一个归因怎样转化为可验证的性能改动。
