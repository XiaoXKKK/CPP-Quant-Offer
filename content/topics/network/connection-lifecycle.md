---
{
  'schemaVersion': 1,
  'id': 'connection-lifecycle',
  'title': '连接生命周期：状态、截止时间与陈旧事件',
  'description': '以单线程事件循环为拥有者，区分连接建立、读 EOF、输出排空和写半关闭，用代次句柄阻止 fd 复用后的陈旧回调。',
  'category': 'network',
  'areas': ['Network Programming', 'TCP/IP', 'epoll / io_uring'],
  'tags': ['connection-state', 'nonblocking-connect', 'shutdown', 'generation-handle', 'lifetime'],
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
  'prerequisites': ['非阻塞 socket 与 readiness', 'RAII 和回调对象寿命', 'TCP 收发方向与应用消息'],
  'related': ['epoll-lt-et', 'tcp-framing', 'raii-exception-safety', 'object-lifetime-layout'],
  'demo':
    {
      'file': 'examples/connection-lifecycle.cpp',
      'platform': 'linux',
      'exercise': '向模型添加一个迟到的工作线程结果事件，携带旧 Handle 和拥有的结果值；关闭并复用槽位后派发，断言新连接的 pending 不变。再让重复关闭请求使用更早的截止时间，验证它只能缩短等待。',
    },
  'references':
    [
      {
        'title': 'Linux connect(2)',
        'url': 'https://man7.org/linux/man-pages/man2/connect.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux socket(7): SO_ERROR',
        'url': 'https://man7.org/linux/man-pages/man7/socket.7.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux getsockopt(2)',
        'url': 'https://man7.org/linux/man-pages/man2/getsockopt.2.html',
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
        'title': 'Linux epoll(7): event cache lifetime',
        'url': 'https://man7.org/linux/man-pages/man7/epoll.7.html',
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
        'title': 'Linux shutdown(2)',
        'url': 'https://man7.org/linux/man-pages/man2/shutdown.2.html',
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
        'title': 'RFC 9293: TCP closing and half-closed connections',
        'url': 'https://www.rfc-editor.org/rfc/rfc9293.html',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'connection-lifecycle-l1-01',
        'level': 'L1',
        'prompt': '应用的 draining 状态为什么不能直接等同于 TCP 的 FIN_WAIT？',
        'answer': 'draining 描述应用停止接收新输出、等待本地待发内容排空的阶段，此时可能还没有调用 shutdown。TCP 状态描述内核协议状态，二者不是同一套枚举。应用还可能有登录、认证、请求确认等阶段，需要单独定义转换条件。',
        'rubric': ['应用状态与协议状态分开', 'draining 未必已半关闭', '业务阶段另行建模'],
        'source':
          {
            'kind': 'derived',
            'rationale': '对照应用排空步骤和 TCP 关闭机制，检查状态含义是否混淆。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l1-02',
        'level': 'L1',
        'prompt': '非阻塞 TCP connect 返回 EINPROGRESS 后，什么时候能进入 open？',
        'answer': '等待连接完成相关就绪，再成功调用 getsockopt 读取 SOL_SOCKET 的 SO_ERROR，值为零才可确认连接成功。可写可能伴随失败，不能直接当作已连接。如果 connect 立即返回零，则已经完成；其他失败应走相应清理和重连策略。',
        'rubric': ['EINPROGRESS 为进行中', '就绪后检查 SO_ERROR', '立即成功与失败分开'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据 connect 的非阻塞完成协议推导应用 open 状态的前置条件。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l1-03',
        'level': 'L1',
        'prompt': '读到 EOF 后，应用为什么可能还需要保留连接对象？',
        'answer': '对端关闭发送方向不自动禁止本端发送，协议可能允许最后一条响应。应用还可能持有待发字节、待确认任务或关闭计时器。本例先记录 read_eof，再按本地输出排空和写半关闭进度决定清理，不能仅凭读 EOF 销毁所有状态。',
        'rubric': ['区分两个方向', '保留协议允许的输出', '关闭依赖多项条件'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据半关闭和本地拥有资源推导 EOF 后对象仍可能存活的原因。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l1-04',
        'level': 'L1',
        'prompt': '本地输出队列为空，为什么不能宣称对端业务已经完成？',
        'answer': '输出队列为空只说明应用待提交的字节已经被本地发送操作接受，不代表网络交付或对端业务执行。内核可能仍有缓冲，业务也可能尚未消费。需要协议层确认、请求标识和恢复规则，才能判定具体操作结果。',
        'rubric': ['本地排空的准确含义', '内核及对端仍可能有工作', '业务确认独立'],
        'source':
          { 'kind': 'derived', 'rationale': '将应用缓冲排空与业务完成分离，推导结果确认边界。' },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l1-05',
        'level': 'L1',
        'prompt': '文件描述符 42 为什么不能永久代表同一条连接？',
        'answer': 'fd 是进程当前打开资源表中的编号，关闭后可被新资源复用。旧事件或异步结果如果只保存 42，可能误操作新连接。应用应使用稳定槽位和代次，派发前验证当前对象身份，fd 只作为该对象持有的系统资源。',
        'rubric': ['fd 编号可复用', '陈旧任务可能命中新资源', '用独立身份校验'],
        'source':
          {
            'kind': 'derived',
            'rationale': '结合 fd 复用和延迟事件推导应用身份不能只用整数编号。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l1-06',
        'level': 'L1',
        'prompt': '指定单线程事件循环拥有连接，能解决哪些生命周期问题？',
        'answer': '由同一个 owner 修改状态、执行 I/O 和关闭 fd，可让这些操作按事件顺序串行化，减少跨线程 close 与正在处理的回调互相冲突。其他线程只提交带身份的命令或拥有值的结果。不过陈旧事件和异步借用仍需检查，单线程不自动防止悬空引用。',
        'rubric': ['状态与资源变更串行化', '跨线程通过消息交接', '仍需处理陈旧事件与借用'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从状态操作的执行者推导单拥有者设计的收益与剩余风险。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l2-01',
        'level': 'L2',
        'prompt': 'getsockopt 返回零，是否就等价于 SO_ERROR 为零？',
        'answer': '不等价。系统调用返回值表示读取选项本身成功与否，具体连接错误写入输出参数。必须先检查调用成功，再检查输出的错误码。SO_ERROR 还会读取并清除挂起错误，因此连接 owner 应统一检查并保存原因，避免多个回调分别读取造成误解。',
        'rubric': ['调用结果与选项值分开', '两层错误检查', 'SO_ERROR 读清及统一记录'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据 getsockopt 与 SO_ERROR 的不同返回渠道推导正确完成检查。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l2-02',
        'level': 'L2',
        'prompt': '本例优雅关闭的转换条件是什么，何时拒绝新输出？',
        'answer': '收到关闭请求后从 open 进入 draining，立即拒绝新输出但继续发送已接受内容。pending 归零后调用写半关闭，成功进入 write_closed，并等待 read_eof。两个方向都结束才按 graceful 清理；任一阶段也可被错误或截止时间终止。',
        'rubric': ['进入 draining 即拒绝新写', '排空后写半关闭', 'EOF 与错误截止分支'],
        'source':
          {
            'kind': 'derived',
            'rationale': '按模型状态和动作顺序推导关闭中的入口限制与完成条件。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l2-03',
        'level': 'L2',
        'prompt': '一批 epoll 事件中前面的回调关闭了后面事件的连接，应怎样处理？',
        'answer': '后续事件必须再次验证对象是否仍存在、代次是否匹配，不能直接使用缓存的裸指针或 fd。移除内核关注项不等于清除了已返回到用户态的事件。关闭可先使逻辑身份失效，待当前借用结束再回收对象，后续命中旧身份的事件直接丢弃。',
        'rubric': ['用户态事件仍可能残留', '派发时重新验证身份', '失效与安全回收分开'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据 epoll 事件缓存的生命周期问题推导批内关闭后的派发规则。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l2-04',
        'level': 'L2',
        'prompt': '模型为什么等 apply 返回后才 reset connection？',
        'answer': 'apply 正在使用对象的成员和引用，若处理中就销毁自己，后续访问可能悬空。本例先在对象内部记录 closed，返回 owner 后再清理 optional。真实系统若允许可重入回调，还应延迟回收或保证借用令牌的有效期，不能只复制这一行 reset。',
        'rubric': ['当前调用仍借用对象', '逻辑关闭先于回收', '可重入场景需额外约束'],
        'source':
          { 'kind': 'derived', 'rationale': '从事件处理期间的对象借用推导两阶段关闭与回收时机。' },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l2-05',
        'level': 'L2',
        'prompt': '为什么重复关闭请求不能不断把截止时间改成现在加十秒？',
        'answer': '这样每次请求都延长等待，缓慢或失效连接可能永远不退出。应记录绝对截止时间，重复请求保留原值或取更早值，使用单调时钟计算剩余等待。模型只在 tick 事件到达时检查，调度迟延仍会使实际清理晚于该时间。',
        'rubric': ['防止重复延长期限', '绝对单调截止', '检查触发与调度限制'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据关闭请求重复到达和定时检查时机推导有限等待策略。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l2-06',
        'level': 'L2',
        'prompt': '给 handle 加 uint64_t generation 后，代次回绕问题是否就不存在？',
        'answer': '仍然存在。若计数绕回旧值而旧句柄仍可能到达，身份比较会再次通过。本例到最大代次时关闭后退休槽位，不再创建新对象。其他方案可以在证明所有旧引用已经排空后重建身份空间，但不能仅因位数很大就省略前提。',
        'rubric': ['有限代次会回绕', '旧句柄重合风险', '退休或可证明的重用条件'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从有限身份空间和迟到事件推导 generation 的耗尽边界。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l3-01',
        'level': 'L3',
        'prompt': '工作线程计算结果稍后返回，怎样避免把它写入已重连的新会话？',
        'answer': '任务携带稳定 Handle 和拥有的数据，完成后把命令投递回连接 owner，由 owner 重新校验槽位与代次并检查当前阶段。取消任务不保证已完成结果不会到达，旧 handle 仍要拒绝。任务若捕获原 Connection 的裸引用，generation 检查无法修复此前已经发生的悬空访问。',
        'rubric': ['结果回 owner 重新校验', '取消仍可能有迟到完成', '不能保留危险裸借用'],
        'source':
          {
            'kind': 'derived',
            'rationale': '将异步任务结果和重连代次结合，推导身份检查与数据拥有关系。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l3-02',
        'level': 'L3',
        'prompt': '关闭超时发生在部分请求字节发送后，业务应报告失败还是成功？',
        'answer': '应分别记录传输关闭原因、已提交和未提交字节，以及业务结果是否已确认。仅凭部分发送或超时不能确定远端执行情况，通常应保留未知结果并按协议查询或恢复。重试要用请求标识、去重或会话序列规则，不能把连接 timeout 等同于订单未执行。',
        'rubric': ['传输状态和业务结果分开', '保留执行未知状态', '按协议恢复和去重'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据关闭时发送进度的不完整信息推导交易执行结果的判定边界。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l3-03',
        'level': 'L3',
        'prompt': '怎样防止大量失败连接或长期 draining 耗尽资源？',
        'answer': '限制并发连接尝试、每连接和全局待发预算，为连接建立及关闭设置截止，并定义超时后丢弃与清理策略。重连还需退避或速率限制，避免失败时形成密集重试。应统计各阶段占用、关闭原因和丢弃量，区分网络失败与事件循环处理不及时。',
        'rubric': ['连接与缓冲预算', '截止和重连节奏', '按阶段与原因观测'],
        'source':
          { 'kind': 'derived', 'rationale': '把有限连接状态延伸到服务级资源预算和恢复负载。' },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l3-04',
        'level': 'L3',
        'prompt': 'shared_ptr 保证 Connection 没被释放，是否足够防止陈旧任务写错连接？',
        'answer': '只能解决部分内存存活问题。对象可能已经逻辑关闭，或者异步结果属于旧会话，即使内存存在也不应继续提交。还需检查代次、阶段和取消策略。引用计数也可能让已关闭对象被长期保留，应给异步任务和保留缓冲设置资源边界。',
        'rubric': ['存活与逻辑有效性分开', '会话身份和阶段校验', '长期保留的资源成本'],
        'source':
          {
            'kind': 'derived',
            'rationale': '比较对象内存寿命与连接逻辑寿命，推导引用计数不能代替状态校验。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l3-05',
        'level': 'L3',
        'prompt': '如何验证两秒连接超时，避免把测试时间当作调度硬保证？',
        'answer': '模型中注入绝对 tick，确定性检查截止前、截止点和重复请求，不靠 sleep 碰巧触发。真实非阻塞连接用单调时钟维护等待预算，EINTR 后按剩余时间重试。操作系统调度与定时粒度可能让实际返回更晚，需要分别记录配置截止和观测完成时间。',
        'rubric': ['假时间确定性边界', '真实等待保留绝对截止', '不宣称硬实时上界'],
        'source':
          {
            'kind': 'derived',
            'rationale': '区分逻辑时间测试和真实 poll 调度，推导超时验证的证据范围。',
          },
        'companies': [],
      },
      {
        'id': 'connection-lifecycle-l3-06',
        'level': 'L3',
        'prompt': '本章 loopback connect 成功与模型测试分别证明了什么？',
        'answer': 'loopback 部分实际创建 TCP socket、发起非阻塞连接，并检查 SO_ERROR，只验证本机连接建立路径。模型验证应用排空、读 EOF 顺序、错误、截止、假 fd 复用和代次退休，不执行这些关闭动作的系统调用。两者都没有证明业务协议或全部内核竞态正确。',
        'rubric': ['真实 syscall 的有限范围', '模型验证的状态不变量', '不扩大测试结论'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据示例各层执行内容审查测试证据与对外结论的一致性。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

连接需要一个明确拥有者，统一处理建立、I/O、关闭和回收。非阻塞 connect 不能见到可写就算成功，要检查 SO_ERROR。读 EOF 与写关闭分别记录，优雅关闭先拒绝新输出、排空待发数据，再关闭写方向并等待对端，整个过程有截止时间。事件和异步结果使用槽位加代次识别连接，避免 fd 复用后误操作新对象。

## 核心概念

内核 TCP 状态描述协议进度，应用状态还要管理输出队列、回调、认证和业务请求。`draining` 是本章的应用状态，表示停止接收新输出并等待已接受数据排空；它不等于某个 TCP FIN 状态。连接建立完成也只表示传输通道可用，应用握手或登录成功应有自己的条件。

本例规定单线程事件循环是 owner，只有它变更连接状态、使用或关闭 fd。工作线程若需要参与，只提交拥有值的结果和稳定 Handle，由 owner 处理。这样可串行化状态变化，但不能消除旧任务、旧事件或可重入回调带来的寿命问题。

fd、对象地址和应用 Handle 也有不同用途。fd 是当前系统资源编号，地址用于一次受控借用，Handle 是可重新验证的逻辑身份。本例 Handle 包含槽位和 generation；每次派发都核对槽内对象仍然存在，且代次相同。

## 原理深入

### 建立完成需要两层结果检查

非阻塞 TCP connect 立即返回零时连接完成；返回 `EINPROGRESS` 时保持 `connecting`，等待完成相关就绪，再读取 SO_ERROR。必须先检查 `getsockopt` 系统调用是否成功，再检查写入输出参数的错误值。就绪事件只提供检查机会，失败连接也可能产生可写或错误事件，见 [connect(2)](https://man7.org/linux/man-pages/man2/connect.2.html) 和 [getsockopt(2)](https://man7.org/linux/man-pages/man2/getsockopt.2.html)。

SO_ERROR 会取出并清除挂起错误，定义见 [socket(7)](https://man7.org/linux/man-pages/man7/socket.7.html)。由 owner 统一读取并记录结果，避免一个回调取走错误后，另一个回调把后续零值误解成原连接一直成功。若连接失败，关闭旧 socket，再按策略创建新的尝试；不要把失败后的 socket 状态当成可随意重用。

### 读方向结束与本地关闭计划分开

对于非零长度的流读取，收到 EOF 表示该接收方向的数据已结束，仍要检查应用解析器是否有截断帧，见 [recv(2)](https://man7.org/linux/man-pages/man2/recv.2.html)。它不自动禁止本端发送。本例协议允许在读 EOF 后准备最后响应，显式请求关闭后才停止接受新输出。

`begin_drain` 进入 `draining`，pending 归零后执行 `shutdown(SHUT_WR)`，成功后进入 `write_closed`。此时继续接收，直到 read_eof 为真或截止到达。读 EOF 先到和本地半关闭先到，两种顺序都必须处理；[shutdown(2)](https://man7.org/linux/man-pages/man2/shutdown.2.html) 与 [RFC 9293 §3.6.1](https://www.rfc-editor.org/rfc/rfc9293.html#section-3.6.1) 描述了两个方向的独立性。

本地 pending 归零只说明应用输出缓冲已交给本地发送路径。内核中可能仍有数据，对端也可能尚未执行业务。模型的 `sent` 事件表示一次已确认的本地字节进度，不是假设对端应用已经收到确认。

### 截止限制等待，不能代替调度

连接建立和关闭使用绝对截止时间。重复关闭请求最多缩短期限，不能每来一次请求就重新延长。真实等待采用单调时钟，EINTR 后重新计算剩余预算，不能从头再等完整超时。

模型只有收到 `tick` 事件才检查时间；owner 不处理事件时，状态不会自行清理。真实 poll 也受时间粒度和调度迟延影响，见 [poll(2)](https://man7.org/linux/man-pages/man2/poll.2.html)。因此“配置两秒等待预算”和“程序一定在两秒内完成关闭”是不同承诺，后者不是本例提供的保证。

## 数据结构/系统内部实现

模型的单槽 Owner 保存 `optional<Connection>`、当前代次和退休标记。Connection 保存 fd 模型编号、阶段、pending、read_eof、截止及结束原因。pending 最大为 32，所有增加先检查剩余预算，进度事件不能消耗超过现有 pending 的字节。

| 阶段           | 接受新输出     | 下一步或结束条件                                  |
| -------------- | -------------- | ------------------------------------------------- |
| `connecting`   | 否             | 检查过的连接结果决定 open 或错误；tick 可触发超时 |
| `open`         | 是，受容量限制 | 记录读 EOF；关闭请求进入 draining                 |
| `draining`     | 否             | 继续已有输出；归零后写半关闭，或错误/超时         |
| `write_closed` | 否             | 等待读 EOF，或错误/超时                           |
| `closed`       | 否             | 保存原因，回收对象，失效旧 Handle                 |

关闭动作先把对象标为 closed，关闭模型 fd 一次；`apply` 返回后 Owner 才 reset optional。它不会在正在执行的方法中销毁该对象。真实系统若允许用户回调重入，应进一步规定借用作用域和延迟回收，不能仅凭单线程就立即删除仍被调用栈引用的对象。

处理过一个事件后，同一批结果中可能仍有该连接的旧事件。[epoll(7)](https://man7.org/linux/man-pages/man7/epoll.7.html) 专门讨论事件缓存中的关闭问题。移除内核关注项和 close 不会把已经返回到用户态的条目凭空删除。派发时重新校验 Handle，可以拒绝旧事件和异步结果；若把已悬空的裸指针直接解引用，再去检查其中代次，顺序已经错了。

代次使用 uint64_t，仍是有限空间。本例在最大代次连接结束后将槽位退休，不回绕复用。正式系统可选择更换身份空间，或在确认所有旧任务和事件排空后重建，但需要相应证明。模型只模拟假 fd 42 的重复使用，不假设 Linux 必须按某个顺序分配编号。

## C++ runnable demo

代码包含两个分开的验证层：`model` 中的状态机、假时间事件和记录动作，以及本机 loopback 上的真实非阻塞 TCP connect。模型的 close、shutdown 只记次数，不调用系统；真实部分不执行代次复用或优雅排空流程。

```cpp include=examples/connection-lifecycle.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/connection-lifecycle.cpp -o /tmp/connection-lifecycle
/tmp/connection-lifecycle
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fno-omit-frame-pointer -fsanitize=address,undefined -fno-pie -no-pie examples/connection-lifecycle.cpp -o /tmp/connection-lifecycle-san
/tmp/connection-lifecycle-san
```

本机 WSL、GCC 13.3.0 的正常构建和 ASan/UBSan 均通过，输出 `lifecycle checks passed; stale handles rejected; generation retired; SO_ERROR=0`。断言是验证的一部分，编译时不要定义 `NDEBUG`。

模型确定性覆盖连接未完成时拒绝输出、容量上限、部分发送、两个方向关闭的两种顺序、关闭后拒绝新输出、重复请求不延长期限、连接错误、建立超时、排空超时、半关闭动作失败、重复事件、旧代次以及代次耗尽。超时例在十个待发字节中已推进三个，记录剩余七个被丢弃；它只描述本地进度，没有判断远端业务结果。

真实部分绑定 `127.0.0.1` 的临时端口，创建非阻塞 client；connect 若进行中，则以 steady_clock 维护两秒 poll 预算。完成提示之后读取 SO_ERROR，非零值或系统调用失败都会退出报错。两秒是用户态等待预算，调度可能让返回更晚。测试没有调用 accept 后的业务处理、没有外部网络，也未用真实 fd 复用验证 generation。

实际 fd 由不可复制 RAII 对象独占，离开作用域关闭一次。Linux close 的错误不做盲目重试，以免误关后来复用的编号，依据见 [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)。示例省略析构中的关闭错误日志；这项省略不改变应用应保存主要连接失败原因的要求。

## 高频追问

“已经取消工作任务，为什么还要检查 generation？”取消和完成可能交错，结果也可能已经排队。完成消息仍携带旧 Handle，owner 收到后必须验证。结果值应自行拥有数据；不要让工作线程在对象关闭后继续访问捕获的 `Connection&`。

“shared_ptr 能否替代 Handle？”它能延长内存寿命，却不保证对象仍处于允许输出的阶段，也不说明结果属于当前会话。即使拿到了强引用，仍要检查逻辑状态。长期保留强引用还可能延长缓冲占用，取消和资源预算需要另外规定。

“收到 HUP 就直接 close 可以吗？”HUP 可能与尚未读出的数据同时出现，不能跳过必要读取和 EOF/解析检查。连接策略也可能要求完成已有响应。应结合实际 I/O 结果、应用协议和关闭阶段决定动作，而不是只根据一个事件位丢弃全部状态。

“优雅关闭超时后，是不是应该无限等到 pending 归零？”截止的目的就是限制资源占用。到期后按协议和业务策略终止，记录尚未提交的字节及未确认请求。若业务不允许简单丢弃，应在上层保存可恢复状态，而不是让网络连接永久阻塞退出。

## 容易答错的点

- 可写只触发连接完成检查，SO_ERROR 才给出对应结果；读取选项成功不等于连接成功。
- 读 EOF 与写方向可用性分开；应用允许最后响应是协议选择，不是所有服务必须采用的策略。
- pending 为零描述本地输出进度，不提供对端业务完成保证。
- `epoll_ctl(DEL)` 不清除已经返回的用户态事件，派发仍需验证对象身份。
- 代次比较要发生在取得安全对象访问之前，不能先解引用已失效指针。
- 大位宽只使代次耗尽更少见，不能把有限计数写成永不回绕。

## 性能分析

本例模型只有一个槽，身份查询和单次状态转换是有界常数工作；实际连接表、计时器和输出缓冲的成本取决于数据结构。把大量连接都集中在同一个 owner 可能降低同步复杂度，也可能让慢回调推迟所有连接的关闭检查，需要限制每轮工作量。

观测应包括各阶段连接数量、pending 字节、建立失败和关闭原因、超时后丢弃量、陈旧事件拒绝次数，以及截止到实际清理的延迟。报告建立和关闭延迟的分位数，控制连接尝试速率、对端读取速度、网络环境及事件循环负载。仅测一条空闲 loopback 连接，无法推导繁忙服务的关闭尾延迟。

连接失败时的重连也消耗 fd、内存和事件处理预算。设置并发尝试上限、重连退避和全局待发上限，测量失败风暴下是否仍能及时处理已有连接。该示例没有性能计时数据，真实 loopback 只作功能核查。

## Quant/Low-Latency 场景

交易会话断开时，网络层应报告建立失败、I/O 错误、正常方向关闭或关闭超时，并保留发送进度和会话身份。业务层再区分哪些请求未发送、哪些已确认，以及哪些结果未知。不能把 timeout 直接映射成“订单未成交”，也不能无条件重发所有已提交请求。

来自风控、持久化或路由工作线程的迟到结果可能属于上一条会话。结果携带请求身份、会话代次和拥有数据，投递回 owner 后校验；重连后的新 fd 不能自动继承旧任务的输出权限。取消只限制后续工作，不能抹去已经到达对端的请求。

进程退出时先停止接受新业务，再推动各连接排空，并用共同停机预算约束全部等待。到期后记录未确认业务并关闭资源，持久化或恢复机制接管剩余责任。等待网络“看起来安静”不能代替明确的停机状态和业务确认。

## 相关专题

- [epoll：LT vs ET](epoll-lt-et.md)：理解 readiness、事件缓存和单轮处理预算。
- [TCP framing](tcp-framing.md)：把接收 EOF 与解析器截断状态联系起来。
- [RAII 与异常安全](../cpp/raii-exception-safety.md)：管理 fd 和错误路径中的唯一拥有者。
- [对象生命周期与布局](../cpp/object-lifetime-layout.md)：区分对象仍存活、借用有效和存储已回收。

## 分层面试题

L1 说明连接状态和各层完成条件；L2 跟踪同一事件批次中的身份、借用与回收；L3 处理重连、迟到任务和未知业务结果。设计评审应同时写出成功路径、截止触发者及资源最终由谁释放，避免让“关闭连接”承担未定义的多层含义。
