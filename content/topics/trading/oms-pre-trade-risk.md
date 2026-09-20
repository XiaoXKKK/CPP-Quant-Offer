---
{
  'schemaVersion': 1,
  'id': 'oms-pre-trade-risk',
  'title': 'OMS 与交易前风控：预留、未知状态和取消竞争',
  'description': '用虚构的单 owner 买单状态机核对风险预留、累计成交、发送失败与取消竞争，避免超时释放未决额度或重复发送。',
  'category': 'trading',
  'areas': ['Trading System', 'Order Book'],
  'tags': ['oms', 'pre-trade-risk', 'reservation', 'order-state', 'idempotency'],
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
  'prerequisites': ['订单、成交与取消', '单写者状态机', '整数溢出和消息身份'],
  'related':
    ['order-book', 'connection-lifecycle', 'snapshot-recovery', 'tail-latency-backpressure'],
  'demo':
    {
      'file': 'examples/oms-pre-trade-risk.cpp',
      'platform': 'portable',
      'exercise': '为 unknown 订单增加显式对账事件，要求对账身份、权威累计成交和终结状态均已验证；测试旧会话对账、重复对账、非终结对账与冲突结果，保持未知期间的风险预留。',
    },
  'references':
    [
      {
        'title': 'FIX Application Layer Order State Changes, EP258, July 2020',
        'url': 'https://www.fixtrading.org/wp-content/uploads/download-manager-files/Order-State-Changes.pdf',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'FIX 4.4 ExecutionReport message dictionary',
        'url': 'https://fiximate.fixtrading.org/legacy/en/FIX.4.4/body_5756.html',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'oms-pre-trade-risk-q01',
        'level': 'L1',
        'prompt': 'OMS 和撮合引擎分别维护什么？',
        'answer': 'OMS 跟踪本方订单意图、发送进度、报告和风险占用；撮合引擎按市场规则决定订单匹配与成交。OMS 收到本地发送成功并不能自行认定对端已接受或成交，需要处理相应业务报告。',
        'rubric': ['本方状态与风险', '撮合决定成交', '发送不是业务确认'],
        'source': { 'kind': 'derived', 'rationale': '根据本章系统边界与发送状态推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q02',
        'level': 'L1',
        'prompt': '为什么交易前检查需要预留额度？',
        'answer': '检查与预留若分开，多个订单可能同时通过同一份剩余额度。本例 owner 在 prepare 中一起完成校验、预留和订单登记，后续订单看到已经扣减的可用量；这只提供进程内串行性，不是持久化事务。',
        'rubric': ['重复使用额度风险', '检查预留同一决策', '持久化范围'],
        'source': { 'kind': 'derived', 'rationale': '根据 prepare 与全局风险不变量推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q03',
        'level': 'L1',
        'prompt': 'Pending Cancel 是否可以释放全部剩余额度？',
        'answer': '不可以。它说明取消请求正在处理，原订单仍可能成交。本例 request_cancel 不改变预留，直到权威取消终结或全部成交才清除剩余预留；取消拒绝会保留未成交部分的风险。',
        'rubric': ['取消未确认', '仍可能成交', '按终结结果释放'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 FIX Pending Cancel 语义与本例取消竞争推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q04',
        'level': 'L1',
        'prompt': '发送后的响应超时应标成拒绝还是未知？',
        'answer': '未知。对端可能已接收甚至成交，只是回报尚未到达。本例保留未决数量对应的预留，禁止再次 dispatch 同一订单；普通 Ack 或部分成交不会自动清除 unknown，仍需权威终结或外部对账。',
        'rubric': ['超时无法证明未执行', '保留风险', '不盲目重发'],
        'source': { 'kind': 'derived', 'rationale': '根据 uncertain 路径及迟到报告测试推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q05',
        'level': 'L1',
        'prompt': 'spent 与 reserved 在本例中分别是什么？',
        'answer': 'spent 是实例存续期间累计买入成交金额，reserved 是仍可能成交数量乘限价的保守预留。两者之和不得超过教学额度。spent 不等于净持仓、亏损或可用资金，本例没有卖出抵消、费用、币种转换和保证金。',
        'rubric': ['累计成交金额', '未决最坏预留', '指标省略范围'],
        'source': { 'kind': 'derived', 'rationale': '根据买单教学规则和风险公式推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q06',
        'level': 'L2',
        'prompt': '部分成交价格优于限价时，风险账本怎样变化？',
        'answer': '将该部分原先按限价预留的金额移出 reserved，把实际成交金额加入 spent；两者之差成为可用额度。未决数量继续按限价预留。对增量数量 d、增量成本 c，本例要求 c 不大于 d 乘限价，因而总占用不会因报告处理而上升。',
        'rubric': ['预留转实际成本', '余量继续预留', '不等式说明'],
        'source': { 'kind': 'derived', 'rationale': '根据报告候选计算与额度守恒推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q07',
        'level': 'L2',
        'prompt': 'Fill 先于 Ack 到达时如何避免状态回退？',
        'answer': '先处理权威成交累计量并更新风险。较旧报告按本协议的每订单单调 report_id 识别为 stale，较新的 Ack 也不能覆盖 pending_cancel 或 unknown，更不能把已 Filled 的订单改回 Live。真实 FIX ExecID 不保证这种数字顺序。',
        'rubric': ['累计量先记账', '状态不回退', '协议限定'],
        'source': { 'kind': 'derived', 'rationale': '根据先成交后确认及旧报告测试推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q08',
        'level': 'L2',
        'prompt': '取消确认包含比本地更高的累计成交量，应先做什么？',
        'answer': '先验证并计入新增成交数量和成本，再释放真正未成交的剩余预留。本例的权威累计报告允许补上未单独收到的成交；较旧成交报告随后到达不会重复记账。这依赖累计报告覆盖更早状态的虚构协议条件。',
        'rubric': ['先补成交', '再释放剩余', '累计覆盖条件'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据 canceled 报告从累计 5 更新至 6 的手算轨迹推导。',
          },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q09',
        'level': 'L2',
        'prompt': '重复报告、同号冲突和旧报告分别怎么处理？',
        'answer': '本例按 session、订单 ID 和报告 ID 定位身份。历史中完全相同的报告返回 duplicate；同号不同内容停止新业务并保留账本；未缓存但编号小于该订单已处理编号的报告返回 stale。后者只适用于本例权威累计、每订单单调编号且无成交更正的条件。',
        'rubric': ['重复身份', '冲突不静默', '旧报告判断前提'],
        'source': { 'kind': 'derived', 'rationale': '根据固定去重历史和累计报告契约推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q10',
        'level': 'L2',
        'prompt': '为什么未知枚举值也要在状态转换前检查？',
        'answer': '从网络整数转换成枚举不保证值属于列出的枚举项。若只检查几个特殊 Kind，其他值可能落入默认处理路径并推进成交或释放额度。本例 switch 显式接受五种 Kind，其余返回 invalid，保持金额不变并进入停止新业务状态。',
        'rubric': ['枚举转换不验证取值', '错误默认路径', '先验证再记账'],
        'source': { 'kind': 'derived', 'rationale': '根据未知 Kind 输入及失败状态断言推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q11',
        'level': 'L3',
        'prompt': '多线程或多进程如何共享同一个风险限额？',
        'answer': '单 owner 可把检查和预留串行化；分片设计必须给每片可证明的额度预算，或使用能原子协调准入的共享服务。不能让各片独立读取同一旧余额后放行。还需考虑 owner 不可用、消息重复和恢复期间的预留重建。',
        'rubric': ['原子准入', '分片额度总和', '故障恢复'],
        'source': { 'kind': 'derived', 'rationale': '根据进程内单 owner 前提向共享预算场景推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q12',
        'level': 'L3',
        'prompt': '进程在订单可能发出之后崩溃，重启时如何处理额度？',
        'answer': '应从持久化意图、发送阶段和权威对账重建未决订单，不能因为内存订单表为空就认定预留为零。落盘顺序、业务身份和重放去重必须共同设计；仍未确认的订单保持保守风险并限制新业务。本例没有持久化或重启恢复实现。',
        'rubric': ['重建未决风险', '持久化顺序与身份', '未知不释放'],
        'source': { 'kind': 'derived', 'rationale': '根据 dispatch 前置状态和 unknown 边界推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q13',
        'level': 'L3',
        'prompt': '去重表已满，是否可以丢掉最旧 ID 继续处理？',
        'answer': '只有在协议的重放窗口和业务生命周期允许时才可回收。否则旧报告回来可能再次计入成交。本例不驱逐历史，容量满后停止新业务并保留未决预留，交给外部对账处理；生产实现需定义保留边界、持久化及恢复策略。',
        'rubric': ['旧副本风险', '回收条件', '容量失败保持风险'],
        'source': { 'kind': 'derived', 'rationale': '根据 report_capacity 耗尽测试推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q14',
        'level': 'L3',
        'prompt': '把本例接到 FIX 网关还缺哪些层？',
        'answer': '需要会话管理、消息与字段校验、真实订单链身份、重传去重以及具体场所的状态规则。FIX 的 ExecType 和 OrdStatus 表达事件原因与当前状态，ExecID 也不是本例可比较大小的 report_id。成交更正、撤销和替换还会打破本例累计量单调假设，不能只做字段改名。',
        'rubric': ['会话和协议适配', '字段语义不同', '更正替换省略'],
        'source': { 'kind': 'derived', 'rationale': '根据 FIX 一手资料与虚构模型范围对照推导。' },
        'companies': [],
      },
      {
        'id': 'oms-pre-trade-risk-q15',
        'level': 'L3',
        'prompt': '风控检查很快，为什么仍要测准入之外的延迟？',
        'answer': '订单可能在发送队列、事件循环或外部确认阶段等待，取消也会与成交竞争。应同时测准入、发送等待、报告处理和 unknown 持续时间，以及预算拒绝和容量失败。只测一次整数比较无法说明端到端风险暴露或取消响应能力。',
        'rubric': ['完整链路测量', '未知与取消窗口', '失败口径'],
        'source': { 'kind': 'derived', 'rationale': '根据状态机阶段和工程指标推导。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

OMS 需要同时记录订单意图、外部确认和风险占用。准入通过时先预留最坏情况下的额度，再把订单交给发送路径；成交后将相应预留转为实际占用，只有确定未执行的部分才能释放。取消请求和超时都不等于订单消失，发送后失败应保留 unknown 及未决风险，通过可去重的报告或对账收敛状态。本文用虚构工程规则验证这一过程，不实现完整交易所或 FIX 协议。

## 核心概念

示例只有买入限价单，数量、价格和金额都是无符号整数单位。外部协议假定每笔成交的单位成本为正且不超过订单限价，没有手续费、卖出、汇率、成交撤销或更正。代码只拿到累计数量与成本，能校验新增汇总成本不超过新增数量乘限价，不能排除其中高低成交价相互抵消；逐笔限价验证需要逐笔明细，本文的聚合检查不能替代它。额度 `limit` 限制本实例的累计买入成交金额与未决预留总和；它不是保证金、净仓位或监管指标。

| 字段               | 本章含义                                                   |
| ------------------ | ---------------------------------------------------------- |
| quantity / filled  | 原始数量 / 已确认累计成交数量                              |
| cost / spent       | 单订单 / 全实例累计成交金额                                |
| reserved           | 仍可能执行的数量按限价计算的预留                           |
| prepared           | 已登记和预留，但尚未交给发送路径                           |
| pending_new / live | 可能已发送，尚待确认 / 已有有效业务报告                    |
| pending_cancel     | 取消请求已提出，原订单仍可能执行                           |
| unknown            | 发送或等待结果不确定，不能据此释放余量                     |
| halted             | 停止新准入、dispatch 和 request_cancel，继续接收可验证报告 |

本地状态名称不逐项等同于 FIX OrdStatus。FIX 的 ExecType 描述为何发送报告，OrdStatus 描述订单当前状态，Pending Cancel 也明确不表示已经取消；状态优先级还可能让一个报告同时反映成交和待取消的事实。[FIX Order State Changes，EP258](https://www.fixtrading.org/wp-content/uploads/download-manager-files/Order-State-Changes.pdf)

## 原理深入

对原始数量 Q、累计成交 F、限价 P，未终结订单预留为 `(Q-F)P`。准入时先检查乘法不会溢出，再确认需要的金额不超过 `limit-spent-reserved`，然后在同一 owner 操作中登记订单并增加 reserved。若多个线程分别检查同一旧余额再各自发送，就可能把一份额度批准多次。

成交新增数量 d、成本 c 后，预留减少 dP，spent 增加 c。本模型要求 `0<c<=dP`，所以非终结报告使总占用变化为 `c-dP<=0`；价格改善释放的是先前多预留的部分。取消终结时先计入报告中尚未记账的成交，再清除余量预留。已发生成交金额不会因取消而归零。

发送边界影响失败处理。`fail_before_send` 只允许 prepared 状态，表示调用方能证明尚未把任何字节交给外部发送路径。`dispatch` 先把订单标记为 pending_new，再由外部传输层尝试发送。此后部分写、连接中断或响应超时都通过 `uncertain` 标成 unknown，余量预留保持。系统不能从“我没有收到 Ack”推导“对端没有收到订单”。

数量也需要分清用途。非终结订单有 `Q=F+仍可能执行数量`；取消、拒绝或本地失败后，有 `Q=F+确定不再执行数量`。终结后的预留为零并不表示 Q 全部成交。本例取消轨迹最终 Q=10、F=6，其余 4 个单位被取消，累计成本仍为 54。

## 数据结构/系统内部实现

`Oms` 由一个线程拥有，全部方法包括 read 都要求串行调用；它没有原子字段或并发读保证。订单表固定 4 项，不回收终结订单，因此实例存续期间不复用订单 ID。报告历史固定 16 项，按订单 ID 和报告 ID 找重复，整个实例固定一个非零 session；构造时该前提由断言检查，验证程序必须启用断言。

教学报告包含 session、订单 ID、报告 ID、事件类型，以及权威累计数量和成本。报告 ID 在每笔订单内单调增长，后面的累计值完整覆盖前面已发生的成交；没有成交更正或撤销。它不要求相邻 ID 连续，因为本例依赖累计覆盖。网络格式解码、身份认证、累计值是否确实权威，均是外部适配层责任。

`on_report` 先排除外来 session 和未知订单，显式检查非零 ID 与五种 Kind，然后核对历史重复。完全相同返回 duplicate，同号内容冲突进入 halted；未缓存但比已处理 ID 更小的报告返回 stale。后者依赖上述单调、完整累计协议，不能一般化为“旧编号一律可丢”。真实 FIX ExecID 是执行报告标识，本身并不提供本例的每订单整数排序。[FIX 4.4 ExecutionReport 字段](https://fiximate.fixtrading.org/legacy/en/FIX.4.4/body_5756.html)

通过身份检查后，函数在局部 Order 副本上计算状态和风险。累计数量不能倒退或超过原始数量，累计成本不能倒退；新增成本受新增数量与限价约束。缺乏对应取消请求的取消结果、无新增成交的 trade、未知 Kind 等均拒绝。候选和全局额度检查通过后才一起写回，最后把报告加入去重表，避免先改数量再发现预算错误。

状态转换保留尚未解决的意图。Fill 早于 Ack 时可以先记账并转 live；pending_cancel 中的部分 Fill 不解除取消意图；已 Filled 的订单收到后续取消拒绝也不会重新变 live。unknown 收到普通 Ack 或部分 Fill 后仍是 unknown，累计成交可以减少余量预留，但不能据此认定剩余状态已经明确。权威取消、拒绝或全部成交可以终结订单。

`halted` 不是丢弃所有输入：它阻止 prepare、dispatch 和 request_cancel，on_report 仍尝试验证已有订单的可信报告，以便更新风险。报告历史已满时无法记录新的去重身份，因而不能应用新的报告，只保留现有账本并等待外部对账。本例没有解除 halted、持久化重建或重启恢复接口。

阻止 request_cancel 是本例冻结新命令的简化选择，不是生产系统必须禁止风险降低操作的规则。实际系统可设置独立受控的安全取消或 kill 路径，明确权限、幂等和报告处理。发生协议冲突后，保留 reserved 只表示没有凭空释放本地未决预算，不能证明本地账本已经与真实执行结果一致；仍需外部查询和对账。

## C++ runnable demo

```cpp include=examples/oms-pre-trade-risk.cpp

```

使用 C++20，测试数据有界，无线程、网络和真实资金操作。断言中包含状态机调用，验证时不能定义 `NDEBUG`。

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  examples/oms-pre-trade-risk.cpp -o oms-pre-trade-risk
./oms-pre-trade-risk

g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  examples/oms-pre-trade-risk.cpp -o oms-pre-trade-risk-san
./oms-pre-trade-risk-san
```

第一条测试轨迹的独立手算如下。订单数量 10、限价 10，实际成交单位成本为 9：

| 事件                           | 累计成交 | spent | reserved | 状态           |
| ------------------------------ | -------- | ----- | -------- | -------------- |
| prepare                        | 0        | 0     | 100      | prepared       |
| dispatch 后收到累计成交 3      | 3        | 27    | 70       | live           |
| 重复 Fill、迟到旧 Ack          | 3        | 27    | 70       | live           |
| request_cancel 后又成交 2      | 5        | 45    | 50       | pending_cancel |
| 响应超时                       | 5        | 45    | 50       | unknown        |
| 权威取消报告，累计成交已到 6   | 6        | 54    | 0        | canceled       |
| 更早成交报告晚到、取消报告重复 | 6        | 54    | 0        | canceled       |

取消报告里的第六个成交没有先作为独立 Fill 被处理，累计值让本例先补记 9 个成本单位，再释放未成交四个单位的预留。它不意味着所有真实取消消息都携带足够的补账信息；缺少这种协议保证时，需要等待完整成交报告或对账。

| 其他验证         | 断言覆盖                                                                    |
| ---------------- | --------------------------------------------------------------------------- |
| 发送前失败与拒绝 | prepared 本地失败释放预留；权威零成交拒绝释放预留；已发送不能走发送前失败   |
| 取消竞争         | Ack 前提出取消、全部成交先到而取消拒绝后到；部分成交伴随取消拒绝恢复 live   |
| unknown          | 普通 Ack 和部分 Fill 后仍 unknown，全部成交后终结；再次 dispatch 被拒绝     |
| 身份与去重       | 外来 session 不改账本，相同报告不重复记账，同号冲突 halted 且金额不变       |
| 数量与价格       | 超原始数量、超限价成本、空 trade、无取消请求的取消确认、未知 Kind=99 均拒绝 |
| 容量             | 四个终结订单仍占表，第五个拒绝；第十七条新报告不应用并保留风险              |
| 整数             | 数量乘价格溢出提前拒绝；最大整数额度下可完整预留、成交并清除余量，未回绕    |

正常严格 warning 构建与 ASan/UBSan 运行通过。未知 Kind=99 的独立故障断言确认 spent=0、reserved=50、halted=true 且订单 unknown，未让该载荷进入普通报告路径。测试没有模拟持久化失败、真实对端违约、成交更正或费率变化，不证明这些场景下额度仍充分。

## 高频追问

### 本地 write 成功后，还能因连接断开而直接撤销订单吗？

write 成功最多说明相应字节被本地发送路径接受。对端是否已收到、业务是否接受以及是否已经成交，需要各自的确认。连接断开后，应保存订单身份和未决风险，进行查询、重放或其他协议支持的恢复。没有证明未发送时，不能调用本例的 fail_before_send。

### 取消请求被拒绝，是风控可以停止跟踪了吗？

取消拒绝可能意味着原订单仍活跃，也可能与已经全部成交的结果交叉。本例先更新累计值，再结合既有状态解释取消拒绝：未终结时回 live 并继续预留，已经 Filled 时保持终结。真实语义还要使用拒绝原因和场所状态，不应仅凭消息名称释放全部风险。

### 普通 Ack 为什么没有把 unknown 清掉？

本例选择保守语义：Ack 可能是迟到的早期确认，不能独自证明后续取消或连接故障之后的状态。它可以携带用于补账的权威累计数据，但剩余执行可能性仍保持。生产系统可以设计带查询身份、报告水位和权威状态的对账响应来解除未知，不能把任意旧 Ack 都当成一次新查询结果。

## 容易答错的点

- 风控放行、发送、对端接受和成交是不同阶段，不能用一个 success 布尔覆盖。
- Pending Cancel 仍需计算余量可能成交的风险，取消请求本身不释放预算。
- 向枚举强制转换一个整数不等于验证了消息类型，未知值必须有失败路径。
- 单 owner 防止本进程并发透支，不能替代跨进程额度协调或持久化事务。
- 终结订单不再预留余量，但已发生成交成本仍在 spent 中。
- 切换网络会话或重启程序不能抹去旧订单责任，未知风险要跨这些边界恢复。

## 性能分析

设订单表大小为 O、历史报告数为 H，查找订单为 `O(O)`，报告去重为 `O(H)`；启用断言时每次 verify 还会遍历订单表。存储为 `O(O+H)`。示例固定 O=4、H=16，用扫描便于审查，不声称能承载生产订单量。

更大的实现可以使用有界索引，但需要考虑 ID 冲突、回收和报告重放窗口。过早删除终结订单或去重身份，会让迟到报告变成未知对象或再次记账。按时间驱逐是否安全，需要协议的最大重放范围、持久化策略和对账机制共同支持。

测量应覆盖准入耗时、待发队列年龄、报告处理延迟、unknown 持续时间、取消请求到权威终结的分布，以及风险拒绝、表满和冲突次数。报告吞吐时同时列出未决订单与占用金额，防止以拒绝更多或跳过确认换取漂亮数字。本文没有性能计时数据。

## Quant/Low-Latency 场景

同一账户的多个策略共享额度时，需要决定谁拥有准入账本。集中 owner 易于维护全局不变量，但会带来队列和单点负载；分片可以降低局部争用，但分片额度总和、转移过程和失败回收必须明确。把余额广播给各策略后允许独立放行，会产生过期余额下的并发透支。

策略侧取消与网关侧成交回报交叉很常见。OMS 应保留订单链身份、取消意图和累计执行量，让策略看到真实的不确定性。不能因策略已不再想持有该订单，就在风险账本里假定它不可能成交。

进程恢复需要从持久化意图、发送阶段和权威报告重建风险。发送阶段日志与实际外部副作用之间可能存在崩溃窗口，系统需要业务身份和对账收敛，不能仅重置内存表再接单。本章的买单额度是教学工程约束，没有覆盖真实资产、结算、授信或合规要求。

## 相关专题

- [Order Book](order-book.md)：区分本方订单管理与市场簿、撮合结果。
- [连接生命周期](../network/connection-lifecycle.md)：处理发送后失败与业务结果未知。
- [快照与增量恢复](snapshot-recovery.md)：理解权威基线、身份及恢复期间的有效性。
- [尾延迟与背压](../performance/tail-latency-backpressure.md)：约束待发队列、报告积压和恢复压力。

## 分层面试题

L1 解释阶段和风险占用，L2 跟踪成交与取消的交叉顺序，L3 处理共享额度、容量与恢复。回答状态机题时同时写出数量、金额和可否再次发送；只画状态名称通常看不出重复扣减或提前释放的错误。
