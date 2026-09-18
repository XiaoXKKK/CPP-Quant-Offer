---
{
  'schemaVersion': 1,
  'id': 'cpp-memory-model',
  'title': 'C++ memory model：从 happens-before 到 SPSC',
  'description': '用双向 release/acquire 证明 SPSC 队列的发布与槽位复用，区分原子性、顺序和进展保证。',
  'category': 'concurrency',
  'areas': ['C++ Memory Model', 'Multithreading / Atomic / Lock-Free'],
  'tags': ['atomic', 'memory-order', 'spsc'],
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
  'estimatedMinutes': 45,
  'prerequisites': ['对象生命周期', 'std::thread 与 join', '环形缓冲区'],
  'related': ['cpu-cache-false-sharing', 'shared-mutex'],
  'demo':
    {
      'file': 'examples/cpp-memory-model.cpp',
      'platform': 'portable',
      'exercise': '将 SPSC 容量改为 1、2、8，保留满/空/回绕断言；画出发布和复用两条 happens-before 链，再尝试解释为什么不能直接改成 MPSC。',
    },
  'references':
    [
      {
        'title': 'C++ draft: data races',
        'url': 'https://eel.is/c++draft/intro.races',
        'kind': 'standard',
        'accessed': '2026-09-18',
      },
      {
        'title': 'C++ draft: atomic ordering',
        'url': 'https://eel.is/c++draft/atomics.order',
        'kind': 'standard',
        'accessed': '2026-09-18',
      },
      {
        'title': 'C++ draft: atomic properties',
        'url': 'https://eel.is/c++draft/atomics.lockfree',
        'kind': 'standard',
        'accessed': '2026-09-18',
      },
    ],
  'questions':
    [
      {
        'id': 'cpp-memory-model-q01',
        'level': 'L1',
        'prompt': '什么情况下会构成 C++ data race？',
        'answer': '不同线程的潜在并发冲突访问，至少一个写且至少一个非原子，若不存在规定的 happens-before 顺序，就会构成 data race。普通变量上的竞争会导致未定义行为。',
        'rubric': ['冲突与非原子', 'happens-before'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q02',
        'level': 'L1',
        'prompt': 'atomic 能自动保护旁边的 payload 吗？',
        'answer': '不能。原子性只覆盖原子对象自身。若把原子标志用作发布协议，需要 release/acquire 等同步并证明消费者观察了对应发布，之后才能访问非原子 payload。',
        'rubric': ['原子性范围', '发布协议'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q03',
        'level': 'L1',
        'prompt': 'volatile 能代替 mutex 或 atomic 吗？',
        'answer': '不能。volatile 主要约束特定访问的优化，不提供线程之间的同步或原子读改写。并发共享状态应使用标准原子或锁，并证明生命周期。',
        'rubric': ['不提供同步', '正确同步原语'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q04',
        'level': 'L1',
        'prompt': 'relaxed 提供哪些保证？',
        'answer': '它保留该原子操作的原子性和该对象的 modification order，但不据此建立周围非原子数据的跨线程发布顺序。纯统计计数可能适用。',
        'rubric': ['原子性和修改序', '无payload发布'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q05',
        'level': 'L1',
        'prompt': 'release 与 acquire 何时建立同步？',
        'answer': '需要 acquire 观察到对应 release 发布的值，或满足标准规定的 release sequence 条件。仅在两个线程各放一个 release/acquire，并不能自动建立同步。',
        'rubric': ['观察关系', '避免凭空同步'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q06',
        'level': 'L2',
        'prompt': 'SPSC 中为什么数据数组可以不是 atomic？',
        'answer': '生产者写入后发布 head，消费者观察发布后读取；消费者读取后发布 tail，生产者观察回收后复用槽位。双向链将每个槽位的冲突访问排序。',
        'rubric': ['发布链', '复用链'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q07',
        'level': 'L2',
        'prompt': '只给 SPSC head 使用 acquire/release 够吗？',
        'answer': '不够。tail 同时承担槽位回收协议；生产者覆盖旧槽位必须排在消费者完成旧读取之后。tail 的 release/acquire 为这个方向建立同步。',
        'rubric': ['读取完成', '覆盖前同步'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q08',
        'level': 'L2',
        'prompt': 'SPSC 为什么多留一个槽位？',
        'answer': '这是该实现区分满与空的方式：head==tail 为空，next(head)==tail 为满。内部 N+1 个槽位提供 N 的可用容量；使用计数或序号也能采用别的编码。',
        'rubric': ['满空编码', '可用容量'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q09',
        'level': 'L2',
        'prompt': 'seq_cst 能修复所有并发错误吗？',
        'answer': '不能。它约束相应原子操作的全序，但无法修复多个生产者同时写普通槽位、悬空指针、越界或错误的所有权设计。仍需算法不变量。',
        'rubric': ['排序不等于算法正确', '生命周期'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q10',
        'level': 'L2',
        'prompt': '测试十亿次无错能证明无 data race 吗？',
        'answer': '不能，测试只覆盖有限执行和硬件。需基于语言内存模型证明同步链，TSan 和压力测试作为补充，弱内存架构测试也不能代替证明。',
        'rubric': ['有限执行', '形式化顺序理由'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q11',
        'level': 'L3',
        'prompt': 'SPSC 改成 MPSC 只需 head.fetch_add 吗？',
        'answer': '不够。位置预留和数据完成发布是不同阶段；慢生产者可能留下洞，消费者不能读取未完成槽位。需逐槽序号或额外提交协议，并重新证明复用。',
        'rubric': ['预留与提交', '未完成槽位'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q12',
        'level': 'L3',
        'prompt': '如何分析 lock-free 与 wait-free？',
        'answer': 'lock-free 关注系统中持续有操作完成，wait-free 要求每个操作在有界步骤内完成。还需检查原子实现、分配器、重试环和外层阻塞是否破坏声明。',
        'rubric': ['系统与单线程进展', '完整路径'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q13',
        'level': 'L3',
        'prompt': '行情队列满了是否可以覆盖最旧元素？',
        'answer': '只有业务允许丢弃且有一致性恢复协议才行。订单簿增量通常不能静默丢失；覆盖也会破坏消费者正在读的槽位所有权，需要新的并发协议。',
        'rubric': ['业务完整性', '槽位所有权'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q14',
        'level': 'L3',
        'prompt': '批量发布 head 会改变什么？',
        'answer': '它可摊薄原子写与一致性流量，但增加元素等待批次提交的时间，改变尾延迟和可见性。需记录批量大小、到达率、满队列行为与端到端延迟。',
        'rubric': ['吞吐与尾延迟', '提交可见性'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'cpp-memory-model-q15',
        'level': 'L3',
        'prompt': '跨 NUMA 节点的 SPSC 为什么可能慢？',
        'answer': '共享游标和数据访问可能走远端一致性或内存路径。需测线程放置、内存 first-touch 和迁移；cache line 分离减少假共享却不会消除真实通信。',
        'rubric': ['NUMA放置', '真实共享仍存在'],
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

C++ memory model 规定线程间的顺序与数据竞争。原子性只保护原子对象；发布非原子数据需要正确的同步。消费者通过 acquire 观察生产者的 release 发布后，才能安全读取。SPSC 还须反向同步槽位复用。正确性靠 happens-before 证明，不能靠“x86 上跑过”。

## 核心概念

| 关系 / 属性        | 解决什么                 | 不能推出什么                 |
| ------------------ | ------------------------ | ---------------------------- |
| sequenced-before   | 同一线程内的求值顺序关系 | 不是所有机器指令都按源码执行 |
| synchronizes-with  | 特定同步操作连接线程     | 任意 acquire 都不会凭空同步  |
| happens-before     | 推导跨线程访问的偏序     | 不代表墙上时钟先后           |
| modification order | 每个原子对象的修改顺序   | relaxed 不产生跨对象总序     |
| lock-free          | 系统级进展属性           | 不保证每线程有限步完成       |

本专题代码目标是 C++20，使用稳定的 release/acquire/relaxed 子集。在线 working draft 持续演进，阅读 [原子顺序条款](https://eel.is/c++draft/atomics.order) 时留意版本，本文不依赖 consume 语义。

## 原理深入

设 producer 把元素写入槽位，然后 release-store 新 head；consumer acquire-load head 并观察到相应发布后，才读取该槽位。这是第一条链：

```text
producer: write slot → release(head)
                            ↓ reads-from / synchronization
consumer:             acquire(head) → read slot
```

只证明这一条还不够。consumer 读取结束后 release-store tail；producer acquire-load tail，确认槽位可重用后才覆盖。第二条链把旧读取排在新写入之前。没有这个方向的同步，环形缓冲在回绕复用时仍可能形成 data race。

线程自己独占修改的游标可以 relaxed-load。读取另一线程的游标用 acquire，发布自己的游标用 release。读到旧游标可能保守地报满或报空，但不能越过已发布的可用范围。算法还依赖“恰好一个 producer、恰好一个 consumer、不并发 reset”的前提。

## 数据结构/系统内部实现

用 `Capacity + 1` 个槽位区分满和空：head 等于 tail 是空；head 的下一位置等于 tail 是满，因此对外可用容量是 Capacity。数据数组不是原子数组，正确性来自游标建立的 happens-before。

这里只存 int，避免把 placement new、异常和析构掩盖在模板参数里。泛化到任意 T 时，要明确对象在何时构造、移动、销毁，以及异常会不会破坏游标不变量。缓存行隔离只是布局优化，详见 [false sharing](../performance/cpu-cache-false-sharing.md)。

## C++ runnable demo

先单线程验证空、满与回绕，再用两个线程传输 100000 个递增值，逐个检查顺序。断言是测试的一部分，运行时不要定义 NDEBUG。

```cpp include=examples/cpp-memory-model.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pthread examples/cpp-memory-model.cpp -o /tmp/spsc-demo
/tmp/spsc-demo
```

示例输出编译目标上 size_t 原子是否 always lock-free。try_push/try_pop 路径操作数有界，但外层重试和线程调度可能无限等待；不能把整个程序叫作 wait-free。测试和 TSan 都不能替代内存序证明。

## 高频追问

“为什么 relaxed 计数器有时没问题？”如果只要求每次递增原子、在线程 join 后统计结果，它不需要发布旁边的对象；若用该计数器宣告非原子 payload 可读，则需要额外同步。

“seq_cst 是否一键解决？”它给 seq_cst 操作增加统一排序约束，但错误的对象生命周期、越界和多生产者竞争仍然错误。先证明算法，再选择足够弱的序。

## 容易答错的点

- volatile 不提供 C++ 线程同步，也不会把复合操作变成原子。
- acquire 不是“刷新所有缓存”；它的作用是语言层顺序保证。
- 两个不同原子对象各自有序，不等于所有线程观察到同一跨对象顺序。
- ABA 与 data race 不同：原子 CAS 可以没有 data race，但仍错误接受被复用的逻辑状态。
- lock-free 不是无锁语法标签，原子实现可能使用库内锁，算法也可能因内存分配而阻塞。

## 性能分析

从每元素一次发布开始，再尝试批量发布，记录吞吐与排队尾延迟的变化。批量通常减少共享游标流量，却让第一个元素等待整批提交。把生产者和消费者分别绑到同核、同 socket 不同核、跨 NUMA 节点，才能分离调度和一致性通信成本。

不要用示例的 yield 重试成本代表生产系统忙等成本。忙等、pause、退避和休眠在延迟与 CPU 占用之间有不同权衡；必须在真实到达分布下测量。不要把编译器输出相同的两段指令解释为所有架构等价。

## Quant/Low-Latency 场景

单 feed handler 向单策略线程交付事件适合 SPSC。队列满时必须明确背压：等待、断流恢复或有审计的降级，不能静默丢订单簿增量。多路 feed 接入时，可每路一个 SPSC，由消费端合并；不应把 push 加一个 fetch_add 就宣称 MPSC 正确。

## 相关专题

- [CPU cache / false sharing](../performance/cpu-cache-false-sharing.md)：正确后再优化共享游标布局。
- [shared_mutex](shared-mutex.md)：读多写少的另一类所有权设计。
- 后续规划：ABA、hazard pointer、epoch reclamation、MPMC 序号协议。

## 分层面试题

先写同步链，再给结论；L3 重点是说明前提与失败模式，而不是背诵 memory_order 枚举。
