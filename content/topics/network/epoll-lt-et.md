---
{
  'schemaVersion': 1,
  'id': 'epoll-lt-et',
  'title': 'epoll：LT vs ET',
  'description': '从 readiness 到 drain-to-EAGAIN，理解 LT/ET、非阻塞 I/O、公平调度与连接生命周期。',
  'category': 'network',
  'areas': ['epoll / io_uring', 'Network Programming'],
  'tags': ['epoll', 'linux', 'reactor'],
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
  'updated': '2026-09-18',
  'reviewed': '2026-09-18',
  'standard': 'C++20',
  'estimatedMinutes': 35,
  'prerequisites': ['文件描述符与 socket', '非阻塞 read/write', 'TCP 字节流'],
  'related': ['cpp-memory-model', 'order-book'],
  'demo':
    {
      'file': 'examples/epoll-lt-et.cpp',
      'platform': 'linux',
      'exercise': '把单连接实验改成两个连接：为每轮设置字节预算，在用户态 ready queue 保留未 drain 的连接，验证热连接不会饿死冷连接。',
    },
  'references':
    [
      {
        'title': 'Linux epoll(7)',
        'url': 'https://man7.org/linux/man-pages/man7/epoll.7.html',
        'kind': 'manual',
        'accessed': '2026-09-18',
      },
      {
        'title': 'Linux epoll_ctl(2)',
        'url': 'https://man7.org/linux/man-pages/man2/epoll_ctl.2.html',
        'kind': 'manual',
        'accessed': '2026-09-18',
      },
      {
        'title': 'Linux read(2)',
        'url': 'https://man7.org/linux/man-pages/man2/read.2.html',
        'kind': 'manual',
        'accessed': '2026-09-18',
      },
    ],
  'questions':
    [
      {
        'id': 'epoll-lt-et-q01',
        'level': 'L1',
        'prompt': 'LT 和 ET 最关键的区别是什么？',
        'answer': 'LT 在条件持续满足时可以重复报告；ET 不能依赖重复通知来处理残留数据。应结合非阻塞 I/O、耗尽或用户态待处理队列说明完整处理策略。',
        'rubric': ['持续就绪', '残留数据处理'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q02',
        'level': 'L1',
        'prompt': 'epoll 通知能保证 read 一定成功吗？',
        'answer': '不能。通知与 read 之间可能有其他消费者读走数据，或状态发生变化。描述符使用非阻塞模式，依然处理 EAGAIN、EINTR、EOF 和错误。',
        'rubric': ['就绪不是承诺', '非阻塞错误路径'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q03',
        'level': 'L1',
        'prompt': 'EAGAIN 与 read 返回 0 如何区分？',
        'answer': '对非阻塞 stream socket，EAGAIN 表示暂时无数据，保留连接；read 为 0 表示接收方向到达 EOF。是否继续发送或关闭由协议与半关闭策略决定。',
        'rubric': ['暂时耗尽', '半关闭'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q04',
        'level': 'L1',
        'prompt': 'EPOLLET 是否自动设置 O_NONBLOCK？',
        'answer': '不会，事件通知方式与 fd 的阻塞属性是两个维度。创建 socket 时或通过 fcntl 设置非阻塞，并对所有 I/O 分支正确处理。',
        'rubric': ['不同维度', '显式设置'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q05',
        'level': 'L1',
        'prompt': '一次 EPOLLIN 能对应几个应用消息？',
        'answer': '没有固定对应关系。TCP 是字节流，应用缓冲可能包含半个、一个或多个消息，必须按协议长度字段或边界进行增量解析。',
        'rubric': ['字节流', '增量分帧'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q06',
        'level': 'L2',
        'prompt': 'ET 只读一半后继续 epoll_wait 会怎样？',
        'answer': '在没有新事件的受控条件下，剩余可读数据可能不再触发通知，处理停滞。继续非阻塞读取到 EAGAIN，或把连接保留在用户态 ready queue。',
        'rubric': ['限定条件', '保留待处理状态'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q07',
        'level': 'L2',
        'prompt': '如何避免 EPOLLOUT 形成忙循环？',
        'answer': '有待发送数据时先尝试写，短写保存偏移，EAGAIN 后关注可写。队列排空后取消 EPOLLOUT，新数据入队应主动唤醒 reactor。',
        'rubric': ['短写偏移', '按需关注'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q08',
        'level': 'L2',
        'prompt': 'EPOLLONESHOT 能替代线程同步吗？',
        'answer': '不能。它仅禁用一次通知后的关注项，需要 MOD 重新 arm。对象访问、工作移交、关闭以及重 arm 的顺序仍需所有权和同步协议。',
        'rubric': ['重新arm', '对象生命周期'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q09',
        'level': 'L2',
        'prompt': '事件到达时发现 fd 已被复用怎么办？',
        'answer': '使用连接 token 或 generation 校验事件归属，并保证校验前不解引用已回收对象。仅比较整数 fd 不够，异步任务需要取消或延迟回收策略。',
        'rubric': ['代数校验', '避免悬空引用'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q10',
        'level': 'L2',
        'prompt': '收到 HUP 时为什么不能立刻丢缓冲区？',
        'answer': 'HUP 可以与仍可读取的数据同时存在。先处理可读数据及完整协议消息，再识别 EOF 并执行连接收尾，避免丢掉最后一段响应。',
        'rubric': ['排空数据', '协议收尾'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q11',
        'level': 'L3',
        'prompt': '热行情连接持续有数据，ET 如何保证公平？',
        'answer': '给每轮连接设置消息、字节或时间预算。耗尽预算但尚未到 EAGAIN 的连接留在用户态就绪队列，轮转服务其他连接后再继续。',
        'rubric': ['处理预算', 'ready queue'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q12',
        'level': 'L3',
        'prompt': '如何公平比较 LT 与 ET 的 p99？',
        'answer': '控制协议、消息分布、CPU 绑定和连接负载，记录尾延迟与系统调用次数；负载生成器不能因服务停顿而降低发包量，否则出现 coordinated omission。',
        'rubric': ['实验控制', '避免遗漏延迟样本'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q13',
        'level': 'L3',
        'prompt': '多线程读取同一行情 socket 有什么风险？',
        'answer': '读取竞争会打乱分帧状态和业务事件归属，即使每次 read 本身安全也不代表解析安全。常见方案是单 reactor 拥有流解析，按产品或任务转交完整事件。',
        'rubric': ['流解析所有权', '事件分发'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q14',
        'level': 'L3',
        'prompt': 'epoll 返回很多事件时如何限制尾延迟？',
        'answer': '批量可以摊薄系统调用，但需要限制单批耗时和单连接预算，并监测积压。极大批次可能提高吞吐却延迟心跳与订单控制消息。',
        'rubric': ['批大小权衡', '控制流延迟'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'epoll-lt-et-q15',
        'level': 'L3',
        'prompt': '从 epoll 接收到行情到订单簿有效还缺什么？',
        'answer': '还需协议解析、会话校验、序号连续性、去重和快照衔接。read 成功只说明收到字节，不说明业务数据完整；gap 时应标记簿无效并恢复。',
        'rubric': ['传输与业务分层', 'gap恢复'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

epoll 报告 I/O 就绪，不是完整消息。LT 在持续就绪时可重复通知；ET 不能依赖新通知处理残留数据。通常用非阻塞 I/O 读写到 EAGAIN；若为公平性提前停下，须在用户态保留待处理连接。ET 不天然更快，还要正确处理短写、EOF 与连接生命周期。

## 核心概念

| 概念      | 精确定义                      | 工程含义                   |
| --------- | ----------------------------- | -------------------------- |
| readiness | 现在尝试 I/O 可能无需阻塞     | 多线程竞争后仍可能 EAGAIN  |
| LT        | 持续就绪时可重复通知          | 可以分批读，但仍应非阻塞   |
| ET        | 就绪通知采用边沿触发语义      | 不要假设“一条消息一个事件” |
| EAGAIN    | 当前无法继续推进              | 不是连接断开               |
| EOF       | 对 stream socket，read 返回 0 | 处理半关闭并按协议决定收尾 |

EPOLLIN、EPOLLOUT 是不同的推进方向。TCP 分帧、半包/粘包属于应用协议解析层，不由 epoll 修复。

## 原理深入

考虑没有并发读者、没有新写入的受控实验：对端先写入 6 字节，接收端收到事件后只读 3 字节。LT 再次等待会看到仍可读；ET 此时不能期待新的通知，剩余 3 字节可能长期滞留。下面的实验将这个条件固定，避免用含糊的“ET 永远只通知一次”解释。

生产循环应区分：正数表示推进；EINTR 重试；EAGAIN/EWOULDBLOCK 表示暂时耗尽；0 表示流结束；其他错误进入连接失败路径。收到 HUP 时也不能直接丢弃缓冲区，先处理可读字节，再收尾。

写侧先尝试发送，短写后保存偏移；到 EAGAIN 才订阅 EPOLLOUT，发送队列清空后取消写关注，否则通常可写的连接会产生无用唤醒。业务线程新加入待发数据时，需唤醒 reactor，不能只等待不存在的新写边沿。具体通知语义以 [epoll 手册](https://man7.org/linux/man-pages/man7/epoll.7.html) 为准。

## 数据结构/系统内部实现

用户态可把连接组织为 `fd → Connection`，保存输入缓冲、解析偏移、输出队列、生命周期代数和 ready 标志。一个简化控制流是：

```text
epoll_wait → 合并到 ready queue → 按预算推进 I/O → 解析完整帧
                                  ├─ EAGAIN：移出 ready queue
                                  └─ 预算耗尽：保留，下轮继续
```

Linux 的 eventpoll 实现维护关注集合与就绪集合；内核内部结构是实现细节。不要把 epoll 简化成“所有操作 O(1)”：注册/删除、唤醒、多核竞争和返回 k 个事件都有成本。EPOLLONESHOT 会在通知后禁用该关注项，处理完需要 MOD 重新 arm；它不是自动的连接所有权或内存回收方案。

## C++ runnable demo

Linux C++20，使用非阻塞 UNIX stream socketpair 排除外部网络噪声。它验证 LT/ET 的部分读取差异、drain 和 EOF，不实现 TCP 协议栈或生产 reactor。

```cpp include=examples/epoll-lt-et.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pthread examples/epoll-lt-et.cpp -o /tmp/epoll-demo
/tmp/epoll-demo
```

预期 LT 的第二次事件数为 1，ET 为 0；二者均读出剩余 def，并确认 EOF。在线沙箱可能限制 epoll/socketpair；遇到系统调用限制，应在 Linux 或 WSL 运行。

## 高频追问

“每次 drain 到 EAGAIN 是否会饿死其他连接？”会，持续输入可能长期占住线程。以字节数、消息数或时间为预算，ET 提前停止时将连接留在应用 ready queue，直到耗尽才回到只依赖内核通知的状态。

“fd 关闭后被复用怎么办？”不要只按整数 fd 识别异步任务；事件关联连接 token/代数，校验生命周期，再访问对象。跨线程移交还需要同步协议，可接着学习 [C++ memory model](../concurrency/cpp-memory-model.md)。

## 容易答错的点

- ET 不等于非阻塞；EPOLLET 并不会设置 O_NONBLOCK。
- EAGAIN 不等于 EOF；对端半关闭也不一定代表本地不能继续发送。
- 多个消息可以合并成一次就绪通知，一次消息也可能要多次 read。
- 不能承诺 ET 比 LT 快一个固定倍数，更不能用连接总数推算尾延迟。
- ONESHOT 重新 arm 和已关闭对象的销毁必须纳入同一生命周期设计。

## 性能分析

比较时保持消息大小分布、连接数、负载发生器和解析逻辑一致，记录 read/write/epoll_wait 次数、每轮事件数、每连接排队时间、吞吐、p50/p99/p99.9。分别测稀疏连接、热点连接和突发流量。负载发生器要记录计划发送时间与实际完成时间，避免系统阻塞时少发请求造成 coordinated omission。

预分配缓冲可以减少分配抖动，但不会消除内核唤醒、调度或协议解析的开销。这个 demo 证明语义，不提供 LT/ET 性能排名。

## Quant/Low-Latency 场景

行情网关可能同时承载快照下载、增量数据和控制连接。热行情连接必须有处理预算；否则登录、心跳与重传请求会被延后。feed handler 还需要独立的 sequence gap 检测，即使 socket 一直可读也不能宣称订单簿有效。TCP 字节流解析完成后，再把事件交给 [Order Book](../trading/order-book.md)。

## 相关专题

- [C++ memory model](../concurrency/cpp-memory-model.md)：reactor 与业务线程发布数据。
- [Order Book](../trading/order-book.md)：从完整消息到确定性状态更新。
- 后续规划：TCP framing、io_uring completion、连接生命周期；未发布专题不生成空文章。

## 分层面试题

下方题库每层 5 题，均为岗位知识推导题。先写答案，再展开参考要点；不能把这些题标成某交易公司的真题。
