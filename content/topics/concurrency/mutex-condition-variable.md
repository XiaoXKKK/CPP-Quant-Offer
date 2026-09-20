---
{
  'schemaVersion': 1,
  'id': 'mutex-condition-variable',
  'title': 'mutex 与 condition_variable：等待、关闭与排空',
  'description': '从谓词和同一把 mutex 的协议解释条件变量，用有界队列检查空满等待、绝对超时、关闭排空及线程异常后的生命周期。',
  'category': 'concurrency',
  'areas': ['C++ Memory Model', 'Multithreading / Atomic / Lock-Free'],
  'tags': ['mutex', 'condition-variable', 'bounded-queue', 'shutdown', 'deadline'],
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
  'prerequisites': ['RAII 和对象生命周期', '数据竞争与 happens-before', '线程创建与 join'],
  'related':
    ['cpp-memory-model', 'shared-mutex', 'raii-exception-safety', 'object-lifetime-layout'],
  'demo':
    {
      'file': 'examples/mutex-condition-variable.cpp',
      'platform': 'portable',
      'exercise': '保留关闭后排空的契约，增加第二个生产者；为每项分配唯一 ID，在 join 后验证每个已接受 ID 恰好出现一次，不根据线程启动顺序断言出队顺序。',
    },
  'references':
    [
      {
        'title': 'C++20 N4861: mutex requirements and synchronization',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/thread.mutex.requirements.mutex',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: unique_lock ownership',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/thread.lock.unique',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: condition variable operations and total order',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/thread.condition',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: condition_variable waits, predicates and destruction',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/thread.condition.condvar',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: timing specifications',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/thread.req.timing',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: steady_clock',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/time.clock.steady',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: data races and happens-before',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/intro.races',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: jthread construction and destruction',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/thread.jthread.cons',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: jthread join synchronization',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/thread.jthread.mem',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: condition_variable_any interruptible waits',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/thread.condvarany.intwait',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: thread destructor',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/thread.thread.destr',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'mutex-condition-variable-q01',
        'level': 'L1',
        'prompt': 'mutex 和 condition_variable 在阻塞队列里分别承担什么职责？',
        'answer': 'mutex 保护队列内容、索引、数量和关闭状态的并发访问，并通过解锁与后续成功加锁建立同步。condition_variable 让线程在当前状态不满足要求时等待通知或超时。是否可以入队或出队由受保护状态上的谓词决定，条件变量不替代互斥访问。',
        'rubric': ['共享状态由 mutex 保护', '等待和可操作状态区分', '解锁与加锁提供同步'],
        'source':
          { 'kind': 'derived', 'rationale': '根据阻塞队列中互斥访问与等待机制的分工设计基础题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q02',
        'level': 'L1',
        'prompt': 'condition_variable::wait 为什么接收 unique_lock，而不是 lock_guard？',
        'answer': 'wait 需要释放 mutex 并进入等待，唤醒后重新获取 mutex，再返回给调用者。unique_lock 提供解锁、重新加锁及所有权状态，lock_guard 没有这些操作。调用前当前线程必须通过该 unique_lock 持有锁；同时在同一个 condition_variable 上等待的线程还必须使用同一 mutex。',
        'rubric':
          [
            '等待时释放并重新获取锁',
            'unique_lock 的所有权管理',
            '同一条件变量并发等待的 mutex 前提',
          ],
        'source':
          { 'kind': 'derived', 'rationale': '根据 wait 的参数与锁所有权前提设计接口理解题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q03',
        'level': 'L1',
        'prompt': '收到通知后为什么还要在锁内重新检查谓词？',
        'answer': '等待可能虚假唤醒；即使有真实通知，另一个消费者也可能先取得 mutex 并取走数据。通知只让等待线程有机会继续，不能替它保留一个元素。谓词重检与后续取走操作必须处于同一临界区，可使用 wait 的谓词重载完成循环。',
        'rubric': ['虚假唤醒', '真实通知后状态也可能改变', '重检和消费保持同一临界区'],
        'source':
          { 'kind': 'derived', 'rationale': '根据唤醒和获得资源之间的竞争设计谓词循环题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q04',
        'level': 'L1',
        'prompt': '没有线程等待时调用 notify_one，后来的 wait 能收到这次通知吗？',
        'answer': '条件变量不为未来等待者保存一次通知。若生产者先在锁内入队，后来的消费者在同一锁内能看到非空状态，就不需要等待。需要保存的是队列内容、计数或状态；不能把通知次数当成已经积累的事件数。',
        'rubric': ['通知不存储给未来等待者', '持久状态保留工作', '先检查谓词再决定等待'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 notify 的当前等待者语义设计事件存储误区题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q05',
        'level': 'L1',
        'prompt': 'notify_one 和 notify_all 分别保证什么，不保证什么？',
        'answer': 'notify_one 在有阻塞等待者时解除其中一个的等待，notify_all 解除所有当前等待者的等待。两者都不保证线程立即运行、按 FIFO 获取 mutex，或醒来时业务谓词仍为真。逐项入队可唤醒一个消费者；关闭影响所有等待者，通常要通知所有相关等待组。',
        'rubric':
          ['解除一个或所有当前等待者', '没有立即运行或公平顺序保证', '关闭需要覆盖所有相关等待组'],
        'source':
          { 'kind': 'derived', 'rationale': '根据通知范围与调度保证的区别设计基本使用题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q06',
        'level': 'L1',
        'prompt': '示例中队列关闭后还有元素，pop 应当返回 closed 吗？',
        'answer': '不应当。该队列的契约是关闭后拒绝新 push，已经接受的元素仍可出队。消费者等待 closed 或非空，返回后先检查是否有元素；只有队列已关闭且为空才返回 closed。若一看到 closed 就退出，尚未排空的元素会被留下。',
        'rubric': ['关闭拒绝新入队', '已接受元素继续排空', '关闭且为空才结束消费'],
        'source': { 'kind': 'derived', 'rationale': '根据队列的关闭与排空状态机设计退出条件题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q07',
        'level': 'L2',
        'prompt': '生产者写入数组后，消费者读取元素的 happens-before 链是什么？',
        'answer': '生产者在 mutex 内写元素并更新数量，然后解锁；该解锁与消费者之后成功获得同一 mutex 的加锁操作同步。消费者随后在锁内读取元素，因此写入通过这条链先行于读取。notify 用于推动等待者继续，不能单独替代共享数据所需的同步协议。',
        'rubric': ['写入先于解锁', '同一 mutex 解锁同步到后续成功加锁', '读取在加锁之后'],
        'source':
          { 'kind': 'derived', 'rationale': '根据互斥锁的发布关系设计可画出访问顺序的机制题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q08',
        'level': 'L2',
        'prompt': '把 ready 改成 atomic，生产者在锁外写 ready 并通知，是否就不会丢唤醒？',
        'answer': '原子访问可以消除该标志上的数据竞争，但消费者可能先读到 false，生产者随后在消费者实际等待前写 true 并通知，消费者最后才进入等待。没有后续通知时它可能一直等待。即使使用顺序一致原子也没有把检查与进入等待连成协议；应让状态修改参与同一 mutex 协议，或另行证明所选等待机制。',
        'rubric':
          ['原子性不等于等待协议', '检查与进入等待之间的时序窗口', '同锁协议或另行证明机制'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据无数据竞争仍可能丢失进展的时序设计原子标志追问。',
          },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q09',
        'level': 'L2',
        'prompt': '修改状态后应先解锁再 notify，还是持锁 notify？',
        'answer': '在状态按同一 mutex 协议修改且对象寿命充足时，两种顺序都可正确。解锁后通知可减少被唤醒线程立即竞争仍被持有的锁的机会，但不能保证更快。持锁通知也合法；若解锁后对象可能被销毁，必须先解决寿命问题，不能对已经销毁的条件变量调用 notify。',
        'rubric':
          ['两种顺序可正确且状态更新在锁内', '锁竞争收益不是性能保证', '解锁后通知需要对象仍存活'],
        'source':
          { 'kind': 'derived', 'rationale': '根据通知位置与对象寿命的独立条件设计实现取舍题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q10',
        'level': 'L2',
        'prompt': '循环中反复 wait_for(lock, 100ms) 为什么可能超出总等待预算？',
        'answer': '若每次醒来发现条件不满足又重新等待完整 100ms，先前已经等待的时间没有扣除，总预算就会被不断重置。应先计算 steady_clock 的绝对 deadline 并在整个操作中复用，或直接使用一次带谓词的 wait_for。即便如此，重获锁与调度也可能使函数晚于 deadline 返回。',
        'rubric':
          ['手写循环重置相对预算', '复用绝对 deadline 或一次谓词 wait_for', '返回时间不是实时上界'],
        'source':
          { 'kind': 'derived', 'rationale': '根据相对超时循环和绝对截止点设计预算管理题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q11',
        'level': 'L2',
        'prompt': '带谓词的 wait_until 返回 true，能证明操作在 deadline 前完成吗？',
        'answer': '不能。返回 true 表示最终谓词为真；即便截止点已过，初始谓词为真时也可以直接继续，超时唤醒后还会在持锁时最后检查谓词。示例的 deadline 限制等待条件的过程，并非严格禁止过期提交。若业务要求过期拒绝，需要在提交点另加时钟检查并明确边界语义。',
        'rubric':
          ['true 描述谓词而非准时完成', '过期后谓词为真仍可成功', '严格提交截止需额外业务规则'],
        'source':
          { 'kind': 'derived', 'rationale': '根据谓词超时重载的返回值设计业务截止边界题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q12',
        'level': 'L2',
        'prompt': 'jthread 析构会 request_stop，普通 condition_variable 等待会自动醒来吗？',
        'answer': '不会，普通 condition_variable 的 wait 没有 stop_token 参数，停止请求也不会自动改变队列的关闭状态。C++20 condition_variable_any 有接收 stop_token 的等待重载，但醒来后仍需区分谓词满足和停止。要保证排空，不能让消费者仅因为收到停止请求就跳过已接受的数据。',
        'rubric':
          [
            '普通条件变量不自动接收停止请求',
            'condition_variable_any 的可中断重载',
            '停止与排空契约必须协调',
          ],
        'source':
          { 'kind': 'derived', 'rationale': '根据协作停止与阻塞等待的不同接口设计生命周期追问。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q13',
        'level': 'L3',
        'prompt': '生产者和消费者共用一个条件变量，再只调用 notify_one，有什么进展风险？',
        'answer': '同一个等待组中可能有等待非满的生产者和等待非空的消费者，notify_one 不根据业务谓词挑选线程。被唤醒者的条件可能仍为假，它再次睡眠，而本可推进的另一类线程继续阻塞。可使用 not_full 和 not_empty 分组通知，或在适当状态变化时 notify_all 并接受额外竞争；仍不能承诺公平性。',
        'rubric': ['notify_one 不按谓词选线程', '错误等待组会失去进展', '分离条件变量或广播的成本'],
        'source':
          { 'kind': 'derived', 'rationale': '根据多类等待谓词和选择性通知设计进展性分析题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q14',
        'level': 'L3',
        'prompt': '把示例的 int 改成任意 T 时，为什么不能直接承诺同样的异常保证？',
        'answer': 'T 的构造、移动或输出赋值可能抛异常；例如出队时先把元素移动坏了再抛出，数量即使没变也未必还能恢复原值。需要明确元素的异常约束与提交顺序，或用句柄转移避免复杂对象在锁内移动。持锁 RAII 只保证解锁，不自动保证队列内容回滚，也应避免锁内调用任意用户代码。',
        'rubric':
          [
            '抛出移动或赋值可改变元素',
            '资源释放与事务回滚不同',
            '类型约束或句柄方案及锁内代码风险',
          ],
        'source':
          { 'kind': 'derived', 'rationale': '根据教学整数队列泛化时的异常边界设计工程题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q15',
        'level': 'L3',
        'prompt': '生产者异常与消费者异常，为什么需要不同的数据处置判断？',
        'answer': '生产者停止新增后，健康消费者仍可把已接受数据排空；示例验证了这一点。消费者自身失败时，close 能阻止新增并唤醒其他线程，却不能证明剩余数据已处理。生产系统需保留剩余项、记录已处理位置并报告失败，按业务契约重试或接管；不能把 close 或 join 成功当作业务处理完成。',
        'rubric':
          [
            '健康消费者可排空生产者已提交数据',
            '消费者失败不等于已处理',
            '剩余数据和处理进度需有明确处置',
          ],
        'source':
          { 'kind': 'derived', 'rationale': '根据不同故障角色对已接受数据的影响设计关闭协议题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q16',
        'level': 'L3',
        'prompt': '如何测试队列关闭竞争，而不依赖 sleep 后线程一定已经进入 wait？',
        'answer': '让关闭与操作任意竞争，检查两种顺序都应满足的契约；例如没有触发 watchdog 时，空队列 pop 与 close 竞争应返回 closed。用确定状态验证空、满、回绕和排空，用唯一 ID 检查接受与消费的对应关系。watchdog 只帮助报告可能卡住，压力测试和 sanitizer 无报告也不能代替协议证明；若要证明某路径实际被覆盖，需要额外可观察的同步点。',
        'rubric':
          ['结果覆盖合法时序而非猜调度', '边界状态与逐项核对', 'watchdog 和动态检测的证据范围'],
        'source':
          { 'kind': 'derived', 'rationale': '根据并发测试的可观察证据设计避免调度假设的验收题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q17',
        'level': 'L3',
        'prompt': '用 mutex 队列承接行情突发，容量有界就能保证延迟有界吗？',
        'answer': '容量限制内存和积压项数，但生产者可能阻塞，锁竞争和操作系统调度也没有由该队列给出的确定上界。std::mutex 不保证 FIFO 公平性。应根据上游是否允许背压制定满队列策略，记录排队时间、等待分布与超时；无法丢失的增量流还需要故障或恢复流程，不能静默丢弃后继续声称状态有效。',
        'rubric': ['有界容量不等于有界延迟', '公平性和调度没有确定保证', '背压或恢复策略与测量'],
        'source':
          { 'kind': 'derived', 'rationale': '根据突发行情负载与阻塞队列成本设计容量取舍题。' },
        'companies': [],
      },
      {
        'id': 'mutex-condition-variable-q18',
        'level': 'L3',
        'prompt': 'close 后立刻销毁队列，为什么即使已经 notify_all 也不够？',
        'answer': '被唤醒线程可能仍在重新获取 mutex，之后还会检查状态、读取元素或再次等待；通知者也可能尚未完成解锁后的 notify。完整队列及其状态必须活到这些访问结束。可先阻止新调用、关闭并通知，再 join 所有使用线程，最后销毁对象；条件变量析构的最低前提不能替整个队列的寿命作保证。',
        'rubric':
          ['通知后仍有对象访问', '停止新调用并 join 再销毁', '区分条件变量最低前提与容器整体寿命'],
        'source':
          { 'kind': 'derived', 'rationale': '根据唤醒与线程退出之间的访问区间设计对象销毁题。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

mutex 保护共享状态，condition_variable 让线程在状态不满足要求时释放锁并等待，醒来后重新获得锁。等待应检查谓词，状态更新也应参与同一 mutex 协议，这样才能把“检查条件”和“进入等待”接起来。通知不保存事件，醒来不代表一定有数据。队列还需要定义关闭语义：示例拒绝新入队，保留已接受的数据供消费者排空，所有线程退出后才销毁对象。

本文以 C++20 为准，示例只使用标准库，验证环境为 Linux 上的 GCC 13.3。互斥锁和条件变量不提供 FIFO 公平性，也不保证等待在某个时间内完成。

## 核心概念

| 对象或状态    | 在本例中的职责                 | 使用前提                                 |
| ------------- | ------------------------------ | ---------------------------------------- |
| `mutex_`      | 保护元素、索引、数量和关闭状态 | 相关访问都持有这把锁                     |
| `unique_lock` | 管理等待期间的解锁与重锁       | 调用 wait 时由当前线程持锁               |
| `not_empty_`  | 等待非空或关闭                 | `closed_` 为真或 `size_` 非零            |
| `not_full_`   | 等待有空间或关闭               | `closed_` 为真或 `size_` 小于 `Capacity` |
| `closed_`     | 拒绝后续入队，允许已有项出队   | 在锁内从 false 变为 true，不重新打开     |

`condition_variable` 的等待函数接收 `unique_lock<mutex>`。同时在同一个条件变量上等待的线程必须使用同一个 mutex；`lock_guard` 不提供等待所需的显式解锁与重锁接口。[unique_lock](https://timsong-cpp.github.io/cppwp/n4861/thread.lock.unique)、[条件变量前提](https://timsong-cpp.github.io/cppwp/n4861/thread.condition.condvar)

条件变量不记录业务数据或累计通知次数。生产者先完成入队而消费者尚未开始等待时，数据仍在队列中。消费者取得锁后发现非空便直接取走，无须补领一次通知。

## 原理深入

### 从状态检查进入等待

一次 wait 包含“释放 mutex 并进入等待”“解除等待”“重新获得 mutex”三个阶段，其中释放与进入等待是一个原子部分。每个条件变量上的通知及等待阶段按一个与 happens-before 一致的未指定全序发生。这不表示所有共享内存操作都自动得到同步。[条件变量操作顺序](https://timsong-cpp.github.io/cppwp/n4861/thread.condition)

消费者持锁检查到队列为空时，生产者暂时不能在同一锁内入队。消费者调用 wait，释放锁并进入等待后，生产者才能持锁修改状态并通知。若生产者更早完成入队，消费者检查时就能看到非空，跳过等待。两种顺序都不需要把通知保存为令牌。

谓词重载相当于在条件不满足时反复等待。除了虚假唤醒，另一消费者也可能在被通知的线程重获锁之前先取走数据。因此“重新检查”和“修改队列”要放在同一临界区，不能检查后解锁，再使用刚才的结果。

### 数据可见性由哪条链保证

本例的发布顺序可以写成：

```text
生产者持锁写入 values_[tail]，更新 size
    → 生产者 unlock(mutex_)
    → 消费者成功 lock(mutex_)，包括 wait 内的重锁
    → 消费者在锁内读取 size 和对应元素
```

同一 mutex 的解锁与后续成功获得所有权的加锁同步，再结合线程内顺序，建立写入到读取的 happens-before。只调用 notify，不能替代这条共享数据访问协议。[mutex 同步要求](https://timsong-cpp.github.io/cppwp/n4861/thread.mutex.requirements.mutex)、[数据竞争与先行关系](https://timsong-cpp.github.io/cppwp/n4861/intro.races)

### atomic 标志仍可能错过通知

下面只描述错误时序，不运行可能挂住的程序。假设消费者拿着 mutex 检查一个原子 ready，而生产者写 ready 时完全不使用该 mutex：

| 顺序 | 消费者                      | 生产者                     |
| ---- | --------------------------- | -------------------------- |
| 1    | 读取 ready，结果为 false    |                            |
| 2    | 尚未调用 wait               | 写 ready=true，调用 notify |
| 3    | 调用 wait，释放锁并进入等待 | 已经离开，不再通知         |

这个标志可以没有数据竞争，但通知发生时消费者尚未等待，之后可能一直睡眠。更强的原子内存序不会消除该时序窗口。采用本例的同锁状态协议，生产者在消费者释放锁前无法完成状态修改；若改用其他原子等待方案，则要重新核对它的状态与等待规则。

## 数据结构/系统内部实现

### 队列状态与提交点

`BoundedQueue<Capacity>` 用固定数组存 int，始终保持 `0 <= size_ <= Capacity`，head 和 tail 落在数组范围内。互斥锁保护这些成员，因此不需要原子索引。一次成功入队在锁内写入元素并增加数量；出队在锁内取值并减少数量。索引按容量回绕，满与空由 size 区分。

等待结束后，push 先检查 closed，保证 close 提交后不再接受新项。pop 先检查 size；已关闭但非空时仍能取出元素，已关闭且为空才结束。close 在锁内更新状态，随后向两个等待组广播。即使当时没有等待者，后来的操作也会检查到关闭状态。

两个条件变量用于区分生产者和消费者。若把两类线程混在一个条件变量上，只用 notify_one，可能唤醒业务谓词仍为假的线程，而能推进的另一类线程继续阻塞。分组通知减少这种问题，但不赋予任何线程 FIFO 次序或无饥饿保证。

### 解锁后通知与寿命

示例在提交队列变更后先解锁，再通知另一组等待者。这允许接收者直接竞争已经释放的锁；实际是否减少唤醒开销取决于实现与调度。持锁通知同样合法，不能把通知位置写成普适性能结论。

解锁后还要使用条件变量，因此队列必须继续存活。示例中队列早于 worker 构造，所有 worker join 后才离开队列作用域。若其他线程能在解锁后销毁对象，这个通知顺序就需要额外的寿命保护。对整个队列，notify_all 后立刻析构也不够：被唤醒者仍会重获 mutex 并访问状态。条件变量自身的析构前提不能代替容器及其锁的完整生命周期检查。[条件变量析构约束](https://timsong-cpp.github.io/cppwp/n4861/thread.condition.condvar)

### 超时只描述等待协议

手写循环若每次都调用完整时长的 wait_for，会在每次醒来后重置预算。可以在操作开始时用 `steady_clock` 计算一次绝对 deadline，反复使用它。单次调用带谓词的 wait_for 本身已按一个截止点处理内部重试，无须再套一个重置时长的外层循环。[steady_clock](https://timsong-cpp.github.io/cppwp/n4861/time.clock.steady)、[等待重载](https://timsong-cpp.github.io/cppwp/n4861/thread.condition.condvar)

带谓词的 wait_until 返回是否最终满足谓词。即使 deadline 已过，谓词为真仍可成功；等待超时后也会在锁内最后检查一次。示例的队列 API 因而不是“过期后禁止提交”的接口。交易请求若有严格的业务有效期，应另外规定提交点的时钟检查及过期拒绝，不能从 wait 返回 true 推导准时完成。重获锁和线程调度也会延迟函数返回。[超时规格](https://timsong-cpp.github.io/cppwp/n4861/thread.req.timing)

## C++ runnable demo

程序检查空、满、索引回绕、重复关闭、关闭后的入队拒绝和 FIFO 排空。跨线程部分验证 close 与空队列 pop、满队列 push 的竞争，结果不依赖 worker 是否已经进入 wait。传输测试先正常接收 128 项，再注入一次生产者异常，确认已经接受的 37 项仍按顺序排空。

```cpp include=examples/mutex-condition-variable.cpp

```

在提供 C++20 线程支持的 GCC 环境编译，不定义 NDEBUG，以保留断言：

```bash
g++ -std=c++20 -O2 -pthread -Wall -Wextra -Wpedantic -Werror examples/mutex-condition-variable.cpp -o /tmp/condvar-demo
/tmp/condvar-demo
```

预期成功输出：

```text
accepted=128 drained=128 injected_failure=0
accepted=37 drained=37 injected_failure=1
empty, full, close, drain and failure checks passed
```

单线程边界检查使用已经到达的 deadline，验证超时状态及“谓词已经满足时仍可成功”的规则；不比较耗时。并发测试的 30 秒 deadline 只是防挂兜底，超时会作为测试失败报告，不是性能要求，也不能证明同步正确。由于锁获取和调度本身没有确定上界，它也不替代测试进程外的超时管理。

生产者异常会被保存，随后关闭队列，健康消费者排空后 join。consumer 写入的数组、计数及异常指针由主线程在 join 后读取，join 建立完成到读取的同步关系。[jthread::join](https://timsong-cpp.github.io/cppwp/n4861/thread.jthread.mem)

消费者自身异常时会关闭队列并向主线程报告失败；这个路径不宣称所有已接受项都已处理。同步原语本身失效，或关闭时再次抛异常，不在本例的可恢复范围内。示例只传 int，没有任意 T 的抛出移动、持久化重试或消费者接管协议，也没有验证多生产者、多消费者的全部执行。

## 高频追问

### close 与 stop_token 怎么配合？

close 是队列协议中的状态变化，stop_token 表达协作停止请求。普通 condition_variable 不会因 jthread 的停止请求自动醒来；C++20 condition_variable_any 提供带 stop_token 的等待重载，返回结果仍说明谓词是否满足。[可中断等待](https://timsong-cpp.github.io/cppwp/n4861/thread.condvarany.intwait)

若要求排空，通常先停止接收新工作，再让消费者处理到“关闭且为空”。消费者简单地执行“收到 stop 就退出”，会留下已经接受的数据。本例通过 close 协议退出，worker 不依赖 jthread 的停止令牌。消费者故障时，生产系统应明确剩余队列由谁接管、已处理位置如何记录，或如何报告无法继续处理。

### jthread 是否让关闭流程自动正确？

joinable 的 jthread 析构会请求停止并 join，但它不知道用户的队列谓词；如果工作线程不能从等待中退出，析构仍可能等下去。普通 thread 在仍 joinable 时析构会 terminate。示例显式 close 和 join，并保证被引用的队列、结果与异常状态活得更久。[jthread 生命周期](https://timsong-cpp.github.io/cppwp/n4861/thread.jthread.cons)、[thread 析构](https://timsong-cpp.github.io/cppwp/n4861/thread.thread.destr)

### 锁保护了队列，为什么还要讨论异常？

unique_lock 的 RAII 能在离开作用域时释放锁，但不会恢复已经修改的元素。泛化为任意 T 时，移动到输出参数可能先改变源对象再抛出；直接复制 int 的提交规则不能原样推广。可以限制元素操作不抛异常，或让队列转移所有权明确的句柄，并把复杂业务处理放到锁外。

## 容易答错的点

| 说法                            | 修正                                                 |
| ------------------------------- | ---------------------------------------------------- |
| notify 的次数就是可消费次数     | 可消费数量保存在队列状态中，通知不为未来等待者累计   |
| wait 返回就可以直接访问元素     | 先持锁检查谓词，真实通知后也可能被其他线程抢先消费   |
| ready 是 atomic，就没有丢唤醒   | 原子访问不连接谓词检查与进入条件变量等待的时序       |
| 必须持锁 notify，否则错误       | 持锁或解锁后通知都可正确，状态协议与对象寿命必须满足 |
| wait_until 返回 true 就没有过期 | true 表示谓词为真，不能证明在截止点前提交            |
| close 意味着队列立即为空        | 本例保留已有项，消费者排空后才结束                   |
| mutex 按申请先后顺序授予所有权  | C++ 标准没有给出这样的公平性保证                     |
| join 成功说明业务处理成功       | join 只等待线程结束，业务结果和异常仍需检查          |

## 性能分析

固定容量 int 队列的一次已满足条件的入队或出队只做常数个索引与元素操作，空间为 O(Capacity)。包含加锁、条件等待和重新调度的完整调用没有由此得到 O(1) 的墙钟耗时保证。短临界区也可能在竞争、抢占或资源紧张时出现较长等待。

notify_all 使更多线程竞争同一 mutex，可能增加无效唤醒和重检。notify_one 的工作量更集中，但需要确保被通知的等待组与状态变化匹配。批量入队若只通知一个消费者，还要检查是否由该消费者继续排空、是否需要更多并行消费者，以及后续通知由谁发出。

比较方案时固定容量、生产者与消费者数量、到达分布和处理成本，记录吞吐、队列深度、入队等待、端到端 p50/p99/p99.9，并区分稳定负载与突发积压。记录 CPU 放置和调度环境，观察唤醒与锁竞争；不能只测空队列的无竞争路径。本文未测性能，不从测试完成速度推导延迟上界或某种通知顺序更快。

## Quant/Low-Latency 场景

行情解析后向后续处理线程交付消息时，有界队列限制内存增长，也会把下游拥塞传回生产者。若生产者负责持续读取网络，阻塞入队还可能影响接收进度。满队列策略必须与数据语义相符：可合并的状态更新需要明确合并规则，不允许缺项的增量流则需要失败标记和恢复流程，不能静默丢弃后继续使用不完整状态。

在退出交易服务或替换组件时，停止接收、关闭队列、排空已接受消息、确认处理结果和 join 是不同步骤。队列已排空只说明元素被取走；若业务处理发生在锁外，还需要单独确认处理完成与外部提交结果。消费者失败时应保留可核对的进度，避免把线程已经结束解释为消息已经送达。

要求很低的尾延迟时，可以评估忙等或分阶段等待，但要计入 CPU 占用和资源竞争。是否使用阻塞队列应由负载与延迟预算决定；固定容量及 mutex 正确性不会自动给出实时调度保证。

## 相关专题

- [C++ 内存模型](cpp-memory-model.md)：沿 mutex 或原子操作建立写入与读取的 happens-before。
- [shared_mutex](shared-mutex.md)：读写所有权和公平性边界，理解不同锁的适用条件。
- [RAII 与异常安全](../cpp/raii-exception-safety.md)：解锁、回滚和业务提交的保证分别是什么。
- [对象生命周期与布局](../cpp/object-lifetime-layout.md)：队列、同步原语及线程借用的销毁顺序。

## 分层面试题

先写出共享状态、它所属的 mutex 和等待谓词，再画出等待、通知与关闭之间的执行顺序。工程题需要说明已接受数据的去向，以及动态测试能验证到哪一层。
