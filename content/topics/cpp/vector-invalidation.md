---
{
  'schemaVersion': 1,
  'id': 'vector-invalidation',
  'title': 'vector：扩容与迭代器失效边界',
  'description': '区分 vector 的容量变化、元素搬移和尾后迭代器失效，结合异常保证写出安全的追加、删除循环与借用接口。',
  'category': 'cpp',
  'areas': ['STL / C++ Object Model'],
  'tags': ['vector', 'iterator-invalidation', 'object-lifetime'],
  'difficulty': 'L2',
  'roles': ['C++ Developer', 'Quant Developer', 'Low-Latency C++ Developer'],
  'companyTypes': ['高频交易', '量化私募', 'Trading Firm'],
  'status': 'published',
  'updated': '2026-09-19',
  'reviewed': '2026-09-19',
  'standard': 'C++20',
  'estimatedMinutes': 45,
  'prerequisites': ['指针、引用与对象生命周期', '半开区间与迭代器', '复制、移动构造与异常'],
  'related': ['raii-exception-safety', 'move-value-categories', 'shared-mutex', 'order-book'],
  'demo':
    {
      'file': 'examples/vector-invalidation.cpp',
      'platform': 'portable',
      'exercise': '在 erase_loop 中加入首元素和尾元素删除用例，再比较逐项 erase 与 std::erase_if 的赋值次数；保留内容断言，不访问失效迭代器。',
    },
  'references':
    [
      {
        'title': 'C++20 draft N4861: vector capacity',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/vector.capacity',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: vector modifiers',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/vector.modifiers',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: vector overview',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/vector.overview',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: range-based for',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/stmt.ranged',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: span',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/views.span',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: vector<bool>',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/vector.bool',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: sequence container requirements',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/sequence.reqmts',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: vector erase_if',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/vector.erasure',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: remove algorithms',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/alg.remove',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: unique_ptr constructors',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.ctor',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC 13.3 libstdc++ vector implementation',
        'url': 'https://raw.githubusercontent.com/gcc-mirror/gcc/releases/gcc-13.3.0/libstdc++-v3/include/bits/stl_vector.h',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'vector-invalidation-q01',
        'level': 'L1',
        'prompt': 'vector 的 size 和 capacity 分别允许你做什么？',
        'answer': 'size 是现有元素数，合法元素下标小于 size。capacity 是不重新分配时能容纳的元素数。reserve 只预留容量，不增加元素；即使 capacity 大于 0，对空 vector 使用 v[0] 仍违反访问前提。',
        'rubric': ['元素范围由 size 决定', 'reserve 不增加元素'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从容量与元素访问范围的区别推导，考查预分配后的边界判断。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q02',
        'level': 'L1',
        'prompt': '一次成功的重新分配会使哪些 vector 句柄失效？',
        'answer': '指向旧元素的引用、指针和迭代器全部失效，旧 end() 也失效。vector 对象自身的地址可以不变；受影响的是它所管理的元素存储。后续访问必须从容器重新取得句柄。',
        'rubric': ['所有元素句柄及 end 失效', '区分容器对象与元素存储'],
        'source':
          { 'kind': 'derived', 'rationale': '依据重新分配的失效规则推导，区分容器与被管理对象。' },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q03',
        'level': 'L1',
        'prompt': 'push_back 没有扩容，旧 end() 还能用于循环条件吗？',
        'answer': '不能。成功追加会使旧 end() 失效，即使已有元素的引用和迭代器仍然有效。循环若需要当前边界应重新获取 end()，并另行保证当前迭代器没有被扩容失效。',
        'rubric': ['旧 end 失效', '已有元素与循环边界分开判断'],
        'source':
          { 'kind': 'derived', 'rationale': '从尾部插入规则推导，检查尾后迭代器这一常见遗漏。' },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q04',
        'level': 'L1',
        'prompt': '未扩容时在中间插入一个元素，哪些旧迭代器还能使用？',
        'answer': '插入位置之前的迭代器、引用和指针保持有效；插入位置及其后的句柄和旧 end() 失效。应使用 insert 返回的迭代器访问新元素，不能继续把原位置当作旧元素的稳定身份。',
        'rubric': ['插入点之前保持有效', '插入点及后缀失效'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据不重新分配的插入规则推导，检验位置边界是否准确。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q05',
        'level': 'L1',
        'prompt': 'resize 缩小和 reserve 较小数值的效果一样吗？',
        'answer': '不一样。resize 缩小会销毁尾部元素并改变 size，保留前缀的句柄仍有效，容量不缩小。reserve 的参数不超过 capacity 时不重新分配，也不改变 size，已有句柄和 end() 保持有效。',
        'rubric': ['resize 销毁尾部元素', 'reserve 较小值不收缩'],
        'source':
          {
            'kind': 'derived',
            'rationale': '对照两个 API 的具体效果推导，检查容量与生命周期的关系。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q06',
        'level': 'L1',
        'prompt': 'shrink_to_fit 之后可以断言 capacity 等于 size 吗？',
        'answer': '不能。shrink_to_fit 是非强制请求，实现可以保留原容量。发生重新分配时所有旧元素句柄及 end() 失效；没有重新分配时保持有效。通用调用代码可以统一重新取得句柄。',
        'rubric': ['请求非强制', '按重新分配判断失效'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据容量收缩请求的保证推导，避免把一次运行结果当作标准。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q07',
        'level': 'L2',
        'prompt': '为什么删除循环应写 it = v.erase(it)，而不能删除后继续 ++it？',
        'answer': 'erase 使删除点及其后的迭代器失效，原 it 不能继续递增。返回值指向删除后接续的元素，删除尾元素时返回新的 end()。只有本轮未删除时才递增，否则会跳过连续待删除元素。',
        'rubric': ['消费 erase 返回值', '连续删除和尾部返回值'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由删除后的失效范围推导安全遍历，并加入连续匹配的边界。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q08',
        'level': 'L2',
        'prompt': '预留足够容量后，为什么仍不能在 range-for 中持续 push_back？',
        'answer': 'range-for 在进入循环前保存 begin 和 end。即使预留容量使元素句柄不因追加而失效，第一次成功追加仍使保存的 end 失效，后续循环比较没有保证。可保存原始 size，用下标读取并在追加前复制所需值。',
        'rubric': ['range-for 缓存边界', '原长度加下标的替代方案'],
        'source':
          {
            'kind': 'derived',
            'rationale': '组合范围循环展开与尾部插入规则，分析看似已预分配的循环。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q09',
        'level': 'L2',
        'prompt': '保存元素下标是否能解决 vector 的所有失效问题？',
        'answer': '下标能在纯扩容后重新定位同一序号的元素，前提是元素顺序未改变且下标仍在范围内。中间插入、删除和排序会改变下标对应的业务对象；需要长期标识时，应使用独立 ID 并维护 ID 到当前位置的映射。',
        'rubric': ['扩容后重新取得元素', '位置不等于业务身份'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据存储位置与业务身份的区别，推导索引句柄的适用边界。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q10',
        'level': 'L2',
        'prompt': 'vector<T> 扩容抛出异常时，能一概认为原内容未改变吗？',
        'answer': '不能。reserve 对不可复制插入且移动构造可能抛出的 T 存在例外，不能承诺失败后原内容未变。尾部单元素插入在 T 可复制插入或可无异常移动构造时提供无影响保证；其他插入路径要核对元素操作和迭代器异常。',
        'rubric': ['抛异常移动且不可复制的例外', '区分操作及类型条件'],
        'source':
          {
            'kind': 'derived',
            'rationale': '结合容量和修改操作的异常条款，检查强异常保证的适用条件。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q11',
        'level': 'L2',
        'prompt': 'span 指向 vector 前缀时，一次不扩容的尾部追加会怎样？',
        'answer': '原前缀元素仍有效，span 保存的长度不会随 vector 增长，因此仍只覆盖创建时的范围。之后若 vector 扩容、销毁所覆盖元素或结束生命周期，span 不会阻止失效，也不会自动更新指针与范围。',
        'rubric': ['视图长度保持原值', '非拥有视图不延长生命周期'],
        'source':
          {
            'kind': 'derived',
            'rationale': '将 span 的借用语义与 vector 追加规则组合，推导视图的有效范围。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q12',
        'level': 'L2',
        'prompt': '为什么每次追加前都 reserve(size() + 1) 可能使总成本变差？',
        'answer': '这种调用可能迫使容量按很小的增量增长；若每次只预留请求数量，第 k 次扩容要搬移 k 个元素，N 次追加总搬移为二次量级。标准没有固定增长倍数保证，应按已知批量预留或让容器管理追加增长。',
        'rubric': ['重复线性搬移累积', '增长策略不固定'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从线性重分配成本推导重复小步预留的后果，区分可能与必然。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q13',
        'level': 'L3',
        'prompt': '行情批处理返回 span 后，下一批还要复用 vector，接口应约定什么？',
        'answer': '约定 span 只在当前批次处理窗口内有效，消费者完成后才能清空或改写对应元素。异步消费者应获得拥有型批次或受控的缓冲区租约。容量预留只处理扩容，不能阻止复用覆盖、并发访问和生命周期结束。',
        'rubric': ['明确借用结束点', '异步所有权与复用协议', '容量保证的局限'],
        'source':
          {
            'kind': 'derived',
            'rationale': '将视图失效规则用于行情批次交接，考查消费者与缓冲区复用。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q14',
        'level': 'L3',
        'prompt': '如何比较预分配 vector 与自然增长的延迟，避免平均值掩盖扩容？',
        'answer': '保持输入、元素类型、分配器和批量分布相同，记录每次追加延迟及容量变化，分别查看扩容与非扩容样本，再报告整体分位数。预分配成本和内存峰值单独记录；计时本身的开销也应校准，不能由摊还复杂度推出单次上界。',
        'rubric': ['按扩容事件分组', '尾延迟与容量成本', '计时开销与实验控制'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从摊还与单次成本的差别设计可归因实验，避免只比较吞吐。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q15',
        'level': 'L3',
        'prompt': '订单对象需要稳定地址，vector<unique_ptr<Order>> 有哪些收益和代价？',
        'answer': 'vector 重新分配会移动 unique_ptr，通常不移动它们拥有的 Order，因此仍存活的订单对象地址可保持稳定。代价是对象分配、额外间接访问及局部性变化。删除拥有者仍销毁订单，指向 unique_ptr 槽位的句柄也仍受 vector 失效规则约束。',
        'rubric': ['槽位与被拥有对象分开', '删除仍结束生命周期', '分配和间接访问成本'],
        'source':
          {
            'kind': 'derived',
            'rationale': '将所有权间接层用于稳定地址需求，比较生命周期与内存访问成本。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q16',
        'level': 'L3',
        'prompt': '热路径预留固定容量后，超出批次上限应怎样处理？',
        'answer': '先区分容量提示与业务上限：reserve 不会禁止继续增长。业务需在追加前检查上限，明确拒绝、拆批或转移到有界队列的策略，并记录溢出。若属于行情处理，不能静默丢事件后继续宣称订单簿有效。',
        'rubric': ['reserve 不施加上限', '显式背压或失败策略', '数据完整性'],
        'source':
          {
            'kind': 'derived',
            'rationale': '将容量管理用于有界批处理，考查超限策略及行情完整性约束。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q17',
        'level': 'L3',
        'prompt': 'ASan 没有报告，是否足以确认未扩容 insert 后保存的迭代器有效？',
        'answer': '不够。原存储仍可能可访问，ASan 未必识别这种逻辑失效；测试恰好读到某个值也不建立标准保证。应按操作、位置及重新分配条件审查句柄，辅以标准库调试迭代器，并让测试只访问规则保证有效的对象。',
        'rubric': ['逻辑失效可能不被 ASan 捕获', '契约审查优先', '调试迭代器的辅助作用'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据失效与内存可访问性的区别，设计不依赖未定义行为的验证方式。',
          },
        'companies': [],
      },
      {
        'id': 'vector-invalidation-q18',
        'level': 'L3',
        'prompt': '批量删除一半元素时，怎样在 vector 上保留顺序并控制搬移成本？',
        'answer': '逐次 erase 会反复移动后缀，最坏达到二次量级。C++20 std::erase_if 可一次压紧保留元素再删除尾部，整体线性并保持保留元素的相对顺序。删除前取得的业务位置仍需更新，谓词不应在执行中修改同一 vector。',
        'rubric': ['逐项删除的最坏成本', '一次压紧保序', '位置映射与谓词约束'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从删除的后缀赋值成本推导批处理策略，并考查保序要求。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

`vector<T>`（不含 `bool` 特化）连续存储元素。成功重新分配后，所有元素引用、指针、迭代器和旧 `end()` 都失效；没有重新分配的插入只保留插入点之前的句柄，删除只保留删除点之前的句柄。`reserve` 可以减少扩容，不能阻止中间操作搬移元素，也不能让旧 `end()` 跨追加继续使用。

## 核心概念

`size()` 是现有元素数量，`capacity()` 是不重新分配时可容纳的数量。合法元素访问范围是 `[0, size())`。空容器执行 `reserve(100)` 后仍没有可通过 `operator[]` 访问的元素；要增加元素应使用插入或 `resize`。`end()` 表示尾后位置，本来就不能解引用。

本文把指针、引用和迭代器统称为元素句柄。失效后不能继续把旧句柄当作原元素的访问凭据。尤其不要通过解引用旧指针、比较旧迭代器与新 `end()`，或打印悬空指针来“测试是否还好用”。地址数值没有变化，也不能替代容器的有效性规则。

以下讨论 C++20、正常满足元素类型要求的 `vector<T>`，操作结果表假设调用成功。`vector<bool>` 使用代理引用，不保证连续的 `bool` 对象存储，不能套用 `T&` 和 `T*` 的普通元素模型。见 [vector 概览](https://timsong-cpp.github.io/cppwp/n4861/vector.overview) 与 [bool 特化](https://timsong-cpp.github.io/cppwp/n4861/vector.bool)。

## 原理深入

### 先判断是否重新分配，再判断位置

重新分配要为元素取得新的存储，原有元素对象在迁移和清理后不再作为原句柄的目标。没有重新分配时，中间插入仍需要为新元素腾出位置，删除则需要让后缀向前接续。存储块保持不变，不代表每个位置仍对应原来的元素。

| 成功操作                                | 旧元素句柄                       | 旧 `end()` |
| --------------------------------------- | -------------------------------- | ---------- |
| `reserve(n)`，`n <= capacity()`         | 全部有效，`size` 不变            | 有效       |
| `reserve(n)`，`n > capacity()`          | 全部失效，`size` 不变            | 失效       |
| `push_back` / `emplace_back`，未扩容    | 全部有效                         | 失效       |
| 插入后新 `size` 超过原容量              | 全部失效                         | 失效       |
| 非空 `insert` / `emplace`，未扩容       | 仅插入点之前有效                 | 失效       |
| 非空范围 `erase`，或删除单元素          | 仅删除点之前有效                 | 失效       |
| 非空容器 `pop_back`                     | 仅被删除元素之前有效             | 失效       |
| `resize(n)`，`n == size()`              | 全部有效                         | 有效       |
| `resize(n)`，`n < size()`               | 仅保留的前缀有效，容量不变       | 失效       |
| `resize(n)`，`size() < n <= capacity()` | 旧元素句柄有效                   | 失效       |
| `resize(n)`，`n > capacity()`           | 全部失效                         | 失效       |
| `shrink_to_fit()`                       | 重新分配则全部失效，否则保持有效 | 同左       |

`erase(first, last)` 接受空范围，返回该位置；示例同时检查空容器上的空范围删除。单元素 `erase(end())`、空容器 `pop_back()` 不满足调用前提。删除所有元素的 `clear()` 结束所有元素生命周期；继续访问旧元素句柄无效。这些调用条件见 [序列容器要求](https://timsong-cpp.github.io/cppwp/n4861/sequence.reqmts)。

表中的规则来自 C++20 草案 [容量操作](https://timsong-cpp.github.io/cppwp/n4861/vector.capacity) 和 [修改操作](https://timsong-cpp.github.io/cppwp/n4861/vector.modifiers)。赋值、`swap` 与带分配器的容器移动另有条款，本文不将这张表扩展为这些操作的规则。

### 尾后位置也有生命周期边界

对 `{10, 20}` 追加 `30` 时，即使容量足够，旧 `end()` 也已经失效。把它看成“现在刚好指向 30”并继续使用，违反了插入规则。C++20 的 [range-for 展开式](https://timsong-cpp.github.io/cppwp/n4861/stmt.ranged) 在循环开始时保存尾后迭代器，因此循环体持续追加同一个 vector 会破坏下一次边界比较。提前 `reserve` 只能处理可能的重新分配。

如果任务是“为原有每个元素追加一个计算结果”，可以先保存原始长度，使用下标循环，并在追加前把所需值复制到局部变量。这样扩容后下一轮从 vector 重新取值；新元素不进入本轮处理范围。若还要在中间插入或删除，下标与原对象的对应关系会改变，需要重新设计遍历策略。

### 异常必须按具体操作判断

`reserve`、`shrink_to_fit` 和单参数 `resize` 在多数异常下保证无影响，但不可复制插入的 `T` 若在移动构造时抛出异常，不能使用这个保证。`reserve(n)` 在 `n > max_size()` 时抛 `length_error`；分配器也可能报告分配失败。按标准要求，“可复制插入”描述的是元素与分配器共同满足的构造要求，不只是检查一个复制构造函数声明。

尾部插入一个元素时，若 `T` 可复制插入，或 `is_nothrow_move_constructible_v<T>` 为真，抛异常后保证无影响。不可复制插入类型的移动构造若抛出，效果可能未指定（unspecified）。这不等于调用本身必然发生 UB，也不能据此假设原序列完整保留。中间插入和范围插入还涉及赋值或输入迭代器异常，不能照搬尾部单元素插入的保证。

`erase` 的后缀赋值可能抛出；标准没有为这种失败提供整体回滚承诺。需要事务式更新时，应先检查元素操作的异常规格，或在独立容器完成新状态后再提交。示例注入的是可复制元素在尾部构造失败，断言无影响只适用于这个明确条件。

## 数据结构/系统内部实现

一个便于理解的实现模型是记录分配区起点、已构造元素的末尾和存储区末尾。`push_back` 有剩余容量时在末尾构造元素并推进逻辑末尾；容量不足时申请新区域、构造新元素与迁移旧元素，成功后释放旧区域。为处理参数引用已有元素和异常回滚，具体实现中的构造顺序会更细致，不能把这个模型当成可直接照抄的容器实现。

[GCC 13.3 的 `stl_vector.h`](https://raw.githubusercontent.com/gcc-mirror/gcc/releases/gcc-13.3.0/libstdc++-v3/include/bits/stl_vector.h) 可以看到 `_M_start`、`_M_finish`、`_M_end_of_storage` 和 `_M_check_len` 等成员。它们解释该版本的布局与容量计算；标准不规定 vector 必须是三个裸指针，不规定对象字节大小，也不保证每次按两倍增长。

中间操作需要区分对象地址与业务身份。例如 `{A, B, C}` 在 `B` 前插入 `X`，结果为 `{A, X, B, C}`；原下标 1 现在代表 `X`。索引在纯扩容后仍能找回原序号，在插入、删除或排序后却不再稳定。需要跨操作引用订单时，可以使用业务 ID，并在重排后维护位置映射。

`span` 保存借用范围，不拥有 vector 的存储。一个覆盖原前缀的 span 在不扩容的尾部追加后仍覆盖原来的元素，长度也保持不变；底层元素失效后，span 不会自动修复。其所有权边界见 [span 定义](https://timsong-cpp.github.io/cppwp/n4861/views.span)。

## C++ runnable demo

程序用八组检查覆盖预留容量、追加、强制重新分配、resize、插入删除、遍历和异常。重新分配通过请求 `old_capacity + 1` 触发，不假设默认增长倍数；调用前只保存值与下标。失效句柄保留在局部变量中时，后续代码不再读取或操作它们。

```cpp include=examples/vector-invalidation.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/vector-invalidation.cpp -o /tmp/vector-demo
/tmp/vector-demo
```

成功输出 `vector boundary checks passed`。`erase_loop` 检查空输入、全删除、全保留和连续匹配；尾部构造失败检查 size、容量、内容和旧 `end()` 均保持。`reserve(max_size() + 1)` 的错误用例先排除加法溢出。`shrink_to_fit` 只检查合法容量范围，不要求请求必须兑现。

示例为可移植 C++20，不含计时结果、定制分配器或多线程访问。它没有执行失效迭代器来制造崩溃。可另用 AddressSanitizer/UBSan 和标准库调试迭代器检查误用，但工具未报告问题不能证明所有失效规则都得到遵守。

## 高频追问

“换成 `emplace_back` 就不会失效了吗？”不会改变容量规则。原地构造可以省去某些临时对象，容量不足时仍需重新分配；没有重新分配时旧 `end()` 仍失效。是否省去一次构造还取决于实参形式和元素类型。

“为什么删除后要接住返回值？”`erase` 返回删除后接续位置的迭代器。删除末尾时返回新的 `end()`；删除中间时，循环下一次应检查刚移来的元素。若再执行一次 `++it`，就可能漏掉连续匹配的元素。逐项删除虽然容易写对，最坏成本仍可能达到二次量级。

“改存 `unique_ptr` 能保住什么？”[unique_ptr 的移动构造](https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.ctor)转移所存指针，指向仍存活对象的 `Order*` 可跨 vector 扩容使用。指向 `unique_ptr` 槽位的迭代器仍会失效，删除拥有者也仍会销毁订单。需要把这两层生命周期写进接口约定。

## 容易答错的点

- `reserve` 不构造可索引的新元素，也不设置容量上限；超出预留容量后 vector 仍可继续增长。
- 不扩容的中间插入会失效后缀句柄。只比较调用前后的 `capacity()` 无法判定每个句柄是否可用。
- `resize` 缩小会销毁元素。其容量不变不能作为旧尾部对象仍可访问的依据。
- `const_iterator` 只约束经该迭代器修改元素；其他可写别名仍能修改容器并使它失效。
- “移动更快”不足以判断扩容路径。实现需要结合异常保证选择迁移方式，不能断言所有元素都必定调用移动构造。
- 没有扩容也不自动获得线程安全；容量、长度和元素访问仍需要明确的所有权与同步协议。

## 性能分析

设操作前有 `n` 个元素，插入 `k` 个元素，插入点之后有 `m` 个元素。不重新分配的插入为 `O(k + m)`；重新分配的插入为 `O(n + k)`。单次尾部追加最坏为线性，连续追加的摊还成本为常数。`reserve` 最多线性于当前元素数；重新分配的 `shrink_to_fit` 也为线性。

删除 `k` 个元素后，需要销毁 `k` 个对象，并对原删除区间之后的 `m` 个元素赋值，成本为 `O(k + m)`。尾部删除不搬移前缀。`resize` 缩小按删除数量销毁；增长在已有容量内按新增数量构造，重新分配时还需迁移旧元素。这里的复杂度以元素操作为单位，昂贵的构造、析构和分配仍会影响实际耗时。

反复从头部 `erase` 会反复搬移大段后缀，累计最坏为 `O(n²)`。如果谓词能一次确定待删除元素，C++20 [`std::erase_if`](https://timsong-cpp.github.io/cppwp/n4861/vector.erasure) 可通过保序的 [`remove_if`](https://timsong-cpp.github.io/cppwp/n4861/alg.remove) 压紧保留元素，再删除尾部，整体线性。它仍会改变业务对象所在位置，需要同步更新外部索引。每次追加前都调用 `reserve(size() + 1)` 也可能因小步扩容产生二次量级搬移。

测量时保持元素类型、输入顺序、分配器和批量分布一致，对比按批预留与自然增长。逐次记录追加延迟和容量变化，分开统计扩容样本与普通样本，同时报告整体 p50、p99、p99.9、峰值容量和预分配成本。对很短的普通追加，计时器开销可能占主要部分，需要校准或用批量测量补充。本文没有提供性能实测，摊还复杂度也不承诺某次追加的延迟上界。

## Quant/Low-Latency 场景

行情解码器可以把一个批次的消息写进 vector，完成后把 `span<const Message>` 借给同线程消费者。接口应规定消费者在返回前完成读取，之后解码器才可清空和复用缓冲区。若消费被移到异步线程，需交接拥有型批次或缓冲区租约，保证最后一个消费者完成前不改写或回收其存储。

预留容量能减少批次内的扩容，但不定义业务上限。系统若要求批次最多容纳 `B` 条消息，应在追加前检查上限，并明确拆批、拒绝或进入有界队列的行为。行情消息不能静默丢弃后继续更新一个声称完整的订单簿；发生无法处理的缺口时，应进入业务定义的失效和恢复流程。

对需要稳定订单地址的结构，`vector<unique_ptr<Order>>` 可以隔离拥有者移动与订单地址，但会引入独立分配和间接访问。连续存储值对象配合 ID 到下标的映射则有不同的局部性和维护成本。先说明取消订单、压紧存储和异步访问分别如何影响对象寿命，再测量所选方案的延迟分布。

## 相关专题

- [RAII 与异常安全](raii-exception-safety.md)：先区分资源回收与状态回滚，再判断容器操作失败后有哪些保证。
- [move 与值类别](move-value-categories.md)：理解移动构造、复制路径和 `noexcept` 如何影响 vector 的迁移选择。
- [shared_mutex：读多写少的边界](../concurrency/shared-mutex.md)：借用引用跨解锁后的有效性，需要同时检查容器修改和访问协议。
- [Order Book](../trading/order-book.md)：把位置失效落实到订单标识、状态更新和行情完整性。

## 分层面试题

L1 检查操作后的有效范围；L2 要结合遍历、异常和视图解释代码能否继续执行；L3 要明确批次交接、超限处理与测量方法。回答操作题时先给出容量与位置条件，再判断要重新取得哪些句柄。
