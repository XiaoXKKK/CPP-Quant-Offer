---
{
  'schemaVersion': 1,
  'id': 'tcp-framing',
  'title': 'TCP framing：从字节流恢复消息边界',
  'description': '用有界长度前缀解析器处理头部和正文分片、多帧、零长帧及 EOF 截断，并验证 partial write、背压和半关闭。',
  'category': 'network',
  'areas': ['Network Programming', 'TCP/IP'],
  'tags': ['tcp', 'framing', 'byte-stream', 'backpressure', 'partial-io'],
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
  'prerequisites': ['socket 与非阻塞 I/O', '字节序和有界数组', '状态机与 RAII'],
  'related': ['epoll-lt-et', 'spsc-queue', 'raii-exception-safety'],
  'demo':
    {
      'file': 'examples/tcp-framing.cpp',
      'platform': 'linux',
      'exercise': '增加包含完整帧后跟半个头部的输入，验证已完成帧仍可取出而 finish 报截断；再把发送脚本中的 EAGAIN 移到不同短写之后，检查累计 offset 和最终字节序列不变。',
    },
  'references':
    [
      {
        'title': 'RFC 9293: Transmission Control Protocol',
        'url': 'https://www.rfc-editor.org/rfc/rfc9293.html',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux recv(2)',
        'url': 'https://man7.org/linux/man-pages/man2/recv.2.html',
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
        'title': 'Linux shutdown(2)',
        'url': 'https://man7.org/linux/man-pages/man2/shutdown.2.html',
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
        'title': 'Linux byteorder(3)',
        'url': 'https://man7.org/linux/man-pages/man3/byteorder.3.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'tcp-framing-l1-01',
        'level': 'L1',
        'prompt': '发送端连续调用两次 send，接收端能否用两次 recv 恢复消息？',
        'answer': '不能按调用次数对应。TCP 提供有序字节流，一次接收可能只拿到部分头部，也可能包含多个应用帧。接收端应依据应用协议恢复边界，保存未完成状态并循环提取完整帧，不把系统调用返回边界当作消息边界。',
        'rubric': ['字节流无应用消息边界', '支持分片与多帧', '按协议状态解析'],
        'source':
          { 'kind': 'derived', 'rationale': '依据 TCP 字节流服务和 recv 语义推导应用分帧要求。' },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l1-02',
        'level': 'L1',
        'prompt': '固定长度、分隔符和长度前缀分帧各适合什么条件？',
        'answer': '固定长度适合结构稳定且大小一致的记录，但可变数据容易浪费空间。分隔符便于文本流处理，却需要转义、扫描及最大长度限制。长度前缀适合二进制可变长记录，必须约定字节序、长度含义和上限，并处理头部本身分片。',
        'rubric': ['固定长的格式与空间约束', '分隔符转义和扫描', '长度前缀的校验要求'],
        'source':
          { 'kind': 'derived', 'rationale': '从三种边界编码方式推导其空间、解析和校验权衡。' },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l1-03',
        'level': 'L1',
        'prompt': '本例四字节长度字段包含头部吗，长度零是否合法？',
        'answer': '长度只计算正文，头部另外占四字节；零长正文合法，所以四个零字节就是一帧。正文上限为 16 字节。这些是本教学协议的选择，不是 TCP 的规定，通信双方必须共享同一份格式定义。',
        'rubric': ['长度不含四字节头', '允许零长', '上限属于应用协议'],
        'source':
          { 'kind': 'derived', 'rationale': '依据本例格式契约检查长度含义与零长消息是否明确。' },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l1-04',
        'level': 'L1',
        'prompt': '为什么不能把收到的四个字节直接转成 uint32_t 指针解引用？',
        'answer': '输入位置可能未对齐，也可能不满足相应对象访问规则，而且本机字节序未必是网络字节序。可以逐字节组装无符号值，或复制到合适的整数对象后使用 ntohl。本例逐字节按高位在前组装，不进行未对齐类型访问。',
        'rubric': ['对齐与对象访问限制', '字节序转换', '安全解码方式'],
        'source':
          { 'kind': 'derived', 'rationale': '结合字节序和字节缓冲访问边界推导安全长度解码方式。' },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l1-05',
        'level': 'L1',
        'prompt': 'recv 返回零和返回 EAGAIN 分别说明什么？',
        'answer': '对请求长度大于零的流 socket，零通常表示对端有序关闭了发送方向且该方向数据已读尽。EAGAIN 或 EWOULDBLOCK 表示当前无法立即读取，后续仍可能有数据。零长度读取也可能返回零，所以不能脱离调用参数和 socket 类型判断 EOF。',
        'rubric': ['非零读取下的流 EOF', 'EAGAIN 是暂时无数据', '限定长度与 socket 类型'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据 recv 返回值说明区分流结束、暂时不可读与零长度调用。',
          },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l1-06',
        'level': 'L1',
        'prompt': 'send 返回正数，能否证明对端已处理了完整业务消息？',
        'answer': '不能。返回值只让应用知道这次本地发送操作接受了多少字节，可能少于请求长度；对端应用处理还需要业务确认。调用方应累计偏移，保留未发后缀及其存储寿命，不因一次正返回就丢弃整条消息。',
        'rubric': ['本地接受不等于业务确认', '按实际字节数推进', '保留后缀与存储'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从 send 的局部完成语义推导发送缓冲区和业务确认要求。',
          },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l2-01',
        'level': 'L2',
        'prompt': '长度字段宣称有 0xffffffff 字节时，解析器应按什么顺序处理？',
        'answer': '先完整解码到能容纳字段的无符号类型，立即与协议上限及本地预算比较，再转换类型、计算总长度或分配。本例在收齐四字节后拒绝大于 16 的长度，不申请相应内存。若一般实现要相加，应先用减法形式检查剩余容量，防止先溢出再校验。',
        'rubric': ['解码后先验上限', '分配和相加在校验之后', '避免整数溢出'],
        'source':
          {
            'kind': 'derived',
            'rationale': '按输入控制的长度流向分析整数运算、内存预算和拒绝位置。',
          },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l2-02',
        'level': 'L2',
        'prompt': 'feed 为什么要返回 consumed，已有完整帧时为什么可能消费零字节？',
        'answer': '一次输入可以包含多帧，解析器只保留一个待取帧，后缀仍属于调用者。已有完整帧未取走时再次 feed 返回零消费和 frame_ready，防止覆盖输出。调用者先处理或转移该帧，再从原输入的 consumed 偏移继续，不能丢弃未消费字节。',
        'rubric': ['输入后缀归调用方', '单帧容量形成背压', '取帧后继续相同后缀'],
        'source':
          { 'kind': 'derived', 'rationale': '从有界输出容量和多帧输入推导消费计数接口的必要性。' },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l2-03',
        'level': 'L2',
        'prompt': '收到 EOF 时，怎样区分干净结束、截断头部和截断正文？',
        'answer': '先处理完已经接收并保留的字节，再结束解析器。位于新帧起点可结束；已有一至三字节头部，或完整头部声明的正文尚未收齐，都报截断。若完整帧尚未被取走，应允许交付该帧后结束，不能因为 EOF 清空有效输出。',
        'rubric': ['先处理已收字节', '按状态区分截断', '保留完整待取帧'],
        'source':
          { 'kind': 'derived', 'rationale': '根据解析状态和输入终止事件推导 EOF 的各类转换。' },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l2-04',
        'level': 'L2',
        'prompt': '发送中经历短写、EINTR、EAGAIN，offset 怎样推进才不会重复发送？',
        'answer': '只有返回正字节数才增加 offset。返回负值且 errno 为 EINTR 时保留偏移，可在预算内重试；EAGAIN 或 EWOULDBLOCK 时暂停并等待可写机会。恢复时发送剩余后缀，不能从帧开头重发；每轮还要控制工作量，避免一个连接独占事件循环。',
        'rubric': ['正返回才推进', 'EINTR 与 EAGAIN 分开', '从后缀恢复及公平预算'],
        'source':
          { 'kind': 'derived', 'rationale': '依据发送返回路径推导累计进度与重试的正确状态更新。' },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l2-05',
        'level': 'L2',
        'prompt': '为什么示例的 close 不像 send 遇到 EINTR 那样重试？',
        'answer': '示例限定 Linux，close 发生错误时 fd 通常已被释放，重复关闭同一个整数可能误关后来复用该编号的描述符。RAII 拥有者先清空自身 fd，再只关闭一次。关闭错误可用于诊断，但跨系统实现不能不看平台规定就复制这个策略。',
        'rubric': ['Linux fd 提前释放语义', '重复 close 的编号复用风险', '拥有者只释放一次'],
        'source':
          {
            'kind': 'derived',
            'rationale': '结合 Linux close 规则和 RAII 所有权推导与 I/O 重试不同的清理策略。',
          },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l2-06',
        'level': 'L2',
        'prompt': '解析器的容量不足与非法长度，为什么不应归成同一种错误？',
        'answer': '长度超过协议允许上限意味着输入不合法，本例进入不可恢复的 failed 状态。一个合法完整帧尚未取走则是下游消费能力不足，解析器暂停消费后续字节，取帧后可以继续。混在一起会把正常背压误判为协议错误，或为非法输入无限扩容。',
        'rubric': ['协议上限与本地消费状态分开', '非法输入锁定错误', '合法背压保留进度'],
        'source':
          {
            'kind': 'derived',
            'rationale': '对照长度拒绝和 frame_ready 暂停，推导错误分类与恢复策略。',
          },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l3-01',
        'level': 'L3',
        'prompt': '一个连接持续发送合法长度但很慢的正文，怎样限制资源占用？',
        'answer': '最大帧长只能限制单帧存储，还应限制连接数、全局与每连接缓冲预算，并给组帧设置符合业务的完成期限或最低进度要求。事件循环需能取消并清理半帧状态。规则要与合法大消息和网络抖动兼容，不能仅靠 socket 是否仍连接判断健康。',
        'rubric': ['长度限制不覆盖时间和连接数量', '全局及连接预算', '组帧期限与清理'],
        'source':
          { 'kind': 'derived', 'rationale': '从有界单帧但可长期未完成的状态推导服务端资源治理。' },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l3-02',
        'level': 'L3',
        'prompt': '下游 SPSC 队列满时，网络解析线程怎样处理已完成帧和未消费字节？',
        'answer': '保留已完成帧及接收缓冲的未消费后缀，暂停继续读取或按有界策略转移，不得覆盖或丢弃。下游恢复后需要显式安排处理，尤其 ET 模式不能只等待一个可能不再出现的新边沿。应把停读、恢复和最大积压纳入同一个连接状态机。',
        'rubric': ['保存帧与输入后缀', '有界停读及恢复', 'ET 需要主动续处理'],
        'source':
          {
            'kind': 'derived',
            'rationale': '把分帧容量与下游背压连接，检查事件循环暂停和恢复的一致性。',
          },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l3-03',
        'level': 'L3',
        'prompt': '如何确定性测试分帧，避免只验证本机一次 recv 的巧合边界？',
        'answer': '把解析器与 socket 分开，固定同一串合法字节，枚举切分位置并逐字节喂入，核对输出帧不变。再覆盖非法长度、零长、最大长度、各截断位置和输出容量耗尽。I/O 层用可控返回脚本验证短写和错误恢复，真实 socket 测试补充系统集成。',
        'rubric': ['解析器与传输解耦', '分片等价性及边界', '脚本与真实 I/O 分层'],
        'source':
          {
            'kind': 'derived',
            'rationale': '按可控输入和不稳定内核边界划分验证层次，设计可复现测试。',
          },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l3-04',
        'level': 'L3',
        'prompt': 'socketpair 测试通过，为什么还不能回答 TCP_NODELAY 是否降低了延迟？',
        'answer': 'AF_UNIX 的 SOCK_STREAM 可以验证流读写和解析器集成，但没有运行 TCP 协议栈的分段、拥塞或 Nagle 机制。评估 TCP_NODELAY 要用真实 TCP 连接，控制报文大小、发送节奏、拓扑和负载，记录端到端分布及系统调用行为，本例没有这类测量。',
        'rubric': ['AF_UNIX 与 TCP 实现范围不同', '真实 TCP 受控实验', '不从功能测试推导性能'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据 socket family 与 TCP 机制边界推导性能实验的适用范围。',
          },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l3-05',
        'level': 'L3',
        'prompt': '交易请求的 send 已完成但连接随后断开，重连后能否直接重发？',
        'answer': '本地发送完成不说明对端应用是否已执行，也不能从断线判断一定未执行。重试需要协议定义请求标识、业务确认、去重或状态查询，并明确会话与序列范围。分帧只能恢复字节中的记录边界，不能提供跨重连的恰好一次业务语义。',
        'rubric': ['已执行状态可能不确定', '请求标识与业务恢复协议', '分帧不提供恰好一次'],
        'source':
          {
            'kind': 'derived',
            'rationale': '将传输完成与应用执行分离，推导交易请求重试的确认需求。',
          },
        'companies': [],
      },
      {
        'id': 'tcp-framing-l3-06',
        'level': 'L3',
        'prompt': '零拷贝地向下游传 span 有哪些额外生命周期要求？',
        'answer': 'span 不拥有接收缓冲，后续 recv、压缩缓冲或连接清理都可能使它失效。必须延长底层存储寿命并阻止覆盖，或使用引用计数块、显式归还协议，也可在边界复制成拥有值。要同时测量复制成本、保留内存量与下游背压，不能仅统计少了一次复制。',
        'rubric': ['span 非拥有与失效来源', '明确存储保留和归还', '内存及背压共同评估'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据输入缓冲和异步下游寿命差异推导零拷贝接口的所有权约束。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

TCP 提供有序字节流，应用需要自己定义消息边界。长度前缀协议要先收齐头部，按约定字节序解码并校验长度，再累计正文；一次输入可能完成多帧，也可能连头部都不完整。收发都按实际字节数推进，遇到 EAGAIN 保留状态。EOF 还要检查是否停在完整帧边界，不能把半条消息当作正常结束。

## 核心概念

应用帧、TCP segment 和 `send/recv` 调用是三个不同层次。应用帧由协议定义；TCP 按自身机制传输字节；系统调用返回当前取得或接受的字节数。所谓“半包、粘包”通常描述应用帧被不同读取边界切开或合并，并不表示 TCP 交付了损坏的报文。[RFC 9293 §2.2、§3.7](https://www.rfc-editor.org/rfc/rfc9293.html) 规定字节流服务并解释分段，PSH 也不是应用记录标记。

| 分帧方案 | 适用特点                   | 需要额外处理                     |
| -------- | -------------------------- | -------------------------------- |
| 固定长度 | 消息大小稳定，边界计算直接 | 可变长字段填充、格式版本演进     |
| 分隔符   | 文本协议便于检查和逐行处理 | 转义、跨输入分隔符、扫描上限     |
| 长度前缀 | 可变长二进制消息           | 字节序、长度含义、上限及头部分片 |

本例选择四字节大端无符号正文长度，长度不含头部，正文允许 0 到 16 字节，内容可包含零字节。这个很小的上限用于边界测试，不是实际交易协议的建议值。收发双方应根据正式格式约定长度、版本、消息类型与字段有效性；TCP 不会替应用解释这些内容。

## 原理深入

### 解析进度由消费的字节决定

`Decoder` 记录头部已收字节数、解出的长度、正文已收字节数和状态。未收齐四字节头部时不能推断完整长度；收齐后先检查上限，才允许正文写入有界数组。`frame_ready` 表示一帧完整，调用者用 `take()` 取出拥有值，解析器恢复到下一帧起点。

`feed()` 返回 `consumed`，避免把一次 `recv` 的所有字节都算成已处理。若输入含两帧，第一帧完成时立即停止，剩余后缀由调用者保留。已有待取帧时再调用 feed，结果为零消费和 `frame_ready`，形成一个明确的暂停点。下游处理完后继续同一后缀；不能把零消费当作应立即无限重试的信号。

### 长度检查必须先于扩容和相加

网络字节序采用高位字节在前，转换函数见 [byteorder(3)](https://man7.org/linux/man-pages/man3/byteorder.3.html)。本例将每个字节组合进 `uint32_t`，没有从任意接收地址解引用整数指针。另一种写法是用 `memcpy` 复制到整数对象，再调用 `ntohl`。

长度解码后立即拒绝大于 16 的值，包括 `0xffffffff`。解析器不根据输入长度动态分配，也不先计算未经检查的 `header + length`。编码测试夹具先约束正文长度，再检查 `needed > capacity - used`，避免先把加法做溢出。协议允许的最大长度、单连接缓冲预算和全局内存预算仍是不同约束，正式服务都需要定义。

### EOF 需要经过解析状态判断

对本例非零长度的流读取，`recv == 0` 表示该接收方向结束；相关返回语义见 [recv(2)](https://man7.org/linux/man-pages/man2/recv.2.html)。先处理完已读入并保留的字节，再调用 `finish()`：

- 位于新帧起点，没有残留头部：转为 `ended`。
- 已有部分头部，或头部完整但正文未收齐：转为 `failed`，记录 `truncated`。
- 已有完整待取帧：保留 `frame_ready`，取出该帧后转为 `ended`。

非法长度和截断都是本例的终止错误，之后 feed 消费零字节，不尝试在任意字节位置猜测下一条消息。若其他协议支持重新同步，需要规定标记、校验和恢复规则。错误发生前已经交付的完整帧也不能靠重置解析器撤销，业务事务边界应另行定义。

## 数据结构/系统内部实现

解析器只存一帧正文和固定元数据，正文数组大小为 16。`Frame` 返回值拥有自己的数组，后续接收不会覆盖已经交付的数据。输入 `span` 则不拥有存储，调用者必须保留未消费后缀；异步交给下游前，要复制或建立明确的缓冲区所有权协议。

| 状态          | 可执行动作                   | 退出条件                    |
| ------------- | ---------------------------- | --------------------------- |
| `need_input`  | 累计头部或正文               | 完整帧、非法长度或 EOF      |
| `frame_ready` | `take()` 交付完整帧          | 取走后继续，或在 EOF 后结束 |
| `failed`      | 查询错误、关闭或重建连接状态 | 本例不提供恢复              |
| `ended`       | 不再接受该输入流的数据       | 新流应创建新解析器          |

发送侧使用 `offset` 记录已被本地发送操作接受的前缀。正返回才推进；`EINTR` 保持位置，`EAGAIN/EWOULDBLOCK` 暂停。错误分支及时保存 errno，其他系统调用不能覆盖本次错误原因。`flush` 每次最多尝试 16 次调用，持续中断或预算用尽时返回 `retry_later`，供调用者重新调度。依据见 [send(2)](https://man7.org/linux/man-pages/man2/send.2.html)。

fd 拥有者不可复制，移动构造转移编号。清理时先将自身置为无效再关闭一次；Linux 下失败的 close 不能盲目重试，否则可能关闭被复用的编号，见 [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)。示例析构忽略关闭诊断，不代表生产系统不需记录清理错误。发送使用 `MSG_NOSIGNAL`，将断开的发送路径交给 EPIPE 错误处理，避免默认 SIGPIPE 动作终止整个进程。

## C++ runnable demo

示例分成无 socket 的解析测试、脚本发送测试和 Linux socket 集成测试。`socketpair(AF_UNIX, SOCK_STREAM)` 创建本机双向流连接，接口见 [socketpair(2)](https://man7.org/linux/man-pages/man2/socketpair.2.html) 和 [unix(7)](https://man7.org/linux/man-pages/man7/unix.7.html)。它验证流式收发与解析器集成，没有运行 TCP 的拥塞、分段或 Nagle 实验。

```cpp include=examples/tcp-framing.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/tcp-framing.cpp -o /tmp/tcp-framing
/tmp/tcp-framing
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fno-omit-frame-pointer -fsanitize=address,undefined -fno-pie -no-pie examples/tcp-framing.cpp -o /tmp/tcp-framing-san
/tmp/tcp-framing-san
```

本机 WSL、GCC 13.3.0 的两种构建与运行均通过，输出为 `framing checks passed; split points=32; frames=3; half-close=ok`。测试依赖启用的 assert，编译时不要定义 `NDEBUG`。31 字节测试流包含正文长 3、0、16 的三帧。验证内容如下：

| 层次        | 验证输入或行为                                                  |
| ----------- | --------------------------------------------------------------- |
| 分片等价性  | 32 个单切分位置及逐字节喂入，输出与整段喂入相同                 |
| 格式边界    | 零长与 16 字节正文接受；长度 17 和 `0xffffffff` 拒绝            |
| EOF         | 第一帧所有 0 到 7 字节前缀，区分干净结束、截断和完整待取帧      |
| 容量        | 待取帧使后续 feed 零消费；64 字节编码缓冲不足时不追加           |
| 发送进度    | 脚本依次注入 EINTR、短写、EAGAIN，恢复后字节序列完全一致        |
| 实际 socket | 初始 EAGAIN、数据收发、单向 EOF、反向回复、EPIPE 不触发进程退出 |

socket 使用非阻塞模式，每次实际 send 最多提交 3 字节，recv 缓冲为 5 字节，仍只按返回值推进，不断言内核必须怎样组合。脚本才负责确定性制造短写与 EINTR。两段 socket 处理循环各设 1024 轮测试预算，耗尽便报告失败；没有在单线程中阻塞写满 socket 缓冲的路径。这个预算是测试退出条件，不是网络服务超时设计。

## 高频追问

“改用 MSG_WAITALL 能省掉状态机吗？”它请求尽量满足指定读取长度，遇到信号、错误或断开仍可能返回不足，且调用者事先还要知道该读多少。它无法替代长度校验、EOF 截断判断和应用协议边界；本例用增量解析处理任意输入切分。

“收到 EOF 为什么还能发回复？”收发方向可以分别关闭。`shutdown(SHUT_WR)` 禁止该端继续发送，但保留接收方向，见 [shutdown(2)](https://man7.org/linux/man-pages/man2/shutdown.2.html)。示例 A 发完后关闭写方向，B 读到 EOF，再向 A 回送一个确认字节；它只是验证反向通路，并非业务处理确认。TCP 的独立方向关闭见 [RFC 9293 §3.6.1](https://www.rfc-editor.org/rfc/rfc9293.html#section-3.6.1)。

“为什么不在 EAGAIN 后立即循环直到成功？”此时需要对端消费、网络进展或新的数据到达，无限循环会占用线程。事件循环通常保留缓冲和进度，等待可读/可写条件后续做；每次工作量也要有预算。可写事件不保证整个应用帧一次发送完成，返回后仍须检查短写和 EAGAIN。

“解析器有了长度上限，资源控制是否足够？”还需要连接总量、输出积压、未消费输入和组帧时间限制。一个客户端可以持续发送合法长度的慢正文，占用状态而不完成消息；长度正确不表示业务语义有效或对端持续取得可接受的进展。

## 容易答错的点

- 一次 send 对应一帧，只是调用方的提交方式；接收方不能据此假设一次 recv 恰好返回一帧。
- TCP_NODELAY 不会为流加入记录边界，PSH 也不能充当自定义协议分隔符。
- 应先检查长度再扩容和计算总大小，不能让超大字段先触发分配或溢出。
- `frame_ready` 的零消费表示需要取走输出；把后缀丢弃或反复原地 feed 都会破坏处理流程。
- 非阻塞无数据通常通过负返回和 errno 表示；非零长度流读取的零返回需要进入 EOF 处理。
- 成功 send 与收到 TCP ACK 都不能替代应用执行确认。重连重发需要协议定义去重和状态恢复。

## 性能分析

本解析器按输入字节推进，不会每来一个字节就重扫已有正文；处理 N 字节的扫描工作为 `O(N)`。`take()` 复制固定正文数组，若把上限推广为 M，单次交付的数组复制成本为 `O(M)`，解析器空间为 `O(M)`。这份教学实现没有零拷贝承诺，帧数量很多或最大帧很大时应重新评估复制策略。

测试性能前分别定义纯解析、系统调用和从消息到达到业务完成的计时范围。控制正文大小分布、输入切分、并发连接、负载速率及下游处理速度，记录吞吐、p50/p99/p99.9、积压字节和 EAGAIN 次数。短消息大量逐次发送与批量发送会改变调用开销和等待时间；减少调用次数不必然降低端到端尾延迟。

真实 TCP 参数实验应使用 TCP socket，保存拓扑、内核、编译器及发送节奏。TCP_NODELAY 等选项可能改变发送行为，但不能从本例 AF_UNIX 结果得出效果。对照测试还要说明是否使用闭环请求、是否把暂停生产期间的需求排除在样本之外。本章只有功能与边界验证，没有性能数字。

## Quant/Low-Latency 场景

交易网关从 TCP 会话接收订单或回报时，framing 层先恢复完整记录，再交给类型与字段校验。完整帧不等于合法订单，更不表示已执行。若本地发送已经完成却未收到业务确认便断线，请求可能尚未处理，也可能已经处理；重连策略需要请求标识、确认、查询或去重机制，不能只依赖发送 offset。

解析后向下游队列提交失败时，保留完整帧和未消费字节，按预算暂停继续读取。恢复时应显式调度剩余工作；ET 事件循环如果因本地背压提前停读，不能只等下一次新边沿。各层的高低水位、最大待发和待处理字节、取消清理应共用清楚的连接状态。

向下游传递接收缓冲的 span 可以省一次复制，但下次 recv、缓冲压缩或连接关闭都可能结束它的有效期。应保留底层内存直到最后一个使用者归还，或者转换成拥有值。评估时同时记录复制成本和被慢消费者占用的内存，避免以“零拷贝”掩盖无界积压。

## 相关专题

- [epoll：LT vs ET](epoll-lt-et.md)：把解析进度和背压接入可读、可写事件处理。
- [SPSC 队列](../concurrency/spsc-queue.md)：下游满时保留消息，并明确何时恢复消费。
- [RAII 与异常安全](../cpp/raii-exception-safety.md)：连接和缓冲的拥有者如何在错误路径释放资源。

## 分层面试题

L1 从字节流、消息边界和返回值开始；L2 跟踪长度、消费偏移、EOF 与错误状态；L3 把解析器放回连接预算、下游背压和业务恢复中。题目中的协议格式属于本例，评审实际系统时应先核对所用协议的长度与关闭定义。
