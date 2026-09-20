---
{
  'schemaVersion': 1,
  'id': 'deterministic-replay-recovery',
  'title': '确定性回放与容灾：日志、输出和主备边界',
  'description': '用显式编码的有界日志重建状态，区分完整前缀、持久化与外部输出交付，并验证损坏拒绝、稳定输出身份和旧主隔离。',
  'category': 'design',
  'areas': ['System Design', 'Algorithm Coding'],
  'tags': ['deterministic-replay', 'recovery', 'write-ahead-log', 'outbox', 'fencing'],
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
  'prerequisites': ['状态机和事件序列', '快照水位', '消息去重与未知结果'],
  'related':
    [
      'snapshot-recovery',
      'oms-pre-trade-risk',
      'connection-lifecycle',
      'tail-latency-backpressure',
    ],
  'demo':
    {
      'file': 'examples/deterministic-replay-recovery.cpp',
      'platform': 'portable',
      'exercise': '增加经过明确版本迁移的配置事件，令新旧规则在日志中有确定切换点；测试旧快照加跨版本后缀与从头回放一致，并让不支持迁移的 reader 保持原发布状态。',
    },
  'references':
    [
      {
        'title': 'Ongaro and Ousterhout: In Search of an Understandable Consensus Algorithm, extended version',
        'url': 'https://raft.github.io/raft.pdf',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux fsync(2)',
        'url': 'https://man7.org/linux/man-pages/man2/fsync.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'SQLite Write-Ahead Logging',
        'url': 'https://www.sqlite.org/wal.html',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'etcd v3.6 Disaster recovery',
        'url': 'https://etcd.io/docs/v3.6/op-guide/recovery/',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'deterministic-replay-recovery-q01',
        'level': 'L1',
        'prompt': '确定性回放保证什么，不保证什么？',
        'answer': '在相同初始状态、相同有序输入和相同执行规则下重建相同状态及输出意图。它不自动保证日志已经持久化、其他副本拥有同一已提交前缀，也不保证外部输出只执行一次。',
        'rubric': ['相同依赖与顺序', '状态和意图', '持久性与外部效果另论'],
        'source': { 'kind': 'derived', 'rationale': '根据复制状态机模型与示例系统边界推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q02',
        'level': 'L1',
        'prompt': '回放时为什么不能重新读取当前时间和随机数？',
        'answer': '它们可能与原执行不同，从而改变决策。应记录实际影响状态转移的时间、随机结果或足够稳定的生成规则及状态，还要固定配置与代码语义版本。本例直接记录 logical_time、draw 和 config。',
        'rubric': ['非确定性依赖', '记录实际输入', '固定版本'],
        'source': { 'kind': 'derived', 'rationale': '根据日志字段和纯整数转移函数推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q03',
        'level': 'L1',
        'prompt': '快照恢复为什么还需要一个明确的日志切点？',
        'answer': '需要知道快照覆盖到哪里、后续从哪里开始，以及哪些记录已经提交。仅拿到一个文件结尾不能证明该位置可用于恢复。本例 snapshot 水位和 target 都是外部可信输入，只校验提供的后缀能否连续到达 target。',
        'rubric': ['快照覆盖边界', '提交边界', '不从文件长度推导'],
        'source': { 'kind': 'derived', 'rationale': '根据 replay 输入契约推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q04',
        'level': 'L1',
        'prompt': '校验和通过能证明日志未被恶意修改吗？',
        'answer': '不能。本例 32 位 FNV 校验和用于发现测试中的意外损坏，存在碰撞，也可以被改写者重新计算。来源认证、防篡改和可信提交信息需要额外机制，不能由一个哈希字段自行证明。',
        'rubric': ['意外损坏范围', '碰撞与重算', '认证另需机制'],
        'source': { 'kind': 'derived', 'rationale': '根据编码校验和及重算后语义故障测试推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q05',
        'level': 'L1',
        'prompt': '输出发送后没有收到 Ack，为什么不能认为没有执行？',
        'answer': '接收方可能已经应用输出，只是确认丢失。发送方应保留未知状态和稳定输出身份，利用接收方去重或权威查询确认结果。本例模拟接收方已应用后 Ack 丢失，恢复重发被识别为重复。',
        'rubric': ['执行与确认分离', '稳定身份', '未知不能当未执行'],
        'source': { 'kind': 'derived', 'rationale': '根据 Sink 丢确认与重发测试推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q06',
        'level': 'L2',
        'prompt': '为什么快照不能只保存业务余额？',
        'answer': '还需保存水位、版本及重建所需依赖，涉及输出时还要保留未确认意图或可证明的输出检查点。本例快照复制 pending outbox，保证水位之前尚未确认的两个输出不会因恢复而丢失。',
        'rubric': ['元数据和依赖', '未决输出', '前缀输出也需恢复'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 snapshot+suffix 与全量 State 相等测试推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q07',
        'level': 'L2',
        'prompt': '遇到截断尾部，可以直接使用已解析的前缀吗？',
        'answer': '要按存储协议判断截断部分是否可能含已承诺数据，不能盲目截掉错误后宣布恢复成功。本例采用保守规则：输入有截断、损坏或达不到可信 target 就整次失败，旧发布值不变；没有实现尾部修复。',
        'rubric': ['已承诺边界', '不静默截断', '本例失败原子性'],
        'source': { 'kind': 'derived', 'rationale': '根据全部 160 个截断长度测试推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q08',
        'level': 'L2',
        'prompt': '日志中重复序号怎样避免重复应用？',
        'answer': '本次回放已见同号且完整 frame 相同才允许跳过；同号不同内容拒绝。快照之前的原始 frame 不在本次证据中，所以覆盖快照的旧 frame 也拒绝，而不是仅凭序号较小就当成已认证重复。',
        'rubric': ['本轮完整内容比对', '同号冲突', '快照覆盖身份边界'],
        'source': { 'kind': 'derived', 'rationale': '根据 seen 数组、重复与未知重叠故障推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q09',
        'level': 'L2',
        'prompt': '为什么输出身份不能直接用新主 epoch？',
        'answer': '同一个业务输出在切主后仍应保持同一去重身份。若更换 epoch 同时生成新输出 ID，接收方可能重复执行。本例用 stream+sequence 标识业务意图，用独立 epoch 判断发送者是否有权限。',
        'rubric': ['业务身份稳定', '授权代次独立', '切主重试'],
        'source': { 'kind': 'derived', 'rationale': '根据 Sink 双重身份和切主测试推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q10',
        'level': 'L2',
        'prompt': '为什么新主知道 epoch 变大，还不足以阻止旧主？',
        'answer': '旧主可能尚未获知切换，仍向外部系统发送。接收方必须检查受信任控制面安装的当前 epoch，拒绝旧 epoch 的写入。本例 Sink 执行这一检查，但 activate 的授权和一致性由外部负责，没有实现选主。',
        'rubric': ['旧主仍可活动', '接收方强制隔离', '控制面前提'],
        'source': { 'kind': 'derived', 'rationale': '根据 epoch 安装与旧主发送拒绝测试推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q11',
        'level': 'L3',
        'prompt': '有 WAL 就能宣称断电后不丢已确认请求吗？',
        'answer': '还要定义何时确认、日志及元数据的同步顺序、fsync 错误如何处理和存储故障范围。SQLite WAL 的同步配置会影响断电后的持久性，Linux 文件 fsync 也不自动同步目录项。本例只有内存数组，不能据此给出任何断电保证。',
        'rubric': ['确认与同步顺序', '错误与存储边界', '模型非持久化'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 fsync 手册、SQLite WAL 与模型范围推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q12',
        'level': 'L3',
        'prompt': '如何避免升级程序后回放得到不同状态？',
        'answer': '固定输入格式、状态 schema、配置和转移规则版本，保留可执行的旧语义或设计可验证迁移。迁移应有明确水位，使用独立历史样本比较旧前缀加新后缀与预期结果；未知版本必须拒绝，不能按当前默认配置猜。',
        'rubric': ['多层版本', '迁移切点和验证', '拒绝未知'],
        'source': { 'kind': 'derived', 'rationale': '根据 frame、snapshot、config 版本检查推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q13',
        'level': 'L3',
        'prompt': '接收方去重后是否已经获得端到端 exactly-once？',
        'answer': '还要确保去重记录与真实副作用按一致的持久化边界更新，且恢复后不会忘记身份；重试窗口也必须在去重保留范围内。本例 Sink 的效果和去重都是内存状态，只验证一次进程存续期间的模型，未覆盖 Sink 崩溃。',
        'rubric': ['去重与副作用一致性', '持久和保留范围', '示例限制'],
        'source': { 'kind': 'derived', 'rationale': '根据未知交付和内存 Sink 的失败边界推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q14',
        'level': 'L3',
        'prompt': '备机回放追上水位后，是否可以马上承接业务？',
        'answer': '还需确认其前缀已提交、业务与输出状态完整、读者缓存需要的恢复边界，以及旧主已被有效隔离。回放到同一整数水位不自动证明来自同一日志历史。本例的 chain 只是非认证的局部检查，接管授权依赖外部协议。',
        'rubric': ['提交和状态完整', '旧主隔离', '水位不足以证明历史'],
        'source': { 'kind': 'derived', 'rationale': '根据日志来源、输出状态和主备控制职责推导。' },
        'companies': [],
      },
      {
        'id': 'deterministic-replay-recovery-q15',
        'level': 'L3',
        'prompt': '怎样验证容灾恢复时间和可接受的数据损失？',
        'answer': '先定义可恢复水位、确认边界及故障范围，再记录快照加载、日志校验、重放、对账、隔离和切换各阶段耗时。用断网、旧主继续运行、日志尾损坏和输出 Ack 丢失等演练检查结果，不用单次内存回放速度代替恢复目标。',
        'rubric': ['边界与故障范围', '阶段指标', '系统级故障演练'],
        'source': { 'kind': 'derived', 'rationale': '根据模型验证范围向实际恢复验收推导。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

确定性回放用相同状态、输入顺序和规则版本重新计算业务状态。要把影响决策的时间、随机结果和配置依赖变成可恢复输入，并从可信快照接上完整日志前缀。它只解决重建的一部分：日志是否持久、输出是否已经执行、切主后旧主是否还能写入，都需要各自的协议。本例用有界内存模型检验这些边界，不把一次成功回放当成真实容灾完成。

## 核心概念

| 问题               | 需要的证据                                     |
| ------------------ | ---------------------------------------------- |
| 状态能否重建       | 初始状态、完整有序输入、固定转移规则           |
| 已确认数据能否保留 | 明确的提交与持久化边界、故障假设和同步错误处理 |
| 快照能否接日志     | 同一流身份、水位、格式版本和连续后缀           |
| 输出是否已交付     | 稳定业务身份、接收方记录或权威查询             |
| 旧主能否继续写     | 由接收方执行的权限代次检查或其他有效隔离       |

复制状态机让副本执行相同有序命令，以得到相同状态和输出；保持各副本日志一致，是另一层协议的责任。Raft 原论文也分别讨论复制日志、状态机以及客户端重试时的命令去重。本章只借用这些职责划分，不实现 Raft。[Raft extended version，第 2、8 节](https://raft.github.io/raft.pdf)

## 原理深入

转移函数可以写成 `next = apply(state, input, rules_version)`。如果 apply 内部临时读取墙钟、调用随机设备或读取正在更新的配置文件，相同日志就可能得到不同结果。调度顺序、超时判定和外部查询结果若影响业务，也应通过明确事件或稳定依赖进入回放范围。仅记录“原始网络包”不一定包含这些因素。

示例规则固定为 config=1：每条事件增加 `amount*2 + draw + logical_time%5`。logical_time 和 draw 都直接来自日志，回放不重新取时间或生成随机数；所有运算使用定义清楚的整数类型。格式版本、快照 schema 和配置版本分别校验，未知版本不按当前默认规则猜测。

快照水位为 W 时，后缀必须从 W+1 连续推进到外部给定的 target。快照还保存前一条校验值和 pending 输出意图，防止只恢复余额而丢掉快照之前尚未确认的输出。target 是调用方已经确认可用的提交边界，本程序不从字节长度、最大序号或校验和自行认定提交。

输出也有自己的失败窗口：接收方应用成功后，Ack 可能丢失。回放只能再次生成相同意图，不能知道外部到底执行过没有。需要稳定输出身份配合接收方去重或查询；去重记录与真实副作用如何共同持久化，也必须另行定义。程序中的 Sink 只保存内存状态，演示同一次存续期间的重复抑制。

## 数据结构/系统内部实现

每个 frame 固定 40 字节，包含十个大端 32 位字：magic、格式版本、stream、sequence、logical_time、draw、amount、config、previous_checksum 和自身校验和。编码逐字段写入，不复制 C++ struct 的对象表示，因此不依赖 padding 或宿主字节序。读取前先检查总长度和整帧边界，再访问字段。

校验和是前 36 字节上的 FNV-1a 32 位计算，其中无符号回绕是算法的一部分。previous_checksum 将当前 frame 与前一条的编码关联起来，帮助发现本例中的错误拼接。它不是认证链：校验值存在碰撞，修改者也可同时重新计算整条链，快照身份和可信来源仍须外部验证。

replay 复制快照为 candidate，对后缀逐条检查格式、校验、stream 和 config。它要求逻辑时间非递减、draw 在 0..9、amount 为正，并验证序号连续及余额相加不溢出。完整 frame 的重复只在本轮 seen 数组中比对；同号冲突拒绝，快照已覆盖但无法比对原 frame 的旧输入也拒绝。该严格策略不会静默吞掉未知历史。

最终水位不等于 target，或任何一步失败，都不替换 published。成功时由单 owner 把完整 State 赋给 published；这里没有并发读者，不把普通赋值称为跨线程原子发布。State 包含 schema、流身份、水位、依赖状态、余额和待输出数组。可信快照的前提包含余额、水位、校验值与 outbox 彼此一致，并对应同一个已提交前缀；本例只做有限结构检查，未编码、认证或证明快照文件真实有效。

Sink 的业务去重键是 `(stream, sequence)`，payload 是该事件产生的增量。发送授权另用 epoch：可信控制面先调用 activate 安装更大 epoch，随后 Sink 只接受当前 epoch 的交付。初始 epoch=0 没有授权，旧 epoch 被拒绝，即使对应业务输出此前未见。activate 没有选主、租约或权限认证，不能让任意发送方用一个更大的数字自封新主。

## C++ runnable demo

```cpp include=examples/deterministic-replay-recovery.cpp

```

程序只操作内存，没有文件写入、fsync、网络和真实切主。要求 C++20，验证时启用断言，不定义 `NDEBUG`。

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  examples/deterministic-replay-recovery.cpp -o deterministic-replay-recovery
./deterministic-replay-recovery

g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  examples/deterministic-replay-recovery.cpp -o deterministic-replay-recovery-san
./deterministic-replay-recovery-san
```

四条输入的独立手算如下，初始余额为零：

| sequence | logical_time | draw | amount | 增量 | 累计余额 |
| -------- | ------------ | ---- | ------ | ---- | -------- |
| 1        | 10           | 1    | 3      | 7    | 7        |
| 2        | 12           | 2    | 4      | 12   | 19       |
| 3        | 15           | 0    | 1      | 2    | 21       |
| 4        | 18           | 3    | 2      | 10   | 31       |

程序把从头回放得到的 State，与水位 2 的快照加后两条日志得到的 State 作整体比较。二者余额为 31、pending 输出为四条，前两条意图也在恢复后保留。接收方先应用第一条，模拟发送方丢失 Ack；安装 epoch=2 后再次交付同一意图得到 duplicate，旧 epoch=1 的下一条被 fenced，其余三条由新 epoch 应用，最终四次效果合计 31。

| 故障或边界 | 实际检查                                                                                                |
| ---------- | ------------------------------------------------------------------------------------------------------- |
| 截断       | 对 160 字节日志取长度 0..159 的每个前缀；整帧边界因达不到 target=4 失败，其余因不足整帧失败，发布值不变 |
| 意外损坏   | 对 160 个字节各翻转最低位，逐次拒绝且发布值不变；没有枚举每个字节的全部八位，也不覆盖所有组合或碰撞     |
| 语义损坏   | 修改版本、stream、时间、draw、config 或前一校验值后重算校验，仍由语义检查拒绝                           |
| 顺序与身份 | 中间 gap、乱序起点、同号不同内容、无法比对的快照覆盖输入均拒绝                                          |
| 提交边界   | 日志到 4 而 target 为 3 或 5 都失败，没有偷偷忽略多余记录或虚构缺失记录                                 |
| 容量和算术 | 日志最多 8 frame，pending 最多 8；过长输入、满 outbox、余额溢出都不发布                                 |
| 序号末端   | 最大序号可从最大减一推进并完成；回到低序号的后续输入拒绝，不按整数回绕接流                              |
| 交付权限   | 未安装 epoch、旧 epoch、非法意图和同号不同 payload 均拒绝                                               |

正常构建及 ASan/UBSan 运行通过。示例返回失败时保留预置发布值 999，证明没有把局部候选泄漏出去；这不是磁盘事务回滚测试。Sink 没有持久化和重启试验，稳定 ID 的效果也不超出它仍记得已应用身份的范围。pending 没有确认后删除流程，到达容量 8 就停止接受新的输出意图；它只演示恢复所需状态，不是完整运行中的 outbox 系统。

## 高频追问

### 发现坏尾部时为何不扫描到下一个 magic 继续？

跳过未知字节可能跳过已承诺事件，也可能把载荷中的偶然字节当作新记录。是否可以丢弃未提交尾部，要由存储格式、提交标记和可信恢复边界决定。本例宁可整次拒绝，也不实施猜测性修复。生产恢复工具应区分未提交尾部、已提交段损坏和版本不兼容，并保留诊断证据。

### 写入 WAL 后立即向客户端确认可以吗？

先定义写入完成指的是复制到用户缓冲、进入内核，还是达到约定的持久性边界。Linux fsync 等待设备报告同步完成，并可能返回错误；文件 fsync 不自动保证目录项已同步。仅有正确记录格式不能替代文件、目录与确认顺序的设计。[fsync(2)](https://man7.org/linux/man-pages/man2/fsync.2.html)

SQLite 的 WAL 文档也区分同步配置与断电持久性，并说明 checkpoint 会改变同步和性能行为。因此“采用 WAL”只是存储方案描述，不能独立推出每次已确认事务的断电保证。[SQLite WAL](https://www.sqlite.org/wal.html)

### 从快照恢复后，旧客户端缓存还能直接继续吗？

不一定。恢复可能把业务版本带回旧位置，客户端此前见过的水位和 watch 状态需要协调。etcd v3.6 文档将恢复后的 revision 差异、watch 缓存失效和新逻辑集群身份分别说明，体现出恢复数据库之外还存在使用者状态与集群身份问题；本例没有实现这些 etcd 操作。[etcd 恢复文档](https://etcd.io/docs/v3.6/op-guide/recovery/)

## 容易答错的点

- 按相同输入与规则重建出预期状态，仍不证明日志已在故障前落盘。
- 校验和验证编码的一部分性质，不能鉴别发布者，也不能决定哪些事件已提交。
- 同一个序号可以属于不同 stream，切主 epoch 与业务输出 ID 也承担不同职责。
- outbox 保存意图，不自动证明外部执行情况；发送后无 Ack 仍可能已经执行。
- 内存中去重成功不等于接收方重启后仍能去重，历史回收还受重试窗口约束。
- 新主完成恢复不代表旧主已停止，接收端权限检查必须覆盖真实副作用路径。

## 性能分析

设输入字节数为 L、frame 数为 R、快照状态大小为 S。本例逐字节解码与校验为 `O(L)`，重复查找最坏 `O(R²)`，复制与发布为 `O(S)`；seen 和 pending 数组都有固定上限 8。vector 只负责构造测试日志，回放入口先拒绝超过 320 字节的输入，未将测试容器当成无限生产日志。

更大的恢复系统需分开测量读取、校验、反序列化、状态转移和输出对账耗时。快照频率改变后缀长度、写入负载和恢复时间，历史版本兼容也可能限制日志清理。不要把内存中 4 条记录的运行速度换算成磁盘恢复吞吐。

恢复验收还应记录可恢复水位、此前确认水位、数据损失边界、恢复耗时及切换期间服务行为。故障演练覆盖断电或写回错误时，需要真实存储测试；覆盖双主时，需要让旧主实际继续尝试副作用，并验证目标端拒绝。本文只提供相应模型的有界功能测试。

## Quant/Low-Latency 场景

策略回放可能依赖定时触发、随机拆单、参数版本和外部风控结果。若只录行情而不录这些输入，回放差异未必是策略代码错误。应明确回放目标是复现当时决策，还是用新规则重算历史；两者使用的规则版本和输出权限不同。

交易网关切主时，内部订单状态可从日志重建，但已发送未确认的订单仍是未知交付。稳定业务身份、对账和目标端允许的去重规则共同决定可否重发。切主后的 epoch 用于隔离旧发送者，不能顺便更换原业务身份而让同一意图再次执行。

灾备演练应覆盖进程重启、机房不可达、旧主仍存活和输出确认丢失等不同故障。恢复到一个完整历史前缀后，还要处理行情新鲜度、订单未知状态和下游缓存，才能按业务条件恢复服务。单次候选状态校验通过不承担这些外部决策。

## 相关专题

- [快照与增量恢复](../trading/snapshot-recovery.md)：定义基线、连续前缀和恢复中的有效性。
- [OMS 与交易前风控](../trading/oms-pre-trade-risk.md)：保留发送后未知订单及其风险占用。
- [连接生命周期](../network/connection-lifecycle.md)：区分连接关闭、本地发送和业务完成。
- [尾延迟与背压](../performance/tail-latency-backpressure.md)：给回放积压、对账和恢复流量设置预算。

## 分层面试题

L1 分清重建、持久化与交付；L2 跟踪前缀、版本和输出身份；L3 设计升级、故障演练及主备隔离。回答容灾方案时说明每个阶段依赖什么证据，以及证据不足时保留哪种未知状态。
