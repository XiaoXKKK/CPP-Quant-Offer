---
{
  'schemaVersion': 1,
  'id': 'move-value-categories',
  'title': 'move 与值类别：从表达式到资源转移',
  'description': '以 C++20 的值类别和重载选择解释 std::move、完美转发、移动后状态与返回值构造，结合 noexcept 分析容器扩容和所有权接口。',
  'category': 'cpp',
  'areas': ['Modern C++', 'STL / C++ Object Model'],
  'tags': ['move-semantics', 'value-categories', 'perfect-forwarding', 'copy-elision'],
  'difficulty': 'L2',
  'roles': ['C++ Developer', 'Quant Developer', 'Low-Latency C++ Developer'],
  'companyTypes': ['高频交易', '量化私募', 'Trading Firm'],
  'status': 'published',
  'updated': '2026-09-19',
  'reviewed': '2026-09-19',
  'standard': 'C++20',
  'estimatedMinutes': 45,
  'prerequisites': ['引用与 const', '构造、赋值与析构', '函数重载与模板参数推导'],
  'related': ['raii-exception-safety', 'vector-invalidation', 'cpp-memory-model'],
  'demo':
    {
      'file': 'examples/move-value-categories.cpp',
      'platform': 'portable',
      'exercise': '先预测 named return 和 reserve 的构造次数，再分别开启与关闭可选复制消除运行；保持静态断言，解释哪些结果由标准保证、哪些只反映当前实现。',
    },
  'references':
    [
      {
        'title': 'C++20 draft N4861: value categories',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/basic.lval',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: decltype specifiers',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/dcl.type.decltype',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: type property predicates',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/meta.unary.prop',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: forward, move and move_if_noexcept',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/forward',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: copy and move constructors',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/class.copy.ctor',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: function template argument deduction',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/temp.deduct.call',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: reference collapsing',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/dcl.ref',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: moved-from library types',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/lib.types.movedfrom',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: initialization',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/dcl.init',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: copy and move elision',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/class.copy.elision',
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
        'title': 'C++20 draft N4861: vector capacity and reserve',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/vector.capacity',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: temporary object lifetime',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/class.temporary',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: unique_ptr constructors',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.ctor',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'move-value-categories-q01',
        'level': 'L1',
        'prompt': 'lvalue、xvalue、prvalue 分类的是对象还是表达式？',
        'answer': '分类的是表达式。同一对象可以由 lvalue 表达式 x 或 xvalue 表达式 std::move(x) 指代。三种基本类别互斥；glvalue 包含 lvalue 和 xvalue，rvalue 包含 prvalue 和 xvalue。',
        'rubric': ['分类表达式', '同一对象的不同表达式', '说清两个集合的组成'],
        'source':
          { 'kind': 'derived', 'rationale': '根据表达式分类与重载选择之间的关系设计基础辨析题。' },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q02',
        'level': 'L1',
        'prompt': '声明 T&& r 之后，为什么表达式 r 是左值？',
        'answer': '变量声明类型和表达式值类别分别判断。r 的声明类型是 T&&，但用名字 r 访问对象得到 lvalue，因此不能仅凭声明中的 && 选择移动重载。std::move(r) 对对象类型产生 xvalue。',
        'rubric': ['区分声明类型与值类别', '命名引用表达式为左值', '解释 std::move(r) 的作用'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据命名右值引用在函数体内的重载行为设计代码解释题。',
          },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q03',
        'level': 'L1',
        'prompt': '执行 std::move(x) 是否已经转移了 x 的资源？',
        'answer': '单独调用只进行引用转换；对对象 x 得到保留 cv 限定的右值引用结果。是否转移资源取决于后续选中的构造、赋值或其他函数实现。仅绑定 T&& 引用不会构造新对象。',
        'rubric': ['转换本身不转移资源', '后续重载决定行为', '引用绑定不构造对象'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据 move 辅助函数的规范返回值区分转换与实际资源操作。',
          },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q04',
        'level': 'L1',
        'prompt': '对 const T 调用 std::move 为什么经常发生复制？',
        'answer': '结果为 const T&&，不能绑定通常的 T(T&&)，因为该参数会丢弃 const。若存在可访问的 T(const T&)，它可绑定这个实参而执行复制。若也没有可用复制构造，初始化可能直接不合法；自定义 const T&& 重载另行分析。',
        'rubric': ['const 被保留', '常规移动构造不可行', '复制回退有条件'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据 cv 限定和构造函数重载候选检验常见移动失败原因。',
          },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q05',
        'level': 'L1',
        'prompt': '移动后的 std::string 可以读取或再次赋值吗？',
        'answer': '成功移动后，除非另有规定，标准库对象处于有效但未指定状态。可以析构、赋新值、调用 empty 等无前置条件的操作；调用 front 前仍要确认非空。不能断言移动后的 string 必为空，也不能把未指定状态当成 UB。',
        'rubric': ['有效但未指定', '操作前置条件', '不能假设为空'],
        'source':
          { 'kind': 'derived', 'rationale': '根据标准库移动后状态要求设计调用前置条件判断题。' },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q06',
        'level': 'L1',
        'prompt': '移动构造与移动赋值在目标对象状态上有什么区别？',
        'answer': '移动构造初始化一个新对象；移动赋值修改已存在的对象，必须处理目标原来持有的资源。移动赋值即使不抛异常，也可能释放旧缓冲区或执行其他回收工作，不能只估算接管源资源的成本。',
        'rubric': ['新对象与已有对象', '旧资源处理', '不抛异常不等于低成本'],
        'source':
          { 'kind': 'derived', 'rationale': '根据资源所有者的构造和替换路径设计成本辨析题。' },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q07',
        'level': 'L2',
        'prompt': '模板中 T&& 与 std::forward<T> 如何共同保留调用者的值类别？',
        'answer': '当 T 是当前调用推导的无 cv 限定模板形参时，T&& 可为转发引用。传入 U 左值时 T 推导为 U&，T&& 折叠为 U&；传入 U 右值时 T 为 U，形参为 U&&。形参名字仍是左值，forward<T> 按 T 恢复相应引用类别。const T&& 不属于此形式。',
        'rubric': ['转发引用的推导条件', '左值实参推导与折叠', 'forward 按 T 转换'],
        'source':
          { 'kind': 'derived', 'rationale': '根据模板实参推导与引用折叠规则设计转发过程解释题。' },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q08',
        'level': 'L2',
        'prompt': '只添加一个自定义析构函数，为什么可能使代码从移动变成复制？',
        'answer': '用户声明的析构函数会阻止隐式声明移动构造，即使析构函数写成 = default。若复制构造仍可用，右值可绑定其 const T& 参数。优先让资源成员承担释放工作；确需自定义特殊成员时，逐项检查并显式声明需要的复制、移动操作。',
        'rubric': ['用户声明析构抑制隐式移动', '复制候选仍可能接受右值', '审查完整特殊成员集合'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据特殊成员生成条件设计维护改动造成行为变化的排查题。',
          },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q09',
        'level': 'L2',
        'prompt': 'is_move_constructible_v<T> 为 true 能证明 T 有移动构造函数吗？',
        'answer': '不能。它检查能否用 T&& 实参构造 T，只有 T(const T&) 的类型也可能满足。显式删除 T(T&&) 则可能让该候选胜出后报错，不能一概认为会回退复制。被定义为删除的默认化移动构造有被重载决议忽略的特别规则。',
        'rubric': ['trait 检查构造表达式', '复制可接受右值', '区分显式删除与默认化后删除'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据构造可行性与候选删除规则设计类型 trait 的边界题。',
          },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q10',
        'level': 'L2',
        'prompt': 'C++20 中 return T{}、return local 和 return std::move(local) 有何区别？',
        'answer': '返回同类型 prvalue 的第一种形式可直接初始化结果对象，不需要复制或移动构造。符合条件的具名局部变量可做 NRVO，但 NRVO 可选；未消除时按 C++20 隐式移动规则选择构造。对该局部变量显式 move 会失去 NRVO 资格，因此一般保留 return local。',
        'rubric': ['同类型 prvalue 的直接构造', 'NRVO 可选且有条件', '显式 move 阻止 NRVO'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据初始化语义和具名返回值优化条件设计返回代码评审题。',
          },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q11',
        'level': 'L2',
        'prompt': 'move_if_noexcept 在什么条件下返回 const T&，这能禁止直接移动吗？',
        'answer': '当 T 的移动构造可能抛异常且 T 可复制构造时，返回类型为 const T&，否则为 T&&。这是供调用者选择构造方式的辅助函数，不会禁止直接 std::move。普通重载决议不会仅因移动可能抛异常就自动选择复制。',
        'rubric': ['两个条件同时满足才返回 const T&', '否则返回 T&&', '区分 helper 与普通重载规则'],
        'source':
          { 'kind': 'derived', 'rationale': '根据异常相关辅助函数的条件返回类型设计机制判断题。' },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q12',
        'level': 'L2',
        'prompt': '为什么 auto&& r = std::move(std::string{"x"}) 会留下悬空引用？',
        'answer': '临时 string 绑定到 move 的引用形参，只生存到包含该调用的完整表达式结束。move 返回的引用不会把生命周期继续延长给 r；分号之后 r 悬空，随后通过它访问已销毁对象会出问题。直接写 auto&& r = std::string{"x"} 则适用局部引用的生命周期延长。',
        'rubric': ['完整表达式结束时销毁', '函数返回引用不传递寿命延长', '区分直接绑定'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据临时对象与引用形参的寿命规则设计悬空引用排查题。',
          },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q13',
        'level': 'L3',
        'prompt': 'vector 扩容时，元素移动可能抛异常且不可复制，会影响什么工程承诺？',
        'answer': '一部分旧元素若已被移走，后续移动失败就难以恢复原值。C++20 对 reserve 的无效果保证排除了不可 CopyInsertable 元素的移动构造抛异常情形，不能承诺失败后业务值完全不变。可改为真实不抛异常的移动、预留有界容量，或先用间接所有权构造待提交数据；分配失败仍需处理。',
        'rubric': ['部分移动破坏回滚条件', 'reserve 保证的例外', '方案代价与分配失败'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据容器扩容异常边界设计保持业务状态一致性的工程取舍题。',
          },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q14',
        'level': 'L3',
        'prompt': '接收并保存消息的接口应选择按值、const T& 还是 T&&？',
        'answer': '按值接收再移动入成员，能统一复制左值和接收右值，但参数先构造，拒绝请求时也可能已经消耗调用者资源。const T& 适合借用或明确复制；T&& 表达允许消耗，函数体仍需显式移动。应根据是否总会接收、复制成本及失败时保留源对象的要求选择。',
        'rubric': ['所有权意图', '参数构造与拒绝时点', '结合失败契约选择'],
        'source':
          { 'kind': 'derived', 'rationale': '根据消息接收接口的所有权和失败时点设计接口权衡题。' },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q15',
        'level': 'L3',
        'prompt': '怎样验证改成移动后确实降低了行情处理链路的延迟？',
        'answer': '固定消息大小分布与容量策略，先检查分配、复制字节数和构造次数，再在关闭轨迹日志的版本测端到端吞吐及 p50/p99/p99.9。分别观察已有容量与扩容、接管新资源与释放旧资源的路径，记录编译器和标准库。不能用一次构造计数或单次计时推导加速倍数。',
        'rubric': ['控制负载和容量', '机制计数与性能测量分开', '尾延迟和回收路径'],
        'source':
          { 'kind': 'derived', 'rationale': '根据移动相关成本来源设计可归因的性能验证题。' },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q16',
        'level': 'L3',
        'prompt': '含内联数组和指向该数组的指针的消息类型，默认移动有什么风险？',
        'answer': '默认成员移动会移动数组元素，并复制指针值，目标指针仍可能指向源对象的内联数组。源对象销毁后就悬空。可以存偏移量、按目标数组重建指针，或限制对象移动；需要同时测试复制、移动、赋值与源对象销毁后的目标访问。',
        'rubric': ['成员移动不修复自引用', '目标指针可能指向源', '重新表示或重建不变量'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据自引用布局与成员移动的组合设计对象不变量审查题。',
          },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q17',
        'level': 'L3',
        'prompt': '有界队列的 try_push(std::move(message)) 返回失败后，如何保证仍能重试原消息？',
        'answer': 'std::move 本身不消耗消息，但 try_push 的实现可能已移动它。接口必须约定失败是否保留实参；实现应在拥有可提交槽位后再消耗资源，或把未接收的所有权返回给调用者。还要定义构造失败和关闭时的处理，不能只根据 bool 结果假设 message 内容未变。',
        'rubric': ['检查失败时实参状态契约', '槽位确认与移动的顺序', '关闭或异常路径的所有权去向'],
        'source':
          { 'kind': 'derived', 'rationale': '根据背压下消息重试与资源转移的交互设计失败处理题。' },
        'companies': [],
      },
      {
        'id': 'move-value-categories-q18',
        'level': 'L3',
        'prompt': '移动赋值已标 noexcept，为什么仍可能造成热路径尾延迟？',
        'answer': 'noexcept 只承诺异常不逃出函数。目标原有资源的析构或释放、成员自身的移动工作仍会执行，可能耗时。应测替换资源时的释放位置；需要延后回收时，可交换到待回收对象并交给受控回收流程，但必须为积压设置容量和背压。',
        'rubric': ['异常承诺不代表耗时界限', '旧资源析构与释放', '延后回收需要容量控制'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据移动赋值中的旧资源处理设计延迟与回收容量取舍题。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

值类别决定表达式如何参与引用绑定和重载选择。`std::move(x)` 对对象产生保留 `const` 的 xvalue；实际资源转移由选中的构造或赋值操作完成。命名的 `T&&` 变量作为表达式仍是左值，转发引用要用 `std::forward<T>` 保留调用者类别。移动后应遵守类型的状态约定，不能假设资源已清空；返回局部值通常直接写 `return local`，保留 NRVO 的机会。

本文以 C++20 草案 N4861 为依据；C++17 引入的同类型 prvalue 直接构造语义在此适用，不套用 C++23 的隐式移动规则。

## 核心概念

每个表达式恰好属于 lvalue、xvalue、prvalue 中的一类。glvalue 包含 lvalue 和 xvalue，表示可确定对象、位域或函数身份的表达式；rvalue 包含 prvalue 和 xvalue。这两个集合在 xvalue 处相交。[值类别定义](https://timsong-cpp.github.io/cppwp/n4861/basic.lval)

下表假设 `x` 是一个 `T` 对象，`r` 的声明为 `T&& r = std::move(x)`，`f()` 按值返回 `T`。

| 表达式         | 值类别  | 解释                                                   |
| -------------- | ------- | ------------------------------------------------------ |
| `x`、`r`       | lvalue  | 通过名字指代对象，`r` 的声明类型不改变这一点           |
| `std::move(x)` | xvalue  | 指代原对象，允许匹配接受右值的重载                     |
| `T{}`、`f()`   | prvalue | 用于初始化结果对象；不应一律想象为先复制出来的临时对象 |
| `42`           | prvalue | 计算一个值；不需要拥有可观察的对象身份                 |

“能否放在等号左侧”无法完整判断值类别，`const` 左值不能被修改，用户自定义的赋值操作也可能接受右值对象。类型、值类别和生命周期应分别分析：一个 xvalue 可以指代尚会生存很久的局部对象。

对 `decltype` 还要区分语法。`decltype(r)` 对未加括号的名字取得声明类型 `T&&`；`decltype((r))` 按表达式类别得到 `T&`。demo 用静态断言检查这一区别。[decltype 规则](https://timsong-cpp.github.io/cppwp/n4861/dcl.type.decltype)

## 原理深入

### 转换后仍需选择重载

`std::move` 的返回类型是 `remove_reference_t<T>&&`，语义相当于对应的 `static_cast`。它不会分配、清空对象或调用移动构造。`T&& alias = std::move(x)` 只是建立引用；直到用该表达式初始化另一个 `T`，才进入构造函数重载选择。[move 与 forward 的规范](https://timsong-cpp.github.io/cppwp/n4861/forward)

若类型同时提供常见的 `T(const T&)` 和 `T(T&&)`，非 `const` 右值通常选择后者。`const T` 经 `std::move` 得到 `const T&&`，不能绑定会丢弃 `const` 的 `T&&`，可用的复制构造便可能胜出。没有可行候选时是编译错误，不能把“移动失败就复制”当作无条件规则。

### 转发引用保留调用者的选择

对函数模板 `template<class T> void relay(T&& value)`，当 `T` 由本次调用推导时，左值实参 `U` 使 `T` 推导为 `U&`。引用折叠规定只要参与折叠的一方是左值引用，结果就是左值引用；`U&& &&` 才折叠为 `U&&`。右值实参则通常令 `T` 为 `U`，形参类型为 `U&&`。[模板推导](https://timsong-cpp.github.io/cppwp/n4861/temp.deduct.call)、[引用折叠](https://timsong-cpp.github.io/cppwp/n4861/dcl.ref)

函数体里的名字 `value` 始终是左值。`std::forward<T>(value)` 根据推导出的 `T` 返回相应引用，让下一层重载保留调用者的左右值属性。这里不恢复 prvalue：prvalue 实参经引用参数转发后，表达式是 xvalue。无条件写 `std::move(value)` 会允许从传入的左值中取走资源，改变包装函数的调用约定。

`const T&&` 不是上述转发引用。类模板中已经确定的 `T` 用于普通成员形参 `T&&` 时，也不能仅因为出现 `&&` 就称为转发引用。这里讨论的是参数转发，不涵盖花括号初始化列表等无法按该形式直接推导的实参。

### 返回值构造有两种不同边界

按值返回 `T` 时，`return T{...}` 的同类型 prvalue 直接初始化结果对象。demo 中 `Immovable` 删除复制和移动构造，仍可由 `make_prvalue()` 返回并初始化变量；其析构函数仍须满足适用的可访问性要求。这个结论来自初始化语义，不能只用“优化器会省掉移动”解释。[C++20 初始化规则](https://timsong-cpp.github.io/cppwp/n4861/dcl.init)

`return local` 若返回同类型、非 `volatile`、具有自动存储期的具名局部对象，且该对象不是函数形参或异常处理形参，可以应用 NRVO。NRVO 是允许的优化，未应用时仍须有合法的构造路径。本文这种普通非 `const` 局部变量按 C++20 隐式移动规则参与重载选择。`return std::move(local)` 不再满足具名对象表达式的 NRVO 条件，通常徒增一次移动。[复制消除条件](https://timsong-cpp.github.io/cppwp/n4861/class.copy.elision)

这里的直接构造示例使用完整对象，不将结论推广到基类等潜在重叠子对象。返回引用也不属于返回值优化：返回对局部对象的引用会留下生命周期问题。

## 数据结构/系统内部实现

### 资源所有者的状态变化

以持有堆缓冲区的对象为例，移动构造可以接管指针、长度和容量，并让源对象进入类定义的可用状态。若对象持有内联数组，移动可能仍要逐元素处理。语言不会替用户定义类发明资源转移协议。

默认化移动构造对基类和成员逐个执行相应初始化。普通原始指针成员只是复制指针值；内联缓冲区加“指向自身缓冲区的指针”若直接默认移动，目标指针可能还指着源对象。可用偏移量表示位置，或者在自定义移动中重建指针。相关不变量必须覆盖复制、移动和赋值路径。[成员移动规则](https://timsong-cpp.github.io/cppwp/n4861/class.copy.ctor)

用户声明复制构造、复制赋值、移动赋值或析构函数，会阻止隐式声明移动构造；`~T() = default` 也属于用户声明。维护资源类时先检查现有特殊成员，能由 `unique_ptr` 等成员管理资源时，尽量让编译器生成外层操作。确需自定义时，要确认每个操作的源状态与目标旧资源处理。

### 移动后的对象仍受操作前置条件约束

标准库类型成功被移动后，除非另有规定，处于有效但未指定状态：不变量仍成立，具体值不能随意预测。可以调用没有额外前置条件的操作，例如 `string::empty()`、`clear()` 或赋新值；调用 `front()` 前应先检查非空。“未指定”允许实现保留不同的值，不等同于未定义行为。[标准库移动后状态](https://timsong-cpp.github.io/cppwp/n4861/lib.types.movedfrom)

某些类型给出更强后置条件。`unique_ptr` 的移动构造会使源指针为空，demo 因而可以断言 `!owner`；对 `string` 不作同样断言。[unique_ptr 移动构造](https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.ctor)

用户定义类型应自行写清契约。demo 的 `Trace` 移动构造把源值设为 `-1`，这是本类型的实现约定；它的默认化移动赋值只赋值整数成员，不会把源值改为 `-1`。移动赋值还需要处理目标旧资源和自移动；标准库的有效但未指定要求不自动替自定义类证明安全。

### noexcept 如何影响扩容路径

`vector::reserve` 需要扩容时，要把已有元素构造到新存储中。若复制失败，旧元素通常还可保留原值；若先移动了一批元素，后面的移动又失败，旧值就可能难以恢复。因此可复制但移动可能抛异常的元素，常见实现会选择复制。标准规定的是容量、复杂度与异常效果，并不要求实现内部必须调用某个 helper。[reserve 的规范](https://timsong-cpp.github.io/cppwp/n4861/vector.capacity)

`std::move_if_noexcept(x)` 在“移动构造可能抛异常且可以复制构造”时返回 `const T&`，否则返回 `T&&`。不可复制时，它仍可能返回会触发抛异常移动的引用。对于不可 CopyInsertable 的元素，`reserve` 的无效果保证排除了其移动构造抛异常的情形；调用方不能承诺失败后每个业务值都保持原样。

`noexcept` 必须反映真实实现。异常逃出不抛异常的函数会调用 `std::terminate`；把会失败的分配路径随意标成 `noexcept` 会改变故障处理方式。普通重载决议也不会因移动函数缺少 `noexcept` 就自动改选复制。[异常说明](https://timsong-cpp.github.io/cppwp/n4861/except.spec)

## C++ runnable demo

程序先核对命名右值引用、转发和 `const` 的重载行为，再测试 `move_if_noexcept`、资源所有权与返回值构造。`Trace` 只携带一个整数，用构造计数解释操作路径；它不是字符串或缓冲区的性能模型。`Risky` 的移动签名允许抛异常，但函数体不注入异常，本示例不验证容器的失败回滚。

```cpp include=examples/move-value-categories.cpp

```

在支持 C++20 的 GCC 或 Clang 环境编译运行：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/move-value-categories.cpp -o /tmp/move-demo
/tmp/move-demo
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -fno-elide-constructors examples/move-value-categories.cpp -o /tmp/move-no-elide
/tmp/move-no-elide
```

`-fno-elide-constructors` 是这些编译器的实验开关，用来观察可选消除；它不会把 C++17 起的同类型 prvalue 初始化改回需要复制的语义。`Immovable` 在两次编译中都应通过。

成功输出包含 `explicit construction: copies=1 moves=1` 和最后一行 `value categories, forwarding, ownership and return checks passed`。`named return` 的计数取决于是否做 NRVO；两种 `reserve` 计数只用于记录库实现的选择，程序不对它们作精确次数断言。扩容以 `capacity() + 1` 为请求值，避免把初次 `reserve(2)` 误解为容量必定恰好为 2。

测试覆盖正常移动、从 `const` 复制、源对象复用和不可移动对象的直接构造。不执行悬空引用访问，不模拟异步队列，也不提供延迟数字。

## 高频追问

### 转发引用能延长临时对象的生命周期吗？

局部声明 `auto&& r = std::string{"x"}` 直接绑定临时对象，适用生命周期延长；换成 `auto&& r = std::move(std::string{"x"})`，临时对象绑定到函数的引用形参，只生存到包含该调用的完整表达式结束。`move` 返回引用不能把寿命延长继续传给 `r`。分号之后再访问已销毁的字符串会产生未定义行为。[临时对象的生命周期及例外](https://timsong-cpp.github.io/cppwp/n4861/class.temporary)

同样，包装函数若通过 `decltype(auto)` 返回 `std::forward<T>(value)`，调用者必须分析底层对象由谁持有、何时销毁。完美转发只保留类型与类别信息，不承担生命周期管理。

### 能从右值构造，就一定存在移动构造函数吗？

`is_move_constructible_v<T>` 检查用 `T&&` 实参构造 `T` 是否可行。只有 `T(const T&)` 的类型也可得到 `true`。[构造 trait 的定义](https://timsong-cpp.github.io/cppwp/n4861/meta.unary.prop)

若显式声明 `T(T&&) = delete`，这个重载可以先被选中，再使程序不合法；不能期望编译器再退回复制。被定义为删除的默认化移动构造另有忽略该候选的规则。demo 分别检查了 `CopyOnly` 和 `DeletedMove`。

### 接收所有权的接口为什么还要讨论失败？

按值形参在进入函数体前就要构造。调用者传入右值后，即使函数随后发现队列已满，资源也可能已经转移到参数。`T&&` 形参的绑定本身不转移资源，可以让函数在确认能接收后才移动；但是否这样实现，要由接口契约保证。只读借用常用 `const T&`，要保存独立副本时则需明确复制。

## 容易答错的点

| 说法                              | 修正与原因                                                      |
| --------------------------------- | --------------------------------------------------------------- |
| 加上 `std::move` 就会调用移动构造 | 转换后仍要重载决议；`const`、删除的候选、不可访问函数都影响结果 |
| `T&&` 变量每次使用都会自动移动    | 变量名表达式是左值，消耗参数时需明确转换                        |
| move 后对象必为空或不可再用       | 状态由类型契约规定；标准库默认是有效但未指定                    |
| 任意 `T&&` 都是转发引用           | 要检查模板推导位置与 cv 限定                                    |
| 返回局部对象要加 move 才快        | 对符合 NRVO 条件的局部对象，这样会失去 NRVO 资格                |
| `noexcept` 表示操作耗时恒定       | 它约束异常传播，不约束分配、析构和执行时间                      |
| 默认移动能正确迁移所有内部指针    | 指针值会被复制，自引用关系需由类设计维护                        |

## 性能分析

先根据类型判断移动做了哪些工作。接管堆缓冲区可能只需有限个字段操作；内联数组移动仍要处理元素。移动赋值还可能释放目标已有资源，调用点的成本因此可能大于移动构造。容器扩容即使对每个元素都使用常数成本的移动，也仍要分配存储并处理已有元素；不能把某个元素的成本写成整个扩容操作的成本。

设计实验时固定消息大小分布、预留容量、分配器及编译器配置。将已有容量下的插入与触发扩容的插入分别记录，再测整体负载，避免只挑容易得到好结果的路径。构造次数、分配次数和复制字节数有助于解释机制；正式计时关闭 `std::cout` 等轨迹输出，并让结果被实际消费，检查优化后的代码是否仍执行预期工作。

报告吞吐和端到端 p50、p99、p99.9，保留预热、重复测量、硬件与标准库版本。若尾延迟出现在旧资源释放，应测从接收消息到回收完成的路径，而不只测 `std::move` 表达式。本文没有性能实测，示例输出不能提供通用加速倍数。

## Quant/Low-Latency 场景

行情解析器可以把拥有原始字节的缓冲区交给后续处理阶段，减少重复保存同一载荷的机会。缓冲区被移动后，解析器不能继续依赖源对象的内容；持有其内联数据视图的其他字段还需检查是否失效。若数据来自共享接收环，单纯移动一个指针不会延长槽位寿命，仍要规定槽位何时能复用。

有界队列的 `try_push` 应明确失败时是否保留输入消息。为支持重试，可以先取得槽位并确认提交条件，再消耗输入；也可以在返回结果中交回未接收的所有权。若对象构造失败或队列关闭，必须能说明当前拥有者是谁，不能在返回 `false` 后默认重发一个已经被移动的消息。

消费者用移动赋值替换上一批数据时，旧缓冲区的析构可能发生在处理线程。若将旧资源交换到回收队列，可把释放工作移到别处，但回收队列本身需要容量和背压策略。移动语义也不建立线程间同步；生产者发布消息和消费者读取消息之间，仍需要符合内存模型的同步关系。

## 相关专题

- [RAII 与异常安全](raii-exception-safety.md)：先理解资源清理与提交边界，再判断移动失败后的状态承诺。
- [vector：扩容与迭代器失效边界](vector-invalidation.md)：继续检查元素搬移后的引用、指针和迭代器可用性。
- [C++ memory model](../concurrency/cpp-memory-model.md)：跨线程交接所有权时，继续核对发布、可见性和对象复用。
- [CPU cache / false sharing](../performance/cpu-cache-false-sharing.md)：移动减少复制后，检查队列元数据与回收计数是否仍造成共享写流量。

## 分层面试题

先按表达式类型和值类别预测重载，再说明对象在调用前后的状态。L3 题需要给出接口失败语义、成本所在的路径和可验证的选择依据。
