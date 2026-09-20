---
{
  'schemaVersion': 1,
  'id': 'udp-multicast-sequencing',
  'title': 'UDP 组播与序列号：从数据报接收到缺口恢复',
  'description': '检查数据报边界与 MSG_TRUNC，区分网络丢包、乱序和重复，并用受会话约束的序列状态机说明何时停止应用增量、何时能够恢复。',
  'category': 'network',
  'areas': ['Network Programming', 'TCP/IP'],
  'tags': ['udp', 'multicast', 'sequence-number', 'packet-loss', 'recovery'],
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
  'estimatedMinutes': 60,
  'prerequisites': ['socket 与非阻塞 I/O', '网络字节序', '快照与增量状态'],
  'related': ['tcp-framing', 'epoll-lt-et', 'order-book', 'raii-exception-safety'],
  'demo':
    {
      'file': 'examples/udp-multicast-sequencing.cpp',
      'platform': 'linux',
      'exercise': '构造同一 session 中延迟整整 65536 个事件的旧报文，解释它为何能与当前 next 同值。保持已有长度和截断检查，提出限制旧包寿命或切换 session 的协议条件；不要把单次取模比较当作完整恢复方案。',
    },
  'references':
    [
      {
        'title': 'RFC 768: User Datagram Protocol',
        'url': 'https://www.rfc-editor.org/rfc/rfc768',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'RFC 8085: UDP Usage Guidelines',
        'url': 'https://www.rfc-editor.org/rfc/rfc8085',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'RFC 1112: Host Extensions for IP Multicasting',
        'url': 'https://www.rfc-editor.org/rfc/rfc1112.txt',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'RFC 3678: Socket Interface Extensions for Multicast Source Filters',
        'url': 'https://www.rfc-editor.org/rfc/rfc3678.txt',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'RFC 1982: Serial Number Arithmetic',
        'url': 'https://www.rfc-editor.org/rfc/rfc1982.txt',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux udp(7)',
        'url': 'https://man7.org/linux/man-pages/man7/udp.7.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux recv(2): recvmsg and MSG_TRUNC',
        'url': 'https://man7.org/linux/man-pages/man2/recvmsg.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux socketpair(2)',
        'url': 'https://man7.org/linux/man-pages/man2/socketpair.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux unix(7)',
        'url': 'https://man7.org/linux/man-pages/man7/unix.7.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux poll(2)',
        'url': 'https://man7.org/linux/man-pages/man2/poll.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux send(2)',
        'url': 'https://man7.org/linux/man-pages/man2/send.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux close(2)',
        'url': 'https://man7.org/linux/man-pages/man2/close.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux IP_ADD_MEMBERSHIP',
        'url': 'https://man7.org/linux/man-pages/man2/IP_ADD_MEMBERSHIP.2const.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux ip_mreqn: group and interface selection',
        'url': 'https://man7.org/linux/man-pages/man2/ip_mreqn.2type.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux IP_MULTICAST_IF',
        'url': 'https://man7.org/linux/man-pages/man2/IP_MULTICAST_IF.2const.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux IP_PKTINFO',
        'url': 'https://man7.org/linux/man-pages/man2/IP_PKTINFO.2const.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux IP_MULTICAST_ALL',
        'url': 'https://man7.org/linux/man-pages/man2/IP_MULTICAST_ALL.2const.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux socket(7): receive buffers and drop counters',
        'url': 'https://man7.org/linux/man-pages/man7/socket.7.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'udp-multicast-sequencing-q01',
        'level': 'L1',
        'prompt': 'UDP 保留数据报边界，是否就保证业务消息完整有序？',
        'answer': '它保留传输单元边界，但不保证交付、顺序或重复保护。一个数据报里可能有多条业务消息，也可能只是应用分片；接收缓冲太小还会截断。应用要按协议检查长度、计数和序列，不能把一次 recv 成功等同业务状态可以更新。',
        'rubric': ['边界与可靠性分离', '业务消息与报文并非一一对应', '长度和序列检查'],
        'source': { 'kind': 'derived', 'rationale': '由 UDP 数据报语义和应用消息封装方式推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q02',
        'level': 'L1',
        'prompt': 'recvmsg 返回正数时为什么仍要检查 MSG_TRUNC？',
        'answer': '返回的字节可能只是一个过大数据报的前缀。输出 msg_flags 中的 MSG_TRUNC 表示尾部因缓冲不足被丢弃，不能按完整报文解析，也不能期待下一次 recv 取回尾部。本例丢弃该报文并使单流接收状态失效，等待恢复。',
        'rubric': ['正数可能是截断前缀', '检查输出标志', '尾部不会在下一次补齐'],
        'source':
          { 'kind': 'derived', 'rationale': '由 Linux recvmsg 截断标志和数据报消费方式推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q03',
        'level': 'L1',
        'prompt': '数据报 socket 的 recv 返回 0，一定表示对端关闭吗？',
        'answer': '不一定，零长度数据报可以产生返回值 0。它与流式连接的 EOF 判断不同，应用应按自己的协议决定空报文是否允许。本例固定格式需要 12 字节，因此空报文作为格式错误处理，但并不称为 UDP 连接关闭。',
        'rubric': ['零长度数据报合法存在', '区分流式 EOF', '应用协议决定处理'],
        'source': { 'kind': 'derived', 'rationale': '由数据报接收返回值与固定帧长度约束推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q04',
        'level': 'L1',
        'prompt': '接收方只记录一个全局 next_sequence 有什么风险？',
        'answer': '不同频道、会话或独立发布源可能有各自的序列域。把它们混在一起会制造假缺口，或把合法更新当重复。应先确认协议定义的流标识，再为每个序列域维护期望值；同一证券出现在多个流中也不意味着这些序列可以直接比较。',
        'rubric': ['识别序列域', '避免跨域比较', '业务标识不替代流标识'],
        'source': { 'kind': 'derived', 'rationale': '由多个独立消息流的序列推进方式推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q05',
        'level': 'L1',
        'prompt': '发现期望 100 却收到 102，能否直接把 next 改为 103？',
        'answer': '若状态依赖连续增量，这会静默跳过 100 和 101。接收方应标记状态不完整，按协议缓冲、补发或重建，在恢复连续性后再发布有效状态。本例采取保守策略，冻结增量应用并等待受信任快照，next 不因缺口而前移。',
        'rubric': ['缺失增量影响状态', '恢复前标记失效', '不按最高到包号跳过缺口'],
        'source': { 'kind': 'derived', 'rationale': '由连续增量状态对完整前缀的依赖推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q06',
        'level': 'L1',
        'prompt': 'bind 到 UDP 端口是否等于在正确网卡加入组播组？',
        'answer': 'bind 确定本地 socket 地址和端口，组成员关系还要用加入组的接口设置，并明确网卡。多网卡环境依赖默认接口可能加入错误链路；发送接口选择又是另一项配置。加入成功也不证明交换网络、源和路由已能把数据送达。',
        'rubric': ['bind 与成员关系不同', '明确接收接口', '配置成功不保证流量到达'],
        'source':
          { 'kind': 'derived', 'rationale': '由组播成员关系与 socket 绑定及接口选择的分工推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q07',
        'level': 'L2',
        'prompt': '16 位序列号从 65535 到 0 后，为什么不能继续用普通小于比较？',
        'answer': '合法回绕会让新值数值更小。本例计算模 65536 的距离，只有协议保证相关事件距离处于半空间范围时，才能用它判断新旧；相距 32768 明确进入歧义。旧包若滞留整圈，与当前序列同值，单靠这 16 位无法识别。',
        'rubric': ['普通数值序不适合回绕', '半空间和寿命前提', '整圈同值无法判断'],
        'source': { 'kind': 'derived', 'rationale': '由有限序列空间的比较规则和旧报文寿命推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q08',
        'level': 'L2',
        'prompt': '新 session 的报文到了，为什么不能自动接受并重置状态？',
        'answer': '报文里的 session 值本身不能证明切换已被授权，也不能提供该会话的完整业务基线。旧会话延迟报文和重启后的新会话需要外层控制区分。本例只忽略不匹配会话，安装新基线由受信任调用者完成，旧状态的新鲜度仍需超时或控制消息管理。',
        'rubric': ['session 字段不自动授权', '需要完整基线', '新鲜度与会话控制在外层'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由会话身份、初始化状态和到包来源可信度的不同职责推导。',
          },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q09',
        'level': 'L2',
        'prompt': '发现乱序后等一段时间，与确认丢包之间有什么区别？',
        'answer': '未来报文先到只说明目前存在缺口，迟到报文仍可能补齐。等待窗口是延迟与内存预算的选择；超过期限或容量后才按恢复规则处理，仍不能证明网络中绝无迟到副本。恢复和实时流交汇时必须去重并保持连续应用。',
        'rubric': ['缺口不立即证明永久丢失', '窗口有时间和容量预算', '恢复交汇仍需去重排序'],
        'source': { 'kind': 'derived', 'rationale': '由乱序到达、超时判断和恢复流合并条件推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q10',
        'level': 'L2',
        'prompt': '一个 UDP 包带多条业务消息时，序号应怎样推进？',
        'answer': '必须依协议判断序号代表包、首条消息还是单条业务事件，再结合消息数量和控制报文规则推进。不能固定每收一个包就加一；零消息心跳也可能有不同约定。本例明确一报文一事件，因此才使用每次成功应用后加一的规则。',
        'rubric': ['确定序号计量单位', '结合计数和控制消息规则', '不泛化本例一包一事件'],
        'source': { 'kind': 'derived', 'rationale': '由数据报封装与应用序列计量单位的差异推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q11',
        'level': 'L2',
        'prompt': 'Linux recvmsg 的输入 MSG_TRUNC 与输出 MSG_TRUNC 相同吗？',
        'answer': '输入标志会在 Linux 支持的数据报类型上请求返回原始报文长度，即使实际缓冲更小；输出标志说明本次接收丢掉了报文尾部。若使用输入标志，返回长度可能大于已复制字节，不能直接据此构造可读 span。本例只检查输出位。',
        'rubric': ['区分调用参数与返回标志', '原长度可能大于复制量', '避免越界解析'],
        'source':
          { 'kind': 'derived', 'rationale': '由 Linux MSG_TRUNC 两种接口位置的不同语义推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q12',
        'level': 'L2',
        'prompt': '为什么 socketpair 数据报测试不能证明真实 UDP 组播链路可用？',
        'answer': 'AF_UNIX socketpair 在本机验证消息边界、截断和 fd 清理，没有使用 IP、网卡、组成员协议或组播路由。其可靠性和排序特征也不能套给 UDP。真实验收还要核对接口、组与源过滤、网络转发、丢包计数以及应用恢复链路。',
        'rubric': ['只覆盖本机数据报接口', '不经过真实组播路径', '另做环境与恢复验收'],
        'source':
          { 'kind': 'derived', 'rationale': '由 AF_UNIX 与 IP 组播协议栈的不同覆盖范围推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q13',
        'level': 'L3',
        'prompt': '快照下载完成后，怎样避免切回实时流时漏增量？',
        'answer': '先取得快照覆盖的精确序列边界，再保留或补取该边界之后的连续增量，丢弃已被快照覆盖的重复，确认完整衔接才恢复有效状态。若暂存溢出或缺口超出补发能力，应重新恢复。本例 install_baseline 仅接收已验证的结果，没有实现这一交接协议。',
        'rubric': ['快照覆盖边界', '连续增量衔接和去重', '溢出时重新恢复'],
        'source':
          { 'kind': 'derived', 'rationale': '由快照与实时增量同时推进时的完整前缀要求推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q14',
        'level': 'L3',
        'prompt': '主备两条组播线收到相同序号，就可以直接去重吗？',
        'answer': '先确认协议保证两条线属于同一逻辑序列域、同一会话且同号内容一致。若只是两个独立发布源，不能合并它们的序列。对可合并副本还需处理不同到达延迟、重复与内容冲突，并检查一路恢复时是否仍保留足够历史。',
        'rubric': ['证明同一逻辑流', '同号内容一致性', '处理延迟与冲突'],
        'source': { 'kind': 'derived', 'rationale': '由冗余线路副本关系和去重身份条件推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q15',
        'level': 'L3',
        'prompt': '应用看见序列缺口，怎样区分网络丢包和本机处理不及？',
        'answer': '联合检查发送侧、网卡和内核统计、socket 丢包辅助信息，以及应用队列积压、截断和解码错误。SO_RXQ_OVFL 只反映该 socket 的丢弃计数，不代表链路上所有损失。序号能指出应用观察不连续，单凭它不能定位发生损失的层次。',
        'rubric': ['跨层证据关联', 'socket 计数的范围', '缺口不是故障位置证明'],
        'source':
          { 'kind': 'derived', 'rationale': '由接收路径分层与 Linux socket 丢弃计数范围推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q16',
        'level': 'L3',
        'prompt': '加大 SO_RCVBUF 能解决持续处理速度低于到达速度吗？',
        'answer': '它能容纳更长突发，但持续输入超过处理能力时队列仍会耗尽，还可能增加排队延迟。应测突发持续时间、消费服务率和尾延迟，并设计过载恢复；不能把大缓冲当作可靠传输。本例固定小报文测试不提供缓冲调优结论。',
        'rubric': ['缓冲只吸收有限突发', '服务率与排队延迟', '过载仍需恢复策略'],
        'source': { 'kind': 'derived', 'rationale': '由有限接收队列容量与输入输出速率差推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q17',
        'level': 'L3',
        'prompt': '多组复用同一个接收 socket 时，如何避免一个坏包冻结所有流？',
        'answer': '先用可信配置和接收元数据识别目的组、接口与源，再路由到相应序列域；元数据截断或无法识别时按整体策略处理，不能猜归属。本例在识别 channel 前就对截断或非法格式冻结单个 receiver，是单流教学策略，不能直接搬到多路复用分发器。',
        'rubric': ['按元数据先分流', '控制数据不全时不得猜测', '本例冻结策略范围'],
        'source':
          { 'kind': 'derived', 'rationale': '由接收元数据、协议校验顺序与失效范围的关系推导。' },
        'companies': [],
      },
      {
        'id': 'udp-multicast-sequencing-q18',
        'level': 'L3',
        'prompt': '行情增量缺失后，为什么不应继续把当前订单簿标成有效？',
        'answer': '丢失的增量可能改变价格、数量或订单状态，后续连续到包不能自动补回缺失影响。应让下游看到明确的失效状态并按业务规则暂停依赖该状态的动作，恢复完整快照与增量衔接后再发布有效版本。监控是否仍有流量不能代替业务状态一致性检查。',
        'rubric': ['缺失影响无法由后续到包消除', '向下游传播失效', '恢复后才重新发布有效状态'],
        'source':
          { 'kind': 'derived', 'rationale': '由订单簿增量的状态依赖与故障期间下游使用条件推导。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

UDP 保留数据报边界，但不保证交付、有序或重复保护。组播把一个流发给一组接收者，可靠性仍需应用协议处理。接收端先检查报文和辅助信息是否截断，再按频道、会话等序列域判断是否连续。发现缺口时，依赖完整增量的状态要标记失效，按协议补发或用快照重建。序号回绕需要位宽、旧包寿命和比较窗口约束；组播加入哪个接口、接收哪个源，也必须明确配置。

## 核心概念

[RFC 768](https://www.rfc-editor.org/rfc/rfc768) 定义了 UDP 数据报服务，但不提供交付与重复保护保证。组播成员关系由 IP 层与网络环境处理；[RFC 1112](https://www.rfc-editor.org/rfc/rfc1112.txt) 将加入组与网络接口联系起来。加入一个组不会得到历史数据，也不意味着每个成员收到了同一套报文。

数据报是传输单元，业务消息是应用协议单元。一份数据报可以装多条消息、控制信息或应用分片。序号可能计包、计消息或计事件；消息数为零的心跳如何推进序号，也由协议规定。本文自定义教学格式严格采用“一份报文一条事件”，不代表任何交易所格式。

| 观察               | 能说明什么                 | 尚不能说明什么               |
| ------------------ | -------------------------- | ---------------------------- |
| 收到未来序号       | 当前观察到的前缀不连续     | 缺失报文一定永远不会迟到     |
| 收到旧序号         | 在协议比较前提下落在旧范围 | 内容一定与历史副本一致       |
| 收到异会话报文     | 它不属于当前已安装的会话   | 新会话已授权、已有完整基线   |
| `recvmsg` 返回正数 | 复制了一些数据             | 该数据报完整且业务状态可更新 |
| 组成员操作成功     | 内核接受了该成员请求       | 远端网络已能正确转发该流     |

本章示例只执行 Linux 本机 `AF_UNIX/SOCK_DGRAM` 通信，不发送 IP 报文、不加入组播组。它验证数据报接收 API 与应用状态机；网卡、交换网络、组成员和组播路由需要独立环境验收。

## 原理深入

### 从接收缓冲到一份完整报文

[Linux udp(7)](https://man7.org/linux/man-pages/man7/udp.7.html) 说明普通接收操作一次交付一份数据报。用户缓冲不足时，报文尾部会被丢弃，并通过 `MSG_TRUNC` 报告；把缓冲扩大后再读，取到的是后续报文，不是原报文的剩余字节。这里讨论普通数据报路径，不启用 UDP GRO 等改变交付形式的扩展。

[recvmsg](https://man7.org/linux/man-pages/man2/recvmsg.2.html) 的输入参数 flags 与输出 `msg_flags` 要分开看。输入 `MSG_TRUNC` 在 Linux 支持的数据报类型上可以请求返回原始长度，返回值可能大于缓冲中真正存在的字节；输出 `MSG_TRUNC` 表示报文尾部被截掉。示例只检查输出位，接收返回值也显式限制为不超过缓冲容量。

如果依赖目的地址、入接口或时间戳等辅助信息，还要检查 `MSG_CTRUNC`，它表示控制信息缓冲不足。缺少用于分流的元数据时，不能从残缺信息猜测报文属于哪个流。每次 recvmsg 前还应重置结构中的容量字段，不能沿用上一次由内核写回的长度。

完成传输层检查后，再验证协议最小长度、版本、消息数量、每条消息长度以及剩余字节。网络报文不应直接 reinterpret_cast 成带本机布局的结构体；显式取字节并按网络字节序组装，可以把长度、对齐和布局假设分开检查。

### 连续前缀决定状态是否有效

接收方需要知道完整基线对应的 `next`。第一次看到序号 500，并不能证明 0 到 499 不重要；有状态增量流通常要先取得有效快照或协议定义的初始状态。

示例只维护一个配置频道的 `(session, next, value, valid)`。相同会话中，只有序号恰等于 next 的完整事件才增加 value，再推进 next。旧范围报文不重复应用；未来报文说明出现缺口，valid 变为 false，next 与 value 保留在最后完整前缀。后续即使缺失报文到达，本例也不自动恢复，而是等待外部确认的基线。

```text
无基线 ──受信任基线安装──> 有效
有效 ──恰好 next 的完整事件──> 有效，推进一个事件
有效 ──缺口 / 歧义 / 截断 / 格式错误──> 无效
无效 ──普通数据报──> 不再应用增量
无效 ──经验证的快照与流衔接──> 有效
```

真实接收器也可以短暂缓冲乱序报文，再请求缺口补发。这样的缓冲必须同时有容量和时间上限，并按相同序列域去重。若缺口仍无法填齐或补发窗口已过，就进入快照恢复。收到缺口后的报文只能证明那些报文存在，不能重建被遗漏事件的影响。

### 回绕排序有协议前提

本例的序号为 16 位。令 `d = (incoming + 65536 - next) % 65536`，计算先扩为 32 位无符号整数，避免依赖有符号溢出。d 为 0 时是期望事件，1 到 32767 归为未来范围，32769 到 65535 归为旧范围，恰好 32768 无法确定方向，状态进入恢复。

这种半空间比较必须建立在协议限制之上：相关报文的最大前进跨度、重排窗口和旧包寿命不能让两种历史变得不可区分。[RFC 1982](https://www.rfc-editor.org/rfc/rfc1982.txt) 给出了序列数算术和半空间歧义；它原本面向 DNS serial，不是所有 UDP 业务协议都会自动采用的规则。本章只用它解释有限序列空间的数学限制。

若一份旧包在同一 session 内滞留了整整 65536 个事件，它与当前 next 可以同值；若跨过半圈，旧值也可能被判进未来范围。代码无法仅从 16 位字段验证这些寿命假设。真实协议需要足够宽的计数、明确的会话轮换与旧包拒绝条件，或在不确定时重新建立基线。不能靠强制类型转换把这些信息补出来。

### 会话切换与恢复交接

session 在本例是只判断相等的 32 位标识，不按大小推断新旧，也不从普通数据报自动切换。外层控制必须确认当前会话、消息格式和快照的覆盖边界。新会话从 0 开始是否正确，需要协议依据；上一会话的延迟副本不能进入新基线。

快照恢复还要安排实时流衔接。若快照覆盖到序号 S，接收器需要连续获得 S 之后的事件，去掉已经包含在快照中的副本，再切换为有效状态。快照下载期间仍在前进的流可以暂存，或在快照完成后从恢复源补取；缓存溢出和恢复失败必须返回无效状态。

示例的 `install_baseline` 只表达“调用者已经验证完整快照和精确 next”这一前提，没有实现下载、补发、缓存或原子切换协议。代码忽略异会话报文也不证明当前旧状态仍新鲜；真实系统还要按控制事件、心跳和超时停止发布过期状态。

## 数据结构/系统内部实现

教学报文长度固定为 12 字节，字段如下。两字节以上整数采用大端编码，delta 只允许 0 到 1000；累加器为 64 位无符号整数，溢出前拒绝应用并将状态标为无效。

| 字节偏移 | 长度 | 字段               |
| -------- | ---- | ------------------ |
| 0        | 2    | magic：`0x55 0x44` |
| 2        | 1    | version：1         |
| 3        | 1    | channel            |
| 4        | 4    | session            |
| 8        | 2    | sequence           |
| 10       | 2    | delta              |

`decode` 在读任何字段前先要求总长度恰好为 12，短包与多余字节都拒绝。`ingest` 先处理截断和格式错误，再检查 channel/session；因此任意非法输入都会冻结这个 receiver。这是输入已经属于一个预期流的保守教学策略。多组复用同一 socket 时，应先结合可信配置与目的组、源、入接口等元数据分发，再决定哪个流失效。

组播接收配置至少需要区分以下责任：

| 配置                | 用途                             | 不能替代什么           |
| ------------------- | -------------------------------- | ---------------------- |
| `bind`              | 选择本地接收地址与端口           | 不代替加入组           |
| `IP_ADD_MEMBERSHIP` | 在指定接口加入 IPv4 组           | 不验证远端源与应用会话 |
| 源过滤接口          | 按协议选择需要的发送源           | 不提供完整应用认证     |
| `IP_MULTICAST_IF`   | 选择本机组播发送接口             | 不代替接收组成员配置   |
| `IP_PKTINFO`        | 取得目的地址和入接口等接收元数据 | 不检查业务序列连续性   |

Linux 的 [IP_ADD_MEMBERSHIP](https://man7.org/linux/man-pages/man2/IP_ADD_MEMBERSHIP.2const.html) 可使用 [ip_mreqn](https://man7.org/linux/man-pages/man2/ip_mreqn.2type.html) 指定组和接口；多网卡环境应明确接口索引或本地接口地址，并检查 setsockopt 结果。[RFC 3678](https://www.rfc-editor.org/rfc/rfc3678.txt) 区分任意源与指定源过滤接口，所需模式由业务网络决定。[IP_MULTICAST_IF](https://man7.org/linux/man-pages/man2/IP_MULTICAST_IF.2const.html) 以及发送 TTL、回环设置属于发送侧配置，不能用来说明接收端已经加入正确组。

在 Linux 多组接收中，可用 [IP_PKTINFO](https://man7.org/linux/man-pages/man2/IP_PKTINFO.2const.html) 辅助核对目的地址和接口；还要留意 [IP_MULTICAST_ALL](https://man7.org/linux/man-pages/man2/IP_MULTICAST_ALL.2const.html) 的交付策略，不能只按“程序曾加入某组”推断它只会收到该组。复用端口的设置及交付行为也应按目标系统验证。

收到包的路径还经过网卡、内核接收队列和应用队列。`SO_RXQ_OVFL` 能随包提供该 socket 自创建以来的丢弃计数，见 [socket(7)](https://man7.org/linux/man-pages/man7/socket.7.html)；它不能解释所有链路丢包。若使用该信息，应处理计数回绕、辅助信息缺失以及没有后续报文时无法立刻得到新计数的情况。

## C++ runnable demo

状态机与字节解析使用有界输入，本机 I/O 使用 Linux 非阻塞 socketpair。先读 `Receiver::ingest` 的状态变化，再看 `check_local_datagrams` 如何把截断结果交给接收器。

```cpp include=examples/udp-multicast-sequencing.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/udp-multicast-sequencing.cpp -o /tmp/udp-sequence-demo
/tmp/udp-sequence-demo
```

目标为 Linux，断言必须保持启用。预期输出：

```text
Sequence boundaries, recovery gate, local datagram truncation and fd cleanup: OK
```

状态测试覆盖没有基线、正常推进、65535→0 回绕、重复旧包、缺口后冻结、受信任基线恢复、不同频道和会话、半空间边界，以及累加器溢出。解析测试覆盖长度 0 到 11、恰好 12、13 字节、错误 magic/version、最大 delta 与超限 delta。对三个连续序号的六种排列，检查只应用缺口前的连续前缀；它不是任意网络轨迹的穷举。

本机 [socketpair](https://man7.org/linux/man-pages/man2/socketpair.2.html) 发送两份 12 字节报文。第一次只给 8 字节缓冲，验证输出 `MSG_TRUNC`；第二次给足缓冲，验证完整取得第二份报文，而没有读到第一份的尾部。随后发送零长度数据报，验证 recvmsg 返回 0，再用非阻塞读取确认队列空时为 `EAGAIN/EWOULDBLOCK`。

[unix(7)](https://man7.org/linux/man-pages/man7/unix.7.html) 描述的本机数据报具有自己的可靠性与排序特征，不能将本次成功运行推广为 UDP 不丢包或不乱序。示例没有经过 IP、IGMP、真实网卡或组播路由，也没有实际组播结果可报告。

两个 fd 由 RAII 管理，成功 socketpair 后的异常注入会退出作用域，再用 `fcntl(F_GETFD)` 核对两者均为 `EBADF`。每次接收最多提交一次 1000ms 的 [poll](https://man7.org/linux/man-pages/man2/poll.2.html) 等待，之后仍用非阻塞 recvmsg；超时或系统调用错误直接报错退出。调度可能使实际等待超过参数值，所以这不是硬实时截止保证。

发送使用非阻塞 [send](https://man7.org/linux/man-pages/man2/send.2.html)，检查错误与完整长度；示例对 EINTR 也直接失败并清理，没有可能无限重复的重试循环。Linux 的 [close](https://man7.org/linux/man-pages/man2/close.2.html) 在析构中不重试 EINTR，避免关闭已复用的 fd。ASan/UBSan 可以补充检查此次有限执行，不能验证组播网络或恢复协议。

## 高频追问

### 为什么不缓存缺口之后的报文？

本例选择冻结，让“未恢复之前不得继续应用”容易检查。生产方案可以缓冲，但要定义内存上限、超时、重叠补发去重与会话切换。只补齐第一次看到的缺口也未必足够：恢复期间仍可能产生新缺口，需要证明直到发布时的连续边界。

### UDP 校验和通过后，还需要检查长度吗？

需要。传输层检查不能证明应用版本、字段范围、消息计数或内部长度彼此一致，也不能解决用户缓冲造成的截断。应用完整性与来源可信度还依赖自己的协议。不要把校验和当作发送方认证或业务语义验证。

### 一包做得更大是否总能减少开销？

较大报文可以摊薄部分固定开销，但要受路径 MTU 和协议上限约束。[RFC 8085](https://www.rfc-editor.org/rfc/rfc8085) 建议避免依赖 IP 分片；任一分片缺失会影响整个原数据报交付。应用自分片也需要分片身份、数量、重组上限与超时，不能只把 UDP 最大长度当作合适发送尺寸。

### 两个接收线程同时读一个 socket 能更快吗？

它们会竞争消费报文，后续处理顺序还可能与接收顺序不同。需要明确每个序列域的所有者和合并位置，保证 next 与业务状态在同一有序处理路径更新。按流分片可减少共享状态，但跨流业务规则仍需自己的排序依据。

## 容易答错的点

- 把 UDP 数据报当 TCP 字节流，截断后继续拼尾部。数据报余部已丢弃，下一次读取是另一份报文。
- 只检查 recvmsg 返回值为正。还需检查 `MSG_TRUNC`，以及使用辅助信息时的 `MSG_CTRUNC`。
- 把返回 0 一律当关闭。零长度数据报需要按应用格式解释。
- 发现高序号就覆盖 next。这样会跳过缺失事件，隐藏无效业务状态。
- 给所有频道共用一个序号。先识别协议定义的序列域，再比较。
- 用普通整数大小比较回绕序号。即使用模运算，也要有半空间与旧包寿命前提。
- 仅凭新的 session 数字就清空并重启。会话授权、基线和生效边界必须另行确认。
- 以组成员操作成功证明全链路可用。接口选择、转发、源过滤与应用恢复还需验证。
- 把接收缓冲开大就称为可靠传输。缓冲只能容纳有限突发，持续过载仍会耗尽。

## 性能分析

本例报文固定 12 字节，解析和状态转移做固定数量操作，空间为 O(1)，没有乱序缓存或动态恢复数据结构。若协议允许每包 m 条可变长消息，至少需要检查相应消息边界；加入 W 条乱序缓存后，空间、查找和合并成本取决于具体表示，不能沿用本例常数开销结论。

网络评估应记录报文大小分布、突发长度、到达率、接收线程服务率、CPU 与网卡队列放置。分别观察网卡/内核丢弃、socket 丢弃、用户缓冲截断、应用队列满和解码失败，不把它们混成一个“丢包率”。较大的接收缓冲可能吸收突发，也可能延长排队，持续到达速度超过消费能力时仍无法稳定运行。

测量吞吐之外，还要记录从接收到状态发布的 p50/p99/p99.9、缺口发现时间、恢复完成时间、失效持续时间与恢复峰值内存。若采用批量接收，观察批量大小对 syscall 开销和等待时间的影响；若双线去重，保留两条线路的相对延迟分布。恢复流量同样需要容量与速率预算，不能让补发进一步压垮正常接收路径。

本章没有网络吞吐或延迟实测，本机 socketpair 断言不构成组播基准。

## Quant/Low-Latency 场景

订单簿增量依赖前序状态，缺失一条消息可能让后续更新无法正确解释。接收器应把“连续、恢复中、失效”等状态传给下游；是否暂停某类策略或切换数据源由业务规则决定，不能保持一个看似正常的 valid 标志继续使用残缺订单簿。

冗余行情线路只有在协议确认其同属一条逻辑流、会话和同号内容一致时，才适合合并去重。同号但内容不同应作为一致性异常处理；独立发布源的序列不能直接取最大值来构造统一顺序。本文只按序号拒绝旧报文，没有保存历史 payload 来检测同号内容冲突。

会话重启、日切和故障切换要有可追溯的控制边界。状态一致性与新鲜度也要分别检查：当前 value 可能是某个完整前缀，却已长时间没有更新。只看 next 连续或网卡仍有数据，不能证明当前交易决策拿到的是及时且适用的状态。

## 相关专题

- [TCP framing](tcp-framing.md)：对照字节流拆帧与数据报边界，避免混用接收假设。
- [epoll LT / ET](epoll-lt-et.md)：把单个非阻塞 socket 接入事件循环时，继续检查读取、错误与调度边界。
- [Order Book](../trading/order-book.md)：理解缺失增量对业务状态的影响及恢复前后的有效性。
- [RAII 与异常安全](../cpp/raii-exception-safety.md)：检查 fd 在构造失败、接收错误和异常退出时的释放路径。

## 分层面试题

L1 先区分数据报边界与交付保证，L2 给出序列域、回绕及恢复状态转移，L3 再检查快照交接、多网卡与过载。回答缺口问题时，说明哪个序号之前仍是完整状态、哪些新到事件尚未应用，以及恢复后凭什么重新发布有效数据。
