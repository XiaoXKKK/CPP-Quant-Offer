---
{
  'schemaVersion': 1,
  'id': 'aba-reclamation',
  'title': 'ABA 与安全回收：比较成功之后，对象还在吗',
  'description': '用安全索引模型拆开逻辑 ABA、地址复用与悬空访问，解释 hazard pointer 和 epoch 的回收条件，再用原子 shared_ptr 栈验证所有权与延迟销毁。',
  'category': 'concurrency',
  'areas': ['C++ Memory Model', 'Multithreading / Atomic / Lock-Free'],
  'tags': ['aba', 'memory-reclamation', 'hazard-pointer', 'epoch', 'shared-ptr'],
  'difficulty': 'L3',
  'roles': ['C++ Developer', 'Quant Developer', 'Low-Latency C++ Developer'],
  'companyTypes': ['高频交易', '量化私募', 'Trading Firm'],
  'status': 'published',
  'updated': '2026-09-19',
  'reviewed': '2026-09-19',
  'standard': 'C++20',
  'estimatedMinutes': 60,
  'prerequisites': ['CAS 与 acquire/release', '对象生命周期与 shared_ptr', '线程启动和 join'],
  'related':
    [
      'cpp-memory-model',
      'shared-mutex',
      'object-lifetime-layout',
      'raii-exception-safety',
      'cpu-cache-false-sharing',
    ],
  'demo':
    {
      'file': 'examples/aba-reclamation.cpp',
      'platform': 'portable',
      'exercise': '在保持节点数量有界的前提下，增加一个持有旧 head 快照、主栈继续弹出后再释放快照的测试，逐步断言 live 计数。解释为何只把值 7 再压栈不能重新使用旧节点，以及现有分阶段并发测试尚未覆盖哪些混合交错。',
    },
  'references':
    [
      {
        'title': 'C++20 draft N4861: object lifetime',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/basic.life',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: shared_ptr comparisons',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.shared.cmp',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: atomic operations',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/atomics.types.operations',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: atomic ordering',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/atomics.order',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: atomic smart pointer operations',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.atomic',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: atomic shared_ptr specialization',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.atomic.shared',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: jthread',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/thread.jthread.class',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Maged M. Michael: Hazard Pointers, IEEE TPDS 2004',
        'url': 'https://www.eecg.utoronto.ca/~amza/ece1747h/papers/hazard_pointers.pdf',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'IBM Research: Hazard pointers publication record',
        'url': 'https://research.ibm.com/publications/hazard-pointers-safe-memory-reclamation-for-lock-free-objects',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'WG21 P2530R3: Hazard Pointers for C++26, 2023 proposal',
        'url': 'https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2530r3.pdf',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Trevor Brown: Reclaiming Memory for Lock-Free Data Structures',
        'url': 'https://www.cs.toronto.edu/~tabrown/debra/fullpaper.pdf',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'aba-reclamation-q01',
        'level': 'L1',
        'prompt': 'CAS 成功为什么不能证明共享状态从未改变？',
        'answer': 'CAS 检查当前比较值是否等于 expected，不记录中间历史。状态可能从 A 变成 B 再回到 A，旧 CAS 仍成功；若操作依赖的 next、成员身份等已经变化，旧假设就可能失效。是否有害要看算法的不变量，不能见到数值返回就认定错误。',
        'rubric': ['比较当前值而非历史', '旧假设可能失效', '是否有害取决于不变量'],
        'source': { 'kind': 'derived', 'rationale': '由 CAS 的比较语义与可变链表时序推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q02',
        'level': 'L1',
        'prompt': '逻辑 ABA 与 use-after-free 是否是同一个问题？',
        'answer': '不是同一个条件。节点始终存活但被移除、改链后重插，也能让旧 next 失效，形成逻辑 ABA；对象被释放后继续解引用则是生命周期错误，即使地址从未回到旧值也会发生。安全回收和状态一致性都需要独立检查。',
        'rubric': ['存活节点也能逻辑 ABA', '悬空访问不要求 A 返回', '分别证明生命周期和状态'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由节点身份变化与对象生命周期结束的不同触发条件推导。',
          },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q03',
        'level': 'L1',
        'prompt': 'retire 一个节点与立即 delete 它有什么区别？',
        'answer': 'retire 表示节点已按算法移除并进入待回收流程，其他线程仍可能保留早先取得的访问资格。只有回收协议确认这些访问不再发生时才可结束对象生命周期并释放存储。把链表断开只改变可达关系，不会自动清除读者的局部指针。',
        'rubric': ['待回收与已销毁分离', '局部引用仍可能存在', '回收依赖协议确认'],
        'source':
          { 'kind': 'derived', 'rationale': '由共享结构移除和读者已有引用的生命周期差异推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q04',
        'level': 'L1',
        'prompt': '给指针加版本 tag 后为什么仍不能直接读 p->next？',
        'answer': 'tag 扩大 CAS 比较的身份信息，却没有保持 p 所指对象存活。读 next 可能发生在检测 tag 变化之前，因此仍需安全回收或拥有关系。有限位宽的 tag 还会回绕，必须说明复用次数和读者停顿假设，不能将版本号视为永久唯一身份。',
        'rubric': ['身份比较不保活', '解引用可能早于 CAS', '有限 tag 的回绕条件'],
        'source': { 'kind': 'derived', 'rationale': '由比较时点、解引用时点和有限代数空间推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q05',
        'level': 'L1',
        'prompt': 'hazard pointer 槽位向回收者表达什么？',
        'answer': '槽位表达某线程正在保护一个可能被移除的对象。保护者按协议发布并验证来源后才能解引用，并持续保留保护到最后一次访问；回收者对已 retire 对象扫描保护记录，暂缓回收仍被保护的对象。槽位不是修改对象字段的互斥锁。',
        'rubric': ['发布与验证访问资格', '覆盖最后访问', '不提供字段互斥'],
        'source':
          { 'kind': 'derived', 'rationale': '由 hazard pointer 保护期与延迟回收协议推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q06',
        'level': 'L1',
        'prompt': 'epoch 回收中的静止状态需要满足什么条件？',
        'answer': '对所用回收域而言，线程已不再保留接下来会访问的旧节点借用。经典 EBR 常把一次结构操作之间作为静止边界；若把裸指针保存到操作外继续用，就破坏这个前提。线程暂时没被调度不等于主动到达静止状态。',
        'rubric': ['无后续旧借用访问', '操作边界假设', '停顿不等于静止'],
        'source':
          { 'kind': 'derived', 'rationale': '由 EBR 的操作期与 quiescent state 条件推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q07',
        'level': 'L2',
        'prompt': '把裸指针头部的原子操作都改成 seq_cst 能消除 ABA 吗？',
        'answer': '不能，顺序一致性不会记录一个值离开又返回的完整历史，也不会延长被指对象的生命周期。demo 的整数索引操作本身就是 seq_cst，旧 CAS 仍会接受重新出现的 A 并写入过期 next。必须修复状态协议和回收条件。',
        'rubric': ['排序不增加历史信息', '不保活被指对象', '结合安全模型解释'],
        'source':
          { 'kind': 'derived', 'rationale': '由顺序一致原子模型仍可发生的合法 A-B-A 时序推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q08',
        'level': 'L2',
        'prompt': 'hazard 发布后为什么还要重新读取共享来源？',
        'answer': '第一次读取与发布保护之间，对象可能已经被移除。重新读取并验证来源是为确认这次保护仍能建立合法访问资格；验证失败就不能解引用旧候选。发布、验证和回收扫描的同步必须配套，随手各放一次 release/acquire 并不能保证双方一定观察到彼此。',
        'rubric': ['读与保护之间存在窗口', '失败时禁止解引用', '保护与扫描需要整体同步证明'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由候选读取窗口和 hazard try_protect 的重读步骤推导。',
          },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q09',
        'level': 'L2',
        'prompt': '保护了链表节点 p，就能任意解引用 p->next 指向的节点吗？',
        'answer': '不能从一个保护直接推出后继也被保护。必须按结构的遍历协议取得并验证后继的保护，必要时保持前驱保护和重新检查可达关系。尤其从已移除节点继续跟随旧链接时，简单重复发布地址可能太晚；不能把 root 保护的步骤机械套到任意指针链。',
        'rubric': ['保护不自动传递到裸后继', '后继需独立协议', '已移除节点遍历的额外风险'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由单对象 hazard 保护范围与论文中的退休节点遍历问题推导。',
          },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q10',
        'level': 'L2',
        'prompt': '为什么 epoch 数字加一不代表现在就能释放全部旧节点？',
        'answer': '需要按回收协议确认所有可能持有旧借用的参与者已越过相应安全边界。全局计数器改变本身没有证明这一点；经典分代方案结合线程公告和待回收批次判断哪些代已安全。不能用墙上时间超时替代读者退出的证据。',
        'rubric': ['检查所有相关参与者', '代推进与待回收批次配合', '超时不等于退出证明'],
        'source': { 'kind': 'derived', 'rationale': '由 epoch 公告、宽限期与旧指针保留范围推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q11',
        'level': 'L2',
        'prompt': 'atomic<shared_ptr<T>> 的 CAS 只比较 get() 吗？',
        'answer': 'C++20 规定等价需要存储指针相同，并且共享所有权或两者均为空。相同 get() 但来自不同控制块的非空 shared_ptr 不等价；demo 用生命周期安全的别名指针验证这一点。普通 shared_ptr 的指针相等判断不能替代这套 CAS 比较语义。',
        'rubric': ['存储指针与所有权同时判断', '不同控制块不等价', '区分普通指针相等比较'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由原子 shared_ptr 的 equivalent 定义与别名构造能力推导。',
          },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q12',
        'level': 'L2',
        'prompt': '不可变共享节点栈的 head 从 A 变到新 B 再回 A，旧 CAS 一定错误吗？',
        'answer': '本例中可以无害：A 一直由所有权保持存活，A 的 value 和 next 不变，新 B 弹出后旧的弹出 A 操作仍可按同一 next 提交。实现禁止把已弹出的 A 改链重插。共享所有权本身不能为任意可变节点算法排除逻辑 ABA。',
        'rubric': ['A 存活且后继不可变', '新 B 的临时变化可无害', '禁止已弹出节点改链重插'],
        'source': { 'kind': 'derived', 'rationale': '由不可变栈后继关系与 CAS 线性化条件推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q13',
        'level': 'L3',
        'prompt': '有线程可能长时间阻塞时，选择 epoch 回收要考虑什么？',
        'answer': '先检查阻塞是否发生在保留旧借用的活动区内。经典 EBR 的慢参与者可能阻止安全代推进，让退休内存持续积压。需要缩短保护区、把 I/O 移出去、监控最老活动代与退休字节，并设置背压或改用适合的方案；不能超时后擅自把线程当作退出。',
        'rubric': ['定位活动保护区内阻塞', '回收停滞与积压', '监控背压而非无证据释放'],
        'source':
          { 'kind': 'derived', 'rationale': '由经典 EBR 对停滞参与者的回收限制推导工程措施。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q14',
        'level': 'L3',
        'prompt': '本例保留一个 head 快照，为什么可能保留整条旧链？',
        'answer': '节点的 next 也是拥有型 shared_ptr，所以旧 head 会传递拥有其后继。主栈已经弹空，快照仍可能让整段节点保持存活；最后释放快照会沿拥有链触发销毁。demo 把链长限制到 128，生产结构还需控制快照寿命、积压和递归析构深度。',
        'rubric': ['后继也是拥有边', '弹出不等于销毁', '保留量和递归析构边界'],
        'source':
          { 'kind': 'derived', 'rationale': '由不可变节点的传递所有权与最后引用释放路径推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q15',
        'level': 'L3',
        'prompt': 'push 分配失败或创建工作线程失败时，示例怎样保持生命周期正确？',
        'answer': 'push 先构造新候选再 CAS，分配失败时该调用尚未发布新头；并发调用仍可能改变栈，所以只能承诺本调用未提交。工作线程捕获异常交给主线程，jthread 作用域退出会 join 已启动线程，栈随后才销毁。关闭时禁止新访问并等现有访问者结束仍是调用者责任。',
        'rubric': ['先构造后发布', '并发异常保证的准确范围', 'join 与栈销毁顺序'],
        'source':
          { 'kind': 'derived', 'rationale': '由候选节点发布顺序、线程异常传播与 RAII join 推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q16',
        'level': 'L3',
        'prompt': '128 个值各弹出一次且 sanitizer 无报告，证明了哪些内容？',
        'answer': '证明此次有限执行通过了值集合、重复计数及最终回收检查，并未证明所有交错或线性化。测试先完成并发生产再并发消费，没有覆盖 push/pop 混合的全部历史。还应根据目标接口构造历史检查、停顿注入和独立模型，内存序与生命周期仍需论证。',
        'rubric': ['有限轨迹覆盖', '明确分阶段测试限制', '补充历史检查与协议论证'],
        'source':
          { 'kind': 'derived', 'rationale': '由有限并发测试的观察范围与线性化证明要求推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q17',
        'level': 'L3',
        'prompt': '代码没有显式 mutex，为什么仍不能称为 lock-free 栈？',
        'answer': 'atomic shared_ptr 是否 lock-free 由实现决定，本机报告 false；引用计数、分配、释放和递归销毁也属于完整操作路径。即使某个 CAS 指令无锁，重试次数和线程调度仍没有逐操作固定上界。需要分别说明数据结构进展、回收进展和实际延迟。',
        'rubric': ['atomic shared_ptr 的实现条件', '完整路径包含分配回收', '区分进展保证与延迟'],
        'source':
          { 'kind': 'derived', 'rationale': '由原子智能指针的进展属性和完整 push/pop 路径推导。' },
        'companies': [],
      },
      {
        'id': 'aba-reclamation-q18',
        'level': 'L3',
        'prompt': '风控配置快照被安全保活后，能否直接视为当前有效配置？',
        'answer': '保活只保证对象可访问，不保证它仍是业务当前版本。风控路径还需规定请求采用哪个配置版本、撤销何时生效，以及旧版本持有多久。安全回收要与这些规则配合；因为内存积压而强制释放读者仍会访问的配置，会破坏生命周期。',
        'rubric': ['保活不等于新鲜度', '定义版本与生效边界', '不能用强制释放解决积压'],
        'source':
          { 'kind': 'derived', 'rationale': '由安全访问与业务配置版本有效性的独立条件推导。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

ABA 指旧操作看到比较值回到 A，便把中间状态当作没有改变。CAS 只比较当前值，旧 next 或节点身份可能已经失效。版本 tag 能扩大身份比较，却不能让即将解引用的对象继续存活，而且有限 tag 会回绕。安全回收还要证明读者何时不再访问：hazard pointer 保护具体节点并在发布后重读验证，epoch 等待相关读者跨过安全边界。本章用不可变节点和原子 shared_ptr 做可运行的所有权基线，不承诺 lock-free。

## 核心概念

逻辑状态错误与生命周期错误可能同时出现，也可以分别发生：

| 情形           | 发生了什么                           | 需要检查什么                 |
| -------------- | ------------------------------------ | ---------------------------- |
| 逻辑 ABA       | 值回到 A，但旧操作依赖的关系已经变化 | 旧快照还能否用于提交         |
| 地址复用       | 旧对象存储被用于新对象，地址数值相同 | 对象与所有权身份是否变化     |
| use-after-free | 对象结束生命周期后仍被访问           | 访问者是否仍拥有有效保护     |
| 延迟回收       | 已移除对象暂时保留存储和生命周期     | 何时能够确认所有相关访问结束 |

ABA 不要求释放内存。一个仍存活的节点被弹出、修改 next 后重新压入，就能让旧操作保存的 next 过期。反过来，线程在别人释放节点后读取字段，即使没有发生地址复用，也已经越过[对象生命周期边界](https://timsong-cpp.github.io/cppwp/n4861/basic.life)。

`retire` 表示进入待回收流程；实际销毁和释放要等安全条件满足。把节点从共享根断开不会清除别的线程已经读到的局部指针。原子指针也只保证该指针对象的原子访问，不自动拥有它所指向的节点。

本文采用 C++20。[N4861 的原子 shared_ptr](https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.atomic.shared) 是示例使用的标准接口；hazard pointer 与 epoch 部分介绍算法协议。引用的 [P2530R3](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2530r3.pdf) 是 2023 年面向 C++26 的提案，用来核对保护机制，不能当作 C++20 已有的库接口。

## 原理深入

### 有限索引也能复现错误时序

用三个始终存活的数组槽位代表 A、B、C，初始链为 `A -> B -> C`。下表的 T1/T2 是单线程模型中按顺序模拟的两个操作，不包含任何悬空解引用：

| 步骤 | 操作                               | head 与记录                        |
| ---- | ---------------------------------- | ---------------------------------- |
| 1    | T1 读取 head 和 next，暂未 CAS     | 保存 A 与旧 next=B                 |
| 2    | T2 弹出 A，再弹出 B                | head=C，B 已不在逻辑栈中           |
| 3    | T2 把 A 的 next 改为 C，再压入 A   | head=A，但链已变为 A→C             |
| 4    | T1 用 expected=A、desired=B 做 CAS | 比较成功，错误地把 B 重新设为 head |

所有数组访问都合法，错误是成员关系与栈语义被破坏。demo 的原子操作采用默认 `seq_cst`，仍会得到这个结果；[原子操作的顺序约束](https://timsong-cpp.github.io/cppwp/n4861/atomics.order) 不会让 CAS 获得历史记录。

### tag 解决比较信息，回收解决访问资格

把 head 编成 `(slot, generation)`，每次修改一起更新 generation，旧 `(A, 0)` 就不能匹配新的 `(A, 3)`。指针与代数必须作为同一次可比较状态处理；拆成两个互不关联的原子字段，并不自动得到组合快照。

若代数只有 k 位，经过 `2^k` 次相应更新就可能重新出现相同代数。使用 64 位会扩大周期，但在允许读者任意久停顿的模型中，仍不能凭位宽证明永不回绕。demo 故意使用两位 tag，分别检查三次变化被拒绝和四次变化后比较身份再次相同。

即使这次 tag 足够区分身份，线程也可能在 CAS 之前就读取 `p->next`。如果 p 已被释放，后面的 CAS 失败无法撤销那次非法访问。宽 CAS 是否 lock-free 还取决于实现；版本检查、存活保证和进展属性都要分别给出依据。

### hazard pointer：先保护并验证，再解引用

[Michael 的原始论文](https://www.eecg.utoronto.ca/~amza/ece1747h/papers/hazard_pointers.pdf) 将保护者的记录与移除者的退休列表连接起来。以一个始终存活的原子根为例，保护流程可概括为：

```text
读取根中的候选 p
按回收库的协议发布对 p 的保护
重新读取根，验证候选仍可作为本次访问的来源
  验证失败：撤销或更新保护，重试；不解引用旧候选
  验证成功：在保护持续期间访问 p
最后一次访问结束后清除保护
```

第一次读取和保护发布之间存在窗口。重读用于验证这次保护能否建立访问资格；保护者与回收者的同步必须保证：不能一方误以为来源仍可用，另一方又遗漏已经生效的保护而释放对象。P2530R3 的 `try_protect` 描述了设置保护后重新 acquire 读取来源的步骤，但 `reset_protection` 自身也承担回收协议义务，不能将它随意替换成一次 release-store 就声称完成实现。

移除者在节点不再按算法可达、也不再需要自己的访问之后，将其 retire。回收扫描读取保护记录集合，对候选退休对象判断是否仍被保护；仍有保护的对象保留到后续扫描。扫描不是把所有槽位同时拍成一个硬件原子快照，但它与退休、保护期之间必须满足整体顺序要求。只看某一瞬间“槽位等于空”不构成充分证明。

对后继遍历还要另作分析。保护 p 不会自动保护裸指针 `p->next` 所指对象，从已退休 p 继续跟随旧链接尤其需要谨慎。[Brown 对回收方案的分析](https://www.cs.toronto.edu/~tabrown/debra/fullpaper.pdf) 讨论了这种适用性限制。本文没有实现 hazard 回收器，伪代码也未给出可直接移植的全部屏障、记录注册与退出协议。

### epoch：用活动区覆盖一组可能访问的节点

经典 EBR 将操作划入活动期。线程进入时公告所参与的代，离开时到达不再保留旧借用的边界；移除的节点按退休代分批保留。只有按照该方案确认相关参与者已经越过安全边界，旧批次才能回收。具体实现可能采用不同的公告、静止位与批次组织，不能只看全局 epoch 数字变大就释放。

这种方法减少了逐节点保护的工作，却把回收进度与参与者状态联系起来。线程若在活动区内阻塞、被长期抢占或停止响应，经典 EBR 可能无法推进回收，其他线程持续移除节点就会增加积压。Brown 的论文还讨论了 DEBRA 等变体；这些方案的容错条件不同，不能把一个变体的保证泛化给所有 epoch 实现。

保护区外仍保存并使用裸指针会破坏 EBR 的前提。用超时强行宣布线程已经退出，也没有证明它恢复后不会继续访问旧节点。工程上可以缩短活动区、避免其中的阻塞 I/O、监控退休字节和最老公告，并在内存压力下采取明确背压。

## 数据结构/系统内部实现

可运行栈采用 `atomic<shared_ptr<const Node>> head_`。每个节点保存不可变整数和拥有型 `next`，每次 push 都新建节点，不提供把旧节点重新压入的接口。局部 `old` 持有节点所有权后才读取 value/next，成功弹出后只返回一个整数副本。

[原子智能指针要求](https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.atomic) 将相关引用计数增加纳入原子操作；相应减少、销毁和释放可以发生在原子更新之后。因而 atomic load 能取得一份有效所有权，不能模仿成“先读裸指针，再对它的引用计数加一”，后两步之间仍可能发生回收。

原子 shared_ptr 的 CAS 等价判断同时考虑存储指针和所有权：存储指针相同，且共享所有权或两者均为空。普通 `shared_ptr` 的 [operator==](https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.shared.cmp) 只按存储指针比较，不能代替这一规则。demo 用两个不同控制块别名到同一个仍存活的局部整数，验证 get 相同而 CAS 拒绝；该局部整数的寿命由词法作用域保证，两个别名均不负责删除它。

栈的同步与所有权路径如下：

```text
push：取得旧 head 的所有权 → 完成新节点构造 → acq_rel CAS 发布
读者：acquire load 或失败 CAS 取得所有权 → 读取不可变 value/next
pop：acq_rel CAS 把 head 改成 next → 返回整数 → 释放局部所有权
销毁：head、快照、局部引用和前驱 next 的最后一份所有权释放
```

成功 CAS 是相应 push/pop 的提交点；空 pop 在观察到空头时返回。失败 CAS 把当前观察值写回 `old`，使用 acquire 后再读取新候选节点。push 失败时原候选仍不可变，所以丢弃它并为新的 old 构造另一候选，而不修改已经共享的 next。[CAS 的失败更新与伪失败规则](https://timsong-cpp.github.io/cppwp/n4861/atomics.types.operations) 需要逐项核对；本例使用 strong，便于确定时序测试不受伪失败干扰。

head 仍可能发生 `A -> 新 B -> A`：B 压入后又被弹出，A 再次成为头。这里 A 始终存活，next 没有改变，旧的弹出 A 操作仍可以合法提交。安全性依赖不可变链接和不重新发布已弹出节点；shared_ptr 控制块比较不能独立修复任意可变节点算法的逻辑 ABA。

`snapshot()` 主要用于教学检查。一个快照持有 head，就会通过拥有型 next 保留整段后继。主栈弹空后，这些节点也未必销毁；释放最后一个快照可能沿链递归析构。本例所有测试的链长上界为 128，不把这一析构方式推广成任意深度的生产栈。

## C++ runnable demo

程序先用索引模型检查错误逻辑，再验证 tag 与回绕、原子 shared_ptr 的所有权比较。随后测试空栈、LIFO、一次分配器故障注入、延迟销毁及旧快照 CAS 拒绝，最后执行两个阶段的有限并发测试。

```cpp include=examples/aba-reclamation.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -pthread examples/aba-reclamation.cpp -o /tmp/aba-demo
/tmp/aba-demo
```

示例需要 C++20 原子 shared_ptr 和 jthread 支持，保持断言启用。本次 GCC 13.3/libstdc++ 环境输出：

```text
atomic shared_ptr head lock-free: false
ABA models, ownership, delayed destruction and 128 items: OK
```

第一行是运行时查询，其他实现可以不同。程序不对该布尔值设置断言，也不据此宣称完整 push/pop lock-free。

确定时序测试中，快照 held 保持两个节点存活，主栈依次弹空后检查 live 仍为 2，释放 held 后检查归零。分配器在下一次 allocate 时抛出 `bad_alloc`，检查本次单线程 push 没有修改 head，也没有多出已构造节点；这只覆盖指定分配边界，不是所有环境错误的穷举。随后保留已弹出节点的旧快照，再压入同值的新节点，旧 CAS 必须失败。

并发阶段先由四个生产者各压入 32 个唯一整数，全部 join 后，再让四个消费者弹空。每个值的原子计数必须恰为 1，所有线程退出后 created 与 destroyed 相等、live 为零。它没有覆盖 push/pop 同时发生的全部交错，也没有用一次值集合检查冒充线性化证明。

工作线程捕获分配等异常写入各自的异常槽，join 后由主线程重抛。线程容器使用 [jthread 的析构 join](https://timsong-cpp.github.io/cppwp/n4861/thread.jthread.class)，若中途创建线程失败，已启动线程也先退出，随后才销毁栈与其引用数据。线程不响应停止请求；这里每个任务的输入有限，join 仍依赖线程获得调度和底层操作能够完成。

可使用 ASan/UBSan 检查有限正常执行中的内存问题。本章不运行悬空访问，也没有实现 hazard/epoch 回收器；sanitizer 无报告不能证明回收算法、进展保证或所有并发历史正确。

## 高频追问

### hazard 槽位保护对象后，还需要原子字段或锁吗？

需要按字段访问方式决定。hazard 保护的是生命周期，多线程修改节点内容仍要满足数据竞争和算法顺序要求。即使节点始终存活，修改 next 后重插也可能使旧操作的逻辑假设失效。本例通过发布后不可变来缩小这些义务。

### epoch 与 hazard 谁更适合长时间读者？

经典 epoch 活动区中的慢读者可能阻止一批甚至持续增长的退休对象回收。固定数量 hazard 槽位通常能更精确地表明哪些对象仍被直接保护，但还要考虑退休列表、扫描频率与线程退出清理；不能从槽位少推出总内存一定有固定上界。长遍历还会增加保护与重验证成本，应根据结构和负载选择。

### 为什么不在 CAS 失败后改 candidate 的 next？

有些实现允许在尚未发布的候选上改写 next，但需要证明该候选从未被任何线程看到。本例选择每次重建不可变候选，使共享后禁止修改的约束更容易审查。代价是竞争重试会增加分配和销毁次数，不适合作为低延迟性能实现。

### 关闭时 head 置空就足够了吗？

置空只移除了根持有的所有权。仍需禁止新访问、等待使用栈对象本身的线程结束，并处理遗留快照或回收域记录。shared_ptr 可以继续保活节点，却不能让已经销毁的 `OwnedStack` 对象继续接受成员调用。hazard/epoch 的域销毁还要满足其线程注册与退休清理协议。

## 容易答错的点

- 把 CAS 成功解释为“从上次 load 到现在没有变化”。它只比较这次操作看到的值。
- 把所有 A→B→A 都算作算法错误。需要检查旧操作依赖的状态是否已改变；不可变栈中的临时新头可能无害。
- 认为 tag 足以保护解引用。身份验证发生在比较时，对象可能更早已经结束生命周期。
- 认为改成 seq_cst 就不用回收协议。顺序约束不会自动保活节点。
- 在发布 hazard 前读取候选的字段，或验证失败后继续使用旧候选。保护窗口尚未建立，重试不能补救已发生的非法访问。
- 将保护一个节点视为保护所有裸后继。每条访问路径都需要自己的合法性条件。
- 认为 epoch 超时可以直接释放。慢线程恢复后仍可能使用旧指针，超时没有提供退出证明。
- 认为 atomic shared_ptr 一定使用无锁指令。标准允许不同实现，完整路径还含分配、计数和析构。
- 把对象安全存活当作业务状态仍有效。订单或配置版本是否可用于当前请求需要另外判断。

## 性能分析

本实现的单次 CAS 尝试只读取固定大小的值和指针，但完整操作包含额外工作。push 每次重试都分配一个新候选；若失败 F 次，就会尝试构造 F+1 个候选。pop 的 CAS 重试也没有逐线程固定次数保证，释放最后引用还可能触发后继链销毁。销毁 k 个节点需要 O(k) 次节点析构，本例把链长限定在 128；不能仅凭头部 CAS 把整个 pop 的最坏成本写成 O(1)。

hazard 扫描的成本取决于记录数量和匹配方式。若 H 个保护槽与 R 个退休候选逐对比较，工作量为 O(HR)；用集合组织保护地址可改变匹配成本，同时引入构建集合、存储与分配开销。论文中的摊还保证有批次阈值和参与者模型前提，不能直接等同某个 API 的固定时延。epoch 通常摊薄逐节点保护工作，但需要扫描参与者公告并批量回收，具体成本由方案决定。

测量时同时记录操作吞吐、p50/p99/p99.9、CAS 重试、分配次数、退休字节峰值、最老保护期和每批析构时间。对 epoch 加入活动区内停顿，对 hazard 加入持续保护和扫描负载，对引用计数加入长快照与最后引用释放。固定线程与内存放置，区分算法更新和回收工作的时间；只测平均 CAS 延迟会漏掉引用计数共享写入及批量释放的尾部成本。

本章没有性能实测。生命周期计数和故障注入会增加原子访问，不应拿 demo 的运行耗时比较回收方案优劣。

## Quant/Low-Latency 场景

行情元数据或风控配置可以用不可变版本向读者发布。原子 shared_ptr 便于明确旧版本何时销毁，但引用计数可能成为多个读线程共同写入的位置。若进一步选择 hazard 或 epoch，应先确认访问路径与保护区长度，再用真实配置更新频率、读者数量和尾延迟目标比较成本。

配置保活不等于配置仍有效。请求使用哪个版本、紧急撤销从哪一条消息开始生效，以及旧请求是否允许完成，都需要业务规则。回收协议只回答旧对象是否还可能被访问，不能代替版本和授权判断。

活动 epoch 内不应无约束地等待网络、日志或外部回调，否则一个慢路径可能让退休内存持续增加。监控要包含最老活动者和待回收字节，在预算接近上限时减少更新、施加背压或进入明确恢复流程。不能因为内存压力就直接释放仍可能被策略线程访问的节点。

订单池或空闲链表的槽位复用也会遇到身份问题。为句柄加入 generation 有助于拒绝过期槽位，但仍需证明访问期间对象不会被销毁或重建；读 generation 后就去访问普通字段，中间仍可能存在复用窗口。固定容量和单线程所有权如果符合架构，通常更容易给出清楚的生命周期边界。

## 相关专题

- [C++ 内存模型](cpp-memory-model.md)：建立发布、读取和 CAS 失败更新所需的同步关系。
- [shared_mutex](shared-mutex.md)：用锁保护读者与更新者时，同样需要安排解锁后的借用寿命。
- [对象生命周期与布局](../cpp/object-lifetime-layout.md)：区分存储地址、对象生命周期和重建后的访问资格。
- [RAII 与异常安全](../cpp/raii-exception-safety.md)：分析候选构造失败、线程退出与资源清理顺序。
- [CPU cache / false sharing](../performance/cpu-cache-false-sharing.md)：继续评估保护记录和引用计数上的共享写入。

## 分层面试题

L1 区分比较值、对象身份和生命周期；L2 沿保护、验证、移除和回收顺序检查每次访问；L3 再加入停顿、异常、关闭与内存预算。说明回收方案时，给出读者最后可能访问旧对象的位置，以及回收者凭什么确认已经越过该位置。
