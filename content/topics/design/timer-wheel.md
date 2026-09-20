---
{
  'schemaVersion': 1,
  'id': 'timer-wheel',
  'title': 'Timer Wheel：到期分桶、取消与推进边界',
  'description': '用有界单线程时间轮与扫描 oracle 验证跨轮到期、取消、槽位复用和关闭，区分逻辑 tick、真实时钟、到期提取与业务回调。',
  'category': 'design',
  'areas': ['Algorithm Coding', 'System Design'],
  'tags': ['timer-wheel', 'timeout', 'deadline', 'cancellation', 'event-loop'],
  'difficulty': 'L3',
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
  'prerequisites': ['取模和双向链表', '有界容量与句柄身份', '单调时钟和绝对期限'],
  'related': ['memory-pool', 'epoll-lt-et', 'mutex-condition-variable', 'benchmark-methodology'],
  'demo':
    {
      'file': 'examples/timer-wheel.cpp',
      'platform': 'portable',
      'exercise': '添加一个 deadline=13 的任务，与 deadline=1、5、9 同桶但跨不同轮，用独立扫描表核对它不提前触发；再取消同桶中间项，检查其他项仍按期输出。不要用 sleep 精度断言代替逻辑模型验证。',
    },
  'references':
    [
      {
        'title': 'Varghese and Lauck, Hashed and Hierarchical Timing Wheels, 1997',
        'url': 'https://www.cs.columbia.edu/~nahum/w6998/papers/ton97-timing-wheels.pdf',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Netty 4.1.138.Final: HashedWheelTimer',
        'url': 'https://netty.io/4.1/api/io/netty/util/HashedWheelTimer.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Netty 4.1.138.Final: Timeout cancellation',
        'url': 'https://netty.io/4.1/api/io/netty/util/Timeout.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux kernel: hrtimers design rationale',
        'url': 'https://docs.kernel.org/timers/hrtimers.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux man-pages 6.19: timerfd_create',
        'url': 'https://man7.org/linux/man-pages/man2/timerfd_create.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: steady_clock',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/time.clock.steady',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'timer-wheel-q01',
        'level': 'L1',
        'prompt': 'timer wheel 如何找到本次需要检查的计时器？',
        'answer': '把到期 tick 对桶数取模，计时器放入对应桶；每次推进一 tick 时检查当前桶。本例还保存完整绝对 deadline，避免只凭相同桶号把下一轮的任务提前触发。桶中可能有多个任务，扫描和到期输出都有成本，因此不能只说数组下标计算是 O(1) 就结束分析。',
        'rubric': ['deadline 映射桶', '完整 deadline 区分轮次', '桶扫描与输出成本'],
        'source': { 'kind': 'derived', 'rationale': '根据时间轮索引与跨轮碰撞的处理机制设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q02',
        'level': 'L1',
        'prompt': 'tick 大小和真实回调延迟是什么关系？',
        'answer': 'tick 决定时间量化粒度。若把未来绝对 deadline 向上取整到 tick 边界，量化增加的等待不足一 tick，但事件循环调度、桶处理和回调排队还会增加迟到。这个量化界限要求时钟映射和推进正确，不能写成回调一定在一 tick 内执行；示例没有真实时钟或回调。',
        'rubric': ['量化与执行延迟区分', '向上取整条件', '示例不测真实时间'],
        'source': { 'kind': 'derived', 'rationale': '根据量化误差与实际调度迟到的不同来源设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q03',
        'level': 'L1',
        'prompt': '到期时间正好是完整一轮后，是否落在当前桶就应立刻执行？',
        'answer': '不应。本例要求 deadline 严格大于 now，桶号相同也仍是未来任务。now=0、四个桶时 deadline=4 放在桶 0，要等推进到 tick 4 才输出。完整 deadline 保留轮次信息；只记录取模结果会混淆当前时刻与未来整轮。',
        'rubric': ['同桶不等于到期', '完整一轮边界', '保留绝对时间'],
        'source': { 'kind': 'derived', 'rationale': '根据模索引丢失轮次信息的边界情形设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q04',
        'level': 'L1',
        'prompt': 'cancel 返回成功具体保证了什么？',
        'answer': '在本单线程实现中，它说明对应 active 计时器已从桶移除并归还槽位，以后不会由该计时器产生新的到期事件。已经取消、已经输出或句柄不匹配时返回 false。它不停止业务操作，也不撤回此前 advance_one 已交给调用方的事件副本。',
        'rubric': ['只取消 active 计时器', '重复和已输出返回 false', '不撤回到期事件或业务'],
        'source':
          { 'kind': 'derived', 'rationale': '根据取消操作的状态边界与实际业务动作差异设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q05',
        'level': 'L1',
        'prompt': '相同 deadline 的任务必须按注册顺序输出吗？',
        'answer': '这取决于接口约定。本例使用桶头插入，没有承诺注册顺序；测试比较同一 tick 的 token 集合，避免把实现遍历顺序当作保证。若业务依赖稳定顺序，应明确 tie-break 规则，例如另存注册序号并按序交付，同时计入排序或尾插维护成本。',
        'rubric': ['本例不承诺 FIFO', '测试只验证约定顺序', '稳定顺序需明确实现'],
        'source': { 'kind': 'derived', 'rationale': '根据同期限任务的顺序合同与桶组织方式设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q06',
        'level': 'L2',
        'prompt': '长计时器落在本次经过的桶里，本例如何避免提前触发？',
        'answer': 'advance_one 检查 slot.deadline 是否不大于 now；尚未到期的项留在原桶，下一轮再检查。本例 deadline=20 会在桶 0 的早期轮次被看到但保留。这种实现简单，却让长任务每轮被重新扫描；分层时间轮可以减少重复检查，但增加层级搬移和边界处理。',
        'rubric': ['完整期限比较', '长任务保留', '重扫成本与分层取舍'],
        'source':
          { 'kind': 'derived', 'rationale': '根据单层时间轮对超出一轮范围任务的处理设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q07',
        'level': 'L2',
        'prompt': '如何让取消不需要遍历整个桶？',
        'answer': '句柄提供槽位索引和本轮注册身份，槽位保存 previous 与 next，因此校验后可以直接摘除头、尾或中间节点。归还槽后，新注册使用本 wheel 实例内递增的 id，旧句柄即使索引相同也无法取消新任务。wheel 不可移动且操作时须仍存活；本例没有并发调用保护。',
        'rubric': ['句柄直接定位', '双链摘除', '身份复用与寿命前提'],
        'source':
          { 'kind': 'derived', 'rationale': '根据直接定位和双链表局部更新实现取消的机制设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q08',
        'level': 'L2',
        'prompt': '事件循环落后了十个 tick，能否把游标直接改到当前桶？',
        'answer': '不能直接这样修改本例，因为中间桶的到期项会被跳过。调用方可逐 tick 补进度并消费每批事件；若限制每轮补偿预算，尚未完成的进度须保留并继续处理。也可以另写有证明的批量推进算法，但必须检查所有遗漏期限，不能把跳过桶当作性能优化。',
        'rubric': ['不能遗漏中间桶', '有界补偿保留进度', '批量推进需要新证明'],
        'source':
          { 'kind': 'derived', 'rationale': '根据事件循环延迟对时间轮推进不变量的影响设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q09',
        'level': 'L2',
        'prompt': '为什么本例返回一个到期批次，而不在桶遍历中直接调用回调？',
        'answer': '先复制到期数据并移除 active 计时器，返回后再由调用方处理，能把桶结构修改与任意业务代码隔开。遍历中直接回调会引入重入、注册、取消、异常和慢回调的问题。本例批次只含整数值，不拥有 token 所指代的业务对象；调用方仍要核对对象和请求代次。',
        'rubric': ['先摘除再交付', '避免任意回调干扰遍历', 'token 不拥有业务对象'],
        'source':
          { 'kind': 'derived', 'rationale': '根据到期提取与回调执行的状态和所有权边界设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q10',
        'level': 'L2',
        'prompt': '这个程序是否每个 tick 都只有 O(1) 工作？',
        'answer': '不是。推进要扫描当前桶 b 个条目，产出 k 个事件；长计时器也可能在其中。本例固定 Batch 初始化八个位置，推广容量 N 时还有 O(N) 的初始化及可能复制成本。因此桶算法部分约为 O(1+b+k)，完整教学接口还含批次成本；同时到期 N 项时至少要处理这些项。',
        'rubric': ['桶占用决定扫描', '到期输出成本', '本例 Batch 全容量成本'],
        'source':
          { 'kind': 'derived', 'rationale': '根据实际代码而非取模操作的复杂度进行审查设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q11',
        'level': 'L3',
        'prompt': '连接超时很少真正到期，为什么可能适合时间轮？',
        'answer': '若应用允许相应粒度、注册和取消很多而到期少，直接入桶与按句柄摘除可能节省维护全局有序结构的成本。仍需检查期限分布、桶碰撞、长任务重扫和取消同步。要求精细最早期限选择或高精度执行时，可以比较堆、平衡树及操作系统计时设施，不能只凭任务总量选择。',
        'rubric': ['注册取消为主的负载', '粒度与碰撞前提', '有序结构的适用需求'],
        'source': { 'kind': 'derived', 'rationale': '根据不同计时负载的主要操作和精度要求设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q12',
        'level': 'L3',
        'prompt': '一批心跳在同一 tick 到期，应如何控制尾延迟？',
        'answer': '先记录到期量、桶扫描成本和业务执行时间，判断瓶颈。可将到期提取与执行分开并对执行设预算，但队列容量、迟到和继续调度要显式管理；协议允许时也可分散初始相位。不能静默丢掉超出预算的任务，或只测注册吞吐就宣称超时处理足够快。',
        'rubric': ['同步到期形成突发', '预算与继续调度', '不静默丢弃且测迟到'],
        'source':
          { 'kind': 'derived', 'rationale': '根据同期限集中触发对事件循环工作预算的影响设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q13',
        'level': 'L3',
        'prompt': '如何把这个逻辑模型接到真实时间？',
        'answer': '先选择符合业务语义的时钟和固定 epoch、tick 宽度，再把绝对期限向上量化并检查转换及加法溢出。事件循环按实际已到边界推进，落后时补偿。持续时间可用合适的单调时钟；Linux 是否把系统挂起算入超时还涉及 CLOCK_MONOTONIC 与 CLOCK_BOOTTIME 的差异。模型本身没有实现这些接入步骤。',
        'rubric': ['时钟 epoch 与量化', '溢出和补偿', '挂起语义与平台边界'],
        'source':
          { 'kind': 'derived', 'rationale': '根据逻辑 tick 到真实计时设施的映射要求设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q14',
        'level': 'L3',
        'prompt': '周期任务应该从上一次 deadline 还是实际完成时刻重新安排？',
        'answer': '固定频率通常以上次计划 deadline 加周期，能避免把每次执行耗时都累积为相位漂移，但落后时需要定义补跑、合并或跳过。固定延迟从实际完成时刻再加周期，保持执行间隔，却会随耗时推迟。本例只实现一次性计时器，周期语义应由上层明确，且仍需处理期限溢出和关闭。',
        'rubric': ['固定频率与固定延迟', '落后时策略', '一次性模型不含周期合同'],
        'source':
          { 'kind': 'derived', 'rationale': '根据周期重排基准对漂移和追赶行为的影响设计。' },
        'companies': [],
      },
      {
        'id': 'timer-wheel-q15',
        'level': 'L3',
        'prompt': 'stop 清空待到期任务后，能否立即释放所有业务对象？',
        'answer': '要看调用方是否还持有已返回的到期批次，以及业务动作是否已经排队或执行。本例 stop 只清 active 槽位并拒绝新注册，批次副本仍保留 token。关闭协议还应停止提交、结清执行队列与对象借用，或让事件凭请求代次识别失效；计时器数量归零并不证明业务引用都已结束。',
        'rubric': ['stop 只清 active', '已输出批次仍在', '关闭与业务寿命协议'],
        'source':
          { 'kind': 'derived', 'rationale': '根据计时器关闭与外部到期工作之间的所有权差异设计。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

Timer wheel 把到期时间映射到一组循环桶，每推进一个 tick，就检查对应桶里的任务。通过句柄直接定位双向链表节点，可以让注册和取消只修改少量元数据；同桶碰撞、长计时器和集中到期仍会增加工作量。本章实现 C++20 单线程、固定容量的逻辑时间轮，保存绝对 deadline 区分不同轮次。它只产生到期事件，不读取真实时钟，也不承诺业务回调准时执行。

## 核心概念

| 概念     | 本例约定                            | 不能由此推出的结论           |
| -------- | ----------------------------------- | ---------------------------- |
| tick     | uint64_t 逻辑时间，advance_one 加一 | 调用一次等于真实经过固定时长 |
| 桶号     | deadline 对 4 取模                  | 同一桶中的任务同时到期       |
| active   | 已注册、未取消、未输出的计时器      | 业务操作仍在进行             |
| 到期批次 | 最多 8 个 Event 的值副本            | token 对应的业务对象仍活着   |
| cancel   | 摘除一个仍 active 且身份匹配的任务  | 已返回的批次或业务动作被撤回 |
| stop     | 清空待到期项并永久拒绝新注册        | 外部业务执行队列已经排空     |

时间轮是一组算法和工程取舍。Varghese 与 Lauck 的 1997 年论文区分单轮范围、哈希扩展和多粒度层级，分别讨论注册、取消、每 tick 维护和到期处理。本例选用单层分桶加绝对 deadline 的简化模型，没有实现分层搬移。[Timing Wheels 原论文](https://www.cs.columbia.edu/~nahum/w6998/papers/ton97-timing-wheels.pdf)

库的具体合同也要单独看。Netty 4.1.138.Final 的 HashedWheelTimer 面向近似 I/O 超时，文档明确 tick 粒度与执行时机的差异；其线程、默认参数和容量策略不能直接当作本 C++ 示例的行为。[Netty HashedWheelTimer](https://netty.io/4.1/api/io/netty/util/HashedWheelTimer.html)

## 原理深入

### 取模保留桶号，绝对期限保留轮次

假设 now=0，四个桶中，deadline 为 1、5、9 的任务都进入桶 1。推进到 tick 1 时，只有第一项满足 deadline 不大于 now，其余留在原桶；tick 5 和 tick 9 再分别处理。只记录桶号就会丢失轮次信息。

deadline=4 则落在桶 0，需等走完四个 tick 才到期。schedule 明确拒绝 deadline 不大于 now，避免把“当前已处理桶中的即时任务”隐式推迟一整轮。已经过期的请求可以由上层走立即处理队列，也可以按业务规则拒绝；本接口选择后者。

### 取消必须关联一次注册身份

一个槽被取消后可能马上用于另一任务，旧索引仍相同，因此句柄同时保存 owner、index 和 id。schedule 给每次成功注册分配本 wheel 实例内单调递增的 id，cancel 只接受 active 且 id 匹配的槽。wheel 不可复制或移动，所有句柄操作都要求原 wheel 仍存活；新 wheel 恰好复用旧地址不属于检测合同。

id 不回绕：达到 uint64_t 最大值时拒绝新注册，最大 id 本身不使用。这个路径在代码中可以检查，但测试没有执行到如此多次注册。逻辑 tick 的末尾则使用 near_limit 实例实际验证，在最大值处停止推进，避免加一回绕。

### 到期提取先完成，再交给业务

advance_one 在扫描时先保存 next，复制 Event，再将当前槽摘链并放回空闲链，最后继续原来的 next。这使链表修改不会丢掉尚未检查的节点。返回批次后，这些任务都已不 active；cancel 返回 false，不能再把它们当作尚未到期的任务撤销。

本实现不在扫描中运行回调，因而没有回调重入和回调抛异常的路径。调用方拿到的只是 token、deadline、emitted_at 的值副本。业务 token 可以被调用方复用，它也不拥有对象；若需识别过期请求，事件数据还应关联业务代次或有效对象引用。

## 数据结构/系统内部实现

heads 保存每个桶的链表头，slots 保存八个任务槽。槽位含绝对期限、业务 token、注册 id、previous、next 和 active。空闲槽用 next 串成另一条链，pending 记录活跃任务数。

| 操作        | 状态变化                                          | 结果                                         |
| ----------- | ------------------------------------------------- | -------------------------------------------- |
| schedule    | 校验关闭、期限、容量和 id，再取空槽并头插桶       | 成功返回句柄；失败返回明确错误且不改任务集   |
| cancel      | 验证句柄，更新相邻链指针，归还槽位                | 成功为 true；陈旧或非 active 为 false        |
| advance_one | now 加一，扫描当前桶并提取到期项                  | 返回批次；关闭或 tick 已到最大值时为 nullopt |
| stop        | 清空桶、关闭全部 active 槽，恢复空闲链并置 closed | 返回本次取消数量；重复调用返回 0             |

检查不变量时，应把空闲链和每个桶放在一起看：每个槽恰好属于空闲链或一个桶；active 槽位于 deadline 对应桶；pending 等于 active 数；成功输出或取消后该注册 id 不再 active。next 在两种状态下用途不同，必须先从桶中正确摘除，再用于空闲链。

任意 tick 至多有八项到期，所以容量为八的 Batch 足以装下本轮全部事件。本例为方便验证把整个数组初始化为零，这带来与容量相关的初始化和可能复制成本，不能在性能分析中漏掉。调用方只应读取 count 以内的有效事件。

相同 deadline 的输出顺序没有 FIFO 合同。本例桶头插入可能改变注册顺序，oracle 因此比较每个 tick 的 token 集合。如果业务需要顺序，应明确并实现 tie-break 规则。

### 从真实时间映射到 tick

真实接入需选择固定 epoch 与正的 tick 宽度 q。先计算 d=deadline−epoch 和 now_time=clock_now−epoch，并检查两者是可表示的非负偏移。期限向上量化为 ceil(d/q)，相应边界不早于原 deadline；当前时间则只推进到已经到达的边界，即 floor(now_time/q)。转换、差值和加法都要检查表示范围。

不能随意用 floor(current/q)+ceil(delay/q) 替代绝对期限的向上量化。例如 epoch=0、当前为 9、q=10、延迟为 2，目标应是 11，对应边界 20；前一个算法却得到边界 10，会提前。此处数字只解释取整，不是示例采用的真实时间单位。

C++20 steady_clock 提供不随物理时间前进而倒退的时点语义，适合表达相应持续时间，但它不保证线程按时获得调度。[N4861 steady_clock](https://timsong-cpp.github.io/cppwp/n4861/time.clock.steady)

Linux 接入还要决定挂起时间是否计入超时：CLOCK_MONOTONIC 不计系统挂起时间，CLOCK_BOOTTIME 包含它。timerfd 可由 epoll 监控，一次 read 可能返回多次到期的累计数量，不能把一次可读事件固定解释成一个 tick。以上是 Linux man-pages 6.19 所述接口语义，本程序没有调用 timerfd。[timerfd_create(2)](https://man7.org/linux/man-pages/man2/timerfd_create.2.html)

## C++ runnable demo

```cpp include=examples/timer-wheel.cpp

```

保持断言启用，测试中的部分操作位于 assert 内：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/timer-wheel.cpp -o /tmp/timer-wheel
/tmp/timer-wheel
```

WSL2 x86-64、GCC 13.3 实际输出：

```text
oracle: 24 ticks, 8 exact emissions, 3 simultaneous at tick 9
boundaries: stale/cross-wheel/closed rejected; tick limit does not wrap
logical ticks only; no wall-clock latency or callback execution measured
```

测试先填满八个槽，取消 token 4，再复用槽位加入 token 99。独立参考表没有桶结构，每个 tick 都扫描全部八项期望记录。其非空结果为：

| tick | 应输出的 token 集合 |
| ---- | ------------------- |
| 1    | 1                   |
| 4    | 2                   |
| 5    | 3                   |
| 9    | 5、6、99            |
| 12   | 7                   |
| 20   | 8                   |

从 tick 1 到 24，程序逐步核对输出数量、集合、deadline、emitted_at 和 pending；空 tick 同样要输出零项。另一个测试将八项全部设为 tick 1，验证整个容量同时到期可一次取出。边界检查覆盖满容量、非未来期限、重复取消、陈旧和跨 wheel 句柄、已输出后的取消、重复 stop，以及 stop 后批次副本仍在。

near_limit 从 UINT64_MAX−1 开始，安排最后一个可表示 tick 的任务，输出一次后再次推进返回 nullopt。它验证的是 tick 边界；注册 id 的耗尽分支仅做代码边界推理，没有冒充运行覆盖。

严格编译和 ASan/UBSan 均通过。所有数据和循环有界，不用 sleep，也不验证某次调度会准点发生。程序没有周期任务、线程同步、真实时钟、网络或业务回调。示例中的 emitted_at 是逻辑 tick，不是观测到的物理触发时间。

## 高频追问

### 事件循环落后时如何推进？

本例每次只走一个 tick，补偿必须依次处理遗漏的 tick，并接收其全部批次。直接把 now 跳到当前时刻再看一个桶，会漏掉中间桶的任务。为了避免一次追赶占满事件循环，可以分轮设置推进预算，但要保留未完成进度并重新调度；不能把未处理的 tick 当作已经处理。

长时间空档可用另一个经过证明的批量算法处理，例如扫描所有 active 项并重新组织状态。这会改变复杂度与输出顺序合同，不是给现有 now 赋一个更大的值就完成了。

### 分层时间轮解决什么问题？

多个粒度不同的轮可以把远期任务放在较粗的层，接近期限时移向更细的层，减少长任务在细轮中反复检查。代价包括层级边界、搬移批次和推进逻辑的复杂度。原论文讨论了哈希与分层扩展的空间、维护成本取舍；本例的单层重扫结果不能代表所有分层实现。[Timing Wheels 原论文](https://www.cs.columbia.edu/~nahum/w6998/papers/ton97-timing-wheels.pdf)

### cancel 成功是否表示业务已经停止？

本例只移除尚未输出的计时器。业务操作可能独立继续，或者之前已经产生了待执行动作。实际库也会定义自己的取消边界，不能跨库套用；例如 Netty Timeout 对已经执行或取消的任务不再产生取消副作用。[Netty Timeout](https://netty.io/4.1/api/io/netty/util/Timeout.html)

### 周期任务从哪里算下一个期限？

固定频率用上一次计划期限加周期，能保持计划相位，但落后时需要规定补跑、合并或跳过。固定延迟从本次完成时刻重新计时，会包含执行耗时带来的推迟。本例只有一次性任务；上层若重新注册，还要处理已经过期、容量耗尽和关闭结果。

## 容易答错的点

| 错误说法                             | 修正与原因                                   |
| ------------------------------------ | -------------------------------------------- |
| 取模后在当前桶，就可以触发           | 完整 deadline 还可能位于下一轮               |
| 桶号回绕等于逻辑时间也回绕           | 桶循环与时间表示是两件事，时间边界需单独处理 |
| tick 是 1 ms，所以回调最多晚 1 ms    | 调度、扫描和业务执行排队还会增加迟到         |
| cancel 成功会停止对应网络操作        | 本例只移除计时器，外部操作有自己的取消协议   |
| 修改 now 可以快速补上所有进度        | 中间桶尚未处理，可能漏掉到期项               |
| stop 后所有 token 都不会再出现       | 已返回的批次仍由调用方持有                   |
| 所有同期限项都会按注册顺序执行       | 本例无此合同，稳定顺序需另行实现             |
| O(1) 注册意味着整个计时设施都是 O(1) | 扫描、输出、批次存储和回调都有独立成本       |

## 性能分析

把容量记为 N、桶数记为 B、当前桶条目数记为 b、到期数记为 k。本例注册和取消只进行固定数量的校验、索引和链更新，为 O(1)；初始化与 stop 为 O(N+B)。桶推进部分需 O(1+b+k)，同一桶可容纳全部 active 项，因此最坏 b 可达 N。

完整 advance_one 还初始化容量 N 的 Batch，返回时也可能复制整块值，因此此教学 API 的成本还包含 O(N)。N=8 时这是小的有界对象，但推广到大容量系统不能忽略。生产接口可以采用调用方提供的输出空间或分批提取；输出满后尚未交付任务的状态必须保留，不能静默丢弃。

本例长计时器每经过对应桶一圈就被检查一次，存活 r 圈便可能被检查 r 次。增加桶数会增加桶头存储，并改变碰撞和访问模式；缩小 tick 会提高理论时间粒度，也增加推进频率。选择需要参考任务间隔分布、取消率和同时到期的峰值，而非只看平均 pending。

堆或平衡树维护有序期限，适合需要直接找到最早期限的设计；时间轮通过分桶接受相应粒度和维护方式。Linux 的 hrtimers 设计说明讨论了高精度需求与当时内核时间轮实现的不同取舍。该文包含历史实现背景，不能把其中某一版本的级联成本描述成今天所有时间轮的统一实现。[Linux hrtimers rationale](https://docs.kernel.org/timers/hrtimers.html)

实际测量应分开记录注册、取消、每 tick 扫描、到期提取及业务执行；迟到可定义为实际业务开始时刻减计划期限，并报告 p50/p99/p99.9 与未完成积压。集中到期和长暂停后的追赶尤其值得单测。程序没有性能实测，sanitizer 结果只用于有限功能路径验证。

## Quant/Low-Latency 场景

会话心跳、请求超时和行情陈旧检测往往有不同的期限合同。请求已完成时取消其超时计时器，可以避免以后再生成到期事件；但一个已交付的事件仍可能晚于请求完成才被处理。因此业务处理应核对请求身份、会话代次及当前状态，不能只看到某个 token 就重试或宣布失败。

大量连接按相同相位注册心跳，会形成集中到期。可以把到期提取与业务执行分开，给执行设置预算，并在协议允许时分散相位。预算会影响迟到和队列占用，需要记录积压并保证后续继续处理。计时器槽位已经释放，不代表其到期工作已经完成。

对误差容忍较大的超时管理，时间轮是可比较的方案；严格精度需求仍要评估时钟、唤醒机制和调度条件，数据结构本身不给硬实时保证。交易决策也不能把“尚未执行超时回调”当作行情依然新鲜的证据，使用时仍应检查所需时间或代次条件。

关闭流程应先阻止新注册，再决定未到期任务的取消政策，随后结清已经输出或排队的工作，最后释放业务对象。本例 stop 返回被取消的 pending 数量，仅覆盖计时器内部状态。

## 相关专题

- [内存池](memory-pool.md)：槽位复用与对象身份检查可以帮助理解陈旧句柄，但不替代业务寿命管理。
- [epoll LT / ET](../network/epoll-lt-et.md)：真实事件循环如何接入文件描述符就绪，以及为什么一次通知不等于一次业务事件。
- [mutex 与 condition_variable](../concurrency/mutex-condition-variable.md)：理解绝对期限、等待谓词和线程关闭；本例本身为单线程。
- [Benchmark 方法](../performance/benchmark-methodology.md)：分别验证逻辑正确性和真实迟到分布，避免用 sleep 断言证明精度。

## 分层面试题

基础题建立桶、期限与取消的语义；机制题检查跨轮、摘链、补进度和到期提取；设计题比较计时负载、真实时钟接入、周期任务与关闭协议。
