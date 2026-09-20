---
{
  'schemaVersion': 1,
  'id': 'ring-buffer',
  'title': 'Ring Buffer：满空、回绕与对象生命周期',
  'description': '实现固定容量、满时拒绝的单线程环形缓冲区，说明槽位与对象的区别、构造异常和借用边界，并用独立队列核对有限操作轨迹。',
  'category': 'design',
  'areas': ['Algorithm Coding', 'System Design'],
  'tags': ['ring-buffer', 'lifetime', 'exception-safety', 'bounded-queue'],
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
  'estimatedMinutes': 45,
  'prerequisites': ['对象生命周期与 RAII', 'std::optional', 'FIFO 队列'],
  'related': ['raii-exception-safety', 'vector-invalidation', 'spsc-queue', 'trading-gateway'],
  'demo':
    {
      'file': 'examples/ring-buffer.cpp',
      'platform': 'portable',
      'exercise': '增加一个可能先修改源对象再抛异常的移动构造类型，用别名参数尝试入队，区分游标和槽位占用不变与原元素值不变。为接口写出准确保证，保留不可重入限制，不用 noexcept 掩盖抛出路径。',
    },
  'references':
    [
      {
        'title': 'C++20 draft N4861: optional class',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/optional.optional',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: optional emplace and assignment',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/optional.assign',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: optional reset',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/optional.mod',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: object lifetime',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/basic.life',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: data races',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/intro.races',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'ring-buffer-q01',
        'level': 'L1',
        'prompt': '环形缓冲区的 head 和 tail 分别表示什么？',
        'answer': '在本章实现中，head 指向逻辑队头，tail 指向下一次成功构造的位置；两者都在 0 到 N−1 之间回绕。只有 size 大于零时 head 才对应存活元素，只有未满时 tail 才对应空槽。其他实现可能给同名游标不同定义，分析时应先固定含义。',
        'rubric': ['明确本章游标含义', '指出空满条件影响槽位是否有效'],
        'source': { 'kind': 'derived', 'rationale': '由环形队列的逻辑顺序和物理槽位映射推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q02',
        'level': 'L1',
        'prompt': 'head 等于 tail 时，如何判断队列是满还是空？',
        'answer': '本章额外保存 size，零表示空，N 表示满，所以全部 N 个槽位都能使用。也可以预留一个空槽或维护其他代次信息，但可用容量及判定公式会变化。仅比较两个模 N 游标而没有额外信息，无法区分这两种状态。',
        'rubric': ['用 size 区分满空', '说明预留空槽的容量代价'],
        'source': { 'kind': 'derived', 'rationale': '由模容量游标在空态和满态重合的性质推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q03',
        'level': 'L1',
        'prompt': '为什么示例用 optional<T> 槽位，而不是直接初始化 T 数组？',
        'answer': 'optional 可以先提供未包含值的槽位，入队时才原位构造 T，出队时结束该 T 的生命周期。这样无需预先默认构造 N 个 T，也不必先创建临时 T 再移动进去。代价包括每槽占用状态和可能的填充；底层不是可直接当作 T 数组使用的布局。',
        'rubric': ['区分存储与存活对象', '说明非默认构造支持和布局成本'],
        'source': { 'kind': 'derived', 'rationale': '由可选对象的空态和按需构造接口性质推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q04',
        'level': 'L1',
        'prompt': '队列满时，try_emplace 如何处理新元素？',
        'answer': '它先检查 full，满时返回 false，不构造槽内 T，也不销毁或覆盖队头。调用方必须处理拒绝，不能把 false 当作已接收。传给函数的参数表达式仍会在调用前求值，因此该接口不保证满时连实参计算也不会发生。',
        'rubric': ['满时拒绝且不覆盖', '区分槽内构造与实参求值'],
        'source': { 'kind': 'derived', 'rationale': '由函数入口容量检查和参数求值边界共同推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q05',
        'level': 'L1',
        'prompt': 'front 返回的指针可以保留到什么时候？',
        'answer': '它只是对应存活 T 的借用。未删除该元素且缓冲区仍存活时，本章插入其他元素不会搬移它；该元素被 pop、clear 或缓冲区销毁后，借用即结束。相同物理槽位后续装入新元素，也不能把旧业务借用自动认作新对象的授权。',
        'rubric': ['指出非拥有借用', '以元素生命周期限定可用范围'],
        'source': { 'kind': 'derived', 'rationale': '由固定槽位地址与元素独立生命周期的差异推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q06',
        'level': 'L2',
        'prompt': '为什么必须在构造成功后才推进 tail 和 size？',
        'answer': '如果先推进再构造，构造抛出时元数据会声称队列含有一个实际不存在的元素。本章仅对空 optional 调用 emplace，抛出后该槽仍为空；推进操作放在成功之后，游标、占用关系和 size 因而保持原状。这个证明还依赖构造过程中不重入当前队列。',
        'rubric': ['关联元数据与存活对象', '解释失败后的空槽及不重入前提'],
        'source': { 'kind': 'derived', 'rationale': '由构造提交顺序和异常路径上的队列不变量推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q07',
        'level': 'L2',
        'prompt': '构造失败后游标没变，能否承诺所有现有元素的值也没变？',
        'answer': '不能无条件承诺。构造参数可能引用现有元素，T 的构造过程可能先修改这个源对象再抛出；也可能修改外部状态。队列能保证自身没有提前提交占用和游标变化。若要保证现有值不变，需要限制参数别名或要求相应构造行为提供更强保证。',
        'rubric': ['识别别名和外部副作用', '区分结构保证与值保证'],
        'source': { 'kind': 'derived', 'rationale': '由用户类型构造副作用与容器提交边界推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q08',
        'level': 'L2',
        'prompt': '物理下标为何没有直接写成 (head + offset) % N？',
        'answer': '若 N 和下标接近 size_t 上界，head 与 offset 的加法可能先发生无符号回绕，得到的模 N 结果未必符合数学加法。本章先比较 offset 与 N−head：未跨末尾就相加，跨末尾就减去到末尾的距离。两条路径的中间值都保持在已证明的范围内。',
        'rubric': ['识别先加后模的回绕问题', '说明分段映射的范围证明'],
        'source': { 'kind': 'derived', 'rationale': '由有限宽整数运算和循环下标映射的边界推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q09',
        'level': 'L2',
        'prompt': '为什么 pop_front 只销毁元素，没有直接按值返回 T？',
        'answer': '本章允许 T 不可移动、不可复制，所以用 front 借用观察、pop 结束生命周期。若按值返回 T，取出过程需要另行定义构造要求和抛异常后的状态；移动失败还可能已经改变源元素。把销毁与值转移分开，让当前接口的异常边界较小。',
        'rubric': ['联系不可移动类型支持', '说明按值取出新增的异常问题'],
        'source': { 'kind': 'derived', 'rationale': '由取值接口的类型约束和异常保证需求推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q10',
        'level': 'L2',
        'prompt': '把本章游标改成 atomic，是否就得到 SPSC 队列？',
        'answer': '还不够。生产者必须在对象构造完成后发布可读状态，消费者完成读取和销毁后才能发布槽位可复用状态，两条交接都需要相应同步。共享 size 也改变写者关系；对象访问、满空检测和关闭流程必须重新证明。当前实现只允许单 owner 顺序调用。',
        'rubric': ['给出发布与复用两条交接', '指出计数和对象访问仍需重新设计'],
        'source': { 'kind': 'derived', 'rationale': '由单线程容器状态与跨线程槽位交接责任推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q11',
        'level': 'L3',
        'prompt': '行情队列满了，覆盖最旧元素是否可作为通用降级方案？',
        'answer': '要先看消息语义。依赖前序增量的订单簿更新被覆盖后，剩余数据可能无法重建正确状态，通常应显式使状态失效并触发恢复。允许只保留最新值的独立状态流可以设计合并策略，但应按业务键、序列和消费者契约实现，不能悄悄改变通用 FIFO 的拒绝规则。',
        'rubric': ['区分增量与可合并状态', '要求显式失效或合并协议'],
        'source':
          { 'kind': 'derived', 'rationale': '由有界队列溢出与下游业务状态连续性的关系推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q12',
        'level': 'L3',
        'prompt': '固定容量就能保证整个入队操作没有堆分配吗？',
        'answer': '固定槽位使缓冲自身无需扩容，但 T 的构造和参数准备仍可能分配。本章 Item 内部 make_unique 就会分配；optional 自身保存 T 不改变这一事实。若目标是不分配的热路径，还要约束 T、输入转换及日志等调用链，并在相应负载下观测分配。',
        'rubric': ['分开槽位存储与 T 的行为', '审查完整调用路径'],
        'source':
          { 'kind': 'derived', 'rationale': '由固定容器空间与用户对象内部资源管理的区别推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q13',
        'level': 'L3',
        'prompt': '怎样验证环形缓冲区，而不只验证一次 push 和 pop？',
        'answer': '可用结构不同的 FIFO 容器作 oracle，枚举有界 push、pop、clear 轨迹并逐步比较返回值、长度和全部逻辑内容。另测容量 1、反复回绕、满时拒绝、非平凡析构与构造失败。测试覆盖的是有限状态和类型行为，仍需单独审查一般下标范围、别名与不重入契约。',
        'rubric': ['逐步比较独立逻辑顺序', '覆盖生命周期及有限测试边界'],
        'source':
          { 'kind': 'derived', 'rationale': '由环形索引错误与对象管理错误的不同检测需求推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q14',
        'level': 'L3',
        'prompt': '如果要提供批量读取 span，当前实现还缺什么？',
        'answer': '逻辑序列可能跨物理末尾，通常需要分段表达；同时本章存储的是 optional<T> 数组，其布局不能当成连续 T 数组。要提供 span<T> 必须重新设计存储及存活对象范围，或选择复制出连续缓冲。还要明确批量借用的失效和消费提交规则。',
        'rubric': ['区分回绕与元素实际布局', '定义借用和提交范围'],
        'source': { 'kind': 'derived', 'rationale': '由逻辑连续性与可选槽位物理布局的差异推导' },
        'companies': [],
      },
      {
        'id': 'ring-buffer-q15',
        'level': 'L3',
        'prompt': '如何判断这份 Ring Buffer 是否适合低延迟组件？',
        'answer': '先确定单线程还是跨线程、元素构造析构成本、容量及满时策略，再用代表性突发负载测端到端排队和处理延迟、拒绝率及高水位。与替代容器比较时保持相同语义和对象类型。本章只有正确性验证，没有计时，所以不能从固定数组和 O(1) 下标直接推出更低的 p99。',
        'rubric': ['先固定业务和线程契约', '给出指标且不把复杂度当性能结果'],
        'source': { 'kind': 'derived', 'rationale': '由容器操作成本与业务排队延迟的不同组成推导' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

Ring Buffer 用固定槽位循环承载 FIFO 元素，通过队头、写入位置和满空信息复用已消费的空间。本章保存 size 区分空和满，N 个槽位都可使用；满时拒绝写入。每个槽用 optional 管理 T 的生命周期，先构造成功再推进游标，出队时立即析构。它是单线程容器；跨线程 SPSC 还需要单独证明对象发布、读取完成和槽位复用的同步。

## 核心概念

容量描述可同时存放多少元素，size 描述当前有多少存活元素。head 是逻辑队头，tail 是下一次构造的位置；空队列的 head 不指向可读 T，满队列的 tail 不指向可写空槽。空和满都会出现 head 等于 tail，所以只用两个模 N 下标不够。

本章选择满时返回 false，也不支持覆盖、自动扩容或阻塞等待。覆盖最旧元素是另一种业务契约，需要说明丢失谁、如何通知消费者以及借用何时失效。不能把同名环形结构都理解成同一种队列行为。

槽位已经分配，不代表其中 T 已经存活。空 optional 不需要默认构造 T；emplace 开始其包含值的生命周期，reset 销毁包含值。包含值直接存放在 optional 对象内，但 T 自己仍可管理动态资源。[N4861 optional](https://timsong-cpp.github.io/cppwp/n4861/optional.optional)

## 原理深入

操作返回到调用方时，保持以下关系：

- `0 <= size <= N`，head 和 tail 都小于 N。
- 从 head 起的 size 个逻辑位置各包含一个 T，其余槽为空。
- 未满时 tail 指向空槽；成功入队只增加最后一个逻辑元素，出队只移除第一个。

插入先判断满，再对空槽 emplace，成功后才推进 tail 并增加 size。optional 的 emplace 若构造抛出，该 optional 不包含值；由于本来就是空槽，队列无需销毁旧元素来回滚。若先改元数据，异常会留下一个被计入 size 的空洞。[N4861 optional.emplace](https://timsong-cpp.github.io/cppwp/n4861/optional.assign)

这里保证的是槽位占用、游标和计数不变。若传入的引用指向队内旧元素，构造函数可能修改它再抛出；对外部状态的副作用也不归队列回滚。进一步承诺原值完全不变，需要对 T 和参数别名作约束。构造、析构期间也禁止重入当前 RingBuffer，否则嵌套操作可能观察或改变尚未提交的状态。

出队先 reset 当前队头，再推进 head、减少 size。T 必须能无异常析构，因此清理路径不会在一半退出。clear 反复出队，缓冲区析构时也执行 clear，确保剩余元素按 FIFO 顺序被销毁。[N4861 optional.reset](https://timsong-cpp.github.io/cppwp/n4861/optional.mod)

## 数据结构/系统内部实现

存储是 `array<optional<T>, N>`，另有 head、tail、size 三个 size_t。模板要求 N 大于零；容量 1 同样可用，无须为区分满空额外浪费一个槽。类删除复制和移动操作，以免在这份基线中引入部分转移、异常回滚和借用失效的另一组规则。

推进单个游标时，到 N−1 就回到零，否则加一。逻辑位置 offset 映射到物理槽时，先计算距离末尾的 `N - head`；不足该距离时才做 head 加 offset，否则减去该距离。这样避免先执行可能回绕的 `head + offset` 再取模。容量不要求是二的幂；使用按位与代替取模需要额外的容量条件。

front 与 at 返回借用指针，空或越界返回 nullptr。插入其他元素不搬移现有对象，弹出其他队头也不结束剩余对象的生命周期，但它们的逻辑下标会前移。对应元素被弹出、清空或随容器销毁后，调用方必须停止使用其借用。即使稍后同一地址被重新使用，本接口也要求重新取得当前元素，不能靠地址相等延续旧业务身份。[N4861 对象生命周期](https://timsong-cpp.github.io/cppwp/n4861/basic.life)

本例没有线程同步。两个线程直接读写这些普通成员或在没有交接的情况下构造、读取、销毁同一槽内对象，不能依赖硬件偶然表现来避免数据竞争。SPSC 的生产者发布可读槽、消费者发布可复用槽是不同同步方向，仅替换游标类型无法完成设计。[N4861 数据竞争](https://timsong-cpp.github.io/cppwp/n4861/intro.races)

## C++ runnable demo

程序适用 C++20，不依赖操作系统 API。Item 删除复制和移动，构造时通过 unique_ptr 持有 Payload；指定输入会在成员已构造后抛出，用计数核对成员清理、成功对象析构和最终无存活资源。这不是模拟真实内存耗尽。

```cpp include=examples/ring-buffer.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/ring-buffer.cpp -o /tmp/ring-buffer
/tmp/ring-buffer
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  examples/ring-buffer.cpp -o /tmp/ring-buffer-san
/tmp/ring-buffer-san
```

实测环境为 WSL2 x86-64、GCC 13.3.0。正常和 ASan/UBSan 构建均完成以下检查：容量 1 和 3 的满空路径、失败构造后的 FIFO、借用未失效时的地址稳定、clear 与析构释放；分别对容量 1、3 枚举长度为 8 的 push/pop/clear 操作轨迹，各 3⁸ = 6561 条，逐步与 std::deque oracle 比较；另执行 10000 步反复回绕。

```text
Nonmovable lifetime, constructor rollback, capacity 1/3: OK
2 x 6561 exhaustive traces and 10000 wrap steps agree with deque.
```

oracle 使用 deque 的逻辑顺序，不复制环形下标算法。生命周期测试不读取被弹出对象的旧指针，也不通过异常后的悬空访问制造反例。本例没有并发实现、阻塞等待、序列号句柄、持久化或性能测量；有限测试和 sanitizer 无报告不能代替一般不变量证明。

## 高频追问

**为什么不直接返回被弹出的 T？** 这需要 T 能被构造到调用方，可能引入移动或复制异常。先借用再 pop 支持本例的不可移动 Item，也把对象转移的保证留给调用方另行定义。若扩展 take 接口，应明确失败后源值是否可能改变，以及何时真正释放槽位。

**满时不构造，是不是完全没有开销？** 容量判断会执行，调用前的参数表达式也会求值。满时不调用槽内 T 构造函数，不代表 `try_emplace(make_expensive_argument())` 中的实参准备被省略。

**为什么不能直接拿连续 span？** 逻辑内容可能跨末尾，并且 optional 数组不等于 T 数组。要批量访问，可以保留逐项接口、复制出连续结果，或专门设计能证明连续 T 存活范围的存储。不能用 reinterpret_cast 跳过布局和生命周期约束。

## 容易答错的点

- “环形缓冲区天然是无锁队列。” 环形描述索引布局；本章是单 owner 容器，线程安全需另行设计。
- “预分配 N 个槽就默认构造了 N 个 T。” 本例初始槽为空，只在成功 emplace 时产生 T。
- “任何 emplace 异常都有强值保证。” 还要检查已有元素的别名和构造副作用。
- “地址没变，旧引用就一直有效。” 元素可以在相同存储上结束生命周期并被替换，本接口借用以原元素存活为界。
- “O(1) 入队就代表固定耗时。” 元数据工作有界，T 的构造可能分配、抛出或执行其他不定时工作。

## 性能分析

入队、出队、front 和 at 的队列元数据工作都是 O(1)，还要分别计入 T 的构造或析构成本。clear 处理当前 k 个元素，工作为 O(k) 加各元素析构；容器析构随后还要销毁数组中的 N 个 optional 槽，整体按 O(N) 的槽位处理加 k 个 T 的析构计算。初始化 N 个 optional 槽也有相应存储和初始化成本，不能只报后续单步操作。空间按 N 线性增长，实际占用还包括每槽状态、对齐填充和三个计数成员。

固定槽位不会因入队而扩容，Item 内部的 make_unique 仍会分配。若目标是消除热路径分配，需要同时约束元素、参数转换和周围调用链。较大的内嵌数组也会增大持有对象，栈上放置时应审查可用栈空间。

本章没有性能数据。比较 deque、其他环形实现或批量接口时，要保持相同容量语义、元素类型与溢出策略，分别统计成功和被拒绝的操作，观察吞吐、队列高水位和端到端 p50/p99/p99.9。只测永不满的整数 push/pop，不能代表突发行情、慢消费者或复杂析构。容量增大可能减少短时拒绝，同时容许更多积压；应把等待时间和处理时间分开。

## Quant/Low-Latency 场景

单线程事件循环可以把待处理消息暂存在环形缓冲区，避免容器扩容，并用返回值把容量不足传给入口。订单网关必须说明何时承诺接收请求：本地入队失败不能伪装成远端拒单，更不能覆盖已经承诺保留的未知结果。

对增量行情，丢弃队头可能使后续更新失去前提。溢出通常需要显式使相关状态失效并进入恢复流程；只有协议允许按键保留最新状态时，才考虑合并，而这会形成不同于 FIFO 的接口。容量应根据可接受的突发和停顿设计，不能期待有限缓冲吸收持续过载。

跨线程交付应使用已证明的队列协议。本章对生命周期和异常的处理可作为检查清单，但共享 size、optional 的访问和借用归还都必须重新分配所有权。关闭时还需决定谁销毁未消费元素、如何等待读者退出，不能让容器随 owner 退出而销毁仍被另一线程读取的对象。

## 相关专题

- [RAII 与异常安全](../cpp/raii-exception-safety.md)：理解成员清理、提交点和异常保证的范围。
- [vector 失效](../cpp/vector-invalidation.md)：比较扩容搬移、元素删除和借用有效期。
- [SPSC Queue](../concurrency/spsc-queue.md)：继续分析生产者发布与消费者归还槽位的同步。
- [交易网关](trading-gateway.md)：把有界队列、结果保留和业务背压连接起来。

## 分层面试题

15 道题按 L1/L2/L3 各 5 道组织，依次检查满空与生命周期、异常和索引推导，以及接口扩展和业务溢出处理。
