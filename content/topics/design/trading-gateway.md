---
{
  'schemaVersion': 1,
  'id': 'trading-gateway',
  'title': '交易网关：发送边界、未知结果与重连恢复',
  'description': '区分业务身份、会话和连接代次，处理部分写、迟到回调、确认丢失与有界背压，通过虚构脚本协议验证未知结果的查询和安全重试条件。',
  'category': 'design',
  'areas': ['System Design'],
  'tags': ['trading-gateway', 'session', 'reconnect', 'idempotency', 'backpressure'],
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
  'estimatedMinutes': 60,
  'prerequisites': ['非阻塞流式发送与报文边界', '业务请求身份', '状态机与单 owner'],
  'related': ['tcp-framing', 'connection-lifecycle', 'matching-engine', 'spsc-queue'],
  'demo':
    {
      'file': 'examples/trading-gateway.cpp',
      'platform': 'portable',
      'exercise': '给查询操作增加独立 query_id，并让同一连接上的较早查询响应在较晚查询之后到达。明确每个响应的业务身份、连接代次和查询版本校验，保留 Unknown 阻塞与原 payload；不能只在回调处理时读取当前 generation。',
    },
  'references':
    [
      {
        'title': 'RFC 9293: Transmission Control Protocol',
        'url': 'https://www.rfc-editor.org/rfc/rfc9293',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux send(2)',
        'url': 'https://man7.org/linux/man-pages/man2/send.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux recv(2)',
        'url': 'https://man7.org/linux/man-pages/man2/recv.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'FIX Session Layer Technical Standard, June 2020',
        'url': 'https://www.fixtrading.org/wp-content/uploads/download-manager-files/FIX_Session_Layer_June_2020.pdf',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Coinbase Exchange FIX Order Entry Messages 5.0',
        'url': 'https://docs.cdp.coinbase.com/exchange/fix-api/order-entry-messages/order-entry-messages5',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Coinbase Exchange FIX Drop Copy 5.0',
        'url': 'https://docs.cdp.coinbase.com/exchange/fix-api/drop-copy',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'trading-gateway-q01',
        'level': 'L1',
        'prompt': 'send 成功返回整条消息长度，是否代表交易请求已被对端业务接受？',
        'answer': '不能。它表示本次发送调用接受了这些字节，不提供业务处理完成的确认；甚至不能单靠该返回值确认最终对端交付。TCP 的传输确认也不等于订单接受或成交。本章写满一帧后进入 awaiting_ack，只有匹配的应用结果才能进入 final。',
        'rubric': ['区分发送、传输与业务确认', '写满后仍等待应用结果'],
        'source': { 'kind': 'derived', 'rationale': '由发送接口返回值与应用确认的不同保证推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q02',
        'level': 'L1',
        'prompt': '业务请求 ID 与连接 generation 为什么不能使用同一个计数？',
        'answer': '业务 ID 关联同一请求及其 payload 和结果，重连后仍可能需要保留；generation 标识本地连接实例，用于拒绝旧连接回调。一次业务请求可能跨多次连接恢复，反过来同一连接也会承载多条业务请求。把重连当成新业务身份可能制造重复提交。',
        'rubric': ['说明两类身份的生命周期', '重连不自动生成新业务请求'],
        'source': { 'kind': 'derived', 'rationale': '由业务结果关联与连接回调隔离的职责差异推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q03',
        'level': 'L1',
        'prompt': '部分写之后下一次发送从哪里开始？',
        'answer': '在同一存活连接上，从已确认被本地发送调用接受的字节偏移继续发送剩余后缀。不可从帧首重复追加，否则对端流可能收到重复前缀；也不能跨新连接沿用旧偏移。断线后先判断业务状态，只有允许重试时才用原身份和原内容重新构造完整帧。',
        'rubric': ['同连接维护 offset', '新连接不能延续旧字节偏移'],
        'source': { 'kind': 'derived', 'rationale': '由流式部分发送与连接重建的字节边界推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q04',
        'level': 'L1',
        'prompt': '本章的 unknown 状态表达什么，为什么不是 rejected？',
        'answer': 'unknown 表示本地已有发送进展但缺少确定应用结果，对端可能尚未处理，也可能已经处理而回复丢失。本地没有证据把它标为业务拒绝。示例阻塞后续命令，等待查询或协议规定的恢复步骤；超时本身不会把未知变成未执行。',
        'rubric': ['保留远端已处理的可能', '超时不是业务拒绝证据'],
        'source': { 'kind': 'derived', 'rationale': '由断线后业务结果不可判定的状态边界推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q05',
        'level': 'L1',
        'prompt': '队列满时，本章怎样处理新的请求？',
        'answer': '本章固定保留 4 条请求，满时返回 full，不接收、不占用该业务 ID，也不覆盖旧请求。已收到应用结果但尚未被本地消费者取走的记录仍占队列容量。只有消费队头结果后才释放槽位，使消费者停滞也能够向入口产生背压。',
        'rubric': ['明确拒绝且不覆盖旧请求', '结果保留计入容量'],
        'source':
          { 'kind': 'derived', 'rationale': '由有界请求与结果生命周期对背压的共同影响推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q06',
        'level': 'L2',
        'prompt': '重连后查到 NotFound，为什么仍可能不能重发？',
        'answer': '查询可能早于旧连接请求的最终应用，或只覆盖有限历史，所以 NotFound 不必然证明原请求永远不会出现。本章仅在同一业务会话、原请求内容一致且旧传输已被脚本明确隔离、没有后续待处理工作时接受 absent_fenced。真实 TCP close 不自动提供这个远端保证。',
        'rubric': ['指出查询与旧请求的竞态', '需要远端处理边界而非本地 close'],
        'source': { 'kind': 'derived', 'rationale': '由查询缺失与远端尚未完成处理之间的竞态推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q07',
        'level': 'L2',
        'prompt': '旧连接的确认或查询回调到达时，应该怎样校验上下文？',
        'answer': '回调携带发起时捕获的 generation，再与当前连接状态比较，并核对业务会话、请求 ID 与原 payload。不能在处理旧回调时重新读取当前 generation 填进去，否则隔离失效。本例还限制查询只能解析队头 unknown；更一般的并发查询需要独立 query_id 或版本。',
        'rubric': ['绑定发起时上下文', '同时核对身份与当前合法状态'],
        'source': { 'kind': 'derived', 'rationale': '由迟到回调和连接代次复用风险的隔离要求推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q08',
        'level': 'L2',
        'prompt': '只记录请求 ID 已见过，能否提供可恢复的去重回复？',
        'answer': '已见标记可以拒绝重复，但无法返回第一次处理的确切结果，也不能发现相同 ID 对应不同内容。本章脚本对端保存业务会话内 ID、payload 和结果，相同内容返回历史结果，内容冲突返回 conflict。生产还需规定保留期限、持久化和故障恢复。',
        'rubric': ['区分已见与结果去重', '绑定 payload 并定义保留边界'],
        'source': { 'kind': 'derived', 'rationale': '由重试结果恢复所需的身份和内容约束推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q09',
        'level': 'L2',
        'prompt': '为什么未知结果可能造成队头阻塞？',
        'answer': '如果后续命令依赖前一请求的业务结果，继续发送可能改变期望顺序或产生无效操作。本章选择一次只推进队头，unknown 或未消费 final 都阻止后续发送，便于证明顺序。生产可按独立业务域并行，但必须先定义跨请求依赖和恢复合并规则。',
        'rubric': ['说明顺序依赖与保守策略', '并行需要明确独立域'],
        'source': { 'kind': 'derived', 'rationale': '由未知结果保留与后续业务顺序之间的关系推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q10',
        'level': 'L2',
        'prompt': '对端回复同一请求两个不同终态，网关应该怎么做？',
        'answer': '先按协议区分合法状态演进和互相矛盾的结果。本虚构协议只有一次请求接受或拒绝，两者不可改写；重复相同结果无操作，冲突结果使网关冻结且不覆盖已完成记录。真实订单生命周期允许接受后成交等演进，不能套用本例的两值终态模型。',
        'rubric': ['按协议区分演进与冲突', '冲突不静默覆盖原结果'],
        'source':
          { 'kind': 'derived', 'rationale': '由简化请求结果模型与真实订单生命周期差异推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q11',
        'level': 'L3',
        'prompt': '同一业务会话的重连与远端业务会话重置应如何区别？',
        'answer': '同一业务会话重连可以在既定结果历史与恢复规则下继续；会话重置可能改变序号空间、去重历史或订单管理范围。本章遇到不同业务会话直接冻结，没有自动清空 pending 或 seen。生产必须先核对远端存活订单、历史结果与身份协议，再决定迁移、取消或重新提交。',
        'rubric': ['解释业务历史可能改变', '不自动清空未知状态或身份'],
        'source': { 'kind': 'derived', 'rationale': '由会话重置对身份和结果历史的影响推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q12',
        'level': 'L3',
        'prompt': 'FIX 会话重传成功，是否自动解决重复下单问题？',
        'answer': '不能混同两个层次。FIX 会话层按序号处理缺口和消息重传，应用层重发未确认业务请求仍需要业务身份及对端约定的重复处理。具体接口可能有不同重置与历史保留规则，应按场所版本实现。本章五字节协议不是 FIX，实现的去重结果也不是 FIX 自动提供的通用保证。',
        'rubric': ['区分会话 retransmission 与应用 resend', '按场所协议处理业务重复'],
        'source':
          { 'kind': 'derived', 'rationale': '由 FIX Session Layer 对会话重传和应用重发的区分推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q13',
        'level': 'L3',
        'prompt': '进程崩溃后要恢复 pending 请求，持久化边界需要包含什么？',
        'answer': '至少要知道稳定业务身份、原始内容、已接受的本地命令顺序及已确认结果，且定义何时向上游声称接收成功。发送进度只是一部分，崩溃时仍可能存在远端已处理但本地没记结果的窗口。恢复要按对端协议对账和去重，不能从重建空队列开始自动生成新 ID。',
        'rubric': ['绑定身份内容顺序与接收承诺', '保留发送后未记结果的未知窗口'],
        'source':
          { 'kind': 'derived', 'rationale': '由网关崩溃点与远端业务提交不可原子覆盖的边界推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q14',
        'level': 'L3',
        'prompt': '仅测量 send 调用耗时，为什么不能代表交易网关延迟？',
        'answer': '完整路径包含入口排队、编码、发送后等待、对端处理、回报解析和本地结果交付，重连时还可能有恢复等待。应分别记录阶段时间以及业务确认的端到端分布，明确接受不等于成交。只测本地发送调用可能漏掉背压、未知结果停留时间和消费者变慢。',
        'rubric': ['覆盖完整确认路径', '区分接受和成交并报告等待'],
        'source':
          { 'kind': 'derived', 'rationale': '由本地发送与完整业务确认延迟的测量范围差异推导' },
        'companies': [],
      },
      {
        'id': 'trading-gateway-q15',
        'level': 'L3',
        'prompt': '脚本传输测试通过后，还缺少哪些真实故障验证？',
        'answer': '本例脚本同步应用完整帧，并保证关闭后不再处理，强于真实网络。还需测试真实流式接收、TLS、系统调用错误、连接生命周期、远端异步处理、查询历史范围、日志崩溃点与重连风暴。脚本只验证所声明模型内的状态转移，不能证明实际交易接口的恰好一次业务执行。',
        'rubric': ['识别脚本的强假设', '列出真实协议和恢复缺失范围'],
        'source':
          { 'kind': 'derived', 'rationale': '由确定性模型与实际异步网络及业务处理差异推导' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

交易网关把上游业务请求转换成对端协议，并维护发送、确认和恢复状态。业务请求 ID 关联原始内容与结果，连接 generation 隔离旧连接回调，重连不应自动产生新业务身份。写完字节仍需等待应用确认；断线后结果不明时保留 unknown，通过协议允许的查询或重传恢复。队列、未决请求和结果保留都要有边界。本章用虚构脚本模型验证这些状态，不连接任何真实交易接口。

## 核心概念

TCP 提供连接内的可靠有序字节流，不包含订单接受、成交或清算的业务含义。Linux send 文档说明发送返回值不能充当隐含的交付成功通知；部分返回只推进相应字节。接收端也必须按应用帧规则处理读到的字节，而不能假设一次 recv 恰好对应一个请求。[RFC 9293](https://www.rfc-editor.org/rfc/rfc9293)、[send(2)](https://man7.org/linux/man-pages/man2/send.2.html)、[recv(2)](https://man7.org/linux/man-pages/man2/recv.2.html)

需要分别维护几类身份：

| 身份            | 关联对象                    | 本章范围             |
| --------------- | --------------------------- | -------------------- |
| 业务会话        | 去重结果与请求命名空间      | 固定为 7，不自动迁移 |
| 请求 ID         | 原始内容及一次接受/拒绝结果 | 1..16，本实例不复用  |
| 连接 generation | 本地连接与其回调            | 1..16，耗尽后冻结    |
| 协议消息序号    | 会话内消息顺序与恢复        | 本章未实现 FIX 序号  |

FIX Session Layer 2020 文档区分会话层 retransmission 与应用层 resend：前者用于恢复消息序列，后者涉及未获得业务确认的应用请求。业务重复处理仍由应用协议负责，不能把会话补包直接等同于业务去重。[FIX Session Layer，第 3.1 和 4.9 节](https://www.fixtrading.org/wp-content/uploads/download-manager-files/FIX_Session_Layer_June_2020.pdf)

真实回报还应区别请求标识、场所订单标识和执行报告标识。例如 Coinbase 的具体 FIX 文档分别列出 ClOrdID、OrderID 和 ExecID，并用状态与执行类型说明接受、成交、取消等变化。本章只有“请求接受/拒绝”这一次响应，不实现订单后续生命周期，也不把 accepted 解释为已成交。[Coinbase Order Entry](https://docs.cdp.coinbase.com/exchange/fix-api/order-entry-messages/order-entry-messages5)、[Drop Copy](https://docs.cdp.coinbase.com/exchange/fix-api/drop-copy)

## 原理深入

本模型一次只推进队头。请求进入队列后保留原 ID 和 quantity，编码成固定五字节帧。每次 pump 最多执行一次脚本 write；零字节进展保持当前偏移，部分写只发送后续片段，写满后等待应用结果。

| 状态         | 允许的主要动作         | 断线后的处理             |
| ------------ | ---------------------- | ------------------------ |
| queued       | 从完整帧开头发送       | 从未写过字节则仍 queued  |
| sending      | 沿同一连接 offset 继续 | 有进展则转 unknown       |
| awaiting_ack | 接受匹配应用结果       | 转 unknown               |
| unknown      | 等待权威查询/恢复证据  | 不自动重发，不放行后续   |
| final        | 等本地消费者取结果     | 结果保留，不改成 unknown |

“写过部分帧”在本脚本中不够触发对端应用，因为脚本只在完整帧后处理。网关仍统一采取保守策略，把任何有发送进展但未确认的断线请求标为 unknown。它不根据本地 offset 猜测真实对端是否还可能收到缓冲中的其他字节。

Unknown 有两种不同恢复结果。查询到已处理记录时，校验会话、ID、payload 后返回该历史结果，不再次应用请求。仅收到普通 NotFound 则继续等待；缺失可能是查询时点或历史范围造成。只有明确证据表明旧请求已越过处理边界、不会再晚到生效，才考虑按对端协议重试。

示例的 `absent_fenced` 是虚构协议的强前提：ScriptTransport 关闭后，完整帧早已同步处理，不完整帧被丢弃且没有后台待处理工作；同一个 Peer 保存会话内全部历史。真实 TCP close 只操作连接，不能自动证明这些远端条件。把普通查询结果或本地关 socket 当作 absent_fenced，会在异步远端处理时留下重复应用窗口。

## 数据结构/系统内部实现

Gateway 用固定数组保存 4 条排队记录，每条包含业务 ID、quantity、不可变帧、offset 和 phase。seen 记录本地已接收的 16 个身份，completed 保存已确认结果；结果消费后 seen 和 completed 仍保留。队列满时拒绝新请求，不占用被拒绝的 ID，也不覆盖已有记录。16 个成功接收身份用完后无法继续创建新身份，本例没有持久化身份分配器。

Peer 是脚本对端，按业务会话和 ID 保存原 quantity 及结果。首次收到 quantity=8 时返回业务拒绝，其余合法数量返回接受；这个规则仅用于测试两种响应，不代表风险规则。同 ID 同内容返回旧结果，不增加应用处理次数；同 ID 不同内容返回 conflict。

每次 connect 在同业务会话内增加 generation；会话不同或代次上限耗尽会冻结。旧 transport 不能用来 pump 当前队头，旧代的应用确认与 resolve 回调也不能改变当前状态。查询应在发起时捕获 generation，响应必须携带该上下文；处理时给旧响应重新贴上当前 generation 会绕过检查。

本例只在队头 unknown 上串行接受恢复证据。更一般的系统同时发起多次查询，或同连接内有不同恢复阶段，还需独立 query_id、阶段版本或日志位置。generation 只能区分连接，不能识别同一连接上两个查询的先后。本章没有将它当作所有回调的充分身份。

确认路径只允许 accepted/rejected 两种合法 verdict，未知枚举、内容冲突或不匹配的业务会话使网关冻结。resolve 同样白名单校验 EvidenceKind。重复相同结果无操作，矛盾结果不覆盖旧值。尚未写完帧的当前请求或其他 ID 的确认被视为 unexpected，不能使队头提前完成。

核心状态与脚本传输都采用固定值类型，没有动态分配、后台线程或可抛出的发送回调。所有权是单 owner；这不提供跨线程的原子提交。真实实现若改成异步队列或分配型缓冲，需要重新审查缓冲所有权、异常、回调取消和销毁顺序。

## C++ runnable demo

```cpp include=examples/trading-gateway.cpp

```

这是 C++20 的本地状态机演示，不是 FIX、TLS 或 socket 实现。五字节帧为 magic、业务会话、ID、quantity 和 XOR 校验；XOR 只用于检查脚本中的帧组装，不提供认证或安全完整性。

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  examples/trading-gateway.cpp -o /tmp/trading-gateway
/tmp/trading-gateway
```

测试穷举五字节帧内部四个位置是否切分，共 16 种分段方式，在每段前加入零进展步骤；每个偏移均与独立期望相等，完整帧前对端应用次数必须为零。帧写满时网关仍无可消费结果，收到应用确认后才 final。

断线测试分别发生在已写 0..5 字节，共 6 个位置。写零字节可直接沿新连接发送完整原帧；其余位置先保留 unknown。部分帧断线时，普通未隔离的缺失证据不能推进，脚本关闭后的强缺失证据才允许重试。完整帧但确认丢失时，对端已有结果，查询直接完成，不再次应用；旧代的迟到确认被拒绝。每条路径都独立断言 ID 1 只处理一次，随后才推进 ID 2。

边界还包括非法 ID/quantity、队列满、已完成结果未消费继续占容量、业务拒绝、重复结果、同 ID 不同 payload、矛盾结果、业务会话变化和 generation 耗尽。额外断言未知 Verdict/EvidenceKind 不产出结果，旧代查询响应保持 unknown 不变。程序无 sleep、无限重试或真实外部流量。

本次 x86-64 WSL2、GCC 13.3.0 严格 O2 与 ASan/UBSan 通过，输出：

```text
16 frame partitions, 6 disconnect positions, session/ID/backpressure: OK
Script transport only; no network or real orders.
```

这些是按场景独立编写的状态、偏移、结果和对端处理次数断言，没有用另一份相同网关实现作唯一 oracle。脚本可以覆盖确定失效时点，但不模拟真实网络乱序处理、服务器排队、进程崩溃、日志损坏或认证。

## 高频追问

### 心跳正常是否说明订单状态已知？

心跳说明某种连接或会话活性，不能替代特定业务请求的结果。网关应分别维护传输活性、协议会话状态与业务未决集合。请求超时可以触发查询或告警，不应自动改成 rejected。本例没有计时器，timeout 的实际调度与退避策略留给外层。

### 取消连接上的订单能消除未知结果吗？

要按接口的具体保证核对。Coinbase 的文档明确提示未确认订单不保证被某些批量取消或断线取消机制覆盖；批量取消请求的成功受理也不等于所有目标订单已经取消。这说明网关需要区分“操作被接收”和最终业务变化，不能只靠一条控制请求清空 unknown。[Coinbase 取消与回报规则](https://docs.cdp.coinbase.com/exchange/fix-api/order-entry-messages/order-entry-messages5)

### 结果已确认，为什么还阻塞后续发送？

本章为简化顺序证明，把消费队头结果作为继续推进的条件。这样结果消费者停滞会形成明确背压，代价是更强的队头阻塞。实际系统可以分离发送窗口和结果队列，但两者都必须有界，并定义结果满时如何限制新请求，不能通过无界堆积隐藏压力。

## 容易答错的点

- “断线重连后把未确认单换新 ID 再发。” 原请求可能已生效，新身份会绕过原去重关系。
- “NotFound 就是从未处理。” 查询的时点、范围和旧请求完成边界必须明确。
- “本地 close 后旧请求不会再执行。” 真实远端可能已接收或正在处理，脚本关闭的强语义不能照搬。
- “连接代次每次回调读取当前值即可。” 必须绑定原操作上下文，否则迟到回调会被误认成当前操作。
- “确认是 accepted，订单生命周期就结束。” 本章结束的是请求响应；真实订单还可能有部分成交、取消和其他状态变化。

## 性能分析

固定大小模型中，每次 pump、确认和查询处理只做有界值操作；pop_result 移动最多 3 条记录。若队列容量推广为 N，当前消费实现为 O(N)，可换成环形队列，但必须保持结果释放与业务顺序。帧大小推广为 L 后，编码和发送复制至少包含相应字节工作，不能把固定五字节演示视为通用零成本。

队列容量要同时覆盖已排队、发送中、等待确认、unknown 和待消费结果。上游到达突发、对端响应变慢或消费者停滞会拉长保留时间；只看平均网络 RTT 无法决定容量。记录队列高水位、拒绝数、unknown 停留时间、重连次数和结果消费延迟，并明确溢出策略。

延迟应按入口接受、完成写入、业务确认及上游取结果分段，另报端到端 p50/p99/p99.9 和吞吐。请求接受、撮合成交与结算具有不同终点，必须选定业务指标。若要模拟独立外部到达流，压测不能因为上一请求未完成就自动减慢输入而不作说明；相应闭环业务则应使用匹配模型。

本例没有计时、网络或真实对端，因此没有延迟和吞吐数字。脚本中的处理次数只能证明声明模型内没有重复应用，不能导出实际交易场所的恰好一次保证。后续优化须先保留同样的故障分类和独立期望，避免用丢失 unknown 或覆盖队列换取表面吞吐。

## Quant/Low-Latency 场景

本章聚焦协议适配和恢复边界。订单生命周期与风险额度应由相应组件维护，网关不能把连接已恢复等同于风险状态已恢复。向上游报告“本地排队成功”或“远端接受”时应使用不同事件，避免业务方把本地入队当作对端承诺。

多连接并行时，可以按账户、场所或业务域划分发送窗口，但需要明确哪类请求必须相互有序。一个取消依赖原订单身份，恢复期间换连接不能失去这层关联。消息序号、业务 ID 与日志位置都应保留可追踪关系，便于将迟到回报和重放结果归到原请求。

进程恢复需要持久化稳定身份、内容与已承诺接收的顺序，配合结果日志和对端恢复协议。本章 Gateway、Peer 的去重历史都只在内存中，重新创建对象会丢失历史，所以不能通过构造新实例实现生产恢复。遇到对端业务会话变化时冻结，正是为了避免未经对账就把旧未知请求映射进新身份域。

## 相关专题

- [TCP framing](../network/tcp-framing.md)：把流式字节读写与完整业务帧分开。
- [连接生命周期](../network/connection-lifecycle.md)：继续审查真实 fd、事件注册和关闭后的引用。
- [Matching Engine](../trading/matching-engine.md)：区分请求接受、撮合提交与结果发布的保证。
- [SPSC Queue](../concurrency/spsc-queue.md)：设计单生产者/消费者之间的有界交付与背压。

## 分层面试题

15 道题按 L1/L2/L3 各 5 道组织。先区分字节发送与业务确认，再解释未知结果、回调上下文和背压，最后讨论具体协议、持久化及故障模型的边界。
