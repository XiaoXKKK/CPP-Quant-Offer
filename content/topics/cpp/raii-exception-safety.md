---
{
  'schemaVersion': 1,
  'id': 'raii-exception-safety',
  'title': 'RAII 与异常安全：从资源释放到提交边界',
  'description': '沿着构造失败和批量更新的异常路径，区分资源清理、基本保证、强保证与 noexcept，并用故障注入验证本地状态。',
  'category': 'cpp',
  'areas': ['Modern C++', 'STL / C++ Object Model'],
  'tags': ['raii', 'exception-safety', 'ownership', 'noexcept'],
  'difficulty': 'L2',
  'roles': ['C++ Developer', 'Quant Developer', 'Low-Latency C++ Developer'],
  'companyTypes': ['高频交易', '量化私募', 'Trading Firm'],
  'status': 'published',
  'updated': '2026-09-19',
  'reviewed': '2026-09-19',
  'standard': 'C++20',
  'estimatedMinutes': 45,
  'prerequisites': ['构造函数与析构函数', 'std::vector 的基本使用', 'try / catch 与 throw'],
  'related':
    ['move-value-categories', 'vector-invalidation', 'shared-mutex', 'epoll-lt-et', 'order-book'],
  'demo':
    {
      'file': 'examples/raii-exception-safety.cpp',
      'platform': 'portable',
      'exercise': '在基本保证版本中增加批量中间出现负数的测试，断言失败后保留的前缀；再给空批量注入失败，并解释这些检查点为什么还不能覆盖所有内存分配失败。',
    },
  'references':
    [
      {
        'title': 'C++20 draft N4861: constructors and destructors during exception handling',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/except.ctor',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: initializing bases and members',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/class.base.init',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: exception specifications',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/except.spec',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: terminate',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/except.terminate',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: unique_ptr destructor',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.dtor',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: unique_ptr move construction',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.ctor',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: unique_ptr reset and release',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.modifiers',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: vector capacity and swap',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/vector.capacity',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: vector modifiers and exception guarantees',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/vector.modifiers',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: container requirements and allocator-aware swap',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/container.requirements.general',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'David Abrahams: Exception-Safety in Generic Components',
        'url': 'https://www.boost.org/doc/libs/1_31_0/more/generic_exception_safety.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++ Core Guidelines: E.6 Use RAII to prevent leaks',
        'url': 'https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Re-raii',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'raii-exception-safety-q01',
        'level': 'L1',
        'prompt': 'RAII 如何处理提前 return 和异常离开作用域？',
        'answer': '资源由已经构造完成的对象拥有，析构函数负责释放。正常离开作用域和向匹配处理器展开栈时都会执行相应析构，省去各出口的手动释放。终止进程等路径不保证执行这些析构。',
        'rubric': ['资源所有权绑定对象生命周期', '正常退出与栈展开', '终止路径限制'],
        'source': { 'kind': 'derived', 'rationale': '由资源所有权、作用域退出和栈展开规则推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q02',
        'level': 'L1',
        'prompt': 'RAII 管理的资源必须位于栈上吗？',
        'answer': '不必。自动存储期的 unique_ptr 可以拥有动态分配对象，成员对象也能拥有文件或锁。需要保证拥有者按约定结束生命周期；只创建一个裸指针不会建立自动释放责任。',
        'rubric': ['区分拥有者与资源的存储位置', '裸指针本身不负责释放'],
        'source': { 'kind': 'derived', 'rationale': '由唯一所有权封装与资源存储位置的区别推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q03',
        'level': 'L1',
        'prompt': '基本异常保证允许失败后留下哪些变化？',
        'answer': '允许状态变化，但组件不变量必须成立且不能泄漏资源。demo 的基本保证批量追加在失败后保留已经追加的非负整数前缀，仍可继续追加；基本保证本身不承诺保留原值或某个固定前缀。',
        'rubric': ['不变量与无泄漏', '可以有部分效果', '前缀属于示例额外契约'],
        'source':
          { 'kind': 'derived', 'rationale': '由异常保证定义与批量追加的逐步执行路径推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q04',
        'level': 'L1',
        'prompt': '强异常保证是否要求函数内部从不修改数据？',
        'answer': '不要求。可以修改私有临时副本，完成后再提交，也可以用不会失败的操作回滚。要求的是抛出异常后，契约涵盖的可观察状态保持调用前的值；成功调用当然可以改变状态。',
        'rubric': ['失败后可观察状态不变', '准备副本或可靠回滚', '区分失败与成功'],
        'source': { 'kind': 'derived', 'rationale': '由强保证的失败后置条件与暂存提交算法推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q05',
        'level': 'L1',
        'prompt': 'noexcept 函数能否调用可能抛异常的函数？',
        'answer': '可以，编译器不因此一律拒绝程序。它可以在内部捕获并处理异常；若异常传播越过 noexcept 函数边界，将调用 std::terminate。标注 noexcept 不会使分配等操作自动成功。',
        'rubric': ['允许调用与内部捕获', '异常越界触发 terminate', '不保证业务成功'],
        'source': { 'kind': 'derived', 'rationale': '由 noexcept 的语言规则与动态失败行为推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q06',
        'level': 'L1',
        'prompt': '为什么获取资源后应立即交给拥有者对象？',
        'answer': '裸资源获取成功到建立拥有者之间若执行了可抛操作，异常会绕过尚未建立的清理责任。使用 make_unique 或让资源包装器在构造时接管，可以缩短或消除这段无人负责释放的路径。',
        'rubric': ['识别获取与接管之间的异常窗口', '立即建立释放责任'],
        'source':
          { 'kind': 'derived', 'rationale': '由多步资源获取时的异常出口和 RAII 接管顺序推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q07',
        'level': 'L2',
        'prompt': 'Session 第二个成员获取失败时，谁负责释放第一个成员？',
        'answer': 'first_ 已完成构造，异常展开会析构它并删除所拥有的 Resource；second_ 没有完成构造。Session 的非委托构造函数失败，所以不调用 Session 析构函数。清理依靠已完成成员自己的析构。',
        'rubric': ['已完成成员自动清理', '未完成成员不析构', '完整对象析构函数不调用'],
        'source':
          { 'kind': 'derived', 'rationale': '由 demo 的成员初始化顺序与非委托构造失败规则推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q08',
        'level': 'L2',
        'prompt': '为什么两个具有强保证的 push_back 不能自动组成一次强保证批量操作？',
        'answer': '第二次 push_back 失败只需保留第二次调用前的状态，第一次成功的追加仍然存在。要保证整个批量失败后无效果，需把批量作为一个事务边界，用暂存副本或可靠回滚恢复调用批量前的状态。',
        'rubric': ['子操作保证仅覆盖各自调用', '第一次成功效果会保留', '定义批量提交边界'],
        'source':
          { 'kind': 'derived', 'rationale': '由连续调用的后置条件与批量异常安全组合问题推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q09',
        'level': 'L2',
        'prompt': '暂存副本加 swap 的强保证证明依赖哪些条件？',
        'answer': '准备期间不能修改原状态，提交必须符合前置条件且不能抛出，提交后的旧状态清理也不能抛出。demo 使用 vector<int> 和默认分配器满足 swap 条件；更换为有状态分配器后还需核对传播规则与相等性。',
        'rubric': ['准备不触碰原状态', '提交与清理不抛出', '分配器前置条件'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由暂存提交的不变量及 allocator-aware swap 的规范推导。',
          },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q10',
        'level': 'L2',
        'prompt': '栈展开中的析构抛出异常，与普通作用域退出时抛出有何区别？',
        'answer': '析构函数若为 noexcept，异常逃出即调用 terminate。显式允许抛出的析构在普通退出时可以传播异常，但在已有异常导致的栈展开中再次让异常逃出也会 terminate。因此资源清理应避免抛出，错误另由显式操作报告。',
        'rubric':
          ['区分异常说明与展开状态', '已有异常展开时第二异常越界会终止', '显式错误报告接口'],
        'source':
          { 'kind': 'derived', 'rationale': '由析构异常说明与 terminate 触发条件的交叉情况推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q11',
        'level': 'L2',
        'prompt': '为什么 vector 对可能抛出的移动构造需要额外考虑？',
        'answer': '迁移旧元素时，已经移动过的源元素可能改变；后续移动失败后未必能够无异常地恢复。C++20 对尾部单元素插入在可复制插入或不抛移动等条件下给出无效果保证；不可复制元素的移动构造抛出时，相关条文允许效果未指定。',
        'rubric': ['源元素已可能改变', '复制或不抛移动的作用', '准确说明例外为效果未指定'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由 vector 尾部插入的条件保证与元素迁移失败路径推导。',
          },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q12',
        'level': 'L2',
        'prompt': '为什么强保证函数成功提交后，旧元素引用仍可能悬空？',
        'answer': '强保证约束失败路径。暂存副本与原 vector 交换后，旧元素位于临时 vector 中；临时对象销毁时旧元素被销毁。demo 返回拥有数据的 snapshot 避免暴露借用引用，接口若允许借用必须说明成功调用的失效规则。',
        'rubric':
          [
            '失败保证不规定成功后的引用有效性',
            'swap 后临时对象持有旧存储',
            '借用接口需声明失效条件',
          ],
        'source':
          { 'kind': 'derived', 'rationale': '由副本提交后的旧存储生命周期与引用有效性推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q13',
        'level': 'L3',
        'prompt': '风控配置更新怎样同时处理验证失败与读线程访问？',
        'answer': '在未发布的新配置上完成解析和跨字段校验，失败时保留旧配置。发布需使用锁或已证明正确的同步协议，旧版本还要等读者结束访问才回收。局部 swap 的异常安全证明不能替代并发发布与生命周期证明。',
        'rubric': ['未发布副本完成验证', '单独证明同步发布', '延后旧版本回收'],
        'source':
          { 'kind': 'derived', 'rationale': '由强保证提交模型在配置发布组件中的工程约束推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q14',
        'level': 'L3',
        'prompt': '订单已发送后本地记录更新失败，可以靠析构回滚吗？',
        'answer': '析构只能执行本地清理，不能撤销对端已收到的订单。系统需保留可恢复的订单标识与状态，区分未发送和发送结果未知，再依据协议通过查询、回报或幂等重试核对；直接重发可能产生重复订单。',
        'rubric': ['外部效果不能靠本地析构撤销', '区分失败与结果未知', '恢复标识与去重语义'],
        'source':
          { 'kind': 'derived', 'rationale': '由异常提交边界与订单网关外部副作用的差异推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q15',
        'level': 'L3',
        'prompt': '如何测试一个声称提供强保证的批量更新函数？',
        'answer': '先保存调用前状态，在每个已识别的可抛步骤注入失败，捕获后比较完整可观察状态并核对资源计数。还需测试成功与空输入，并把分配、元素复制和回滚失败纳入覆盖设计；当前 demo 的检查点未穷举内存分配失败。',
        'rubric': ['逐个失败位置与前后状态比较', '资源与边界检查', '说明故障注入覆盖限制'],
        'source':
          { 'kind': 'derived', 'rationale': '由异常安全测试方法与本示例有限故障注入范围推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q16',
        'level': 'L3',
        'prompt': '低延迟热路径采用复制后提交，需要测量哪些成本？',
        'answer': '除提交时间，还要测复制、分配、旧数据销毁与释放发生在哪个线程，记录端到端 p50/p99/p99.9、吞吐及峰值内存。按状态规模和批量长度扫描，分别测成功和失败路径；swap 为常数复杂度不代表整个调用为常数复杂度。',
        'rubric': ['覆盖准备与清理全过程', '规模、尾延迟和峰值内存', '复杂度与耗时边界'],
        'source':
          { 'kind': 'derived', 'rationale': '由暂存副本的完整生命周期与热路径测量需求推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q17',
        'level': 'L3',
        'prompt': 'close 或 flush 失败需要业务处理时，RAII 接口应如何设计？',
        'answer': '提供可检查结果的显式 close、flush 或 commit，让调用者在仍能采取行动时处理失败；析构承担不抛出的兜底释放。明确失败后是否仍拥有资源及能否重试，并按具体系统 API 定义，不能把所有 close 错误都当作安全重试。',
        'rubric': ['显式可检查操作', '析构兜底且不抛出', '失败后所有权与重试契约'],
        'source':
          { 'kind': 'derived', 'rationale': '由析构错误报告限制与外部资源关闭协议的差异推导。' },
        'companies': [],
      },
      {
        'id': 'raii-exception-safety-q18',
        'level': 'L3',
        'prompt': '用回滚日志替代整份复制，怎样避免回滚再次失败？',
        'answer': '修改前先准备足够的撤销记录，确保记录写入失败时尚未改原状态；记录必须能用不分配且不抛出的操作恢复。若元素赋值、日志增长或释放会抛出，需调整表示或降低保证，不能靠 catch 后重试就声称强保证。',
        'rubric': ['先备撤销信息再修改', '回滚操作与存储需求不抛出', '不满足条件时调整契约'],
        'source': { 'kind': 'derived', 'rationale': '由事务回滚对撤销步骤的异常安全要求推导。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

RAII 把释放责任放进拥有者对象的析构函数，正常退出和异常栈展开都能按生命周期清理资源。它能帮助防止泄漏，但业务状态是否回到原值还要看操作的异常保证：基本保证保留不变量，强保证要求失败后状态不变。常见做法是先在临时副本上完成可失败的工作，再用不抛出的操作提交。`noexcept` 限制异常越过函数边界，违约会触发 `std::terminate`，不会自动完成回滚。

## 核心概念

资源可以是动态内存、文件句柄、锁或容量配额。拥有者负责释放，借用者只在约定的生命周期内访问。自动存储期的 `unique_ptr` 可以拥有堆上对象，类成员也能拥有资源；RAII 不要求资源本身位于栈上。获取资源后，应立即建立拥有关系，避免在裸资源与包装对象之间插入可抛出的操作。[Core Guidelines E.6](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Re-raii) 给出了这一资源管理约定。

异常保证描述一次操作失败后的状态，需先明确它覆盖的对象和可观察效果。

| 保证       | 异常发生后的契约                   | 本文例子                       |
| ---------- | ---------------------------------- | ------------------------------ |
| 基本保证   | 不变量成立，不泄漏资源，值可以改变 | 批量追加已完成的前缀可以保留   |
| 强保证     | 契约涵盖的可观察状态与调用前一致   | 临时副本失败，原账本不变       |
| 不抛出保证 | 操作不会向调用者传播异常           | 受条件约束的 `swap` 与资源清理 |

这些术语可参照 [Abrahams 的异常安全说明](https://www.boost.org/doc/libs/1_31_0/more/generic_exception_safety.html)。该资料用于术语和推理方法；其中早期标准库细节应以本文引用的 C++20 N4861 条文为准。

`noexcept` 是语言中的异常说明。一个返回错误码的函数可以不抛出，同时报告业务失败；声明了 `noexcept` 的函数也可能因为违约而终止进程。因此评审时要分别检查异常能否传播、失败如何报告、状态留下什么变化。

## 原理深入

### 构造完成了多少，就清理多少

demo 中 `Session` 先构造 `first_`，再构造 `second_`，最后执行构造函数体。第二次获取失败时，`first_` 已经负责第一个资源，异常展开会析构它；`second_` 尚未完成初始化。若在构造函数体抛出，两个成员都要清理，顺序为 `second_`、`first_`。

这个非委托构造函数未成功返回，所以不会调用 `Session::~Session()`。如果把释放工作全部放进完整对象的析构函数，再让成员只是裸资源句柄，就可能漏掉构造中途失败的路径。已完成成员自己的析构负责填补这条路径。[N4861 的构造异常规则](https://timsong-cpp.github.io/cppwp/n4861/except.ctor) 还单列了委托构造的情况：目标构造已经完成后，委托构造函数体抛出时会调用完整对象的析构函数。

成员按类中声明的顺序初始化，修改初始化列表的书写顺序不会改变它。一个成员需要在另一个成员析构期间继续存在时，应把被依赖者声明在前，让它后析构。相关顺序由 [class.base.init](https://timsong-cpp.github.io/cppwp/n4861/class.base.init) 规定。

### 清理完成不代表状态恢复

设账本原来是 `[10]`，一次调用要追加 `[20, 30, 40]`。直接连续调用 `push_back`，在追加 `30` 之前失败，账本会留下 `[10, 20]`。内存没有泄漏，条目仍符合非负整数的不变量，但整个批量已经产生部分效果。

`append_strong` 将原账本复制到 `staged`，只修改这份副本。复制构造失败时，容器的构造清理路径负责已完成的部分；副本构造完成后，验证或追加失败会触发临时 vector 的析构。这些路径都没有写入原账本。全部完成后，`entries_.swap(staged)` 交换新旧存储。此时 `staged` 接管旧存储，离开函数时清理。证明分为两个阶段：提交前可以失败，但不动原状态；提交及其后的清理必须不抛出。

反过来，原地更新后再尝试回滚，需要证明撤销操作也能完成。若撤销仍依赖可能抛出的复制赋值，第二次失败会使“恢复旧值”的承诺落空。回滚日志可以减少整份复制，但必须在修改前准备好撤销信息，并保证应用撤销记录时不需要新的可失败操作。

### 析构与异常边界

不带显式异常说明的析构函数，异常说明由相关基类和成员析构决定，通常为不抛出；不能简单概括成所有析构都无条件 `noexcept`。若异常逃出不抛出的析构函数，或者在已有异常的栈展开中又逃出一个析构异常，程序将调用 `std::terminate`。[except.spec](https://timsong-cpp.github.io/cppwp/n4861/except.spec) 与 [except.terminate](https://timsong-cpp.github.io/cppwp/n4861/except.terminate) 分别规定了这两类条件。

析构函数内部可以捕获错误，但吞掉错误后仍要满足释放责任。需要调用者处理的写入、刷新或提交失败，应放进显式接口；析构只负责不抛出的兜底清理。失败后句柄是否仍然有效、能否再次关闭，要依据所包装的系统 API 约定。

## 数据结构/系统内部实现

`unique_ptr` 可以按“空拥有者”和“拥有一个对象”理解。[移动构造](https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.ctor) 后接收者负责释放，源拥有者为空；借用的裸指针不会因此取得释放权。默认删除器最终销毁并释放动态对象，自定义删除器则可以配对其他资源 API。C++20 对 [unique_ptr 析构](https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.dtor) 要求删除器调用不抛出；把会抛出的清理动作塞进删除器不符合这个使用条件。具体对象大小与删除器存储方式属于实现细节。

账本示例只有一个 `vector<int>` 成员，不变量为“所有已存条目均非负”。`snapshot()` 返回值副本，避免调用者长期持有内部元素引用。它自身也可能因分配失败而抛出，但不会改变账本。若接口改为返回内部 `span`，则成功提交后旧引用会随着临时 vector 的销毁而悬空；失败时的强保证并未承诺成功时保留引用。

| 阶段        | `entries_` | `staged`         | 失败后的处理                     |
| ----------- | ---------- | ---------------- | -------------------------------- |
| 复制旧值    | 原状态     | 正在构造         | 原状态不变，已构造部分由容器清理 |
| 验证、追加  | 原状态     | 新状态的中间结果 | 临时 vector 析构                 |
| `swap` 提交 | 新状态     | 旧状态           | 本例满足不抛出条件               |
| 函数退出    | 新状态     | 销毁旧存储       | `int` 析构和默认分配器释放不抛出 |

本例使用默认分配器，`vector<int>::swap` 可作为不抛出的提交动作，并用 `static_assert` 检查异常说明。推广到有状态分配器时，需要同时核对交换的合法性：不传播分配器且两者不相等时，交换行为未定义，单看函数签名的 `noexcept` 不够。[vector.capacity](https://timsong-cpp.github.io/cppwp/n4861/vector.capacity) 和 [容器分配器要求](https://timsong-cpp.github.io/cppwp/n4861/container.requirements.general) 给出了对应条件。

这是一段单线程本地状态更新算法。`swap` 不提供线程间原子可见性，资源自动释放也不会消除其他线程的悬空访问。

## C++ runnable demo

程序用资源计数与释放轨迹检查构造失败清理，再比较基本保证与强保证。资源用普通动态对象模拟，未接入文件系统或网络。失败检查点按“已追加条目数”触发：`0` 表示首次追加前，批量长度表示全部准备完成但尚未返回或提交。

```cpp include=examples/raii-exception-safety.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/raii-exception-safety.cpp -o /tmp/raii-demo
/tmp/raii-demo
```

使用 C++20 标准库，无操作系统专用 API。不要定义 `NDEBUG`，示例依靠断言验证结果。预期输出为：

```text
RAII cleanup, ownership transfer, basic/strong guarantees: OK
```

验证内容包括第一资源获取失败、第二资源获取失败、构造函数体失败、正常析构顺序、移动后源拥有者为空、对空拥有者重复 `reset`，以及批量长度为 3 时的四个失败位置。基本保证版本核对保留的前缀并继续追加；强保证版本逐次与旧快照比较。空账本、空批量、成功提交和批量中途遇到负数也有断言。

注入点没有替换分配器，不能据此声称已经穷举 `bad_alloc`。要扩展到真实分配失败，可增加带计数器的测试分配器，再从第一次分配开始逐个失败；同时检查该分配器对 `swap` 的传播与相等性条件。当前资源计数器仅用于单线程小规模测试，不是可并发使用的资源监控器。

## 高频追问

### 所有成员都用了 RAII，类就一定满足基本保证吗？

如果类同时维护条目 vector 和条目总数，先递增总数再追加元素，追加失败时两个字段可能不一致。RAII 会回收临时资源，却不会推导并修复跨字段不变量。应调整更新顺序、合并状态，或用可靠的回滚恢复一致性。

### 两个有强保证的操作顺序执行，组合操作也有强保证吗？

第二个操作只负责恢复到它开始前的状态，第一个成功操作的效果仍会保留。demo 的基本保证批量追加正好展示了这点。评审者应先圈定调用者要求原子完成的范围，再检查这个范围是否只有一次不会失败的提交。

### 为什么移动构造的异常说明会影响 vector？

扩容时，如果移动已经改变若干旧元素，随后一个移动又抛出，容器未必能恢复旧值。对尾部单元素插入，C++20 在元素满足可复制插入或不抛移动等条件时给出无效果保证；不可复制元素的移动构造抛出时，相关条文允许效果未指定。这里应查具体操作的 [vector.modifiers](https://timsong-cpp.github.io/cppwp/n4861/vector.modifiers)，不能把“vector 总会复制”或“用了移动就有强保证”当作通用结论。

## 容易答错的点

- “构造抛出后总会调用本对象的析构。”非委托构造失败时，清理的是已经完成的成员和基类；委托构造有目标构造已完成后的例外。
- “加上 `noexcept` 就把失败处理好了。”越界异常会触发终止，函数不会因此获得回滚能力，也不保证分配成功。
- “基本保证就是不泄漏内存。”锁、句柄等资源也需要释放，跨字段不变量也必须保持。
- “`swap` 不会抛出，所以任何两个容器都可以交换。”还要满足分配器等前置条件。未定义行为不会因为 `noexcept` 而变得合法。
- “强保证保护调用前的所有引用。”它约束失败路径；成功替换存储可能使旧引用失效。
- “用了 RAII，进程终止前一定会释放资源。”`terminate` 等路径不具备通常的作用域退出保证；未捕获异常与越过不抛出边界时，终止前是否展开还存在实现定义的范围。

## 性能分析

设旧账本有 `n` 条，批量有 `m` 条。强保证示例复制旧数据，再追加并清理旧存储，整体工作量为 O(n + m)，峰值额外存储也为 O(n + m)。`swap` 本身是常数复杂度，但只占调用中的一步。即使批量为空，当前代码也会复制旧账本；若业务把空更新定义为无操作，可以在复制前直接返回。

基本保证版本省去整份副本，批量尾部追加的摊还开销与 `m` 有关，但某次重新分配仍可能搬移已有的 `n` 个元素。它和强保证版本的失败契约不同，比较耗时时要先确认业务允许部分提交。回滚日志适合修改范围较小且撤销动作可靠的表示，所需日志容量、元素恢复成本及失败时的路径也要纳入测量。

没有实测数据时，只能描述上述操作和分配成本。若要评估热路径，固定编译器、优化参数、输入规模与失败频率，分别测正常更新和注入失败。记录吞吐、端到端 p50/p99/p99.9、分配次数和峰值内存；计时范围必须包含临时对象销毁。将日志输出放在计时区外，并检查旧状态是否恰好在最敏感的线程上释放。

异常实现的展开表、异常对象管理与生成代码依赖工具链，C++ 不规定固定纳秒成本。“没有抛出就完全没有性能影响”需要具体生成代码和测量支持，不能从 RAII 的语义推导。

## Quant/Low-Latency 场景

风控参数更新可以先在未发布对象上解析、校验限额之间的约束，完成后再发布。这样失败时仍可使用旧版本。但多个读线程参与后，还要解决发布同步和旧版本回收。后台构造新对象、前台替换指针、后台释放旧对象是一种分工设想，真正实现时必须证明最后一个读者已经离开，不能在替换后立即删除旧对象。

订单网关的边界不同：发出的报文可能已经被对端接收，本地异常无法撤销该外部效果。设计需要区分“确定未发送”“发送结果未知”和“已确认”，保留可恢复的订单标识与状态，再根据具体协议的查询、回报和去重能力处理。把异常统一映射成“下单失败”后直接重发，会掩盖结果未知的情况。

行情处理线程的缓冲区和锁适合交给 RAII 管理，但析构位置也影响延迟。如果一次退出会批量归还大量缓冲区，清理成本可能集中在该线程上。延迟回收可以改变成本落点，同时需要有界队列、背压和所有权转移约定；队列满时的处理不能静默丢弃唯一拥有者。

## 相关专题

- [move 与值类别](move-value-categories.md)：继续分析所有权转移、移动后状态，以及移动操作的异常说明。
- [vector 扩容与迭代器失效](vector-invalidation.md)：把异常保证与成功更新后的借用失效规则一起检查。
- [shared_mutex](../concurrency/shared-mutex.md)：在异常安全之外检查锁内状态、不变量和解锁后的借用生命周期。
- [epoll LT / ET](../network/epoll-lt-et.md)：把资源拥有关系应用到文件描述符、连接关闭和错误路径。
- [Order Book](../trading/order-book.md)：理解多字段更新的一致性要求，再分析批量行情更新能否部分生效。

## 分层面试题

先用 L1 检查所有权、保证等级与 `noexcept`，再用 L2 沿构造、迁移和提交路径解释失败后留下的状态。L3 要给出恢复边界、测试方案或测量范围。回答工程题时写清“哪些对象不变、哪些外部效果不受该保证覆盖”，并说明所需前提。
