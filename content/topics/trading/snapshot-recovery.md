---
{
  'schemaVersion': 1,
  'id': 'snapshot-recovery',
  'title': '快照与增量恢复：从水位到完整前缀',
  'description': '校验快照覆盖边界、会话和恢复轮次，在有界缓存中重放连续增量，构造完整候选后发布，并保留失败前的诊断视图。',
  'category': 'trading',
  'areas': ['Market Data', 'Feed Handler', 'Order Book', 'Trading System'],
  'tags': ['snapshot', 'recovery', 'watermark', 'sequence', 'atomic-publication'],
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
  'prerequisites': ['Order Book 与事件重建', '序列号、会话和 gap', 'shared_ptr 与 release/acquire'],
  'related':
    ['order-book', 'udp-multicast-sequencing', 'cpp-memory-model', 'tail-latency-backpressure'],
  'demo':
    {
      'file': 'examples/snapshot-recovery.cpp',
      'platform': 'portable',
      'exercise': '为虚构协议增加有界快照分片组装层，用 snapshot_id、片号、总片数和校验结果产生 complete；覆盖分片重复冲突、缺片和旧 attempt 迟到，并保持恢复失败时当前发布句柄不变。',
    },
  'references':
    [
      {
        'title': 'Coinbase Exchange WebSocket Channels: Full Channel',
        'url': 'https://docs.cdp.coinbase.com/exchange/websocket-feed/channels',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Binance Spot API: WebSocket Streams and local order book recovery',
        'url': 'https://raw.githubusercontent.com/binance/binance-spot-api-docs/master/web-socket-streams.md',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: atomic shared_ptr specialization',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.atomic.shared',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'snapshot-recovery-q01',
        'level': 'L1',
        'prompt': '快照 watermark 在本例中表示什么？',
        'answer': '表示同一会话中所有序号不大于 W 的事件已经体现在快照里，因此重放从 W 之后开始。它不是下载结束时间，也不是最近收到的最高包号；真实协议可能使用消息区间、产品内序号或其他边界。',
        'rubric': ['覆盖完整前缀', '重放起点', '协议条件'],
        'source': { 'kind': 'derived', 'rationale': '根据虚构协议定义与真实协议边界对照推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q02',
        'level': 'L1',
        'prompt': '为什么获取快照前要开始缓存增量？',
        'answer': '快照生成与下载期间实时流仍在前进。先保留增量，才能在快照水位确定后去掉已覆盖部分，并检查后续是否连续。如果从下载结束后才接流，中间窗口可能已丢失。缓存仍需要容量和时间限制。',
        'rubric': ['下载期间流继续', '覆盖窗口', '有界缓存'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 Coinbase Full Channel 的缓存与快照顺序推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q03',
        'level': 'L1',
        'prompt': '为什么序号必须和会话或序列域一起比较？',
        'answer': '相同数字可能来自不同产品、频道或重启后的会话，不能表示同一事件。恢复必须先确定序列域兼容，再判断前后与重复。本例只建模一个域，session 变化后允许从新的基线开始。',
        'rubric': ['数字不等于事件身份', '先检查域', '新会话新基线'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 Token、Delta、Snapshot 的身份字段推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q04',
        'level': 'L1',
        'prompt': '保留旧簿和继续使用旧簿有什么区别？',
        'answer': '保留旧完整状态有助于诊断和展示，但发现缺口后不能继续将其标为完整最新状态。本例 begin 发布旧 payload 的 invalid 版本；下游仍需遵守有效性和新鲜度规则，停止依赖失效数据的决策。',
        'rubric': ['诊断价值', '显式失效', '业务门禁'],
        'source': { 'kind': 'derived', 'rationale': '根据 begin 的发布行为与旧 reader 边界推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q05',
        'level': 'L1',
        'prompt': '重复消息可以一律忽略吗？',
        'answer': '对本轮缓存中已见的同域同序号事件，本例检查内容是否一致：完全相同的重复不再次应用，同号不同载荷使本轮恢复失败。它没有比对历史已发布或快照所覆盖事件的原始载荷。真实区间协议还需按其规则识别重复和覆盖。',
        'rubric': ['身份与内容', '冲突处理', '区间协议例外'],
        'source': { 'kind': 'derived', 'rationale': '根据重复去重和冲突毒化测试推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q06',
        'level': 'L1',
        'prompt': '原子发布是否保证读者读到最新市场状态？',
        'answer': '原子发布保证读者取得一个完整版本的句柄，避免拼接不同版本的字段；它不保证网络没有延迟，也不撤销已经取得的旧句柄。业务仍需判断该版本的水位、有效性和使用时机。',
        'rubric': ['一致版本', '新鲜度不同', '旧句柄仍存活'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据不可变 Published 与 atomic shared_ptr 的职责推导。',
          },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q07',
        'level': 'L2',
        'prompt': '快照覆盖到 10，缓存只有 11 和 13，能否先发布到 13？',
        'answer': '不能，缺少 12。可以在预算内继续补取 12，再在候选上重放 11、12、13；未补齐前保留 invalid 的旧发布视图。本例先部分计算候选再遇 gap，也不会把候选写入发布指针。',
        'rubric': ['不能跨 gap', '补齐再恢复', '候选隔离'],
        'source': { 'kind': 'derived', 'rationale': '根据 gap repair 的独立手算测试推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q08',
        'level': 'L2',
        'prompt': '同一 session 为什么还需要恢复 attempt？',
        'answer': '同一会话可以重试多轮快照请求，早先轮次的响应可能晚到。异步请求发出时绑定 attempt，响应沿用这个上下文，接收方与当前 token 比较。处理响应时重新读取当前 token 会把旧响应误标成新响应。',
        'rubric': ['同会话多轮', '请求上下文携带', '不能现场重贴标签'],
        'source': { 'kind': 'derived', 'rationale': '根据同 session 旧 attempt 的拒绝测试推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q09',
        'level': 'L2',
        'prompt': '为什么缓存溢出后不能随便丢一条继续恢复？',
        'answer': '丢弃可能删除快照之后仍必需的增量，接收端失去连续性证据。本例将本轮标为 poisoned，拒绝发布并要求重开。若协议与保留区间能证明被丢条目已经被快照覆盖，可设计更精细策略，但不能假定这一点。',
        'rubric': ['连续证据丢失', '本轮失败', '优化需要覆盖证明'],
        'source': { 'kind': 'derived', 'rationale': '根据固定容量和失败恢复测试推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q10',
        'level': 'L2',
        'prompt': 'commit 的 cut 是什么，为什么不能随意取最高序号？',
        'answer': 'cut 是调用方希望本次恢复覆盖到的边界，必须位于快照及已验证连续增量能到达的位置。最高收到号中间可能有缺口。本例还拒绝缓存中存在 cut 之后事件的提交，避免清缓存时丢掉未来事件；它没有实现实时流交接。',
        'rubric': ['明确提交边界', '最高不等于连续', '不丢未来缓存'],
        'source': { 'kind': 'derived', 'rationale': '根据 bad_cut 和尾部 gap 测试推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q11',
        'level': 'L2',
        'prompt': 'Snapshot.complete 为 true 能证明快照正确吗？',
        'answer': '不能。本例将其视为外部可信组装层给出的条件，没有实现分片完整性、校验和或来源认证。真实输入还需确认所有分片属于同一快照、元数据一致且覆盖范围符合协议，不能由普通布尔字段自证真实性。',
        'rubric': ['外部前提', '组装完整性', '内容与来源校验'],
        'source': { 'kind': 'derived', 'rationale': '根据示例 Snapshot 输入契约与省略范围推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q12',
        'level': 'L2',
        'prompt': 'release/store 与 acquire/load 在发布路径上保护什么？',
        'answer': '候选内容在 release store 前构造完成，读到该发布值的 acquire load 使读者可以看到这些初始化。读者持有 shared_ptr 保持对象存活，对象保持 const。它不保护并发修改恢复缓存；begin、buffer、commit 仍要求单 owner。',
        'rubric': ['初始化发布链', '对象寿命与不可变', 'owner 方法不并发'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 C++20 原子 shared_ptr 与本例写者约束推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q13',
        'level': 'L3',
        'prompt': '下载快照比实时增量生成更慢时如何防止恢复循环失败？',
        'answer': '记录快照耗时、缓存增长与重放处理率，设置内存、时间和重试预算。可考虑更接近当前水位的快照、协议支持的补发、分区恢复或隔离资源；持续追不上时应保持无效并报警，不能仅无限加缓存或无间隔重试。',
        'rubric': ['观测差额', '资源与重试预算', '失效而非假成功'],
        'source': { 'kind': 'derived', 'rationale': '根据有界缓冲和追赶工作量推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q14',
        'level': 'L3',
        'prompt': '多个 instrument 的快照可以拼成同一时刻的市场视图吗？',
        'answer': '除非协议提供共同切点或可验证的跨域一致性，否则各 instrument 可能只在自己的水位上完整。本地一次原子发布能固定一个组合版本，但不会补出跨频道的全局时间一致性。需要说明可接受的偏差以及策略是否依赖跨品种同步。',
        'rubric': ['各域水位', '原子发布的局限', '跨域业务假设'],
        'source': { 'kind': 'derived', 'rationale': '根据单序列域模型的适用边界推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q15',
        'level': 'L3',
        'prompt': 'shared_ptr 发布在低延迟系统中有哪些代价与替代条件？',
        'answer': '它简化读者寿命，但有分配、引用计数和可能的内部锁，不能承诺 lock-free。旧读者保留版本会推迟回收。若改用双缓冲、epoch 或其他回收机制，必须证明写者不会覆盖仍被读者访问的版本，并测回收延迟与内存峰值。',
        'rubric': ['实际成本', '旧版本滞留', '替代需要寿命证明'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据 atomic shared_ptr 标准边界与不可变版本保留推导。',
          },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q16',
        'level': 'L3',
        'prompt': '恢复成功后，怎样切回实时流才不漏事件？',
        'answer': '由同一 owner 确定重放切点和后续输入顺序，保留切点后的事件，并把 next 与候选状态作为一次受控状态转换交给正常处理器。转换期间不能出现两个并发写者，也不能清掉尚未应用的缓存。本例 commit 只发布到 cut，实时交接仍需另行实现与测试。',
        'rubric': ['单 owner 交接', '保留未来事件', '原子视图不替代路由状态'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 commit 的职责和 inactive 返回边界推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q17',
        'level': 'L3',
        'prompt': '如何验证恢复结果，避免测试只重写一遍实现？',
        'answer': '先给出小型快照和手算最终状态，枚举增量排列及重复，再注入缺口、跨会话和容量故障。失败后比较发布句柄与 payload，确认没有半更新；恢复后比较独立期望。真实协议还需使用已知正确的历史回放和格式校验，不能把模型测试当成接入认证。',
        'rubric': ['独立期望', '故障注入', '失败状态与适用范围'],
        'source': { 'kind': 'derived', 'rationale': '根据六排列、句柄不变及故障恢复测试推导。' },
        'companies': [],
      },
      {
        'id': 'snapshot-recovery-q18',
        'level': 'L3',
        'prompt': '为什么序列号和 attempt 不能简单地溢出回零？',
        'answer': '回零会使旧消息或旧异步响应与新身份碰撞，普通大小比较也不再正确。本例在最大序列前要求换会话，attempt 耗尽直接拒绝 begin。需要回绕的真实协议必须定义比较窗口和旧消息寿命，不能从整数类型自动推导安全性。',
        'rubric': ['身份碰撞', '显式耗尽策略', '真实回绕条件'],
        'source':
          { 'kind': 'derived', 'rationale': '根据序列与 attempt 耗尽测试及会话身份不变量推导。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

快照恢复需要把一个已知完整的状态，与它之后连续的增量接起来。先确认快照和增量属于同一序列域，取得快照覆盖水位，过滤重叠，再逐条校验连续性并重放到候选状态。候选完整后才发布；缺口、缓存溢出或旧会话响应都不能让半恢复的簿变成有效。旧完整状态可以保留作诊断，但下游要看到它已经失效，恢复期间也要有内存和时间预算。

## 核心概念

本章示例使用虚构协议，不实现 Coinbase、Binance 或其他交易所接入。系统只有一个逻辑序列域，状态为四个无符号计数器，增量是对某个计数器做加法。它足以暴露重复重放、漏事件和部分发布的问题，但没有价格档、订单优先级或撮合。

| 字段或状态    | 教学协议定义                                     | 不能据此推断的事实                  |
| ------------- | ------------------------------------------------ | ----------------------------------- |
| session       | 外部确认的非零会话身份                           | 数字较大不自动表示更新会话          |
| watermark `W` | 快照包含该 session 内所有 `sequence <= W` 的事件 | 快照下载完成不等于 W 之后没有事件   |
| attempt       | 本地恢复轮次，随异步请求上下文携带               | 同 session 的所有响应并不属于同一轮 |
| cut           | 本轮打算重建到的连续前缀边界                     | 最大到包号不自动构成连续前缀        |
| valid         | 当前发布版本可作为模型中的完整基线               | 内存中仍存在不等于业务可继续使用    |
| complete      | 外部可信快照组装层提供的完成条件                 | 本例没有验证分片完整性或内容真实性  |

真实序列域还可能包含产品、频道、源和交易日。一个域内的序号不能与另一个域直接比较。会话切换也应由受信任的协议或控制路径确认，不能因一个陌生包携带新 session 就自动接受它。

## 原理深入

设快照状态是 `S(W)`，后续事件按序为 `E(W+1)...E(C)`。只有这些事件属于同一域、没有缺口、每条只应用一次，才可以把 `apply(S(W), E(W+1)...E(C))` 当作截至 `C` 的状态。快照已覆盖的事件必须跳过；重复 ADD 会使计数多加，缺一条则使计数少加，即使后续包连续到达也不能补回未知影响。

下载快照期间实时流仍会推进，因此需要先建立增量保留窗口，再获取基线。Coinbase Exchange 的 Full Channel 文档描述了先订阅缓存、再取订单簿快照，并丢弃序号不大于快照序号的缓存消息后重放的流程。其消息类型还有各自的簿更新条件，不能把每条消息都理解为同一种 ADD。[Coinbase Full Channel](https://docs.cdp.coinbase.com/exchange/websocket-feed/channels)

Binance Spot diff-depth 事件带首尾更新 ID `U/u`，一个事件可能覆盖一段更新区间。其官方流程检查快照与首个保留区间的衔接，并在 `U` 超过本地更新 ID 加一时重新恢复；价位数量是设置新值，零数量移除价位。该边界和事件语义与本例“每条单独递增一个序号并做加法”不同，不能直接套用本例的相等判断。文档还提醒初始快照深度有限，未覆盖且未再更新的价位不能被当作完整全簿。[Binance 官方恢复流程](https://raw.githubusercontent.com/binance/binance-spot-api-docs/master/web-socket-streams.md)

恢复期间保留旧状态，与失败时继续使用旧状态，是两项不同决定。本例开始恢复时发布同一旧 payload 的 `valid=false` 版本，随后所有重放只修改局部候选。这样缺口或溢出可以返回失败，而不会留下“部分应用了新事件”的共享状态。只有全部校验通过才替换发布句柄。

## 数据结构/系统内部实现

`Recovery` 由一个 owner 串行调用 `begin`、`buffer` 和 `commit`。缓存使用长度为 8 的固定数组，按序号检查重复，允许有限乱序；提交时复制数组并做插入排序。相同序号且所有字段相同的事件返回 `duplicate`，不占新槽；同号冲突、非法事件或容量不足会将本轮标为 `poisoned`，必须重新 `begin`。

`begin(session)` 增加本地 attempt，清空这一轮缓存并发布 invalid 旧视图。Token 必须在发起异步工作时保存，响应回来沿用原 Token；若处理旧响应时重新读取当前 Token，就失去了轮次隔离。session 或 attempt 不匹配返回 `foreign`，不会污染当前轮次。attempt 耗尽拒绝重开，不能绕回去复用身份。

`commit` 按以下顺序构造候选：

1. 检查 Token、session、poisoned 与 complete；确认 cut 不早于快照水位，也不使同 session 已发布的前缀倒退。
2. 从快照复制候选，跳过序号不大于 W 的缓存事件。
3. 拒绝任何超过 cut 的缓存事件；其余事件必须严格等于候选水位加一，数量相加前检查剩余范围。
4. 确认候选最终水位恰好等于 cut，构造不可变 `Published` 并一次发布。

缺口允许在容量内补齐后重试同一轮，候选每次从快照重建，之前的局部修改不会累加到下一次尝试。缓存冲突或溢出则不继续同轮，因为这一轮保留证据已不可靠。本例把最大序号留作耗尽边界，`watermark` 或 cut 等于最大值时拒绝提交，最大值减一仍可发布，后续需要切换会话。

发布对象包含 `session`、watermark、四个计数器和 valid。`atomic<shared_ptr<const Published>>` 的 release store 与读取到该值的 acquire load 使初始化对读者可见，shared_ptr 保留对象寿命，const 防止发布后改写。同一个 reader 应从一次 load 得到的句柄读取全部字段，不能各字段分开 load 后拼起来。C++20 没有保证这种原子 shared_ptr 总是 lock-free。[N4861 原子 shared_ptr](https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.atomic.shared)

一次发布不会撤销读者已经持有的旧有效句柄。需要立即阻止旧状态驱动业务时，应在决策路径检查当前有效性或业务代次，或由独立门禁协调；仅凭内存安全不能完成这项授权变化。`Recovery` 实例及其中原子成员的初始化和销毁必须避开对该实例的并发访问；旧 `Published` 对象则由 shared_ptr 在最后一个拥有者释放后销毁，不需要全局停止其他版本的读取。本例测试单线程执行，只演示发布 API 与不可变版本语义。

## C++ runnable demo

```cpp include=examples/snapshot-recovery.cpp

```

使用 C++20，不依赖网络或真实交易所。测试中断言包含操作调用，运行验证时不能定义 `NDEBUG`。

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  examples/snapshot-recovery.cpp -o snapshot-recovery
./snapshot-recovery

g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  examples/snapshot-recovery.cpp -o snapshot-recovery-san
./snapshot-recovery-san
```

主测试给出水位 10 的快照 `{10,20,30,40}`。事件 11 给第一个计数器加 2，事件 12 给第二个加 3，事件 13 给第一个加 5，手算期望是 `{17,23,30,40}`。程序枚举三条事件的全部六种排列，同时加入快照覆盖的序号 10 和完全相同的重复 12，最终状态都必须与该常量相等。

| 验证             | 预期及实际断言                                                            |
| ---------------- | ------------------------------------------------------------------------- |
| 重叠、重复和乱序 | 六种排列得到相同独立期望，已覆盖事件不重放，重复不再次加量                |
| 中间缺口         | 只有 11、13 时返回 gap，发布句柄不变；补 12 后可以恢复                    |
| cut 边界         | cut=12 而缓存有 13 时拒绝，cut=14 但没有 14 时返回 gap；都不清掉必要事件  |
| 旧身份           | 旧 session、同 session 的旧 attempt，以及旧快照响应均拒绝                 |
| 缓存及冲突       | 第九条不同事件触发 buffer_full；同号不同量触发冲突；两者都要求新一轮恢复  |
| 算术与耗尽       | 数量溢出、最大序号边界、attempt 耗尽不发布；最大序号减一可作为水位        |
| 输入条件         | incomplete 快照、零事件序号、越界 instrument、零 session 都有失败路径     |
| 旧视图           | 恢复失败后仍保留 invalid 旧 payload；此前读者保留的不可变旧对象没有被改写 |

正常构建和 ASan/UBSan 运行均通过，输出最后一行为 `fictional protocol only; no exchange or network integration`。分配失败的行为来自代码顺序推理：`make_shared` 在 store 前完成，若抛出，不会替换现有发布句柄；本例没有注入 OOM，不把这一分支称为运行验证通过。begin 分配 invalid 版本失败时本轮尚未启动，外部发现 gap 的调用方仍需走自己的失败门禁，不能依赖一次未完成的 begin 已经使数据失效。

`Snapshot.complete` 只是外部前提，程序没有分片解析、快照认证或校验和。提交也没有实现接回实时流：超过 cut 的已缓存事件会使提交失败，成功后 buffer 返回 inactive。生产集成要由 owner 接管后续事件和 next，保留交接期间的输入。状态在 cut 上完整，不表示与远端当前时间完全同步。

## 高频追问

### 快照水位落在增量区间中间怎么办？

先确认协议是否允许区间重叠、事件内更新是否可整体应用，以及载荷是绝对状态还是操作序列。不能把一个区间事件按数字随意拆成独立事件，也不能为了统一接口而假设所有 feed 都要求第一条序号严格等于 W+1。为协议适配层建立边界用例，让恢复器消费已经明确语义的输入。

### 已经排序，为什么仍会恢复失败？

排序只能改变现有事件的顺序，不能制造缺失事件。排序后 11、13 之间仍然缺 12；排序也不能证明两个同号载荷哪个可信。还要检查快照覆盖范围、数量有效性与 session。缓冲区提供的数据不足时，保留失效状态比发布一个看起来有序的错误簿更可控。

### 下载完之后再提高 cut，就能追上实时流吗？

提高 cut 会增加待验证和重放的工作。要同时确认保留窗口没有溢出，以及处理速率是否足以缩小差距。本例只接受一组有限事件，不会在 commit 中等待更多输入。生产系统应把接收、重放和交接做成可观测状态，并设置最大恢复时间；超出预算时结束本轮，而不是无界循环。

## 容易答错的点

- 收到完整 HTTP 响应不自动证明业务快照覆盖正确；快照身份、分片和水位仍需按协议确认。
- watermark 不能用本地墙钟或接收时间替代，不同频道也未必有可比较的序号。
- 最高收到号说明见过该编号，不能证明之前的事件全在。
- 重复 ADD 与绝对数量设置有不同效果，去重策略不能脱离事件语义。
- 发布指针是原子的，不意味着恢复缓存可以被多个线程无同步改写，也不意味着发布操作 lock-free。
- 旧状态保留和旧状态有效是不同属性；完整前缀与足够新鲜也是不同条件。

## 性能分析

设缓存事件数为 B，状态大小为 S。本例 buffer 线性检查重复，单次 `O(B)`；提交复制状态和缓存，再用插入排序，成本 `O(S+B²)`，重放本身为 `O(B)`。示例固定 B 不超过 8、S 为四个计数器，这些是测试规模，不能据此推断生产恢复成本固定。

较大缓存可使用按序号索引或有界重排结构，但仍需记录窗口、重复冲突和淘汰依据。真实订单簿复制会占内存和时间，发布后的旧版本可能被慢读者保留，候选加当前视图并不是总内存的绝对上界。测量应包括峰值存活版本、缓存字节、分配失败、快照耗时、重放速率、追赶距离，以及从 invalid 到 valid 的时间分布。

恢复和实时处理共用 CPU、网络或存储时，恢复流量会改变正常路径尾延迟。可以限制并发恢复、分区预算和重试频率，再观察是否仍能在缓存及时间预算内追上。本文没有性能计时或提速数字；六排列测试提供的是确定性正确性证据。

## Quant/Low-Latency 场景

行情簿失效后，策略需要知道哪些产品或频道不可用。若缺口影响整个序列域，不能只因其中一只标的最近没有更新，就继续宣称它完整。恢复视图应携带身份、水位和有效范围，让下游按依赖关系停止或降级，而不是仅在日志里写一次 gap。

多个标的共享一个序列域时，本例这种整体基线有明确切点；若分属独立频道，分别恢复得到的版本不自动构成同一时刻的全市场快照。跨品种计算需要额外定义可接受的数据年龄和频道间偏差，本地一次指针替换不能产生协议没有提供的全局一致性。

重连或切备用源时，要重新证明源身份、序列可比性和消息等价性。两个源都出现序号 100，不足以证明可以混流去重。保留恢复原因、输入摘要、候选水位和失败路径，可帮助回放判断错误来自丢包、快照覆盖、载荷解码还是发布交接。

## 相关专题

- [Order Book](order-book.md)：把完整事件前缀映射为价格档、订单索引与数量不变量。
- [UDP 组播与序列号](../network/udp-multicast-sequencing.md)：理解序列域、重复、乱序及缺口后的失效处理。
- [C++ memory model](../concurrency/cpp-memory-model.md)：核对发布初始化与读者访问之间的同步关系。
- [尾延迟与背压](../performance/tail-latency-backpressure.md)：为恢复缓存、慢消费者和重试设置可观测预算。

## 分层面试题

L1 解释水位、身份与有效性；L2 跟踪重叠、缺口和发布时机；L3 处理持续追赶、协议适配及下游使用边界。评审时给出具体快照水位和一组乱序事件，再分别写出“当前发布了什么”和“还缺什么”，能检查恢复状态是否被描述清楚。
