---
{
  'schemaVersion': 1,
  'id': 'market-data-fanout',
  'title': 'Market-data Fanout：慢消费者与状态交付',
  'description': '用两个增量订阅者和一个完整状态邮箱演示有界行情广播，明确慢消费者隔离、失效通知、批次提交范围和数据生命周期。',
  'category': 'design',
  'areas': ['System Design'],
  'tags': ['fanout', 'market-data', 'slow-consumer', 'backpressure', 'snapshot'],
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
  'estimatedMinutes': 55,
  'prerequisites':
    ['Feed Handler 的有效性与序号', '有界队列和对象所有权', '增量事件与完整状态的区别'],
  'related': ['feed-handler', 'spsc-queue', 'mutex-condition-variable', 'benchmark-methodology'],
  'demo':
    {
      'file': 'examples/market-data-fanout.cpp',
      'platform': 'portable',
      'exercise': '让慢消费者在第二批到来前仅消费一条，手算第三批时的剩余容量和失效位置；再让源在已有积压时收到坏批次，验证所有订阅者的健康门禁、源状态不变，以及先前已取得的值副本仍需业务有效性判断。',
    },
  'references':
    [
      {
        'title': 'LMAX Disruptor user guide: multicast and gating',
        'url': 'https://lmax-exchange.github.io/disruptor/user-guide/',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'NATS documentation: slow consumers',
        'url': 'https://docs.nats.io/learn/resilient-clients/slow-consumers',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'ZeroMQ socket options: high water marks and conflation',
        'url': 'https://zeromq.github.io/libzmq/zmq_setsockopt.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'gRPC flow control',
        'url': 'https://grpc.io/docs/guides/flow-control/',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: shared_ptr ownership',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.shared',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: data races and synchronization',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/intro.races',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'market-data-fanout-q01',
        'level': 'L1',
        'prompt': '多个线程从一个队列取消息，就等于向多个订阅者广播了吗？',
        'answer': '通常不是。同一个工作队列往往把每条消息交给其中一个消费者，广播则要求各订阅者按自己的合同获得对应数据。可以为每个订阅者保留队列或游标，也可以共享日志，但要分别定义进度、回收和落后策略。本例为两个增量订阅者各保留一个队列。',
        'rubric': ['工作分摊与广播区别', '每订阅者独立进度', '队列或游标的交付合同'],
        'source':
          { 'kind': 'derived', 'rationale': '根据工作队列与多播交付的消费者语义差异设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q02',
        'level': 'L1',
        'prompt': '增量队列满了，为什么不能覆盖最旧的一条然后继续标为 valid？',
        'answer': '消费者可能需要每条增量才能从已有状态推导新状态，覆盖会破坏连续性。必须选择等待、可靠保留并重放，或明确使该消费者失效后恢复。本例不覆盖增量，整批放不下就停用这个订阅者；其他订阅者可以继续，但不能把该订阅者写成无损交付。',
        'rubric': ['增量依赖历史连续性', '满时须显式策略', '失效隔离不等于无损'],
        'source': { 'kind': 'derived', 'rationale': '根据增量状态更新的不可随意遗漏性设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q03',
        'level': 'L1',
        'prompt': '什么样的数据可以只保留最新一份？',
        'answer': '当订阅合同只需要最新完整状态，且新值覆盖了所需状态域并携带明确版本或源序号时，可以合并中间状态。本例保存两个证券在同一源进度下的完整数量数组。若下游要每次变化、成交事件或审计历史，或者消息只是部分字段增量，就不能用最新一条替代全部过程。',
        'rubric': ['完整且覆盖所需状态域', '版本或源进度', '事件和审计合同不适用'],
        'source':
          { 'kind': 'derived', 'rationale': '根据完整状态替换与事件历史保留的不同合同设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q04',
        'level': 'L1',
        'prompt': '给每个消费者设有限队列，是否就能解决持续处理不过来的问题？',
        'answer': '有限队列限制这一阶段的积压，只能吸收一定突发；持续输入超过处理能力时终会满。还要规定满后的反馈、落盘、隔离或失效策略，并统计其他阶段和在途对象的内存。队列容量本身不能同时保证无限时长、任意慢消费者和无损交付。',
        'rubric': ['缓冲只吸收有限突发', '需要满后的策略', '全路径资源与长期速率约束'],
        'source':
          { 'kind': 'derived', 'rationale': '根据有限容量与持续速率差导致的积压增长设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q05',
        'level': 'L1',
        'prompt': '为什么事件除了 sequence 还需要 generation 或会话身份？',
        'answer': '不同重建或会话可能重新使用相同序号，generation 用来区分它们，避免旧消息混入新状态。它不自动证明状态新鲜，也不替代恢复协议。本例固定 generation 为 7，只有一个已建立的初始状态，没有实现换代、重连或基于时间的过期检测。',
        'rubric': ['区别不同状态代次', '不等于新鲜度或恢复', '固定代次示例边界'],
        'source':
          { 'kind': 'derived', 'rationale': '根据重连或重建时序号可重复使用的身份问题设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q06',
        'level': 'L2',
        'prompt': '失效通知也走已经满的数据队列，会有什么问题？',
        'answer': '通知可能和数据一起被拒绝，下游仍以为自己的状态有效。本例将 Health 放在队列之外，满时直接置 invalid，pop 先检查它，即使队列仍有四条也不返回数据。这只在单线程同步模型中成立；跨线程要定义同步，跨进程要定义可达的控制协议和失联处理。',
        'rubric': ['满队列可能阻塞失效信息', '控制状态独立于数据槽', '同步与失联处理边界'],
        'source':
          { 'kind': 'derived', 'rationale': '根据数据通道饱和时控制信息仍须可观察的要求设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q07',
        'level': 'L2',
        'prompt': 'pop 已经返回事件，随后订阅者被标为 invalid，这个值会自动失效或被撤回吗？',
        'answer': '不会。pop 返回的是拥有自身字段的值副本，健康门禁只影响后续取数，不能撤销调用方已取得的对象。内存仍有效也不表示它能代表当前有效行情。实际业务需要规定在何时检查代次与健康状态、怎样处理已经在途的工作，不能只在入队时检查一次。',
        'rubric': ['门禁不撤回已返回副本', '内存寿命与业务有效性区别', '在途工作与使用时点协议'],
        'source': { 'kind': 'derived', 'rationale': '根据发布后值副本的寿命与后续失效标记设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q08',
        'level': 'L2',
        'prompt': '广播结果显示源已接受、A 已入队、B 已失效，能否把整个批次原样重试？',
        'answer': '不能把这个结果当成全局失败后盲目重试，源和 A 已经推进，可能造成重复或序号错误。应分别处理源提交和各订阅者交付状态。示例每个消费者整批接收或拒绝，但不同消费者不构成一个全局事务；B 需要自己的恢复流程，本例不会自动重放。',
        'rubric': ['识别已经提交的部分', '逐订阅者结果处理', '不承诺跨消费者原子事务'],
        'source':
          { 'kind': 'derived', 'rationale': '根据广播批次的源提交与各订阅者交付范围设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q09',
        'level': 'L2',
        'prompt': '一条消息更新证券 A，下一条更新证券 B，只保留后者就得到最新全市场状态了吗？',
        'answer': '没有，A 的最新值被丢掉了。可以按键维护完整状态，或构造覆盖整个订阅域的快照；还要说明各键是否对应同一一致性切面。示例先把所有连续增量应用到两个证券的源状态，再用整个数组覆盖邮箱，不能把只存最后一个 Delta 当成同一实现。',
        'rubric': ['最后消息不等于每键最新状态', '完整订阅域', '跨键一致性边界'],
        'source':
          { 'kind': 'derived', 'rationale': '根据多证券更新与全局最新消息之间的信息损失设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q10',
        'level': 'L2',
        'prompt': '用 shared_ptr 广播一个对象，能否自动解决慢消费者与并发修改问题？',
        'answer': '共享所有权能让对象活到最后一个持有者释放，但慢消费者也可能因此延长回收并耗尽对象池。shared_ptr 的计数管理不会自动同步被指向对象的可变字段；共享同一个指针实例的修改也需要相应同步。可采用不可变对象加受控发布，并对每个消费者的未归还引用设预算。',
        'rubric': ['所有权延长寿命', '慢持有者影响回收', '引用计数不保护对象字段'],
        'source':
          { 'kind': 'derived', 'rationale': '根据共享所有权、回收进度与线程同步的不同职责设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q11',
        'level': 'L3',
        'prompt': '每个消费者一份队列，与共享日志加多个游标，各有什么成本？',
        'answer': '独立队列容易单独限流和隔离，但消息复制或引用管理随消费者数增加。共享日志减少重复存储，却需要在复用槽位前确认仍受保护的读者都已完成；保留最慢读者会限制生产者前进。若移除落后读者，必须让它失效并停止访问已回收位置，不能只把其游标从最小值计算中删掉。',
        'rubric': ['复制成本与共享存储取舍', '最慢受保护读者限制回收', '移除读者需处理寿命与失效'],
        'source':
          { 'kind': 'derived', 'rationale': '根据独立队列与共享 ring 的读者进度和复用条件设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q12',
        'level': 'L3',
        'prompt': '某个审计消费者要求每条消息都保留，又可能长时间停顿，应怎样设计？',
        'answer': '先确定停顿和保留的上限，以及生产者是否允许被它阻塞。需要无损时可以依赖有容量预算的持久日志和重放，或在容量耗尽时停止接受新数据；仍要处理磁盘、确认和恢复失败。不能用本例的慢消费者失效策略宣称审计数据已完整保存，也不能靠有限 RAM 支持无限停顿。',
        'rubric': ['明确无损合同和停顿上限', '可靠保留或反压及失败处理', '有限资源不承诺无限积压'],
        'source':
          { 'kind': 'derived', 'rationale': '根据审计无损要求与慢消费者隔离之间的矛盾设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q13',
        'level': 'L3',
        'prompt': '慢消费者重连后，拿到一张快照再接实时增量，怎样避免中间缺口？',
        'answer': '快照应标明覆盖的状态域、代次和 through_sequence，并确保之后从正确边界连续接入增量。通常需要暂存或重放快照生成期间的增量，丢弃已被快照覆盖的前缀，验证后续连续后再标有效。这里只给出接口要求，本例没有恢复入口，清空队列也不会把 invalid 改回 valid。',
        'rubric': ['快照范围、代次与序号边界', '增量暂存或重放衔接', '验证完成再恢复有效性'],
        'source': { 'kind': 'derived', 'rationale': '根据快照与持续增量流之间的连续性衔接设计。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q14',
        'level': 'L3',
        'prompt': '如何为每消费者缓冲区定容量，而不是随手设一个很大的数字？',
        'answer': '用目标负载下累计入队、出队和显式丢弃数量计算积压，估计需要吸收的峰值，再加上有依据的余量和批次需求；无丢弃阶段可看入队与出队之差。消息大小可变时同时限制条数与字节，并计算在途对象、网络缓冲和重放存储。平均速率乘平均延迟可能漏掉突发和停顿，容量更大还可能容纳更长的陈旧数据等待。',
        'rubric': ['按累计净积压与突发建模', '条数、字节和在途预算', '容量与陈旧延迟的取舍'],
        'source':
          { 'kind': 'derived', 'rationale': '根据各阶段积压、消息大小与批量要求设计容量题。' },
        'companies': [],
      },
      {
        'id': 'market-data-fanout-q15',
        'level': 'L3',
        'prompt': '广播吞吐没有下降，为什么仍不能断言所有消费者都健康？',
        'answer': '发布方可以在隔离慢端后保持吞吐，而某个消费者已失效或积压。需要分别观察源接受量、各端入队和处理进度、队列年龄、失效原因、重放距离及端到端延迟；覆盖式状态订阅还要看源序号和新鲜度。写入框架或队列成功只代表相应阶段接受，不代表对端业务已处理或持久化。',
        'rubric': ['发布吞吐掩盖个体失效', '每消费者进度与年龄指标', '阶段接受不等于业务确认'],
        'source': { 'kind': 'derived', 'rationale': '根据局部广播指标与订阅者实际处理状态设计。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

Market-data fanout 把同一份行情分发给多个订阅者，需要为每个订阅者定义顺序、积压上限和落后后的处理方式。增量消费者不能在漏消息后继续当作有效；只要求最新完整状态的消费者可以合并中间版本。失效通知还必须在数据队列满时保持可观察。源已接受、各端已入队和业务已处理是不同进度，不能用一个“发送成功”代替。

本文以 C++20 实现单线程确定性模型：两个有界增量订阅者和一个完整状态邮箱。它使用虚构的两个证券数量变化，不实现网络、撮合、持久日志、并发队列或恢复协议。所有操作按测试安排顺序执行，不依赖线程调度或睡眠。

## 核心概念

| 对象       | 保存的内容                            | 交付合同                             |
| ---------- | ------------------------------------- | ------------------------------------ |
| 源状态     | generation、next 和两个证券数量       | 仅接受合法且连续的整批增量           |
| 增量订阅者 | 容量四项的独立 FIFO                   | 有效期间按顺序交付，放不下整批就失效 |
| 状态订阅者 | 最新一份完整双证券状态                | 可以跳过中间状态版本，不提供事件历史 |
| Health     | valid、generation、首次失败原因及位置 | 不占数据槽，取数前检查，失效保持     |
| Report     | 源是否接受、各增量端的投递结果        | 不表示处理完成或持久确认             |

多个消费者竞争一个工作队列，通常每条消息只交给其中一个；广播要求多个订阅者各自获得数据。LMAX Disruptor 的文档用 multicast 和独立 consumer sequence 说明了这种区别，并通过 gating 约束生产者不覆盖受保护读者仍需要的槽位。[Disruptor multicast 与 gating](https://lmax-exchange.github.io/disruptor/user-guide/)

“不可丢增量”是消费者保持有效所需的语义，不表示任何实现都能用有限资源永久无损。本例在不能继续连续交付时使该订阅者失效，没有保存可供它恢复的可靠日志。

## 原理深入

### 先提交源批次，再分别处理订阅者

源从 generation=7、sequence=0、数量 `[100,200]` 的已建立状态开始，next=1。每批一到三条 Delta，包含 generation、sequence、instrument 和 change。处理器先复制源状态，逐条检查代次、连续序号、证券索引和数值范围，全部通过才提交源状态。

之后分别向两个增量端 offer。每个 offer 都先检查整批是否放得下，再复制全部事件；放不下时该端只改变 Health，不接收任何前缀。另一个订阅者和完整状态邮箱仍能前进。因此一次合法批次可以同时得到以下结果：

```text
source_accepted = true
consumer 0 = accepted
consumer 1 = invalidated
latest state = 已覆盖为当前完整源状态
```

这不是跨订阅者的全局事务。调用方不能把其中一个失效解释为“什么都没发生”，然后盲目重试整批。本例源序号已经前进，重复提交还会触发源验证失败。

若输入批次本身非法，源数量和 next 都不提交，所有订阅者进入或保持失效，Report 的投递结果为 not_attempted。invalidated 表示这次 offer 因容量不足导致失效，inactive 表示该增量端之前已经失效。本模型没有重新启用操作。

### 控制状态不与数据争抢最后一个槽

慢消费者先积压 sequence 1–4。收到包含 5、6 的批次时，它没有空位，Health 立即记为 invalid、reason=slow_consumer、failed_at=5，队列仍保留四条旧记录。pop 检查 Health 后返回空，不需要先从满队列里找到一条“失效消息”。

这是单线程的同步门禁。实际跨线程时，Health 的发布和读取要有正确同步，并定义它与数据代次的关系；跨进程还要考虑控制通道不可达、断线和心跳超时。仅把普通 bool 换成 atomic，也不能自动解决多字段一致性或业务检查时点。[C++20 数据竞争与同步](https://timsong-cpp.github.io/cppwp/n4861/intro.races)

门禁不能撤回已经 pop 或 take 返回的值副本。业务层必须规定这些在途工作何时再次检查有效性，以及失效后如何终止使用旧状态。内存仍存在与行情仍可用是两个条件。

### 只合并能覆盖目标状态域的完整值

从 `[100,200]` 开始，六条增量的手算结果如下：

| sequence | 变化         | 应用后的完整状态 |
| -------- | ------------ | ---------------- |
| 1        | 证券 0 加 5  | `[105,200]`      |
| 2        | 证券 1 减 10 | `[105,190]`      |
| 3        | 证券 0 减 3  | `[102,190]`      |
| 4        | 证券 1 加 20 | `[102,210]`      |
| 5        | 证券 0 加 8  | `[110,210]`      |
| 6        | 证券 1 减 5  | `[110,205]`      |

每批两条提交后，邮箱分别存入 through_sequence=2、4、6 的完整数组。未取走的旧快照可以被新快照覆盖，因为此订阅者只需要最新完整状态。若只保留最后一条“证券 1 减 5”，就丢失了证券 0 的变化，也无法从任意旧状态还原当前值。

这种规则必须来自订阅合同。ZeroMQ 的 CONFLATE 选项保留最后一条消息，它不会自动按证券键构造完整状态，文档还限制了 multipart 用法。不能只打开一个消息合并选项，就宣称保留了任意行情增量语义。[ZeroMQ CONFLATE](https://zeromq.github.io/libzmq/zmq_setsockopt.html)

## 数据结构/系统内部实现

每个 DeltaConsumer 拥有四项数组、head、count 和独立 Health。pop 按值返回一条，清空槽位，再推进 head；生产者写入位置为 `(head + count + i) % capacity`。容量足够的检查在复制之前，测试覆盖正常回绕、空队列和只剩一项时拒绝两项。

Health 只记录第一次失效。一个端已因 slow_consumer 失效，随后源也失效时，它仍保留原始原因；Fanout.valid 则表示源是否有效。这有助于保留首次故障线索，但不是完整故障历史。discard_invalid_backlog 仅清空已失效端的本地存储，不会恢复 valid。

LatestConsumer 用一个 optional 保存 Snapshot，包含固定 generation、through_sequence 和完整数量数组。take 返回值副本并清空邮箱；健康检查失败时不返回旧快照。一个已取出的快照可以在 Fanout 销毁后继续存在，但没有因此获得“始终代表最新行情”的保证。

代码限制 change 在 -1000 到 1000 且不为零，每个证券数量在 0 到 1000。计算先在 int64_t 暂存状态上进行，范围足以容纳检查前的中间值。序号推进前验证 uint32_t 的剩余范围，不做回绕；示例固定从 1 开始，实际测试没有运行到序号上界。

按值复制使本例的队列槽和返回对象寿命容易检查。如果改成共享不可变对象，可以减少大 payload 复制，但必须预算未归还引用和对象池。shared_ptr 管理共享所有权，不会自动保护对象的可变内容；最慢的持有者也可能延迟最终回收。[C++20 shared_ptr](https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.shared)

## C++ runnable demo

```cpp include=examples/market-data-fanout.cpp

```

编译时保持断言启用，测试中的部分操作位于断言表达式内：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/market-data-fanout.cpp -o /tmp/market-data-fanout
/tmp/market-data-fanout
```

输出：

```text
fast: 6 ordered deltas match oracle; slow: invalid at 5 with 4 pending
latest: complete state at 6 is [110,205]; slow reclamation does not reactivate
invalid source batches commit nothing; owned snapshot survives producer lifetime
single-thread model only; no networking, persistence, or recovery implemented
```

快消费者每批后取走两条，逐条比对预先写出的六个中间状态；慢消费者始终不取，第三批因容量不足失效。源继续接受第七条，快端仍能入队，慢端返回 inactive。覆盖式订阅者可以直接取到 sequence 6 的完整 `[110,205]`。

其他断言验证：输入 batch 被覆盖后队列副本不变；只剩一槽时两条批次完全拒绝；第一条合法、第二条会产生负数量时源不部分提交；空批次、超长批次、坏代次、坏序号、未知证券、零变化和超范围变化使源失效；取出的快照在生产者销毁后仍拥有数据。

在 WSL2 x86-64、GCC 13.3 下，严格编译和 ASan/UBSan 运行通过。该测试只验证确定性状态与值寿命，没有创建线程、发送网络消息、模拟消费者崩溃或测量延迟。没有实现持久化确认、重放存储或重连恢复。

## 高频追问

### 为什么不给每个消费者无限大的缓冲？

长期消费速率不足会让积压持续增长，最终耗尽有限内存。大缓冲还可能把过时行情留在队列中很久。需要先定服务合同，再选等待、拒绝、状态合并或恢复；不同消费者可以有不同选择。NATS 文档说明慢消费者信号和 pending limits 需要显式观察，具体丢弃或通知行为又依客户端而异。[NATS slow consumers](https://docs.nats.io/learn/resilient-clients/slow-consumers)

### 共享一份 ring 是否一定比独立队列好？

共享 ring 减少重复存储，但复用槽位前必须保证仍受保护的消费者不会再访问它。若所有人都参与 gating，最慢端会限制整体推进；若把慢端排除，就要使它失效并结束对旧槽位的借用。独立队列更容易按端隔离，代价是复制、元数据及缓存流量。

### 入队成功后可以当作对端确认吗？

入队只说明本地某一阶段接收。对端可能尚未取出，更未完成业务或持久化。网络框架也有类似阶段差异，例如 gRPC 写入流返回不代表数据已经发到网络。需要何种确认，应由调用方合同明确规定。[gRPC flow control](https://grpc.io/docs/guides/flow-control/)

## 容易答错的点

| 错误说法                         | 修正与原因                                     |
| -------------------------------- | ---------------------------------------------- |
| 多个消费者取同一队列就是广播     | 工作分摊通常只交给其中一个，广播要维护每端进度 |
| 增量覆盖最旧项也能继续 valid     | 状态可能缺少必需变化，必须失效或补齐           |
| 全局最新消息包含所有证券的最新值 | 多键状态需要完整覆盖或按键维护                 |
| 失效消息入队失败可以暂时不管     | 下游可能一直把不连续的数据视为有效             |
| 一端失败就表示整个广播未提交     | 源和其他端可能已经推进，不能盲目重试           |
| 清空慢端队列就能恢复             | 本地状态和源序列之间的缺口还在                 |
| 有限队列等于全系统内存有界       | 网络缓冲、在途副本和重放存储也需要预算         |
| 值副本还活着就能继续用于当前决策 | 对象寿命不保证业务有效性或新鲜度               |

## 性能分析

若一批有 m 条、增量订阅者数为 n，独立队列的复制工作约为 O(nm)，另有源状态更新及完整快照成本。完整状态包含 k 个项目时，按值生成快照需要 O(k)；本例 n=2、m<=3、k=2，因此这些操作都有固定上界。推广到真实证券集合时不能沿用“两项复制很小”的假设。

对一个队列，定义累计成功入队量 A(t)、累计出队量 D(t) 和从队列显式丢弃的累计数量 R(t)，则待取积压为初始积压加 A(t)−D(t)−R(t)。本例 discard_invalid_backlog 增加 R，拒绝入队的批次不计入 A；没有丢弃时 R 为零。容量应覆盖目标场景中的最大积压和批次要求；平均速率乘平均延迟不能完整描述突发、停顿和恢复追赶。变长消息还应有字节预算。出队后尚未处理的对象要另算，丢弃也不等于完成，不能因为 count 降了就声称全链路已完成。

本例每个增量端最多保存四项，状态邮箱最多一份，返回对象在测试中也保持有界。调用方若无限保留 pop/take 的值副本，队列上限不会自动限制这些外部存储。网络方案同样要区分应用队列、框架缓冲和内核缓冲；例如 ZeroMQ high-water mark 的具体行为取决于 socket 类型，不能把一个参数当作通用的端到端内存或交付保证。[ZeroMQ high-water marks](https://zeromq.github.io/libzmq/zmq_setsockopt.html)

衡量广播链路时，分别记录源接受率、每端入队率和处理率、队列占用及最老消息年龄、失效次数、重放距离，并观察端到端 p50/p99/p99.9。源吞吐保持稳定可能只是慢端已被隔离。覆盖式订阅还应记录快照代次、源进度和年龄；“最新保存的一份”不证明上游此刻仍在更新。本例没有性能实测或固定加速结论。

## Quant/Low-Latency 场景

同一行情流可能被状态重建、审计和界面展示消费。重建所需的增量必须连续；审计可能要求可靠保留每个事件；只显示最新完整状态的界面可以接受中间状态被合并。应先列清每种消费者允许丢掉什么，再配置队列和过载策略，不能用一个 PUB/SUB 标签代替这些约定。

这里的慢端隔离可以让其他端继续，但失效端没有得到无损保证。若某端要求所有事件完整保存，需要有容量和恢复预算的可靠日志，或让其压力最终阻止源继续接受；磁盘和确认失败也属于设计的一部分。有限资源无法同时支持无限停顿、持续输入和永久无损。

慢端重建时，应取得带代次与 through_sequence 的完整状态，并把暂存或重放的增量接到这个边界后，验证连续性再恢复有效。控制状态还要与已经在途的旧代次工作协调。本模型固定一个代次且没有恢复入口，discard_invalid_backlog 只回收存储，不承担这些步骤。

## 相关专题

- [Feed Handler](../trading/feed-handler.md)：广播前先建立规范化事件、连续性与源有效性。
- [SPSC Queue](../concurrency/spsc-queue.md)：若扩展为跨线程队列，需要另外证明发布和槽位复用的同步。
- [mutex 与 condition_variable](../concurrency/mutex-condition-variable.md)：理解阻塞、关闭和排空合同；本例选择同步失效隔离。
- [Benchmark 方法](../performance/benchmark-methodology.md)：区分局部入队成本、消费者排队和端到端可用延迟。

## 分层面试题

基础题区分交付合同；机制题检查批次、所有权与控制状态；设计题要求写清容量、恢复边界和不同消费者的过载取舍。
