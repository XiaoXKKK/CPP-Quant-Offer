---
{
  'schemaVersion': 1,
  'id': 'feed-handler',
  'title': 'Feed Handler：从字节到可发布事件',
  'description': '沿解码、字段验证、标准化、序号状态和发布所有权实现一条有界行情处理路径，校验重复、重叠、缺口与失败时的状态边界。',
  'category': 'trading',
  'areas': ['Market Data', 'Feed Handler'],
  'tags': ['feed-handler', 'market-data', 'decoder', 'sequence', 'ownership'],
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
  'prerequisites': ['固定宽度整数与字节序', 'span 的借用语义', '消息序号与有效性状态'],
  'related':
    ['order-book', 'cpp-memory-model', 'mutex-condition-variable', 'benchmark-methodology'],
  'demo':
    {
      'file': 'examples/feed-handler.cpp',
      'platform': 'portable',
      'exercise': '为 FH1 的测试加入最大 uint32_t 价格和数量，写出独立 normalized Event 期望；再验证一个重叠帧的未见后缀恰好填满发布区，之后的新帧整体失败且 next_sequence 不前进。',
    },
  'references':
    [
      {
        'title': 'Nasdaq TotalView-ITCH 5.0 specification',
        'url': 'https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Nasdaq MoldUDP64 protocol specification v1.00',
        'url': 'https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/moldudp64.pdf',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: span',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/views.span',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: type accessibility and byte access',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/basic.lval',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: data races and synchronization',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/intro.races',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux recv: message length and truncation',
        'url': 'https://man7.org/linux/man-pages/man2/recvmsg.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'feed-handler-q01',
        'level': 'L1',
        'prompt': 'Feed Handler 接收到字节后，发布行情事件前还需要做什么？',
        'answer': '需要确认帧边界和长度，按协议解码并验证字段，处理证券标识及价格单位，再按对应通道和会话的序号状态决定哪些事件可以发布。事件跨过接收 buffer 的寿命时还要有明确所有权。成功收包或成功解析一条记录，都不代表当前行情连续且可用。',
        'rubric': ['解码与字段验证', '标准化和序号有效性', '发布数据的所有权'],
        'source':
          { 'kind': 'derived', 'rationale': '根据字节输入到下游事件之间的完整处理链设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q02',
        'level': 'L1',
        'prompt': '为什么不能把网络 buffer 直接转换成自定义结构体指针读取字段？',
        'answer': '网络格式的偏移、宽度和字节序不由本机结构体布局决定，结构体还可能有填充和对齐要求。指针转换也不自动证明对应对象已经存在且可以按该类型访问。本例先检查长度，再逐字节组合无符号整数；它不依赖接收地址对齐或编译器的结构体打包方式。',
        'rubric': ['布局和字节序不同层次', '对齐与对象访问边界', '先检查范围再解码'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 wire layout 与 C++ 对象访问规则的差异设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q03',
        'level': 'L1',
        'prompt': '收到一个包后，next_sequence 应增加一还是增加消息数？',
        'answer': '取决于协议的序列单位，不能从网络包数量猜测。本文 FH1 的 first 是第一条记录的序号，后续记录连续编号，接受完整新后缀后 next_sequence 到达 first 加 count。真实 MoldUDP64 同样以消息序号描述首条消息，但它的帧结构和控制消息规则并不等于 FH1。',
        'rubric': ['先确认协议序列单位', '区分包与消息', '不把教学格式当真实协议'],
        'source':
          { 'kind': 'derived', 'rationale': '根据消息聚合导致的包计数与消息序号差异设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q04',
        'level': 'L1',
        'prompt': '标准化价格时直接转成 double 是否足够？',
        'answer': '还需要确定源精度、目标单位、范围和舍入规则。本文将整数 price_e4 先提升到 uint64_t，再乘 10000，得到整数 price_e8，并检查价格和数量的业务组合。真实接入可能存在不同 tick、精度或特殊值，应依协议和参考数据处理，不能把这里的倍数当成通用规则。',
        'rubric': ['明确源和目标单位', '转换前检查类型范围', '保留协议与参考数据条件'],
        'source':
          { 'kind': 'derived', 'rationale': '根据整数价格标准化与源精度可能不同的条件设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q05',
        'level': 'L1',
        'prompt': '把 span 放进下游队列，是否就把接收数据的所有权交出去了？',
        'answer': '没有。span 只是指向既有连续对象的视图，不延长接收 buffer 的寿命。若接收线程复用 buffer，下游可能读到另一帧或已经失效的对象。本例发布按值保存的 Event，返回后覆盖原输入仍保持结果；零拷贝方案需要另外设计 buffer 归还和复用条件。',
        'rubric': ['span 不拥有存储', '复用导致借用失效', '值复制或明确 buffer 回收协议'],
        'source': { 'kind': 'derived', 'rationale': '根据接收缓冲区复用与跨阶段借用寿命设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q06',
        'level': 'L2',
        'prompt': '一帧里第一条合法、第二条字段错误，为什么示例不先发布第一条？',
        'answer': '本例选择整帧字段校验完成后再提交。所有规范化事件先放在最多四项的局部数组里，遇到坏字段就停用通道，发布区和 next_sequence 保持原值。这样不会产生半帧已发布但序号状态无法一致提交的问题；这是本例的批次策略，不声称所有协议都要求帧级原子处理。',
        'rubric': ['局部暂存后提交', '失败不推进序号或发布前缀', '限定为示例策略'],
        'source': { 'kind': 'derived', 'rationale': '根据迟发现坏记录时的发布与序号一致性设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q07',
        'level': 'L2',
        'prompt': '期望序号是 102，收到从 101 开始的两条记录，应怎样处理？',
        'answer': '本文先完整校验目标会话帧，然后跳过 101，只提交未见的 102，成功后 next_sequence 变为 103。前提是同一通道、会话和序号对应的内容不可变。本例没有保留已发布 payload 历史，因此不会检测同一身份但内容冲突的副本；不能把按序号跳过写成已验证内容相同。',
        'rubric': ['处理未见后缀', '成功后再推进序号', '身份不可变前提与冲突检测边界'],
        'source':
          { 'kind': 'derived', 'rationale': '根据重传区间重叠与已处理前缀的归属关系设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q08',
        'level': 'L2',
        'prompt': '遇到没有映射的证券 locate，可以直接丢掉记录并推进序号吗？',
        'answer': '要先区分有意不订阅与无法解释语义。前者可以在明确的通道序号协议下过滤业务发布，后者可能表示缺失参考数据或版本不匹配。本例只有固定的 locate 1 和 2，未知值使整帧失败并停用通道，没有默默将未知数据当成已经处理。生产接入应把参考数据版本和流状态一起管理。',
        'rubric': ['有意过滤与未知映射区分', '序号处理与业务发布分开', '参考数据版本及失效策略'],
        'source':
          { 'kind': 'derived', 'rationale': '根据证券映射缺失与正常订阅过滤的不同语义设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q09',
        'level': 'L2',
        'prompt': '发布区只剩一个位置，但新后缀有两条，示例如何保持状态一致？',
        'answer': 'Publisher 在复制前检查整批容量，不够就拒绝全部后缀。Handler 保持 next_sequence 和已发布历史不变，返回 backpressure 并变为 invalid；后续 ingest 返回 blocked。它只实现有界失败策略，没有排队反馈、重放或恢复机制，调用方不能忽略状态继续把旧行情当作当前有效值。',
        'rubric': ['整批容量检查', '发布和 next_sequence 不部分提交', '停用与完整背压机制区分'],
        'source':
          { 'kind': 'derived', 'rationale': '根据消费者承载能力不足时的发布事务边界设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q10',
        'level': 'L2',
        'prompt': '收到另一个 session 的帧，能直接把当前 session 和 next_sequence 改成新值吗？',
        'answer': '不能仅凭这一帧重设状态，新会话还涉及序号锚点和参考数据等条件。FH1 的 session 由调用方预先建立：头部、长度及序号范围合法后，不同 session 返回 wrong_session 且不切换。坏头或坏序号在身份过滤之前会停用当前处理器，因此本例也不保证所有外来会话数据都被无害忽略。',
        'rubric': ['会话切换需建立状态', '明确过滤发生顺序', '不自动重置或跳过异常'],
        'source': { 'kind': 'derived', 'rationale': '根据会话边界与处理器启动锚点的关系设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q11',
        'level': 'L3',
        'prompt': '跨线程发布时，队列已满又检测到 gap，怎样通知下游行情失效？',
        'answer': '应事先设计能够传达失效状态的控制路径，而不能只尝试向同一已满数据队列塞一条通知后丢弃。可结合带代际的共享状态、保留控制容量或暂停消费协议，但需要明确同步和下游检查时点。本例的同步调用方直接读取 Handler.valid，未实现跨线程健康发布，也不能把普通 bool 当成并发通知。',
        'rubric': ['失效通知不能静默丢失', '控制状态与代际的交付协议', '同步和示例范围'],
        'source':
          { 'kind': 'derived', 'rationale': '根据数据通道饱和时下游必须感知失效的要求设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q12',
        'level': 'L3',
        'prompt': '合并两路冗余行情时，先到的消息按 sequence 去重是否足够？',
        'answer': '先确认两路是否共享同一序列域、会话和不可变消息身份，再设计去重、重排及缺口规则。还要考虑同一身份内容冲突、某一路滞后、参考数据切换和重放边界。FH1 单路示例没有 payload 历史比对或双路仲裁，不能直接复制其小于 next 就跳过的逻辑，宣称冗余合并已经正确。',
        'rubric': ['核对共同序列域', '冲突、滞后及重放条件', '单路示例不证明双路仲裁'],
        'source': { 'kind': 'derived', 'rationale': '根据冗余来源合并时的身份与内容一致性设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q13',
        'level': 'L3',
        'prompt': '协议升级增加字段或消息类型时，如何决定能否兼容跳过？',
        'answer': '需要依据版本协商、消息长度及未知字段的协议语义判断，尤其要知道它是否影响后续状态。不能因为知道字节数就默认忽略。本例只接受 version 1 和固定记录布局，未知版本直接失效。生产上应使用版本化 decoder、相应参考数据和真实报文回放验证，再确定哪些扩展可以安全跳过。',
        'rubric': ['兼容性由协议语义决定', '状态影响与长度都要考虑', '版本化解析与回放验证'],
        'source': { 'kind': 'derived', 'rationale': '根据二进制格式演进与下游状态依赖设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q14',
        'level': 'L3',
        'prompt': '解码器微基准更快，为什么 Feed Handler 的端到端 p99 仍可能更差？',
        'answer': '端到端还包含收包等待、批次积累、参考数据查询、序号处理、发布排队和消费者处理。优化可能扩大批次或改变缓存与所有权成本，使局部均值下降而排队尾部上升。应按同一消息身份关联各阶段时间，保留负载、积压、缺口及失效事件，不能用纯解码时间替代数据可用时间。',
        'rubric': ['分解完整路径', '批量和排队的尾部取舍', '统一身份与测量口径'],
        'source':
          { 'kind': 'derived', 'rationale': '根据解析局部成本与下游可用延迟的测量差异设计。' },
        'companies': [],
      },
      {
        'id': 'feed-handler-q15',
        'level': 'L3',
        'prompt': '检测到 gap 后，把 next_sequence 改成当前帧首序号并恢复 valid，有什么问题？',
        'answer': '这样只跳过了检测到的缺失区间，没有补回可能影响当前状态的消息。应按具体协议通过重放或与明确序号边界关联的快照重新建立状态，再证明增量连续后恢复发布。FH1 检测 gap 后停用且没有恢复入口；构造函数的 session 和 next 是调用方已建立的前提，不是自动完成了快照恢复。',
        'rubric': ['跳序号不等于恢复状态', '重放或快照的明确衔接点', '启动前提与实现功能区分'],
        'source':
          { 'kind': 'derived', 'rationale': '根据缺失消息可能影响状态及恢复连续性要求设计。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

Feed Handler 将输入字节转换成下游可使用的行情事件。它需要按协议验证帧和字段，完成证券及价格单位映射，再结合通道、会话和序号决定发布范围。发布之后谁拥有数据也必须明确。检测到缺口或无法完成发布时，应让下游知道状态已失效；只丢掉坏包继续发送“正常行情”，会把不连续的数据伪装成完整状态。

本文使用 C++20 和虚构的 FH1 教学协议。它只承载一个已建立会话中的顶层报价替换，不是 Nasdaq ITCH、MoldUDP64 或其他交易所的实现，不包含网络收包、快照恢复、订单簿撮合或真实交易。

## 核心概念

| 阶段     | 要回答的问题               | FH1 示例的做法                             |
| -------- | -------------------------- | ------------------------------------------ |
| 帧边界   | 输入是否包含完整且恰好一帧 | 12 字节头加 count 个 12 字节记录           |
| 解码     | 哪些字节构成哪种值         | 显式大端读取，不解释为本机结构体           |
| 验证     | 值和组合是否属于已知协议   | 版本、数量、证券、side、保留位、价格与数量 |
| 标准化   | 下游使用什么身份和单位     | 固定 locate 映射及整数 price_e8            |
| 序号状态 | 哪个未见区间可以连续提交   | 单通道、指定 session、next_sequence        |
| 发布     | 事件存储归谁，满时怎么办   | 按值写入有界历史，整批失败则停用           |

网络载荷和业务消息不是同一层。Nasdaq TotalView-ITCH 5.0 定义行情消息，其文档列出外层交付选项；MoldUDP64 v1.00 的包头则描述会话、第一条消息序号和消息数量，一个包可以承载多条消息。这些资料用于说明层次，FH1 的字段与规则由本文另行定义。[ITCH 5.0](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf)、[MoldUDP64](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/moldudp64.pdf)

这里的报价是每个证券每一侧的绝对顶层状态替换，零数量表示清空该侧。它不包含 order ID、逐订单队列或深度档位，不能用于推导 MBO 队列优先级。

## 原理深入

### 解码前先证明范围可读

ingest 先检查至少有 12 字节，再访问头部。count 必须为 1 到 4，完整长度必须恰好为 `12 + count * 12`，所以每条记录的所有字段都在已知范围内；截断和额外尾字节均拒绝。be16/be32 自身不承担任意地址的校验，调用者先建立读取前提。

代码逐字节组合无符号整数，不依赖 CPU 本机字节序、对齐或结构体填充。把接收字节 reinterpret_cast 成某个 struct，并不会自动满足 C++ 对象访问条件；即使编译器支持 packed 扩展，也仍须处理长度和字节序。[C++20 类型访问规则](https://timsong-cpp.github.io/cppwp/n4861/basic.lval)

真实接收层还应确认数据报没有被截断。例如 Linux recvmsg 的 MSG_TRUNC 表示尾部数据被截去；不能只把保存下来的前缀交给业务 decoder 后宣称完整。本文直接接收内存中的完整候选帧，不实现 socket 和接收元数据检查。[Linux 接收长度与截断](https://man7.org/linux/man-pages/man2/recvmsg.2.html)

### 一帧通过校验后再改变发布状态

FH1 先验证头部、长度和序号范围，然后过滤不同 session。对目标 session，逐条检查并标准化到最多四项的局部数组。任一条失败，整帧不发布，next 保持不变，valid 变为 false。第二条坏记录不会留下第一条已经发布的前缀。

字段通过后，再解释消息区间 `[first, end)`，其中 end 用 uint64_t 计算并验证可以保存为 next：

```text
end <= next       → 完全重复，不发布
first > next      → 缺口，停用通道
first <= next < end → 跳过旧前缀，提交从 next 开始的后缀
提交成功          → next = end
容量不足          → 不提交，next 不变，停用通道
```

对于目标 session，即使整帧序号已经旧了，也要先完成字段校验。这个顺序是示例选择的保守策略；它没有对旧消息 payload 作历史比对。去重依赖“同一通道、session、sequence 的内容不可变”这一协议前提，冲突副本检测不在本例内。

### 事件拥有值，状态由同步调用方观察

Event 保存 session、sequence、内部证券 ID、side、整数价格和数量，全部按值复制。Publisher 不持有输入 span 或局部 staged 数组的指针；ingest 返回后接收 buffer 可以复用。测试把输入数组填零后仍与显式事件 oracle 比较，验证这条所有权路径。[span 是借用视图](https://timsong-cpp.github.io/cppwp/n4861/views.span)

Publisher 只是单线程、有八个位置的历史存储，没有 drain、队列同步或持久化。本例在一次同步调用内先提交完整后缀，再推进 next，不存在回调在两步之间观察状态。该批次不可部分提交的性质不等于机器原子操作，也不能直接推广到多线程队列或多个下游。

## 数据结构/系统内部实现

FH1 帧头如下。所有多字节整数均为大端。

| 偏移 | 字节数 | 字段    | 本文规则                              |
| ---- | ------ | ------- | ------------------------------------- |
| 0    | 2      | magic   | 固定 0x46、0x48                       |
| 2    | 1      | version | 仅接受 1                              |
| 3    | 1      | count   | 1 到 4；没有 heartbeat 或结束会话编码 |
| 4    | 4      | session | 非零，由调用方预先建立                |
| 8    | 4      | first   | 第一条记录序号，后续逐条递增          |

每条记录从自己的起点计算偏移：

| 偏移 | 字节数 | 字段     | 本文规则                                     |
| ---- | ------ | -------- | -------------------------------------------- |
| 0    | 2      | locate   | 1 映射到 1001，2 映射到 1002                 |
| 2    | 1      | side     | 1 为 bid，2 为 ask                           |
| 3    | 1      | flags    | 必须为 0                                     |
| 4    | 4      | price_e4 | 单位为 10^-4，先提升再乘 10000 得到 price_e8 |
| 8    | 4      | quantity | 数量单位为 1；清空该侧时数量与价格同时为 0   |

非零数量要求价格非零，零数量要求价格也为零。最大 uint32_t 价格乘 10000 可以放入 uint64_t；这里没有浮点转换或舍入。它只验证教学格式，不包含真实市场的 tick、价格区间、证券状态或交易资格规则。

每个 Handler 对应一个外部已区分的通道，Event 中因此没有另存 channel ID。多通道系统需要在路由或事件身份中保留它，不能只凭 sequence 合并。真实参考数据也可能随会话变化，例如 ITCH 的 stock locate 按日分配；FH1 的固定映射没有实现动态字典和参考数据 epoch。[ITCH 身份及价格字段](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf)

FH1 不支持序号回绕，允许发布的消息序号最高为 UINT32_MAX−1，UINT32_MAX 可作为耗尽后的 next。状态处理如下：

| 结果                             | 当前帧是否发布         | next 与有效性                                |
| -------------------------------- | ---------------------- | -------------------------------------------- |
| published                        | 仅发布未见后缀         | next 到 end，保持 valid                      |
| duplicate                        | 不发布                 | 保持原状态                                   |
| wrong_session                    | 不发布，也不解释记录体 | 头部和序号范围先通过才可到此分支；保持原状态 |
| malformed / sequence_range / gap | 不发布                 | next 不变，invalid                           |
| backpressure                     | 不发布任何新后缀       | next 不变，invalid                           |
| blocked                          | 不继续解析             | 已 invalid，保持原状态                       |

不同 session 不会触发自动切换，但坏头、非法长度或坏序号在 session 过滤之前会停用处理器。接入层应先把数据路由到正确通道，并按部署要求制定异常来源策略，不能把本例描述成对任意外来报文都无害忽略。

## C++ runnable demo

```cpp include=examples/feed-handler.cpp

```

编译运行时保持断言启用，因为测试在断言表达式中调用 ingest：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/feed-handler.cpp -o /tmp/feed-handler
/tmp/feed-handler
```

预期输出：

```text
normalization/ownership/duplicate/overlap: 3 oracle events passed
truncation/fields/late-record/range failures: no partial publication
session/gap/backpressure/sequence exhaustion: state checks passed
FH1 teaching protocol only; no transport, recovery, or trading implemented
```

测试内置 36 字节 golden 帧，以及三条独立写出的规范化事件。前两条来自 golden，第三条来自重叠帧的未见后缀；重复的旧记录不会再次发布。辅助 packet 编码器用于组织其他测试，golden 与期望事件没有依靠它生成，避免编码器与解码器共享同一错误时只互相验证。

失败测试遍历 golden 的全部 36 种短前缀，还覆盖额外尾字节、未知版本、坏数量、零 session、未知证券、坏第二条 side、保留位、价格数量组合及序号范围。发布区剩一项而新后缀有两项时，已发布的七条历史保持不变，next 不前进，处理器失效。断言也检查失效后的合法帧仍被 blocked。

本次在 WSL2 x86-64、GCC 13.3 上通过严格编译及 ASan/UBSan。没有网络接入、持久化、跨线程发布、双路仲裁或性能测量；这些断言不能证明真实交易所协议已经被完整支持。

## 高频追问

### 能不能用 receive buffer 做零拷贝发布？

可以设计，但需要让 buffer 所有者知道所有借用者何时结束。队列中仅保存一个地址不足以表达这个约定；慢消费者会延迟归还，池耗尽时仍需有界策略。复制较小的 Event 有明确寿命，是否值得换成共享 buffer，应测完整发布与回收路径。

### 为什么序号状态不能按每个证券随意拆开？

序列域由协议定义。若同一通道交错发送多个证券，某证券没有新消息不代表通道停顿；过滤不订阅证券也不能使通道 next 停在它的序号上。可以按业务需要分发证券事件，但顺序验证要覆盖协议要求的域。FH1 在验证整帧后才发布，不实现订阅过滤。

### 成功发布之后，下游一定有完整可用的状态吗？

还依赖下游启动状态、之前的完整历史和后续失效通知。示例构造函数要求 session 与 next 已建立；这并未证明调用方已经拿到初始快照，也没有创建一个完整 Order Book。调用方必须按自身状态协议消费事件，并在 Handler invalid 时停止将旧值视为当前有效行情。

## 容易答错的点

| 错误说法                               | 修正与原因                                   |
| -------------------------------------- | -------------------------------------------- |
| 收到了数据就说明行情有效               | 帧、字段、连续性和下游状态仍需验证           |
| 一包对应一个 sequence                  | 包可以聚合多条业务消息，按协议确定序列单位   |
| span 可以延长 receive buffer 寿命      | 它是借用视图，不负责存储释放或归还           |
| sequence 小于 next 已证明 payload 相同 | 本例只按身份跳过，依赖消息不可变前提         |
| 别的 session 全部可以安全忽略          | 示例先校验头部和序号，异常可能先触发 invalid |
| 满了丢两条，再推进 next 就行           | 下游会缺事件；本例整批拒绝并停用             |
| 普通 bool valid 能通知其他线程         | 并发访问需要正确同步与可见性协议             |
| 跳到最新序号就恢复了状态               | 还未补齐缺失消息造成的状态差异               |

## 性能分析

每帧最多四条记录，ingest 的检查和复制都受这一上界约束；将条数推广为 m 时，解析与标准化为 O(m)。当前热路径没有动态分配，使用固定局部数组和有界 Publisher。测试辅助 packet 使用 vector，不属于接收处理路径。

固定数组减少了这个实现的分配来源，不构成固定延迟保证。真实路径还可能等待收包、访问参考数据、发现 gap、记录日志或等待消费者。测量时分别记录字节可读、字段完成、连续性通过和下游可用的时间点，并用相同消息身份关联；时间戳来自不同钟域时还需处理校准和误差。

比较零拷贝、批次大小或 decoder 分支布局时，保存报文类型与大小分布、突发强度、CPU/内存放置和消费者行为。除吞吐及 p50/p99/p99.9 外，还应记录被拒帧、gap、失效持续时间和发布积压。较大的批次可能摊薄成本，也可能让前面的记录等待整批校验；这些取舍需要真实负载证据，本例没有给出加速结论。

跨线程传递还需补上发布与复用同步。仅先写 Event 再写普通计数器，不会自动建立 happens-before；本例的 single-thread Publisher 不能被多个线程同时调用。[C++20 数据竞争与同步](https://timsong-cpp.github.io/cppwp/n4861/intro.races)

## Quant/Low-Latency 场景

行情接入层可以把“解析失败”和“当前状态不可用”作为独立可观测信息交给下游。前者用于定位原始帧和协议问题，后者决定已有报价是否还能被当作当前状态。坏帧前已提交的历史可以留作审计，但不能因此继续对外宣称行情连续。

当消费者落后、发布区已满时，失效状态不能只依赖向同一满队列再塞一条控制消息。实际系统需要能够交付的状态通道、代际或保留控制容量，以及下游检查协议。这里的调用者同步看到返回值和 valid，未实现这种并发控制路径。

接入恢复需要把快照边界、重放区间和当前增量流联系起来。FH1 只负责停在已知失效点，没有自动重连、重传请求或恢复入口，也不生成订单。与这些功能衔接时，应先说明哪个状态已经被重建、从哪个序号继续，再恢复下游可用标记。

## 相关专题

- [Order Book](./order-book.md)：规范化事件后仍需按明确业务语义维护状态；FH1 顶层报价不等于 MBO 订单事件。
- [C++ 内存模型](../concurrency/cpp-memory-model.md)：扩展跨线程发布时，需要证明数据可见与存储复用关系。
- [mutex 与 condition_variable](../concurrency/mutex-condition-variable.md)：理解有界队列、关闭和排空；本例 Publisher 只是同步历史存储。
- [Benchmark 方法](../performance/benchmark-methodology.md)：分别测局部解码与完整下游可用路径，保留采样对象和环境条件。

## 分层面试题

题库从帧和字段开始，进一步检查序号、所有权与失败路径。工程题需要写清协议前提、失效如何交付给下游，以及示例没有实现的部分。
