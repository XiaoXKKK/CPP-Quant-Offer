---
{
  'schemaVersion': 1,
  'id': 'cpu-cache-false-sharing',
  'title': 'CPU cache / false sharing：正确却很慢',
  'description': '区分局部性、真实共享与假共享，用可重复实验识别 cache line 争用，而不是迷信 padding。',
  'category': 'performance',
  'areas': ['CPU Cache / NUMA', 'Performance Engineering', 'Low-Latency Programming'],
  'tags': ['cache', 'false-sharing', 'numa'],
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
  'updated': '2026-09-18',
  'reviewed': '2026-09-18',
  'standard': 'C++20',
  'estimatedMinutes': 40,
  'prerequisites': ['原子读改写', '结构体布局与对齐', '线程绑定基础'],
  'related': ['cpp-memory-model', 'shared-mutex'],
  'demo':
    {
      'file': 'examples/cpu-cache-false-sharing.cpp',
      'platform': 'portable',
      'exercise': '先在同一物理核的 SMT 线程、再在不同物理核运行；记录 CPU 拓扑和 cache line 大小，用 perf c2c 定位争用地址，解释 padding 结果不稳定的原因。',
    },
  'references':
    [
      {
        'title': 'Linux kernel: False Sharing',
        'url': 'https://docs.kernel.org/kernel-hacking/false-sharing.html',
        'kind': 'manual',
        'accessed': '2026-09-18',
      },
      {
        'title': 'C++ hardware interference size',
        'url': 'https://eel.is/c++draft/hardware.interference',
        'kind': 'standard',
        'accessed': '2026-09-18',
      },
      {
        'title': 'Linux perf c2c manual',
        'url': 'https://man7.org/linux/man-pages/man1/perf-c2c.1.html',
        'kind': 'manual',
        'accessed': '2026-09-18',
      },
    ],
  'questions':
    [
      {
        'id': 'cpu-cache-false-sharing-q01',
        'level': 'L1',
        'prompt': '什么是 false sharing，是否属于 data race？',
        'answer': '它是独立变量因共享缓存行且存在写访问而互相影响的性能现象。变量可以都是原子并完全正确，所以 false sharing 不等于语言层数据竞争。',
        'rubric': ['独立变量共行', '性能与正确性'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q02',
        'level': 'L1',
        'prompt': 'true sharing 与 false sharing 如何区分？',
        'answer': 'true sharing 是线程操作同一逻辑数据，false sharing 是独立逻辑数据共享一致性粒度。padding 可缓解后者，却不能移除前者所需的真实通信。',
        'rubric': ['逻辑对象', 'padding边界'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q03',
        'level': 'L1',
        'prompt': '两个线程只读同一缓存行会假共享乒乓吗？',
        'answer': '通常不会产生由写入引起的行失效乒乓。需要至少存在写访问才会触发这类干扰；纯读仍可能受带宽、容量或 NUMA 放置影响。',
        'rubric': ['至少一个写者', '其他瓶颈'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q04',
        'level': 'L1',
        'prompt': 'relaxed atomic 为什么仍可能很慢？',
        'answer': 'relaxed 放松顺序约束，但保留原子操作。原子读改写仍可能需要缓存行的独占访问，多个核心竞争同一行的成本不会凭空消失。',
        'rubric': ['顺序与原子性', '行所有权'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q05',
        'level': 'L1',
        'prompt': 'alignas(64) 保证所有机器消除假共享吗？',
        'answer': '不能。它是 C++ 布局对齐要求，实际缓存行或一致性粒度取决于硬件，结构体内部布局也要检查。应记录目标平台和字段偏移。',
        'rubric': ['布局与硬件区别', '检查实际地址'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q06',
        'level': 'L2',
        'prompt': '为什么只对结构体开头对齐可能不够？',
        'answer': '同一结构体内相邻字段仍可能处在一行。需要检查成员偏移、对象大小以及数组元素步长，按不同写入线程的所有权分开热点字段。',
        'rubric': ['内部字段', '数组stride'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q07',
        'level': 'L2',
        'prompt': '普通整数计数器可以作为 atomic 的公平基线吗？',
        'answer': '需看线程所有权与优化结果。共享普通计数器可能有 data race；完全局部的循环则可能被优化折叠。比较前先确保语义和实际工作一致。',
        'rubric': ['无数据竞争', '防止优化折叠'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q08',
        'level': 'L2',
        'prompt': '为什么 padding 可能让程序变慢？',
        'answer': '它增加工作集、TLB 压力与内存带宽，可能破坏原本有用的同线程局部性。应只针对测到的共享行热点进行布局调整，并保留对照。',
        'rubric': ['工作集成本', '针对性测量'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q09',
        'level': 'L2',
        'prompt': '如何验证地址级别的共享行争用？',
        'answer': '先定位耗时函数，再结合 perf c2c 等工具和对象布局映射争用地址，核查线程读写模式。硬件事件及权限有限制，单看 cache miss 总数不够。',
        'rubric': ['地址与布局关联', '工具限制'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q10',
        'level': 'L2',
        'prompt': '为什么 benchmark 应交替运行布局版本？',
        'answer': '固定顺序可能受预热、频率与系统负载漂移影响。交替顺序、多轮分布和相同工作量可降低偏差，但仍需绑核和拓扑控制来支持因果判断。',
        'rubric': ['顺序偏差', '实验控制'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q11',
        'level': 'L3',
        'prompt': '如何优化交易系统的每线程统计计数？',
        'answer': '让每线程写自己的分片并隔离热点字段，低频聚合；同时记录聚合滞后、读侧引入的一致性访问和内存占用。不能无条件承诺比全局计数更好。',
        'rubric': ['写所有权分片', '聚合代价'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q12',
        'level': 'L3',
        'prompt': 'SPSC 游标分行后为什么仍有跨核流量？',
        'answer': 'head/tail 的发布和观察以及实际数据传递是真实共享，无法通过 padding 消除。分行主要减少独立写者对同一行的无谓争夺。',
        'rubric': ['必要通信', '优化范围'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q13',
        'level': 'L3',
        'prompt': '跨 NUMA 测量需要控制哪些变量？',
        'answer': '控制线程绑定、首次触页线程、内存策略和数据规模，核查迁移及实际节点归属。否则测到的差异可能来自远端内存，而非字段布局本身。',
        'rubric': ['first-touch', '实际放置'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q14',
        'level': 'L3',
        'prompt': '从吞吐提高能推断订单 p99 降低吗？',
        'answer': '不能。批量和缓冲可提高吞吐却增加等待与排队尾部。必须测端到端时间，包括进入队列、处理、发送，并使用合理到达过程。',
        'rubric': ['不同指标', '端到端测量'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpu-cache-false-sharing-q15',
        'level': 'L3',
        'prompt': '怎样证明一次 padding 优化值得合入？',
        'answer': '在代表性负载中用地址级争用证据建立原因，只改变布局做多轮对照，保留正确性测试，记录延迟分布和内存成本，确认不同部署拓扑没有显著回退。',
        'rubric': ['因果证据', '收益与回归'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

false sharing 是不同线程访问逻辑独立的变量，却因共用缓存行且存在写入而相互干扰。程序可以没有 data race 仍很慢。先定位争用地址，再按写入所有权分离布局或分片。padding 会增加内存足迹，64 字节也不是通用保证，效果须在真实拓扑下测量。

## 核心概念

| 现象               | 原因                   | 可能措施                         |
| ------------------ | ---------------------- | -------------------------------- |
| true sharing       | 多线程操作同一逻辑状态 | 减少共享、批量化、所有权转移     |
| false sharing      | 独立状态共用一致性粒度 | 布局分离或线程局部分片           |
| capacity miss      | 工作集超过有效缓存容量 | 压缩数据、分块访问               |
| remote NUMA access | 线程与内存放置不匹配   | 绑定与 first-touch，验证实际分配 |

cache coherence 与 C++ memory consistency 不是一个层次。一致性协议处理缓存行状态；语言模型规定何种程序行为合法。硬件 cache 一致不能挽救 C++ data race。

## 原理深入

两个线程各自递增独立原子计数器，如果计数器在同一行，写入所需的行所有权可能在核心之间反复转移。relaxed 省掉的是部分顺序约束，不是原子读改写，也不会消除行的独占需求。

对照实验把计数器类型改为 alignas(64)，让数组元素之间有至少相应对齐间隔。在常见 64 字节缓存行目标上可减少两者共享一行；若硬件一致性粒度更大、线程落在同一核心或原子实现用了锁，解释就会不同。更多定位方法参见 [Linux false sharing 文档](https://docs.kernel.org/kernel-hacking/false-sharing.html)。

这里不能使用无同步的共享普通计数器制造“更快”基线，因为它会形成 data race。也不能只用普通局部变量循环，因为编译器可能把循环折叠成一次计算，测到的是完全不同的工作。

## 数据结构/系统内部实现

缓存行把地址空间划成固定粒度，组相联缓存又将地址映射到 set；局部性好的连续遍历通常比指针追逐更有利于预取。MESI/MOESI 之类状态机是理解一致性的模型，不代表每个微架构都使用完全相同的实现或成本。

结构体开头 alignas 不足以保证所有内部字段都分离；需要检查 sizeof、alignof、字段偏移和数组 stride。标准的 hardware_destructive_interference_size 可作为实现提供的提示，但跨不同编译目标或 ABI 边界不能随意假设其固定值。demo 明确选择 64 作为实验参数，不伪装成动态探测。

## C++ runnable demo

两个线程分别递增自己的原子计数，Packed 与 Padded 执行相同工作。开始信号在两个线程就绪后发布，四轮交替运行顺序，结束后校验数量。时间包含释放开始信号后的调度与 join 成本，不是精密单指令计时。

```cpp include=examples/cpu-cache-false-sharing.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pthread examples/cpu-cache-false-sharing.cpp -o /tmp/cache-demo
/tmp/cache-demo
```

测试只要求数值正确，不断言 Padded 一定更快。在线编译器和 CI 共享机器的时间只能帮助观察，不能用来支持生产延迟承诺。正式实验增加预热、多轮统计、线程绑核、拓扑记录以及硬件计数器证据。

## 高频追问

“所有字段都 padding 是否最优？”未必。布局膨胀增加 cache/TLB 压力，浪费带宽，还可能破坏同线程访问字段的 locality。先按写入所有者分组，把不同核心频繁写入的数据分开，而非机械地对每个字段加 64 字节。

“读线程也会受影响吗？”会。一线程写 A、另一线程读同一行上的 B，即使逻辑变量不同，读副本也可能不断失效；两线程都只读则没有这种写引起的一致性乒乓。

## 容易答错的点

- 没有 data race 不代表没有 false sharing；后者是性能问题。
- memory_order_relaxed 仍会执行原子操作和相关一致性通信。
- padding 不能修复 true sharing，也不能修复内存序错误。
- 不应背诵所有机器都固定的 L1/L2/内存纳秒表；延迟取决于拓扑和访问类型。
- 单次 wall-clock 差异不是因果证据，虚拟机和在线沙箱尤其容易受噪声影响。

## 性能分析

分三步：先确认真实负载的热点；再把热点映射到地址、cache line 与读写线程；最后做只改变布局的 A/B 对照。Linux 上可尝试 `perf c2c record` / `perf c2c report`，但硬件事件支持和权限依平台而异，不能把工具不可用解释为没有争用。

记录 CPU 型号、cache line 大小、核心与 NUMA 拓扑、编译器和参数、线程绑定、数据规模、预热与重复次数、原始分布。用中位数描述中心位置，用分位数评估长尾，同时观察吞吐。跨 NUMA 对照时，首次触页策略与迁移可能改变内存归属，需单独核查。

## Quant/Low-Latency 场景

每线程统计计数、SPSC head/tail、行情产品状态和撮合分片元数据都可能出现 false sharing。按线程写入所有权设计布局，并由低频聚合线程读取统计，通常比让每笔消息更新全局原子计数更值得尝试；聚合本身仍会引入共享访问和观测滞后。

对于 SPSC，隔离游标可减少无关写争用，但消费者仍需观察 producer 发布的数据，真实通信不会消失。把吞吐优化转成端到端延迟收益之前，要把等待批次、排队和输出路径一起测量。

## 相关专题

- [C++ memory model](../concurrency/cpp-memory-model.md)：先证明正确，再测布局。
- [shared_mutex](../concurrency/shared-mutex.md)：只读业务下的同步元数据写入。
- 后续规划：NUMA first-touch、perf flamegraph、分支预测、benchmark methodology。

## 分层面试题

每个性能结论都应带条件、指标和验证手段。题目按机制与岗位需求推导，没有公司真题归属。
