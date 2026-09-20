---
{
  'schemaVersion': 1,
  'id': 'io-uring',
  'title': 'io_uring：提交、完成与资源回收',
  'description': '以单个 NOP 核对 Linux SQ/CQ 发布与消费协议，区分提交和完成错误，并说明异步缓冲区、取消请求、CQ 压力与能力探测的边界。',
  'category': 'network',
  'areas': ['Network Programming', 'epoll / io_uring'],
  'tags': ['io-uring', 'async-io', 'completion-queue', 'resource-lifetime', 'linux'],
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
  'prerequisites': ['Linux fd 与 mmap', 'acquire/release 的发布与复用', '异步请求的资源生命周期'],
  'related': ['epoll-lt-et', 'cpp-memory-model', 'raii-exception-safety', 'numa-first-touch'],
  'demo':
    {
      'file': 'examples/io-uring.cpp',
      'platform': 'linux',
      'exercise': '在同一线程内顺序提交两次不同 user_data 的 NOP，每次消费完成后再复用槽位；检查两个完成身份，并保持失败清理和受限环境 SKIP 行为。',
    },
  'references':
    [
      {
        'title': 'Linux v6.8 UAPI: io_uring.h',
        'url': 'https://raw.githubusercontent.com/torvalds/linux/v6.8/include/uapi/linux/io_uring.h',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'liburing 2.8 manual: io_uring overview and ring ordering',
        'url': 'https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring.7',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'liburing 2.8 manual: io_uring_setup and features',
        'url': 'https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_setup.2',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'liburing 2.8 manual: io_uring_enter',
        'url': 'https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_enter.2',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC manual: __atomic builtins',
        'url': 'https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'liburing 2.8: acquire/release barrier helpers',
        'url': 'https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/src/include/liburing/barrier.h',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'liburing 2.8 manual: io_uring_submit',
        'url': 'https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_submit.3',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'liburing 2.8 manual: read request buffer lifetime',
        'url': 'https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_prep_read.3',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'liburing 2.8 manual: cancel requests and completion',
        'url': 'https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_prep_cancel.3',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'liburing 2.8 manual: CQE consumption',
        'url': 'https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_cqe_seen.3',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'liburing 2.8 manual: opcode probing',
        'url': 'https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_get_probe.3',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'liburing 2.8 manual: NOP',
        'url': 'https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_prep_nop.3',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux man-pages: syscall wrapper return convention',
        'url': 'https://man7.org/linux/man-pages/man2/syscall.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux man-pages: close and descriptor lifetime',
        'url': 'https://man7.org/linux/man-pages/man2/close.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux kernel: io_uring_disabled',
        'url': 'https://docs.kernel.org/admin-guide/sysctl/kernel.html#io-uring-disabled',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'io-uring-q01',
        'level': 'L1',
        'prompt': 'io_uring 的普通读写完成，与 epoll 的就绪通知有什么区别？',
        'answer': 'epoll 通知 fd 的就绪状态，应用随后调用读写并处理实际结果；io_uring 的普通读写请求由 SQE 描述，CQE 给出该请求的执行结果。CQE 不只是一条可以读写的提示，但 io_uring 也提供 poll 类操作，不能把所有 opcode 都当作已经完成数据传输。',
        'rubric': ['就绪与操作结果区分', 'SQE 描述请求及 CQE 携带结果', 'poll 类 opcode 的边界'],
        'source':
          { 'kind': 'derived', 'rationale': '根据就绪驱动与完成驱动模型的不同职责设计基础题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q02',
        'level': 'L1',
        'prompt': 'SQ 和 CQ 的 head、tail 分别由谁推进？',
        'answer': '在本文单提交者、单消费者模型中，用户态填写 SQE 和 SQ 索引并发布 SQ tail，内核消费后推进 SQ head。内核写 CQE 并发布 CQ tail，用户态读取完成后推进 CQ head。各自写入的位置不同，观察对端进度时还要遵守相应内存序，不能靠普通变量读写猜测可见性。',
        'rubric': ['SQ 用户发布内核消费', 'CQ 内核发布用户消费', '对端进度与内存序'],
        'source': { 'kind': 'derived', 'rationale': '根据双环的生产者消费者关系设计所有权题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q03',
        'level': 'L1',
        'prompt': 'user_data 有什么用途，为什么不能按 CQE 顺序匹配请求？',
        'answer': 'user_data 是应用填写并由完成记录带回的标识，用来把 CQE 关联到请求状态。独立请求可能乱序完成，因此不能按提交顺序逐一匹配。若标识来自对象指针或可复用槽位，还必须保证对象活到相关完成处理结束，或用带代际的 ID 避免旧完成被认成新请求。',
        'rubric': ['完成关联标识', '独立请求可乱序完成', '指针寿命或代际防复用'],
        'source': { 'kind': 'derived', 'rationale': '根据乱序完成及标识复用风险设计请求关联题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q04',
        'level': 'L1',
        'prompt': 'CQE 的 res 小于零时，应读取 errno 解释该请求失败吗？',
        'answer': '不应当，异步请求错误以负 errno 数值放在 cqe.res 中，当前线程的 errno 可能属于其他调用。还要区分调用层：本例使用 libc syscall 包装器，系统调用整体失败返回 -1 并设置 errno；liburing 许多 API 直接返回负错误码。提交调用成功也不代表每个请求执行成功。',
        'rubric':
          [
            'CQE res 为负错误码',
            'syscall 包装器与 liburing 返回约定区分',
            '提交成功不等于操作成功',
          ],
        'source':
          { 'kind': 'derived', 'rationale': '根据同步调用错误与异步请求错误的不同通道设计题目。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q05',
        'level': 'L1',
        'prompt': '一次读请求的 CQE res 为正，但小于请求长度，应怎样处理？',
        'answer': '它表示本次成功读取的字节数，不能据此认定所需消息已经收齐。需要按协议保留已读范围并决定是否继续请求；零长度结果也要按具体操作解释，例如普通文件读可能表示 EOF。完成一个 I/O 请求与完成一条业务消息是不同状态。',
        'rubric': ['正值按实际字节数处理', '短读与零结果依操作语义', 'I/O 完成不等于消息完整'],
        'source':
          { 'kind': 'derived', 'rationale': '根据普通读操作的部分完成语义设计结果处理题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q06',
        'level': 'L2',
        'prompt': '为什么填完 SQE 后才 release 更新 SQ tail，读取 CQE 前要 acquire 读取 CQ tail？',
        'answer': 'SQ tail 的发布要排在 SQE 和索引写入之后，否则内核可能看到新槽位却尚未看到完整请求。CQ tail 的 acquire 观察与内核发布配合，使后续读取对应 CQE 内容；消费完成再 release 更新 CQ head，才能允许该槽位被复用。本例依赖 Linux UAPI 与 GCC 原子内建的实现协议，不是纯 ISO C++ 对内核线程的保证。',
        'rubric':
          ['内容先于索引发布', 'CQ acquire 后读及消费后 release', '明确 Linux 和编译器实现边界'],
        'source':
          { 'kind': 'derived', 'rationale': '根据共享 ring 的发布与复用关系设计内存序机制题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q07',
        'level': 'L2',
        'prompt': 'SQ head 已经越过某项，为什么仍不能立刻释放它的读写 buffer？',
        'answer': 'SQ head 推进表示提交描述已被消费，操作可能仍在执行。SQE 槽位、辅助描述结构和实际数据 buffer 的寿命要求不同；普通单次读写的 buffer 应保持有效到原操作完成，写缓冲区还不能被提前改写。SUBMIT_STABLE 等特性涉及提交时的数据导入，不能解释成数据 I/O 已经完成。',
        'rubric':
          ['描述已消费与操作完成不同', 'SQE 元数据与 payload 分开', '普通数据 buffer 保留到完成'],
        'source':
          { 'kind': 'derived', 'rationale': '根据提交描述和异步数据访问的不同结束点设计寿命题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q08',
        'level': 'L2',
        'prompt': '发布包含普通 fd 的 SQE 后，能否立即 close 并复用这个 fd 编号？',
        'answer': '若内核还没消费请求并取得相应文件引用，提前关闭或复用编号会破坏应用对目标对象的判断。内核取得引用后可能维持底层对象的寿命，但这不等于应用可忽略描述符和请求状态管理。简单的保守协议是保留 fd 到相关原操作完成，再统一关闭；关闭 fd 也不能代替处理取消和完成结果。',
        'rubric': ['消费前 fd 编号复用风险', '描述符编号与内核文件引用区分', '保守寿命与完成协议'],
        'source':
          { 'kind': 'derived', 'rationale': '根据异步提交与描述符复用时机设计 Linux 生命周期题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q09',
        'level': 'L2',
        'prompt': '取消请求返回一个 CQE 后，原请求的状态应怎样收尾？',
        'answer': '取消操作本身有自己的 user_data 和 CQE，它与目标请求的 CQE 要分别处理。取消可能成功，也可能因目标已完成而找不到；EALREADY 表示执行已推进到无法取消的阶段，仍须等原操作的最终 CQE。按本文采用的 liburing 2.8 文档，成功取消时原完成在提交返回前已被发布，应用仍要消费它并结清原请求状态。不能仅看到取消 CQE 就复用原请求 ID。',
        'rubric':
          ['取消与原请求分别关联', '成功或竞争失败的不同情况', '原操作最终完成驱动资源回收'],
        'source':
          { 'kind': 'derived', 'rationale': '根据取消操作与目标操作的两套完成状态设计竞态题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q10',
        'level': 'L2',
        'prompt': '为什么不能只看 uname 的内核版本就决定启用某个 io_uring opcode？',
        'answer': '构建头文件、运行内核、回移补丁及管理员限制是不同条件。需要检查 setup 返回与 features，按需要探测 opcode 支持，再验证实际文件、设备和参数组合的请求结果。较新的版本号不能排除 seccomp 或 io_uring_disabled 限制，opcode 探测成功也不保证每种 fd 都支持该操作。',
        'rubric': ['编译头与运行能力分开', 'features 和 opcode 探测', '部署限制及实际操作仍需验证'],
        'source': { 'kind': 'derived', 'rationale': '根据内核接口演进与运行权限设计能力协商题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q11',
        'level': 'L3',
        'prompt': 'CQ 消费不及时会发生什么，NODROP 能否让应用忽略 CQ 容量？',
        'answer': 'CQ 满会把压力传到完成处理路径。支持 NODROP 时，内核可把暂时放不进 CQ 的事件保存在内部溢出结构，应用仍需及时回收 CQ 并推动相应处理；严重内存不足仍可能丢失事件。没有该特性时边界更严。应限制在途请求、监测溢出相关状态并定义失败处置，不能把 NODROP 当无限队列或绝不丢完成的承诺。',
        'rubric': ['CQ 满与内部暂存', 'NODROP 仍受资源限制', '在途上限和错误处置'],
        'source': { 'kind': 'derived', 'rationale': '根据完成队列压力与内核特性边界设计背压题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q12',
        'level': 'L3',
        'prompt': '关闭使用普通异步读写的服务时，哪些步骤不能被一个超时返回替代？',
        'answer': '应停止接收和提交新请求，跟踪已经发布及在途操作，按策略请求取消并处理原操作的最终完成，再回收 buffer、请求上下文及 fd，最后销毁 ring。用户态等待超时只表示应用不愿继续等待，不证明内核已停止访问资源。若无法确认结束，必须保留资源或走经过证明的终止方案，而不能像 NOP 示例一样直接推广清理。',
        'rubric': ['停止新增并跟踪在途状态', '原操作终结后回收外部资源', '超时不是取消或完成证明'],
        'source':
          { 'kind': 'derived', 'rationale': '根据普通数据请求和 NOP 的寿命差异设计退出协议题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q13',
        'level': 'L3',
        'prompt': '把本例的单线程 ring 直接交给多个提交线程，为什么不够？',
        'answer': '多个线程会竞争 SQE 分配、SQ 索引和本地 tail，单独把 tail 改成原子并不保证每个线程独占槽位，也不保证按正确顺序发布完整请求。CQ 多消费者还需要协调领取和归还。可以给 ring 的用户态操作加合适同步，或采用每线程 ring 和明确交接，比较锁竞争、资源用量与负载分配。',
        'rubric':
          ['槽位预留与发布是独立问题', '多消费者回收也需协议', '同步共享与每线程 ring 的取舍'],
        'source':
          { 'kind': 'derived', 'rationale': '根据单生产者协议扩展到多线程的缺失条件设计题目。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q14',
        'level': 'L3',
        'prompt': '打开 SQPOLL 就能保证没有系统调用，而且比 epoll 更快吗？',
        'answer': '不能。SQPOLL 的内核线程可减少活跃期间提交所需的系统调用，但它休眠后可能需要唤醒，初始化和其他路径也仍有调用，轮询本身占用 CPU。性能还取决于批量、在途深度、文件或网络路径、缓存与调度。应在同等业务负载下比较吞吐、尾延迟和 CPU 用量，NOP 不能代表网络或存储性能。',
        'rubric': ['SQPOLL 减少调用有条件', 'CPU 与调度成本', '同负载测量且 NOP 证据有限'],
        'source': { 'kind': 'derived', 'rationale': '根据轮询模式的收益和成本设计性能取舍题。' },
        'companies': [],
      },
      {
        'id': 'io-uring-q15',
        'level': 'L3',
        'prompt': '已有 epoll 网关迁移到 io_uring 时，怎样避免只替换 API 却破坏业务状态？',
        'answer': '先选一个边界清楚的 I/O 路径，重新定义 buffer 所有权、部分读写、请求 ID、关闭与取消状态，再设置在途上限和完成处理预算。保留能力探测与部署不支持时的后备路径，在真实报文和拥塞条件下比较。CQE 成功只说明对应 I/O 的结果，不保证对端应用已确认订单，也不应把一条 CQE 当成一条完整业务消息。',
        'rubric': ['状态机和所有权随完成模型调整', '背压及部署后备路径', 'I/O 结果与业务确认区别'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据网关从就绪模型迁移到完成模型的状态变化设计题目。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

io_uring 是 Linux 的异步 I/O 接口，用户通过提交队列发布请求，内核通过完成队列返回结果。SQE 发布、CQE 消费和槽位复用都需要按共享 ring 协议进行；完成顺序可能与提交顺序不同，用 user_data 关联请求。普通数据 buffer 要活到相应操作完成，提交成功或发出取消请求都不等于可以回收。批量与轮询可以改变调用成本，但不会保证所有路径没有系统调用，也不保证比现有方案更快。

## 核心概念

本章示例以 Linux v6.8 UAPI 布局和默认 ring 模式为边界，用 C++20、GCC 原子内建及 Linux 共享映射实现。实际构建头文件包为 `linux-libc-dev 6.8.0-139.139`，运行内核为 WSL2 Linux 6.18.33.2；机制参考固定的 liburing 2.8 文档与源码。未启用 SQPOLL、IOPOLL、扩展 SQE/CQE、multishot 或延后 task work，不把这些新模式的规则混入默认模式。[Linux v6.8 UAPI](https://raw.githubusercontent.com/torvalds/linux/v6.8/include/uapi/linux/io_uring.h)

| 对象       | 保存什么                              | 谁负责推进                   |
| ---------- | ------------------------------------- | ---------------------------- |
| SQE        | opcode、参数、user_data 等请求描述    | 用户填好后交给内核消费       |
| SQ         | 本文模式下的 SQE 索引数组及 head/tail | 用户发布 tail，内核推进 head |
| CQE        | user_data、res 和 flags               | 内核填写，用户读取           |
| CQ         | 完成槽位及 head/tail                  | 内核发布 tail，用户推进 head |
| 请求上下文 | buffer、业务进度、取消和回收状态      | 应用按完成协议管理           |

普通读写 CQE 给出已执行操作的结果，epoll 则主要报告就绪状态，应用还要调用读写。io_uring 也有 poll 类操作，所以必须根据 opcode 解读完成含义。本文的 NOP 没有文件数据传输，用于检查 ring 往返及清理。[io_uring 编程模型](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring.7)、[NOP](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_prep_nop.3)

示例采用一个用户线程提交并消费。内核会并发访问共享 ring，但这不使用户态部分自动成为支持任意多个生产者和消费者的队列。

## 原理深入

### 请求发布与完成回收

默认 SQ 中的索引数组指向 SQE 数组，CQ 则直接存放 CQE。示例只使用 SQE 0，按当前 tail 选择 SQ 索引槽。这个间接关系和各字段偏移都来自 setup 返回参数，不能把一套固定地址布局写进程序。

发布和消费顺序如下：

```text
用户填 SQE 与 SQ 索引
    → release 发布 SQ tail
    → 内核读取并消费请求，推进 SQ head
    → 操作完成，内核写 CQE 并发布 CQ tail
    → 用户 acquire 观察 CQ tail，复制 CQE
    → release 推进 CQ head，允许完成槽位复用
```

用户必须等对端消费后再复用相应槽位；复制 CQE 后可以归还 CQ 槽位，但业务请求资源是否能回收还要依据操作语义。示例使用 GCC `__atomic` 内建并要求 32 位操作无锁，针对 Linux UAPI 的共享字段实施 acquire/release。它不把 mmap 中的内核结构伪称为由 ISO C++ 单独定义的跨内核同步对象。[ring 内存顺序](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring.7)、[GCC 内建](https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html)、[liburing barrier 实现](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/src/include/liburing/barrier.h)

### 三种结果通道

| 观察位置                   | 本章应怎样解读                                 |
| -------------------------- | ---------------------------------------------- |
| libc syscall 包装器返回 -1 | 读取 errno，表示这次系统调用整体失败           |
| liburing API 返回负错误码  | 按该 API 的返回约定处理，不能机械读取 errno    |
| CQE 的 res                 | 请求结果；负值为负 errno，非负值按 opcode 解释 |

本例通过 syscall 调用 setup 和 enter，所以用第一种约定处理调用错误。enter 返回消费了多少项，也不表示这些项已经全部成功执行。普通读取的正 res 是实际字节数，可能少于请求长度；NOP 的成功 res 为 0。[syscall](https://man7.org/linux/man-pages/man2/syscall.2.html)、[提交返回值](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_submit.3)、[普通读取](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_prep_read.3)

### 完成顺序与请求身份

两个独立请求可以乱序完成。user_data 应足以找到对应状态，使用指针时保证上下文仍活着，使用槽号时防止尚未结束的旧请求与新请求重号。取消请求也应使用独立的完成标识，目标请求标识放在取消参数中。

本文普通 NOP 采用一项请求对应一项完成。multishot 可以有多项完成，带 `IORING_CQE_F_MORE` 的记录表示后面还可能有完成；`IOSQE_CQE_SKIP_SUCCESS` 等选项也会改变数量关系。因此不能把本例的一对一计数推广为所有 opcode 与 flags 的共同保证。[完成标志与关联](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring.7)

## 数据结构/系统内部实现

### setup、映射和能力检查

setup 返回 ring fd、实际容量、offset 和 features。SINGLE_MMAP 表示 SQ/CQ ring 可共用一次映射，此时映射长度取两者需求的较大值，SQE 区域仍需单独映射。没有这个特性时分别映射两个 ring。程序检查容量、掩码、偏移、对齐和映射长度上限，并避免对共用地址重复 munmap。[setup 与 features](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_setup.2)

可编译的头文件不证明当前环境允许建立 ring。setup 可能受内核支持、管理员策略、seccomp 或资源限制影响；较新的 uname 版本也不能绕过这些条件。需要具体 opcode 时，可以使用 REGISTER_PROBE 或 liburing 的探测接口，再核对实际 fd、设备和参数组合。本例不调用 opcode probe，而是通过真实的单次 NOP 验证所需路径。[opcode 探测](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_get_probe.3)、[管理员禁用设置](https://docs.kernel.org/admin-guide/sysctl/kernel.html#io-uring-disabled)

### SQE、buffer 与 fd 的寿命不同

SQ head 前进表明请求描述已被消费，不能据此释放正在参与 I/O 的数据 buffer。普通单次读写的 buffer 应保持有效到原操作完成；写操作使用的数据还不能在此之前被业务覆盖。辅助描述结构的导入时点要结合 opcode 和 SUBMIT_STABLE 等特性判断，不与实际数据传输的结束点混为一谈。

fd 编号可能被复用。发布请求后立刻 close，若内核尚未取得请求所需的文件引用，应用就失去了清楚的目标寿命保证。内核取得引用后可能维持底层对象，但简单的应用协议仍可选择将 fd 保留到相关原操作完成。close 与取消是不同动作，也不能从 close 成功推导业务 I/O 已按期结束。[read buffer 寿命](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_prep_read.3)、[close 的文件引用边界](https://man7.org/linux/man-pages/man2/close.2.html)

### CQ 满不是可以忽略的后台细节

CQ 空间不足时，完成处理会受到压力。支持 NODROP 的内核可以将暂时放不进 CQ 的事件保存在内部溢出结构，等待 CQ 腾出空间；极端内存不足仍可能丢完成，相关错误和计数不能忽略。不支持该特性的旧路径有更严格的丢失与提交限制。具体返回值还与内核版本和 enter flags 有关。[NODROP 边界](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_setup.2)、[enter 错误](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_enter.2)

`IORING_SQ_CQ_OVERFLOW` 可提示存在需要处理的溢出完成，cq_overflow 字段不能当作所有内部待刷事件的队列长度。应用应及时消费 CQ，按所用模式推动内核处理，同时限制在途操作。本例只提交一个 NOP，没有主动制造 CQ 满或验证溢出恢复。

## C++ runnable demo

示例直接使用 Linux UAPI 和 syscall，无需 liburing 链接参数。它建立一个默认 ring，发布一项带固定 user_data 的 NOP，检查 CQE 身份、res、flags 和没有发生丢失的计数，再归还槽位并释放资源。

```cpp include=examples/io-uring.cpp

```

在有相应 Linux UAPI 头文件的 GCC 环境编译，保留断言：

```bash
g++ -std=c++20 -O2 -pthread -Wall -Wextra -Wpedantic -Werror examples/io-uring.cpp -o /tmp/uring-demo
/tmp/uring-demo
```

本次 GCC 13.3、WSL2 Linux 6.18.33.2 输出：

```text
sq_entries=2 cq_entries=4 single_mmap=1 nodrop=1
NOP completed: user_data matched, res=0, cq_overflow=0
ring resource cleanup passed
```

容量与 feature 值以 setup 返回为准，这一行是本机观察。代码不要求所有内核都返回同样的 feature 组合。setup 遇到 ENOSYS、EPERM、EACCES 或 EOPNOTSUPP 会打印 SKIP 和原因；EINVAL、内存不足等其他错误不会被一概伪装成不支持。SKIP 只表示能力路径未执行，不能记作 NOP 已完成。本机 `io_uring_disabled` 为 0，普通构建和 ASan/UBSan 均实际完成了 NOP。

轮询最多尝试 100000 次，并检查两秒用户态 deadline；超限明确失败。每次 enter 的 min_complete 为 0、flags 为 0，不请求 GETEVENTS 等待。这个 watchdog 不是系统调用、调度或 close 的硬实时上界，也不是生产 I/O 的取消协议。[enter 调用模式](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_enter.2)

代码只发布一次 SQE，重试时根据 SQ head 计算尚待消费的项数，不重新发布一份 NOP。EINTR、EAGAIN 会在这个有界循环中重试。完成内容先复制到本地，再推进 CQ head，归还后不再引用旧 CQ 槽位。[CQE 回收](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_cqe_seen.3)

异常清理之所以可在此处收敛，是因为唯一可能在途的操作是无外部 buffer、无应用文件依赖的 NOP。资源对象释放 ring 映射并关闭 ring fd，记录清理错误。不得把它推广成“普通异步读写超时后直接释放 buffer”。Linux close 出错也不盲目重试，以免误关已复用的描述符编号。

## 高频追问

### 取消 CQE 与原操作 CQE 怎样区分？

取消是一项独立请求，目标标识用于匹配原操作，取消请求自身也有 user_data。默认单目标取消成功时 res 为 0；找不到目标可能是它已经完成，也可能是标识错误。`-EALREADY` 表示执行已推进到无法取消的阶段，之后仍须等原操作的最终 CQE；它可能正常完成，也可能因取消而中断，不能把该错误理解成稍等后重试必然成功。

按本文采用的 liburing 2.8 文档，成功取消时原操作的完成在提交返回前已被发布，应用仍须消费它并结清原请求状态。不要假定两条完成以固定顺序被应用处理，也不要只看到取消结果就丢弃原请求状态。[取消语义](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_prep_cancel.3)

### 注册 buffer 或文件后，可以忘记寿命管理吗？

注册会改变资源引用和访问方式，但仍需管理注册项、在途请求和后续注销或更新的关系。它不使应用可以任意覆盖正在被读取的数据，也不让请求上下文自动延长寿命。本文不实现固定资源、multishot 或零拷贝通知；这些模式需要依据各自最终完成和通知规则设计回收。

### 为什么生产项目通常不手写这些映射偏移？

liburing 封装了布局、屏障、提交与完成处理，以及不同 feature 下的路径。手写 UAPI 示例有助于看清所有权，但扩大到注册资源、task work 或不同 CQE 大小时，维护成本会增加。选择封装不取消应用自身的 buffer、fd 和业务状态责任。

## 容易答错的点

| 说法                              | 修正                                         |
| --------------------------------- | -------------------------------------------- |
| enter 返回 1 就表示那次 I/O 成功  | 它描述提交消费结果，操作结果还要读 CQE       |
| CQE 错误从当前 errno 读取         | 异步错误在 res 中，按负错误码解释            |
| SQ head 前进后所有资源都能释放    | 描述已消费不代表数据操作已经完成             |
| 按提交顺序依次处理结果就能配对    | 独立请求可乱序完成，按 user_data 关联        |
| 一条 SQE 永远对应一条 CQE         | multishot 和跳过成功完成等选项会改变数量关系 |
| NODROP 表示无限完成缓冲           | 内部暂存仍占资源，极端内存不足仍可能丢失     |
| 取消 CQE 就是原请求最终结果       | 两项请求分别处理，原操作资源按其最终状态回收 |
| ring 共享内存使用户数据天然零拷贝 | 控制路径共享与数据路径是否复制是不同问题     |

## 性能分析

批量提交与批量消费可以分摊系统调用和同步成本，但增加批量也可能让早到请求多等一段时间。在途深度过大还会积压 buffer、请求上下文和完成事件，需要与内存预算、设备或连接承载能力一起控制。

SQPOLL 使用内核线程观察提交队列，活跃期间可减少提交调用；它进入休眠后可能需要显式唤醒，轮询也占用 CPU。IOPOLL 是另一类针对支持路径的完成轮询，不能把这两个选项当成通用提速开关。[轮询模式及条件](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_setup.2)

比较 epoll、普通系统调用和 io_uring 时，固定真实报文或块大小、并发连接、在途深度、CPU 放置和缓存条件，记录吞吐、CPU 占用、端到端 p50/p99/p99.9、提交批量及 CQ 积压。还要包括短读写、拥塞、取消和关闭路径。本例没有性能测量，NOP 完成只验证有限控制路径，不能代表网络或存储设备性能，也不证明用户数据零拷贝。

## Quant/Low-Latency 场景

在行情落盘、网关收发或记录重放中采用 io_uring，需要把 buffer 所有权和请求 ID 纳入状态机。入队、内核已消费、I/O 完成、业务消息完整以及对端确认，分别对应不同状态。发送 CQE 成功不能代替交易协议中的订单确认。

完成处理线程若长时间执行业务逻辑，CQ 可能积压。可以设置每轮完成处理预算和在途上限，把重工作交给后续阶段，同时保留 buffer 的借用与归还协议。取消或连接关闭时，原请求的完成仍须能找到有效上下文；代际 ID 与明确的终结计数有助于处理晚到完成。

迁移已有 epoll 路径时，先选择范围小且结果可核对的一段 I/O，保持部署能力探测和不支持时的后备路径。用实际负载验证改善来自批量、等待方式还是设备路径，避免把接口名称当成延迟目标已经实现。

## 相关专题

- [epoll 的 LT 与 ET](epoll-lt-et.md)：就绪通知模型，以及读写循环中的业务状态。
- [C++ 内存模型](../concurrency/cpp-memory-model.md)：发布、观察与槽位复用的顺序关系。
- [RAII 与异常安全](../cpp/raii-exception-safety.md)：局部资源释放和异步操作终结的不同边界。
- [NUMA 与 first-touch](../performance/numa-first-touch.md)：ring、buffer 与处理线程的放置及测量条件。

## 分层面试题

先区分“提交描述已被消费”和“原操作已经终结”，再说明由谁持有 buffer、fd 和请求上下文。性能题必须给出可比较的负载和完成处理条件。
