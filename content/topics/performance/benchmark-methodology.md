---
{
  'schemaVersion': 1,
  'id': 'benchmark-methodology',
  'title': 'Benchmark 方法：测量范围、样本与归因',
  'description': '用两种求和实现演示正确性校验、预热、交替采样和原始数据记录，说明计时开销、优化边界、分位数与服务负载模型。',
  'category': 'performance',
  'areas': ['Performance Engineering', 'Low-Latency Programming'],
  'tags': ['benchmark', 'measurement', 'percentile', 'compiler-optimization', 'latency'],
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
  'estimatedMinutes': 50,
  'prerequisites': ['C++ 容器与整数范围', '编译优化与可观察行为', '延迟和吞吐的区别'],
  'related': ['cpu-cache-false-sharing', 'numa-first-touch', 'io-uring', 'cpp-memory-model'],
  'demo':
    {
      'file': 'examples/benchmark-methodology.cpp',
      'platform': 'portable',
      'exercise': '保持原始样本输出，分别改变 passes 与输入工作集大小；将 count 取为 1024 的整数倍并同步更新闭式期望，重新记录计时范围和环境，检查批次均值变化，不能把它当作单请求尾延迟。',
    },
  'references':
    [
      {
        'title': 'C++20 N4861: steady_clock',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/time.clock.steady',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: high_resolution_clock',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/time.clock.hires',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Google Benchmark: user guide',
        'url': 'https://google.github.io/benchmark/user_guide.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC: extended asm and memory clobber',
        'url': 'https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'LLVM: benchmarking tips',
        'url': 'https://llvm.org/docs/Benchmarking.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Google Benchmark: reducing variance',
        'url': 'https://google.github.io/benchmark/reducing_variance.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'R stats: sample quantile algorithms',
        'url': 'https://stat.ethz.ch/R-manual/R-devel/library/stats/html/quantile.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'HdrHistogram: corrected and raw value recording',
        'url': 'https://raw.githubusercontent.com/HdrHistogram/HdrHistogram/master/README.md',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux perf-stat: events and repeated runs',
        'url': 'https://man7.org/linux/man-pages/man1/perf-stat.1.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'benchmark-methodology-q01',
        'level': 'L1',
        'prompt': '给一个函数测耗时前，至少要定义哪些条件？',
        'answer': '先定义一次工作包含什么、输入和状态如何准备、起止时间在哪里，以及结果单位。还要确认两种实现满足同一正确性约定。若分配和初始化在计时外，得到的就是已准备输入上的执行耗时，不能写成含创建和销毁的端到端延迟。',
        'rubric': ['明确工作与计时边界', '固定输入和正确性约定', '指标名称与实际范围一致'],
        'source':
          { 'kind': 'derived', 'rationale': '根据基准计时范围与调用方端到端路径的差异设计。' },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q02',
        'level': 'L1',
        'prompt': 'steady_clock 的纳秒输出能证明计时精度达到一纳秒吗？',
        'answer': '不能。steady_clock 提供不倒退且相对真实时间以稳定速率前进的时钟语义；duration 转成纳秒只是单位转换。有效分辨率、读取成本和环境噪声仍依赖实现及机器。两个紧邻读数相同并不表示读时钟完全没有成本。',
        'rubric': ['单调语义与精度区分', '输出单位不决定分辨率', '允许紧邻读数相同'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 C++ steady_clock 保证和本例单位转换设计。' },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q03',
        'level': 'L1',
        'prompt': '为什么预热样本通常不放进稳态测量统计？',
        'answer': '首次访问可能带入缺页、缓存填充或初始化等成本，预热用于接近所要研究的重复执行状态。是否排除取决于问题：若用户关心首次请求，就要另测冷启动。固定预热四轮只是示例参数，不证明机器已经稳定，也不证明所有数据常驻某级 cache。',
        'rubric': ['说明预热改变的状态', '冷启动与稳态分别测量', '预热轮数不构成稳态保证'],
        'source':
          { 'kind': 'derived', 'rationale': '根据示例预热阶段和生产首次请求的不同目标设计。' },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q04',
        'level': 'L1',
        'prompt': '一次批量执行 32 次，耗时除以 32 后得到什么？',
        'answer': '得到该批次每次执行的平均耗时，包含被计入批次的循环、调用和测量辅助开销。没有记录每次执行的起止时间，就无法恢复其中每次执行的延迟分布。因此多个批次均值的 p99 也不是所有单次请求延迟的 p99。',
        'rubric': ['是批次平均值', '包含批次内辅助成本', '不能恢复单请求分布'],
        'source': { 'kind': 'derived', 'rationale': '根据批量计时的聚合损失与尾延迟口径设计。' },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q05',
        'level': 'L1',
        'prompt': '只有 20 个样本，按 nearest-rank 算出的 p99 与 p99.9 能说明什么？',
        'answer': '按升序第 ceil(n*p) 个值取分位数，n 为 20 时这两个分位数都选择第 20 个值，也就是样本最大值。它们描述这组样本，但不足以可靠估计总体尾部。报告时应同时给样本数、算法和采样对象，不能用更多小数位掩盖样本不足。',
        'rubric': ['说明 nearest-rank 算法', '两个值均为最大样本', '不等于可靠总体尾部估计'],
        'source':
          { 'kind': 'derived', 'rationale': '根据经验分布分位数定义及小样本位置推导设计。' },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q06',
        'level': 'L2',
        'prompt': '循环末尾打印 checksum，为什么仍可能没有测到预期的重复工作？',
        'answer': '打印保证结果被观察，但若输入和函数结果对编译器足够明确，它仍可能折叠计算、将结果移出循环或复用一次结果。本例在每轮使用 GCC/Clang memory compiler barrier 使输入内存重新成为编译器需要考虑的状态，并检查优化构建汇编。这个屏障不会阻止函数内部向量化等合法优化。',
        'rubric': ['保留结果与保留重复计算区分', '说明折叠或外提风险', '有限屏障加目标构建核查'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据编译器优化对基准可观察输出与重复执行的不同处理设计。',
          },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q07',
        'level': 'L2',
        'prompt': '本例空 asm 的 memory clobber 会把数据从 CPU cache 清掉吗？',
        'answer': '不会。它告诉编译器该处可能读写相关内存，从而约束值复用及移动；它没有发出刷新 cache 的指令，也不是 CPU 序列化指令或线程间同步操作。输入仍可能留在缓存中。要研究冷缓存，必须另定义并验证准备条件，不能只改一个编译器屏障。',
        'rubric': ['属于编译器层约束', '不刷新缓存或序列化 CPU', '不建立线程同步'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据 GCC memory clobber 的编译器语义及硬件边界设计。',
          },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q08',
        'level': 'L2',
        'prompt': '先连续跑完 A 再连续跑完 B，与交替 AB/BA 有什么差别？',
        'answer': '整段 A 后整段 B 容易把温度、频率或后台负载随时间的变化混入方案差异。交替顺序让两种方案更接近同一时间段，并平衡谁先执行。本例确定性 AB/BA 便于复现，但仍可能与周期噪声相关，也会共享缓存状态；随机化或跨进程重复可以补充检查，不能声称交替消除了所有偏差。',
        'rubric': ['识别时间趋势混杂', '交替平衡先后顺序', '保留周期噪声与状态共享边界'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 A/B 实验顺序与系统状态变化的混杂关系设计。' },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q09',
        'level': 'L2',
        'prompt': '两个实现输出相同，能否直接认为正确性验证已经充分？',
        'answer': '两者可能共享同一个错误。应使用独立的预期值或参考方法，并覆盖空输入、尾部和数值范围。本例为短数组各前缀写明预期值，再利用输入置换性质计算完整数据的闭式总和。有限测试仍不证明所有输入正确，换成浮点归约时还需重新约定误差和运算次序。',
        'rubric': ['避免仅互相比对', '独立 oracle 与边界', '说明有限覆盖及浮点差异'],
        'source':
          { 'kind': 'derived', 'rationale': '根据等价实现可能共享缺陷及示例独立求和校验设计。' },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q10',
        'level': 'L2',
        'prompt': '测一次空计时区间，再从所有结果中减去它，是否能得到真实函数耗时？',
        'answer': '不能保证。空区间可以显示计时读数和分辨率的大致量级，但目标路径还包括调用、循环、屏障以及可能不同的代码与缓存状态；计时成本也会波动。简单相减可能放大误差。本例保存紧邻时钟读数而不做扣除，并通过有界批次摊薄读时钟的相对影响。',
        'rubric': ['基线不是恒定精确成本', '目标路径与空路径不同', '批量摊薄并保留原始值'],
        'source':
          { 'kind': 'derived', 'rationale': '根据测量开销、计时分辨率与基线相减的误差设计。' },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q11',
        'level': 'L3',
        'prompt': 'B 的平均耗时变小了，如何判断能否用于生产替换？',
        'answer': '先确认输出、失败语义和资源成本等价，再检查测量是否覆盖真实输入规模、冷热状态和部署编译选项。保留原始样本和多轮独立运行，查看波动、排队与端到端尾部是否恶化。微基准可以支持某条局部路径的假设，但生产替换还需要真实调用链及负载下的证据。',
        'rubric': ['语义与资源等价', '负载和构建代表性', '局部指标与端到端验证分开'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据局部测量到生产替换之间的外推限制设计工程决策题。',
          },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q12',
        'level': 'L3',
        'prompt': '低延迟服务的压测器收到响应后才发下一条，会带来什么测量问题？',
        'answer': '若目标是模拟独立于响应时间持续到达的请求，服务变慢时该压测器也减少发出请求，会漏掉本应继续到达并排队的工作，这属于 coordinated omission 的典型来源。应按目标到达模型安排发送，记录计划发送、实际发送和完成时刻，并报告生成器落后、拒绝及超时。若真实业务就是等待响应后再发，请明确测的是闭环负载，不必把它一概视为错误。',
        'rubric': ['与独立到达模型关联', '慢服务导致发送量下降', '闭环模型适用边界与超时处理'],
        'source':
          { 'kind': 'derived', 'rationale': '根据响应时间依赖的采样缺失推导服务压测负载模型题。' },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q13',
        'level': 'L3',
        'prompt': '基准出现几个很慢的样本，应直接删掉再算 p99 吗？',
        'answer': '先保留数据并查明慢样本是否来自调度、缺页、迁移、热状态或实现自身路径。若这些因素属于部署环境，删除会掩盖实际风险；若实验目标明确排除某类干扰，应事先定义筛选规则并同时报告原始和筛选结果。不能看到 B 变慢后才临时决定删哪些点，也不能把少量相关样本当大量独立观察。',
        'rubric': ['先保存并解释慢样本', '按目标预先定义排除条件', '防止选择性删数与独立性假设'],
        'source':
          { 'kind': 'derived', 'rationale': '根据噪声归因、生产相关性与事后筛选偏差设计。' },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q14',
        'level': 'L3',
        'prompt': '改动后指令数下降、wall time 却上升，该怎样继续定位？',
        'answer': '先核对工作量、正确性和计时范围是否一致，再查看是否增加了 cache miss、分支失误、等待、频率变化或调度干扰。计数器必须选对作用域并注意可用事件和多路复用；整进程 perf stat 还包含初始化和输出。本例测量循环的汇编可确认调用仍在，但单看指令数不能决定实际耗时。',
        'rubric': ['先检查等价工作', '多种机制与环境归因', '计数器作用域及多路复用边界'],
        'source':
          { 'kind': 'derived', 'rationale': '根据硬件计数器与 wall time 不同观测对象设计归因题。' },
        'companies': [],
      },
      {
        'id': 'benchmark-methodology-q15',
        'level': 'L3',
        'prompt': '为了降低噪声，是否应该固定把所有基准都缩小到 L1 并关闭所有动态频率行为？',
        'answer': '应按问题选择条件。受控配置有助于隔离代码差异，但把真实大工作集缩到 L1 会改变瓶颈，关闭频率变化也可能偏离部署行为。可分别运行机制实验和部署配置实验，完整记录 CPU、内存放置及频率策略。虚拟机无法观察宿主状态时应记录未知条件，而不是声称环境已隔离。',
        'rubric': ['控制变量与代表性权衡', '工作集变化会改变问题', '明确部署配置和未知条件'],
        'source':
          { 'kind': 'derived', 'rationale': '根据降低方差与保持真实部署条件之间的工程取舍设计。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

Benchmark 用受控输入和明确计时边界检验性能假设。先验证两种实现做了同样的工作，再记录构建、机器、预热、采样顺序和原始结果，检查编译器是否保留了待测计算。平均值、批次分位数和请求尾延迟描述的对象不同。一次运行较快只能说明当时条件下的观察，不能直接推出生产路径更快。

本文以 C++20 为语言范围，用 GCC/Clang 扩展限制部分编译器优化；实测为 WSL2 x86-64、GCC 13.3。计时循环不依赖 Linux API，但非 GCC/Clang 构建会在正确性校验后明确跳过计时。这里研究有限的热身后求和路径，没有模拟服务请求队列。

## 核心概念

| 术语       | 本文含义                                  | 需要保留的区别                             |
| ---------- | ----------------------------------------- | ------------------------------------------ |
| 工作量     | 读取 65536 个 uint32_t 并求 uint64_t 总和 | 输出相同不自动证明边界和失败语义相同       |
| wall time  | 两次 steady_clock 读数间的经过时间        | 包括期间可能发生的抢占；不是线程 CPU 时间  |
| 批次       | 连续执行 32 次同一实现                    | 降低读时钟的相对占比，也改变状态与统计对象 |
| 重复样本   | 每种实现 20 个批次总耗时                  | 不假定彼此独立，也不是 640 个单次延迟样本  |
| 样本分位数 | 对记录值排序后的指定位置                  | 算法、样本数及采样对象必须一起说明         |
| 可归因实验 | 尽量只改变要研究的因素                    | 较低波动仍不能排除系统性偏差               |

steady_clock 的读数不会随物理时间前进而减少；标准没有因此保证一次读取只花多少纳秒。high_resolution_clock 也不因名字就保证单调，它可以是其他时钟的别名。计时前要看所选时钟的语义。[C++20 steady_clock](https://timsong-cpp.github.io/cppwp/n4861/time.clock.steady)、[high_resolution_clock](https://timsong-cpp.github.io/cppwp/n4861/time.clock.hires)

## 原理深入

### 先确定测量对象

示例的计时区间从 start 到 stop，包含 32 次输入 compiler barrier、函数指针调用、求和、checksum 累积，以及循环和末尾结果屏障。分配、输入生成、正确性检查、样本排序和输出在区间外。读时钟本身也有成本，因此这里测的是这条整批执行路径，不能命名为“纯加法指令耗时”。

把批次总耗时除以 32，可以比较该重复执行条件下的每次平均成本。一次特别慢的调用会被同批其他调用摊薄；原始批次值也无法告诉我们究竟是哪次调用慢了。若要回答请求 p99，需要按请求定义到达、排队和完成边界，并记录对应样本。

### 结果被使用不等于重复计算被保留

若函数对固定输入始终返回同一个编译期已知结果，即使最后打印 checksum，编译器也可能折叠计算或复用结果。本例让输入指针出现在空 asm 的输入中，并使用 memory clobber；它限制编译器跨该点假设内存不变。结果屏障位于结束读时钟之前，形成对 checksum 的编译器数据依赖。[GCC extended asm](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html)

这些约束不禁止函数内部向量化、展开或重新选择指令。源码中的四个累加器也不保证机器代码恰有四条独立累加链。Google Benchmark 的 DoNotOptimize 同样不承诺保留表达式内部的每一步计算；使用框架后仍要核查实际目标构建。[优化防护的边界](https://google.github.io/benchmark/user_guide.html#preventing-optimization)

compiler barrier 没有刷新 CPU cache，也不是硬件序列化或线程同步。它还可能限制本来合法的代码移动，因而改变被测路径。应记录其位置，并检查优化构建的汇编。本次 GCC 13.3、`-O2` 的 measure 循环中，两次 steady_clock::now 之间保留了每轮间接调用和 checksum 累加；这只是该构建的检查结果。

### 固定输入与正确性约定

输入为 `data[i] = (17*i + 11) mod 1024`，共 65536 项。17 与 1024 互质，每个 1024 项区间覆盖 0 到 1023，完整总和可独立计算为 `64 * 1023 * 1024 / 2 = 33521664`。再用长度 0 到 5 的短数组校验空输入、四路循环尾部和 UINT32_MAX，避免只让 A、B 互相作答案。

这些有界整数求和不会超出 uint64_t，批次 checksum 也在范围内。若将实验换成浮点求和，需要先规定允许误差、求和顺序和编译选项；相同数学表达式不保证不同归约次序产生相同浮点结果。

## 数据结构/系统内部实现

程序用预先分配的数组保存 20 对 Sample，每项记录 total_ns 与 checksum。第 0 对先 A 后 B，第 1 对先 B 后 A，随后交替，共 10 对 AB、10 对 BA。四对预热也按相同顺序交替，但不进入统计。全部测量结束后才打印原始数据，避免每一轮都把输出成本夹在两种实现之间。

交替可以减小整段 A 后整段 B 带来的时间趋势影响，不能消除周期性干扰或两者共享缓存状态。更正式的实验可以使用记录种子的随机顺序、跨进程重复及独立运行批次；还要确认这些改变没有换掉要研究的负载。[重复与交错运行](https://google.github.io/benchmark/user_guide.html#random-interleaving)

分位数使用 nearest-rank：将 n 个值升序排列，取第 `ceil(n*p)` 个，本文只接受 `0 < p <= 1`。这是经验分布逆函数的一种约定，不是所有软件的默认算法。代码用整数分子和分母计算 rank，并限制样本数和分母最多 1000，避免乘法溢出。[样本分位数算法](https://stat.ethz.ch/R-manual/R-devel/library/stats/html/quantile.html)

当 n=20 时，p50 取第 10 项，p99 与 p99.9 都取第 20 项。增加输出小数位不会增加尾部信息；把同一批次内 32 次未单独计时的调用算作 32 个样本，也不会得到缺失的请求分布。

## C++ runnable demo

```cpp include=examples/benchmark-methodology.cpp

```

优化构建用于观察性能，保持正确性断言启用：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/benchmark-methodology.cpp -o /tmp/benchmark-methodology
/tmp/benchmark-methodology > /tmp/benchmark-methodology-raw.txt
```

输出包含输入大小、编译器、预热与重复数、20 个紧邻时钟读数、20 对原始批次时间和每批 checksum。不要只保存最后的分位数。前几项及结尾的固定字段如下，时间值随运行变化：

```text
correctness passed; expected_sum=33521664
elements=65536 bytes=262144 passes_per_batch=32 warmup_pairs=4 measured_pairs=20
round,order,a_batch_ns,b_batch_ns,a_checksum,b_checksum
n=20: p99 and p99.9 both select max; no reliable tail estimate
batch means are not per-request latencies; no winner asserted
```

每批 checksum 应为 1072693248。程序明确校验这个值，不对 A、B 的快慢作断言。空计时区间的读数只用于观察时间单位和读时钟成本的大致量级，既不从目标耗时中扣除，也不将其中的 0 解释为零开销。

2026-09-19 的一次优化构建运行使用 Intel Core i7-10700，WSL2 内核 6.18.33.2、GCC 13.3；虚拟机可见 16 个逻辑 CPU、1 个 NUMA 节点。允许 CPU 0–15，程序未绑核或绑定内存，没有控制宿主负载、睿频或温度；guest 未暴露所查询的 scaling_governor 文件。262144 字节输入被反复读取，没有执行 cache flush，也没有验证缓存命中层级。

这次运行的 A 批次均值 p50 为 21205.562 ns/次，B 为 10193.438 ns/次；对应 p99 与 p99.9 都取样本最大值，分别为 22733.625 和 11787.094 ns/次。它们仅描述这一次运行中的批次均值，完整 20 对原始值保存在本章作者审查记录，不能从这些数值推出跨机器加速比。两种实现同为 O(n)，本例没有逐项证明耗时差异的硬件原因。

常规严格编译与 ASan/UBSan 均通过。Sanitizer 改变指令、内存访问和布局，其输出中的时间不进入上述性能记录。计时器分辨率、操作系统调度与虚拟化条件也不由 sanitizer 验证。

## 高频追问

### 为什么不用 volatile 保证“没有优化”？

volatile 访问具有特定可观察语义，但不会让周围所有表达式都停止优化；给整个输入加 volatile 还会改变加载行为，使被测程序偏离正常代码。需要保留的是实验所需的工作，并允许生产构建通常会做的优化。可以检查结果、采用有限编译器屏障，再核查汇编；不能把某个关键字当成通用证明。

### 为何不减去一次空循环的耗时？

空循环可能被完全删除，或具有与目标不同的调用、分支和缓存行为。即使保留它，两个测量也会各自带噪声，差值不一定是可独立相加的“纯函数成本”。本例保留计时读数，使用批次摊薄相对开销；若工作短到仍由测量主导，应调整实验粒度并明确随之改变的统计对象。

### 预热到稳定是不是越久越好？

先确定目标是冷启动、持续运行还是一段突发流量。较长预热可能改变频率、温度、分配器状态和 cache，使结果更接近某个稳态，也可能离实际突发路径更远。固定四对预热是可复现配置，不是已经稳定的证据；要研究稳定性，可以保留分阶段时间序列并检查趋势。

## 容易答错的点

| 错误说法                           | 修正与原因                                       |
| ---------------------------------- | ------------------------------------------------ |
| 纳秒输出说明误差小于一纳秒         | 单位转换不能提高有效分辨率或降低噪声             |
| 打印结果就能保留全部计算           | 常量折叠、循环外提和结果复用仍可能发生           |
| compiler barrier 会清 cache        | 它约束编译器，不发出缓存刷新或 CPU fence         |
| 手动四路展开必然更快               | 机器指令由目标编译器决定，资源和依赖也可能变化   |
| 20 个数就能可靠报告服务 p99.9      | 本例该位置就是最大批次均值，缺少可靠尾部证据     |
| 慢样本都是噪声，可以删掉           | 先判断是否属于真实部署路径，并保留原始记录       |
| sanitizer 通过且更快就能发性能结论 | sanitizer 构建用于功能检查，性能需要目标优化构建 |
| 绑到一个 CPU 就彻底隔离环境        | SMT、共享缓存、频率、宿主和内存放置仍会影响结果  |

## 性能分析

A、B 都读取 n 项，时间复杂度 O(n)，额外状态为常数。样本存储与排序属于测量框架，位于计时区间外。工作集、向量化、依赖链及调用开销都可能影响观察；本例不以某次比值给单一机制定因。

准备更可信的比较时，保存 CPU 拓扑、内核、编译器完整版本及选项、二进制或源码标识、输入生成方法、工作集、亲和性、内存位置、预热和原始样本。重复次数应由噪声与所需精度决定；低方差不能证明没有系统性偏差。不要在观察结果后才选择最有利的运行或删点规则。[LLVM 测量建议](https://llvm.org/docs/Benchmarking.html)

控制频率、选择获准 CPU 或隔离测试机有助于排查干扰，但应同时保留与部署一致的实验。把工作集缩到 L1、关掉部署中存在的状态变化，会改变问题本身。记录无法观测的条件比宣称环境“已稳定”更准确。[方差来源与配置边界](https://google.github.io/benchmark/reducing_variance.html)

需要定位差异时，可以使用 perf stat 观察指令、周期、调度、缺页或适用的 cache 事件。先检查事件支持、权限和多路复用比例；指标名称及硬件意义依机器而异。直接对整个程序运行 perf stat，会把初始化、预热、排序和输出也算入，不能把整进程计数精确对应到某一个批次。[perf stat 的事件与重复运行](https://man7.org/linux/man-pages/man1/perf-stat.1.html)

对于服务延迟，coordinated omission 与负载产生方式有关。若真实请求按独立计划到达，生成器却必须等上个响应才发下一条，服务停顿会同步减少发出的请求，漏掉原本应排队的工作。应保存计划发送、实际发送和完成时刻，明确延迟从哪里开始；生成器跟不上、拒绝和超时也要报告。HdrHistogram 的 expected-interval 校正依赖预期采样间隔等假设，不能凭空恢复任意真实到达流。[缺失采样与校正模型](https://raw.githubusercontent.com/HdrHistogram/HdrHistogram/master/README.md)

如果真实业务本来就是完成后再发下一条，闭环测试可以回答这一负载下的问题。本例测连续求和成本，没有外部到达流，也不尝试用校正方法生成虚构的请求延迟。

## Quant/Low-Latency 场景

比较行情解码器时，可以先用固定报文集合测试字段解码和校验是否等价，再测已准备 buffer 上的处理成本。报文复制、跨线程排队、缺页和异常报文恢复若没有包含在区间内，应在报告中列明。微基准找到的局部改进，需要放回 feed handler 的完整数据流验证。

订单网关还要区分本地编码完成、进入发送队列、发送完成和交易所业务确认。若系统靠加大批次提高吞吐，批次均值可能下降，但等待凑批的请求延迟可能增加。用实际负载模型同时报告接受量、完成量、拒绝与超时，以及从约定起点测得的延迟分布，才能判断是否适合低延迟路径。

## 相关专题

- [CPU Cache 与 false sharing](./cpu-cache-false-sharing.md)：理解工作集、共享缓存和硬件一致性对观察的影响。
- [NUMA 与 first-touch](./numa-first-touch.md)：区分线程放置与页位置，为测量保存内存条件。
- [io_uring](../network/io-uring.md)：区分提交、完成与业务确认，避免用 NOP 结果外推真实 I/O 性能。
- [C++ 内存模型](../concurrency/cpp-memory-model.md)：区分编译器屏障、硬件行为和语言层线程同步。

## 分层面试题

题库按测量基础、机制判断和工程决策分层。作答时先指出采样对象与前提，再解释指标能支持什么结论。
