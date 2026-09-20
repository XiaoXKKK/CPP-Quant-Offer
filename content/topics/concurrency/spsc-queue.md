---
{
  'schemaVersion': 1,
  'id': 'spsc-queue',
  'title': 'SPSC 队列：回绕、槽位生命周期与排空',
  'description': '用持有资源的消息实现有界 SPSC 环，证明发布与槽位复用两条同步链，验证满队列重试、回绕和停止后的资源清理。',
  'category': 'concurrency',
  'areas': ['C++ Memory Model', 'Multithreading / Atomic / Lock-Free'],
  'tags': ['spsc', 'ring-buffer', 'object-lifetime', 'backpressure'],
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
  'prerequisites': ['release/acquire 与 happens-before', '移动构造、析构与 RAII', '环形缓冲区'],
  'related':
    [
      'cpp-memory-model',
      'cpu-cache-false-sharing',
      'raii-exception-safety',
      'mutex-condition-variable',
    ],
  'demo':
    {
      'file': 'examples/spsc-queue.cpp',
      'platform': 'portable',
      'exercise': '把边界测试增加到物理槽数 4 和 7，重新计算资源总数并保留满时不移动、单槽释放后可重试及 FIFO 断言；说明每次复用前哪一次 acquire 保护了 optional 的销毁与重建。',
    },
  'references':
    [
      {
        'title': 'C++20 draft N4861: atomic memory ordering',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/atomics.order',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: data races and coherence',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/intro.races',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: object lifetime',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/basic.life',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: optional construction',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/optional.ctor',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: optional assignment and emplace',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/optional.assign',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: optional reset',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/optional.mod',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: thread join',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/thread.thread.member',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: atomic lock-free property',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/atomics.lockfree',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC 13.3 instrumentation options',
        'url': 'https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Instrumentation-Options.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'spsc-queue-l1-01',
        'level': 'L1',
        'prompt': 'SPSC 中的单生产者、单消费者具体限制哪些操作？',
        'answer': '同一队列的入队路径由一个执行者负责，出队路径由一个执行者负责，两者可以并发。不能同时让两个线程调用 try_push，也不能让两个线程调用 try_pop。构造应在启动使用线程之前完成，销毁和重置须等所有访问停止。',
        'rubric': ['分别限制入队与出队执行者', '允许一推一取并发', '生命周期需外部协调'],
        'source':
          { 'kind': 'derived', 'rationale': '从索引单写者及槽位所有权推导调用方必须遵守的前提。' },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l1-02',
        'level': 'L1',
        'prompt': '本例物理槽数为 5 时，可存几条消息，怎样判断空和满？',
        'answer': '可存 4 条消息。read 与 write 相等表示空，next(write) 与 read 相等表示满，始终保留一个空槽区分两种状态。物理槽数和可用容量必须在接口中说清，否则容易产生越界预期或容量少一的问题。',
        'rubric': ['容量是物理槽数减一', '空时索引相等', '满时下一写位等于读位'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据环形状态编码推导容量边界，检查空满条件是否一致。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l1-03',
        'level': 'L1',
        'prompt': 'try_push 返回 false 时，传入 std::move(packet) 会丢掉消息吗？',
        'answer': '本例先检查是否有空位，失败时不执行移动构造，所以 packet 保留原资源。std::move 只是允许移动的类型转换，真正转移发生在槽位 emplace 时。调用者可以保留同一条消息重试，成功后才推进输入位置。',
        'rubric': ['失败在移动之前返回', 'std::move 本身不搬运', '成功后才推进输入'],
        'source':
          {
            'kind': 'derived',
            'rationale': '结合 try_push 的执行顺序与移动语义推导失败时的所有权。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l1-04',
        'level': 'L1',
        'prompt': '为什么槽位用 optional<T>，而不是启动时构造 Slots 个 T？',
        'answer': 'optional 可以先处于无值状态，在生产者获得槽位后构造 T，消费者移出后 reset 销毁槽内对象。这允许本例使用没有默认构造函数的 Packet，并明确每一轮复用的生命周期。optional 本身没有线程安全保证，仍依赖两条索引同步链。',
        'rubric': ['区分存储与 T 的生命周期', '支持非默认构造负载', 'optional 不自动同步'],
        'source':
          {
            'kind': 'derived',
            'rationale': '用 optional 的构造与 reset 规则解释非平凡负载的槽位管理。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l1-05',
        'level': 'L1',
        'prompt': '消费者得到空结果，能否马上判断生产已经结束？',
        'answer': '不能。空结果只表示本次读取没有取得已发布元素，生产者可能随后发布，原子读取也不保证获取全局最新状态。本例消费者事先知道总条数，收到全部条目才结束；在线服务则需要额外关闭协议。',
        'rubric': ['暂时为空与结束分开', '远端读取可能保守', '有限条数或关闭协议'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从并发空队列观察推导停止条件，防止把暂时无数据当成终止。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l1-06',
        'level': 'L1',
        'prompt': '队列对象和资源统计对象应在何时销毁？',
        'answer': '必须先停止所有队列访问并 join 使用线程，再销毁队列；统计对象还必须活到最后一个 Resource 析构之后。排空是业务上的消费要求，静止队列也能安全销毁未消费的 optional 值，但这不等于消息已经被业务处理。',
        'rubric': ['先停止访问并 join', '统计对象覆盖资源寿命', '清理资源不同于消费消息'],
        'source':
          {
            'kind': 'derived',
            'rationale': '结合 join 和 RAII 的寿命要求推导队列关闭后的清理顺序。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l2-01',
        'level': 'L2',
        'prompt': '消费者凭什么安全读取生产者构造的普通 Packet？',
        'answer': '生产者先在槽位完成 Packet 构造，再 release 写 write；消费者 acquire 读到相应发布或包含此前发布的后续推进，才读取该槽位。构造经由同步关系 happens-before 消费读取。数据成员不必各自变成 atomic，但不能越过已发布边界访问。',
        'rubric': ['构造先于 release 发布', 'acquire 观察相应推进', '槽位读取有 happens-before'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据 release/acquire 和队列发布顺序推导普通负载的可见性。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l2-02',
        'level': 'L2',
        'prompt': '消费者释放 read 为什么也要用 release，生产者为何需要 acquire？',
        'answer': '消费者先移出负载并 reset 槽内 optional，然后 release 推进 read。生产者 acquire 看到允许该槽位复用的推进后，才能重新 emplace。该方向保护旧对象的访问和销毁发生在新对象构造之前，省略它会让读写或生命周期操作缺少跨线程顺序。',
        'rubric': ['移出和 reset 在释放之前', '生产者获取复用许可', '保护生命周期与存储复用'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从同一槽位跨轮重用推导反向同步链，而非只证明首次发布。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l2-03',
        'level': 'L2',
        'prompt': '自己的索引用 relaxed 读取，是否也能把远端索引降为 relaxed？',
        'answer': '不能按同一个理由处理。自己的索引只由本端修改，线程内顺序和原子一致性支持读取自身推进结果。远端索引承担跨线程发布数据或释放槽位的职责，acquire 与对端 release 建立同步；只保证原子读写并不足以保护普通槽位。',
        'rubric': ['索引单写者前提', '本端读取不负责跨线程获取', '远端 acquire 保护普通数据'],
        'source':
          {
            'kind': 'derived',
            'rationale': '对照索引访问者和同步职责，检查 relaxed 使用理由是否完整。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l2-04',
        'level': 'L2',
        'prompt': '环索引不断重复同一个数，为什么这里不需要 CAS 的 ABA 标签？',
        'answer': '本例不通过 CAS 将旧值当成仍未变化的所有权证据。两端各自独占推进一个索引，保留空槽限制双方不能越过对方一整圈，原子一致性限制同一观察者退回更早的修改。正确性仍需要这些不变量；把相同索引方案搬到多生产者队列不能沿用本证明。',
        'rubric': ['没有 CAS 的旧值认领', '单写者与不能套圈', '不能直接扩展到多生产者'],
        'source':
          {
            'kind': 'derived',
            'rationale': '结合环位置重复和单写者边界解释本算法与 ABA 认领问题的区别。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l2-05',
        'level': 'L2',
        'prompt': '为什么本例限制 T 的移动构造和析构都不抛异常？',
        'answer': '出队需要先移动，再销毁槽内对象，最后发布槽位可复用。若移动抛出，原值可能已经变化，而读索引尚未推进，重试语义会变复杂。本例选择 noexcept 约束让状态转换完整执行；这种约束不证明操作不会分配、加锁或耗时很长。',
        'rubric': ['异常可能留下已改变的源值', '索引与生命周期转换要一致', 'noexcept 不等于低时延'],
        'source':
          { 'kind': 'derived', 'rationale': '依据移出、销毁与推进的顺序分析泛型负载的异常边界。' },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l2-06',
        'level': 'L2',
        'prompt': '只给队列增加 done 原子标志，消费者怎样避免漏掉最后一条？',
        'answer': '由生产者在最后一次入队成功后 release 设置 done，之后不再发布。消费者若先尝试出队失败，再 acquire 看到 done 为真，应重新检查队列并继续排空；只有确认关闭后的再次空结果才可退出。关闭前的空观察不能代表最后一次发布已被观察。',
        'rubric': ['最后发布先于关闭标志', 'done release/acquire', '观察关闭后重新出队'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从两个原子的观察先后推导关闭确认与最终排空的必要步骤。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l3-01',
        'level': 'L3',
        'prompt': '行情解析线程向策略线程传消息，满队列时应怎样设计背压？',
        'answer': '先按消息语义决定阻塞等待、拒绝、降载还是触发恢复，不能默默覆盖未消费槽位。需要保留失败消息的所有权、统计积压和失败次数，定义最大等待与停机行为。若丢弃会造成增量状态缺口，应使下游状态失效并执行相应恢复流程。',
        'rubric': ['背压服从消息语义', '失败所有权与观测', '缺口须显式失效和恢复'],
        'source':
          {
            'kind': 'derived',
            'rationale': '把有界队列失败路径用于行情数据流，推导不可隐含丢失的处理要求。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l3-02',
        'level': 'L3',
        'prompt': '怎样测量 SPSC 延迟，避免只测到空队列和资源分配开销？',
        'answer': '区分 try 操作成本、满空重试和入队到处理完成的延迟，记录负载速率、容量、占用分布及失败次数。固定线程和内存放置，分开预分配与逐条分配负载，报告吞吐和尾分位数。闭环生产会在阻塞时停止施压，需解释该负载是否代表真实到达过程。',
        'rubric': ['定义计时边界与重试', '控制放置和分配', '观察尾延迟及到达过程'],
        'source':
          { 'kind': 'derived', 'rationale': '根据队列排队与重试机制设计可归因的性能实验。' },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l3-03',
        'level': 'L3',
        'prompt': '把读写索引分到不同缓存行，为什么还不能承诺固定收益？',
        'answer': '两端写入相邻索引可能引起伪共享，分离布局值得测量。但槽位本身、远端索引读取和资源指针追踪仍会产生访存，收益也取决于核心拓扑和负载。对齐值应依据目标实现验证，比较同样业务量的吞吐、尾延迟和缓存指标，不能从布局变化直接推导倍数。',
        'rubric': ['伪共享机制', '剩余访存与拓扑因素', '实测且注明对齐假设'],
        'source':
          { 'kind': 'derived', 'rationale': '从两个单写索引的缓存流量推导布局优化的适用边界。' },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l3-04',
        'level': 'L3',
        'prompt': 'try_push 没有循环，能否直接宣布整个系统 wait-free？',
        'answer': '不能。元数据路径的算法步骤有界，但 atomic<size_t> 是否 lock-free 取决于实现，T 的移动和析构还可能执行耗时或阻塞操作。调用方满时反复重试依赖消费者继续运行，线程调度也影响完成。必须说明讨论的是哪层操作及其原语前提。',
        'rubric': ['有界路径不等于所有原语有界', 'atomic 与 T 的前提', '外层重试依赖对端'],
        'source':
          {
            'kind': 'derived',
            'rationale': '拆分队列内部路径和外部重试，检验进展保证的适用范围。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l3-05',
        'level': 'L3',
        'prompt': '现在线程数增加为两个生产者，能否仅把 write 改为 fetch_add？',
        'answer': '不能。两个生产者可能同时认领或越过可用槽位，预先推进索引又会让消费者看到尚未构造完成的数据。多生产者设计需要分别处理位置预留、初始化完成、发布次序和回收，可能采用每槽序列等协议；原 SPSC 的单写者证明已经失效。',
        'rubric': ['认领与发布需要分开', '防止可见但未初始化', '重新证明多生产者协议'],
        'source':
          {
            'kind': 'derived',
            'rationale': '移除单写者假设后检查槽位认领与发布，定位扩展中的同步缺口。',
          },
        'companies': [],
      },
      {
        'id': 'spsc-queue-l3-06',
        'level': 'L3',
        'prompt': '怎样验证回绕和资源寿命，而不等某次线程调度碰巧触发边界？',
        'answer': '先用容量一和非二次幂槽数做顺序状态测试：填满、失败保留消息、释放一个、成功重试、按序排空，并重复跨多圈。再做固定条数的并发传输，逐条核对序列和校验值，join 后检查资源创建销毁一致。Sanitizer 是补充，不能替代不变量和同步证明。',
        'rubric': ['确定性覆盖边界', '并发按序及资源检查', '动态工具的证明边界'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据本例状态转换和 RAII 计数设计不依赖偶然调度的验证方案。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

SPSC 是单生产者、单消费者队列。本例用有界环和一个保留空槽区分空满，生产者 release 发布构造好的元素，消费者 acquire 后移出；消费者销毁槽内对象，再 release 允许生产者复用。索引同步必须同时覆盖这两个方向。满时调用失败并保留输入，排队重试和停止由调用方处理，队列销毁前必须停止访问并 join。

## 核心概念

`write` 指向下一次写入位置，只有生产者推进；`read` 指向下一次读取位置，只有消费者推进。这里的 `Slots` 是物理槽数，可用容量为 `Slots - 1`。`read == write` 表示空，`next(write) == read` 表示满。保留空槽简化了状态编码，也意味着创建 5 个槽只能存 4 条消息。

每个 `optional<T>` 在无值、构造完成、移出及销毁之间转换。队列传递的是拥有资源的值，出队不返回槽位引用。调用者拿到返回值后可以继续处理，生产者随后复用原槽位不会改变该返回值。本例 `Packet` 使用 `unique_ptr<Resource>` 转移所有权；泛型类型也必须遵守移动后的对象约定，不能把悬空的外部借用包装成所谓“拥有值”。

一次失败表示当前调用没有取得元素或空位，不提供全队列实时快照。远端索引读取可以滞后，因此失败允许保守；调用方可按协议重试。SPSC 前提也包括不并发重置、不在工作线程仍访问时销毁。顺序边界测试由同一线程先后充当两端，满足没有同侧并发访问的要求。

## 原理深入

### 两个方向分别保护什么

生产者获得可用位置后，先执行 `slots_[write].emplace(...)`，再 release 推进 `write`。消费者 acquire 观察到允许读取该元素的发布后，才移动槽内负载。对于消费者一次观察到多个已发布元素的情况，生产者在被观察到的 release 之前完成的构造也被覆盖。同步依据见 [atomic ordering](https://timsong-cpp.github.io/cppwp/n4861/atomics.order)。

```text
生产者：构造槽内 T → write.store(release)
                              ↓ 被消费者 acquire 观察
消费者：write.load(acquire) → 移出 T → reset 槽位
```

另一条链从消费者返回生产者。`reset()` 销毁槽内已移动的对象后，消费者才 release 推进 `read`。生产者 acquire 观察到允许该槽位重用的推进，才能在同一 optional 中构造新值。它保护的不仅是普通字段读取，还包括旧对象的生命周期结束和新对象的开始，参见 [对象生命周期](https://timsong-cpp.github.io/cppwp/n4861/basic.life)。

```text
消费者：移出 T → reset 槽位 → read.store(release)
                                     ↓ 被生产者 acquire 观察
生产者：read.load(acquire) → 在获准槽位构造下一轮 T
```

本端索引只有本端写，读取它不负责接收对端的数据，因而使用 relaxed。远端索引承担上述同步责任，不能只因“索引本身是 atomic”就去掉 acquire。普通槽位与 optional 的状态标记都依靠这两条链获得安全访问顺序。

### 回绕为何不允许覆盖未消费对象

索引始终位于 `[0, Slots)`。`next` 到达最后一个位置时回到零，没有无界递增计数，也不要求槽数是二次幂。每次推进前检查边界，生产者无法跨过保留空槽追上消费者，消费者也不能超过已发布写位。

从某一端看来，对方原子索引的观察受 [原子一致性规则](https://timsong-cpp.github.io/cppwp/n4861/intro.races) 约束，不能在已观察过较新修改后退回更早的修改。滞后观察至多让本端提前报告满或空；单写者、推进检查和保留槽共同阻止它把绕回的数值解释成可跨越未交接区域的许可。这里没有 CAS 根据旧数值认领槽位，不能把这套推理直接用于 MPSC 或 MPMC。

### 负载抛异常时，索引不变还不够

移动构造抛出时，源对象可能已经改变。出队若只保证 `read` 未推进，并不能保证下一次还可取到原消息。本例要求 `T` 的移动构造和析构均不抛异常，保持“移出、销毁、允许复用”转换完整。`Packet` 不可复制，也没有默认构造函数，表明这些能力不是队列的必要前提。

`noexcept` 是接口约束，不保证函数不加锁、不分配或执行时间有界。泛型使用者仍须检查负载实现。`optional` 的 [移动构造](https://timsong-cpp.github.io/cppwp/n4861/optional.ctor)、[emplace](https://timsong-cpp.github.io/cppwp/n4861/optional.assign) 与 [reset](https://timsong-cpp.github.io/cppwp/n4861/optional.mod) 分别承担结果构造、槽内构造和销毁；返回局部 optional 时即使没有 NRVO，受约束的移动也可完成。

## 数据结构/系统内部实现

队列包含固定的 `array<optional<T>, Slots>` 和两个原子索引。初始化只创建无值的 optional，不预先创建 `Slots` 个 `T`。成功入队在一个空 optional 内移动构造对象，成功出队将对象移入返回值，再 reset 原槽。失败路径不访问槽位内容。

| 路径     | 元数据与负载动作                           | 返回后的所有权     |
| -------- | ------------------------------------------ | ------------------ |
| 入队成功 | 检查远端 read，构造槽内值，发布 write      | 槽内值接管移动结果 |
| 入队失败 | 发现未取得空位，未执行移动                 | 调用者保留输入     |
| 出队成功 | 检查远端 write，移动结果，reset，发布 read | 返回值持有消息     |
| 出队失败 | 未取得已发布元素，不访问槽内值             | 无返回消息         |

`Packet` 的资源在启动线程前分配。传输期间仅移动拥有者，消费者持有的返回值离开作用域时释放资源。`Counters` 使用原子计数避免测试本身引入数据竞争；这些统计操作不是队列算法的组成部分。示例故意保留简单索引布局，缓存行隔离和远端索引缓存留给受控性能实验。

没有并发 `size()`、`clear()` 或在线 `close()` API。分别读取两个索引不能自动得到同一时刻的队列大小；观测接口需要说明一致性要求。默认析构会清理仍有值的槽位，但前提是所有访问已经停止。清理未消费消息的资源与完成业务消费是两件不同的事。

## C++ runnable demo

程序分三类验证：确定性的空满与回绕测试、静止但未排空队列的析构、有限条数的并发 FIFO 传输。调用线程是唯一消费者，另一个 `jthread` 是唯一生产者。两端预先约定每轮 6000 条，消费全部消息后显式 join；没有在线关闭功能或超时取消协议。

```cpp include=examples/spsc-queue.cpp

```

```bash
g++ -std=c++20 -O2 -pthread -Wall -Wextra -Wpedantic -Werror examples/spsc-queue.cpp -o /tmp/spsc-demo
/tmp/spsc-demo
g++ -std=c++20 -O1 -g -pthread -Wall -Wextra -Wpedantic -Werror -fno-omit-frame-pointer -fsanitize=address,undefined -fno-pie -no-pie examples/spsc-queue.cpp -o /tmp/spsc-san
/tmp/spsc-san
g++ -std=c++20 -O1 -g -pthread -Wall -Wextra -Wpedantic -Werror -fno-omit-frame-pointer -fsanitize=thread -fno-pie -no-pie examples/spsc-queue.cpp -o /tmp/spsc-tsan
/tmp/spsc-tsan
```

代码为可移植 C++20，以上是 Linux GCC 测试命令。正常运行输出以 `SPSC checks passed; resources=55282` 开头；最后的 `always_lock_free` 是实现性质，不应把其他平台返回 0 当作功能错误。计数来自边界测试的 1280 个资源、未排空清理的 2 个资源和并发传输的 54000 个资源。

边界测试使用物理槽数 2、3、5，分别可存 1、2、4 条；每种执行 128 轮填满、失败保留输入、消费一个、重试成功及排空。它们保证覆盖相应状态，不依赖操作系统安排出一次“恰好满队列”的调度。并发测试用物理槽数 2、3、17，每种运行三轮，逐条检查序列号和校验值，join 后检查所有 Resource 已释放。索引范围和每轮传输条数有界，等待对端的重试次数没有固定上界。

本机 WSL、GCC 13.3.0 的正常和 ASan/UBSan 构建运行通过。独立 TSan 构建成功，启动时报 `ThreadSanitizer: unexpected memory mapping`，未得到该工具的竞争检查结果。GCC [插桩选项](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Instrumentation-Options.html) 说明 TSan 与 ASan 需分开使用。有限运行无报告也不能代替前述同步证明。

## 高频追问

“为什么不用一个 done 标志让消费者结束？”在线服务可以添加，但须定义最后发布和排空顺序。生产者在最后一条成功入队后 release 设置 done，之后不再发布。消费者若一次出队失败，再 acquire 读到 done 为真，必须重新出队检查并继续排空；只有确认关闭后的再次空结果才可退出。先前的空观察可能发生在最后发布之前，不能用它证明队列已经排空。这个协议还依赖只有该生产者关闭，以及不在关闭后继续写。

“队列为空时，主线程为什么仍要 join？”生产者最后推进 write 后可能仍在执行函数尾部，空队列只描述队列状态。成功的 [join](https://timsong-cpp.github.io/cppwp/n4861/thread.thread.member) 等待该线程完成，并与线程完成同步，之后才可销毁它仍可能访问的对象。

“把 read、write 都改为 seq_cst 能支持两个生产者吗？”更强内存序不能补上槽位认领协议。两个生产者仍可能处理同一个位置；用 fetch_add 预留位置也不等于内容已经构造完成。必须区分预留、完成和发布，并重新证明容量与复用规则。

“能否让消费者拿着槽位引用直接处理？”可以设计相应 API，但引用有效期必须延长到处理结束，之后才能发布 read。当前实现把值移出后立即释放槽位；返回内部引用再推进 read 会让生产者覆盖消费者仍在访问的对象。

## 容易答错的点

- 只解释生产者到消费者的发布，会漏掉消费者销毁后允许生产者重建的反向同步。
- `std::move` 不会在满队列检查之前自动取走资源。本例失败可重试，别先推进输入位置再看返回值。
- `optional` 的有值标记也是普通状态，需要同一槽位的同步保护；它不会自动成为并发容器。
- `try_pop` 为空不表示生产结束，资源被析构也不表示消息被业务处理。
- 索引回绕采用显式范围归零，本例没有依赖无符号单调计数溢出来恢复正确性。
- 移动和析构不抛异常只能满足状态转换约束，不能单凭这个声明推导低延迟或进展保证。

## 性能分析

每次 try 调用的元数据路径没有重试循环，检查次数有界。总成本还包含 `T` 的移动、析构及返回值构造，不能对任意 `T` 承诺固定时间。槽位存储为 `O(Slots × sizeof(optional<T>))`，初始化与最终逐槽清理随槽数增长，负载外部拥有的内存另计。

`atomic<size_t>` 是否 lock-free 属于实现性质，见 [atomic lock-free property](https://timsong-cpp.github.io/cppwp/n4861/atomics.lockfree)。即使本机该类型总是 lock-free，负载操作和调用方的重试仍需单独分析。满时循环直到成功依赖消费者取得进展；`yield()` 不保证立刻调度对方，也不给重试次数上界。本例不把整个传输过程称为 wait-free。

测量时区分一次 try 的成本、空满重试和从到达到处理完成的排队延迟。记录容量、到达速率、占用分布和失败次数，控制核心拓扑、线程绑定与内存放置。将连续小值、预分配资源句柄和逐条分配负载分别测试，避免把分配器或指针追踪的差异归到索引算法上。

相邻原子索引可能产生伪共享，分离缓存行或缓存远端索引值得做对照，但需要验证目标平台和刷新规则。批量发布可减少共享索引流量，也会推迟消费者看见消息，应同时比较吞吐和 p50/p99/p99.9。闭环生产者在队列满时停止生成新工作，可能隐藏真实到达过程的排队压力；报告时说明负载模型。本章没有计时数据，也未测得优化收益。

## Quant/Low-Latency 场景

行情解析线程到策略线程是一条可能满足 SPSC 的链路：一个发布端、一个消费端，队列内保持 FIFO。若多个解析线程合并到同一队列，就已经改变前提。按连接或分区建立多个 SPSC 可以保留单写者条件，但合并后的跨源顺序要由上层协议定义。

队列满时应依据消息语义选择背压。无法丢失的状态增量不能被静默覆盖；若系统决定拒收或丢弃，必须记录缺口，让依赖该状态的下游进入失效和恢复流程。某些可合并快照允许保留较新状态，但这需要显式合并协议，不能由生产者擅自修改消费者拥有的槽位。

停机时先停止接受新工作，完成约定的最后发布，再确认关闭并排空，等待线程退出后释放存储。若不要求处理剩余消息，也应记录被丢弃范围并按业务策略结束；示例中的未排空析构测试只证明资源清理安全。在线故障、超时和取消还需额外状态，本例有限条数协议没有覆盖这些场景。

## 相关专题

- [C++ memory model](cpp-memory-model.md)：补齐 acquire/release 与 happens-before 的基础规则。
- [CPU cache 与伪共享](../performance/cpu-cache-false-sharing.md)：设计索引布局和线程放置的对照实验。
- [RAII 与异常安全](../cpp/raii-exception-safety.md)：理解移动失败、资源拥有者与销毁边界。
- [mutex 与 condition_variable](mutex-condition-variable.md)：对照阻塞等待、关闭谓词与通知协议。

## 分层面试题

L1 检查队列状态、失败所有权和结束条件；L2 追踪同一槽位跨两轮复用的同步与生命周期；L3 将这些约束放进背压、停机和性能实验。回答时说明物理槽数、调用者数量、负载操作及停止协议，不能只写出两个原子索引就省略前提。
