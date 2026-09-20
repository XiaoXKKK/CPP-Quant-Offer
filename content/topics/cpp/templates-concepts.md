---
{
  'schemaVersion': 1,
  'id': 'templates-concepts',
  'title': '模板与 concepts：约束接口和实例化边界',
  'description': '用 C++20 报价适配器解释 requires、模板实例化、SFINAE 与约束重载，并区分编译期可检查条件和运行时业务语义。',
  'category': 'cpp',
  'areas': ['Template / Compile Time', 'Modern C++'],
  'tags': ['templates', 'concepts', 'requires', 'sfinae'],
  'difficulty': 'L2',
  'roles': ['C++ Developer', 'Quant Developer', 'Low-Latency C++ Developer'],
  'companyTypes': ['高频交易', '量化私募', 'Trading Firm'],
  'status': 'published',
  'updated': '2026-09-19',
  'reviewed': '2026-09-19',
  'standard': 'C++20',
  'estimatedMinutes': 50,
  'prerequisites': ['函数模板和函数重载', '引用、const 与 decltype', '编译错误与运行时错误的区别'],
  'related':
    ['move-value-categories', 'raii-exception-safety', 'vector-invalidation', 'order-book'],
  'demo':
    {
      'file': 'examples/templates-concepts.cpp',
      'platform': 'portable',
      'exercise': '新增一个返回 const int64_t& 的报价适配器，预测 QuoteLike 检查结果；再设计允许该接口的约束并说明复制值、借用寿命和 noexcept 的变化，保留正反 static_assert。',
    },
  'references':
    [
      {
        'title': 'C++20 draft N4861: requires expressions',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/expr.prim.req',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: implicit instantiation',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/temp.inst',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: template argument deduction',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/temp.deduct',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: template name resolution',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/temp.res',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: constraint logical operations',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/temp.constr.op',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: atomic constraints',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/temp.constr.atomic',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: constraint normalization',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/temp.constr.normal',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: partial ordering by constraints',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/temp.constr.order',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: language-related concepts',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/concepts.lang',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: equality preservation',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/concepts.equality',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: constexpr if',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/stmt.if',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC developer options: compilation time reports',
        'url': 'https://gcc.gnu.org/onlinedocs/gcc/Developer-Options.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'templates-concepts-q01',
        'level': 'L1',
        'prompt': 'concept 给函数模板增加了什么，是否会在运行时检查每个对象？',
        'answer': 'concept 为模板参数命名一组编译期约束，函数关联约束不满足时不能作为可行候选。它不在运行时逐个检查对象；同一报价类型既可能含正常价格，也可能含倒挂或越界价格，这些值仍需运行时处理。',
        'rubric': ['模板参数与可行候选', '类型条件不验证对象值'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从约束参与重载与报价值校验的分工推导，区分编译期和运行时。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q02',
        'level': 'L1',
        'prompt': 'requires 子句与 requires 表达式分别放在哪里、表示什么？',
        'answer': 'requires 子句把约束关联到模板或模板化函数声明；requires 表达式通过一组需求形成 bool 值，可用来定义 concept。requires requires(T x) { x.f(); } 中，第一个引入子句，第二个开始表达式。',
        'rubric': ['子句约束声明', '表达式产生 bool'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据两种 requires 语法角色推导，检查常见的重复关键字写法。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q03',
        'level': 'L1',
        'prompt': 'requires 表达式中的四类需求各检查什么？',
        'answer': '简单需求检查表达式是否合法，类型需求检查类型名是否有效，复合需求可继续检查 noexcept 和表达式结果类型，嵌套需求要求另一个约束成立。这些被检查的表达式不会为了检查而在运行时执行。',
        'rubric': ['四类需求的区别', '被检查表达式不求值'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据 requires 需求分类推导，要求能把语法对应到检查目标。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q04',
        'level': 'L1',
        'prompt': 'requires { std::is_integral_v<T>; } 能排除 double 吗？',
        'answer': '不能。这里是简单需求，只要求 std::is_integral_v<T> 这个表达式能形成，结果为 false 仍然合法。要限制整数，可用 std::integral<T>，或在需求体内写 requires std::is_integral_v<T>; 作为嵌套需求。',
        'rubric': ['合法表达式与 true 的区别', '嵌套需求或标准 concept'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从简单需求不检查布尔结果推导，构造可由静态断言核对的反例。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q05',
        'level': 'L1',
        'prompt': 'requires(const T& x) 与 requires(T& x) 会筛出同一组接口吗？',
        'answer': '不一定。const 左值只能调用相应的 const 可用成员，非 const 左值还可能匹配可写或不同引用限定的重载。约束应模拟函数体实际使用的 cv 和值类别，否则检查通过也未必能执行函数体中的调用。',
        'rubric': ['const 和重载选择', '约束与实际调用一致'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由需求中局部参数类型推导接口差异，检查只读适配器边界。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q06',
        'level': 'L1',
        'prompt': '复合需求的 same_as<int64_t> 和 convertible_to<int64_t> 有什么区别？',
        'answer': 'same_as 要求 decltype((表达式)) 恰好是 int64_t，引用和 const 差异也会影响结果。convertible_to 允许符合要求的隐式和显式转换，并有转换结果一致等语义要求；它不自动检查价格单位、数值范围或业务允许的舍入。',
        'rubric': ['精确类型与可转换', 'decltype 保留引用性质', '转换的语义范围'],
        'source':
          {
            'kind': 'derived',
            'rationale': '对照标准类型概念与复合需求结果类型规则，推导适配器约束。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q07',
        'level': 'L2',
        'prompt': '一个函数调用在 requires 中合法，为什么实际调用仍可能编译失败？',
        'answer': '检查可能只需要函数声明，不需要实例化函数体。示例 body_checked_later 显式返回 int，因此 int 实参能形成调用表达式；真正调用时才实例化 value.missing()，随后报错。若返回类型由 auto 推导，检查时就可能需要实例化函数体。',
        'rubric': ['声明与函数体的实例化时机', 'auto 返回推导的影响'],
        'source':
          { 'kind': 'derived', 'rationale': '由显式返回类型的调用检测构造反例，检查实例化边界。' },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q08',
        'level': 'L2',
        'prompt': 'SFINAE 能否把模板函数体中的所有错误都变成候选移除？',
        'answer': '不能。SFINAE 针对相应替换的直接上下文，函数体错误或替换时触发的其他实例化错误可能是硬错误。concepts 提供显式约束与约束排序，但也不把任意函数体错误变成 false；应把接口真正需要的操作放进约束并独立编译函数体。',
        'rubric': ['直接上下文', '实例化副作用与函数体错误', '约束不能吞掉任意错误'],
        'source':
          {
            'kind': 'derived',
            'rationale': '对照模板推导和 requires 的失败规则，检验两者的适用边界。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q09',
        'level': 'L2',
        'prompt': '先检查 HasPriceType<T> 再检查 T::price_type 是整数，为什么有用？',
        'answer': '约束合取按从左到右检查，左侧不满足时不再检查右侧满足性，因此可先建立右侧访问嵌套类型所需的前提。requires 需求序列也按词法顺序停止检查；这不允许模板外非法表达式，也不能用短路修复约束规范化时的非法参数映射。',
        'rubric': ['合取短路', '需求序列顺序', '短路保护的边界'],
        'source':
          {
            'kind': 'derived',
            'rationale': '结合约束逻辑与规范化规则，分析安全前置检查及其限制。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q10',
        'level': 'L2',
        'prompt': '为什么复制同一段布尔条件并加一个条件，不一定得到更受约束的重载？',
        'answer': 'subsumption 按规范化后的原子约束身份和逻辑结构排序，不做一般数学蕴含证明。两个位置各写一次 is_integral_v<T>，原子约束来源不同。共享命名 concept 后，在它上面合取额外条件，才能在相应参数映射下保留可识别的共同原子。',
        'rubric': ['原子约束来源及参数映射', '不做一般逻辑证明', '复用命名 concept'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据原子约束相同性与 subsumption 推导可验证的重载歧义。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q11',
        'level': 'L2',
        'prompt': '把检查写进 if constexpr 和写成函数 requires，有什么调用层面的差别？',
        'answer': '函数约束在候选可行性检查中参与筛选；if constexpr 在被选中模板的函数体内选择实现分支。一个没有接口约束的模板不会因为某分支不适用就自动退出重载。模板内丢弃的依赖分支可不实例化，但非依赖错误仍需合法。',
        'rubric': ['候选筛选与函数体分支', '被丢弃分支的依赖性'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据约束检查与 constexpr if 的阶段区别，分析接口和实现分工。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q12',
        'level': 'L2',
        'prompt': 'QuoteLike<Quote> 为 true 为什么不能证明报价没有倒挂或读取副作用？',
        'answer': 'QuoteLike 只声明嵌套类型、const 调用和返回类型要求，没有证明 bid 小于等于 ask，也不执行 getter 检查副作用。即使加 noexcept，也只排除潜在抛出表达式，不验证报价一致性或延迟。语义约定要用说明、审查和适当的运行时验证落实。',
        'rubric': ['语法条件与业务语义', 'noexcept 的限定', '语义验证方法'],
        'source':
          { 'kind': 'derived', 'rationale': '用类型相同而价格值不同的输入推导编译期检测的局限。' },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q13',
        'level': 'L3',
        'prompt': '大型模板库换成 concepts 后，如何评估编译成本是否改善？',
        'answer': '固定编译器、优化级别和类型集合，分别测全量构建、典型头文件修改后的增量构建、峰值内存和目标文件大小。使用编译器阶段报告定位解析、实例化或优化成本，并重复测量。约束能减少某些候选工作，但不能据此承诺整个构建更快。',
        'rubric': ['全量与增量构建', '类型集合和工具链控制', '阶段归因而非预设收益'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从候选筛选与实例化工作量设计编译性能实验，避免无证据的提速结论。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q14',
        'level': 'L3',
        'prompt': '行情适配器什么时候保留模板，什么时候考虑类型擦除边界？',
        'answer': '编译期已知且数量有限的适配器可保留模板以便静态选择和潜在内联；运行时选择实现或需要减少接口暴露时，可在边界使用类型擦除。后者需要评估间接调用、所有权、可能的分配与 ABI 约定，不能只比较一段调用指令。',
        'rubric': ['类型集合与选择时机', '内联和实例化成本', '类型擦除的生命周期与调用成本'],
        'source':
          {
            'kind': 'derived',
            'rationale': '将静态适配器用于工程边界设计，比较构建和运行时成本。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q15',
        'level': 'L3',
        'prompt': '通用算法的 concept 应要求默认构造和复制能力吗？',
        'answer': '取决于算法真实使用的操作。若只借用 const 引用读取报价，额外要求默认构造和复制会拒绝可用适配器，并限制资源拥有者。约束还应明确返回值或借用引用、异常传播与所需值类别；不能用宽泛概念掩盖实际接口需要。',
        'rubric': ['按实际操作设约束', '避免额外排除类型', '借用和异常接口'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从只读算法对类型能力的需求推导约束设计，考查过度约束的代价。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q16',
        'level': 'L3',
        'prompt': '给已有函数模板新增或加强 concept，为什么要做兼容性审查？',
        'answer': '旧调用可能从可行变为不可行，也可能改选其他重载，甚至因为共同原子约束来源改变出现歧义。应编译代表性调用类型、检查重载返回类型和语义，并保留失败测试；库接口还要检查显式实例化和跨模块使用，不把约束改动当纯注释。',
        'rubric': ['可行候选集合变化', '重载选择和歧义', '调用矩阵回归'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由约束参与重载选择推导接口演进风险，要求可执行兼容性验证。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q17',
        'level': 'L3',
        'prompt': '怎样测试 concept，避免只有一种成功类型的 static_assert？',
        'answer': '设计恰好缺一个能力的类型，例如缺 getter、返回类型错误、仅非 const 成员和潜在抛出成员；再验证重载选择及实际函数调用。需要诊断的失败用例单独编译并检查失败原因。无须诊断的 ill-formed 情形不能写成必须编译失败的可移植测试。',
        'rubric': ['单一差异的负例', '实际调用与重载选择', '区分要求诊断与 IFNDR'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据可满足性与实例化边界设计测试矩阵，避免错误的编译失败断言。',
          },
        'companies': [],
      },
      {
        'id': 'templates-concepts-q18',
        'level': 'L3',
        'prompt': '报价 getter 都通过 concept 后，接入交易链路前还要验证哪些语义？',
        'answer': '确认价格单位与范围、bid 和 ask 是否来自同一有效快照、借用数据的寿命以及调用是否会推进或消费底层流。检查缺口、过期和非法值的处理，并让错误阻止未经验证的数据进入下游。concept 的返回类型与 noexcept 检查不能证明这些条件。',
        'rubric': ['单位与快照一致性', '寿命和读取副作用', '错误路径与下游边界'],
        'source':
          {
            'kind': 'derived',
            'rationale': '将只读报价接口放入行情数据流，考查无法由类型检查证明的契约。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

函数模板根据模板实参形成特化，按需要实例化声明或函数体。C++20 concepts 把类型与表达式要求写在接口上；约束不满足时，相应函数不能成为可行候选。`requires` 能检查调用是否有效、结果类型和异常规格，但不会验证价格范围、比较关系或线程安全。模板体错误也不一定能被约束检查提前发现。

## 核心概念

本章只讨论 C++20 中的接口约束、实例化和重载选择，不展开模板元编程工具箱。`template<QuoteLike T>` 表示 `T` 必须满足 `QuoteLike`；它没有要求 `T` 继承某个基类，也没有创建运行时接口对象。

`requires` 有两个位置相邻但作用不同的用法。requires 子句关联到声明，决定其约束；requires 表达式用一组需求计算 `bool` 值，可以作为 concept 的定义。`requires requires(T x) { x.f(); }` 中，第一个关键字引入子句，第二个引入表达式。[requires 表达式规范](https://timsong-cpp.github.io/cppwp/n4861/expr.prim.req) 给出了四类需求：

| 需求     | 示例形式                                                        | 检查内容                                             |
| -------- | --------------------------------------------------------------- | ---------------------------------------------------- |
| 简单需求 | `quote.bid_ticks();`                                            | 该表达式是否合法，不要求其结果为 true                |
| 类型需求 | `typename T::price_type;`                                       | 类型名是否有效；仅写一个类模板特化名不要求其完整定义 |
| 复合需求 | `{ quote.bid_ticks() } noexcept -> std::same_as<std::int64_t>;` | 表达式有效，且不潜在抛出，结果类型满足后面的约束     |
| 嵌套需求 | `requires std::same_as<typename T::price_type, std::int64_t>;`  | 指定约束必须成立                                     |

需求中的表达式不求值。`requires(const T& quote)` 引入的是检查时使用的局部参数记号，没有为它分配存储或构造对象。因此，写出这种参数不要求 `T` 可默认构造。

复合需求检查的是 `decltype((表达式))`。返回 `int64_t` 的 getter 与返回 `const int64_t&` 的 getter 不会同时满足 `same_as<int64_t>`。换成 `convertible_to<int64_t>` 会接受更多类型，但转换合法仍不能证明价格单位和范围符合业务约定。标准概念的定义和语义见 [language-related concepts](https://timsong-cpp.github.io/cppwp/n4861/concepts.lang)。

## 原理深入

### 约束通过后，函数体仍有工作要检查

模板的非依赖名称与语法可以在模板定义阶段检查，依赖模板参数的操作则可能要等具体类型已知。实例化也有粒度：需要类模板特化的完整类型时，并不意味着同时实例化所有普通成员函数体；函数定义在程序语义要求时才实例化。边界见 [模板名称解析](https://timsong-cpp.github.io/cppwp/n4861/temp.res) 和 [隐式实例化](https://timsong-cpp.github.io/cppwp/n4861/temp.inst)。

示例的 `body_checked_later(T)` 显式声明返回 `int`。在 `requires` 中形成 `body_checked_later(value)` 调用时，无须靠函数体推导结果类型，所以 `DeclarationCallable<int>` 为 true。实际调用才暴露函数体中的 `value.missing()` 错误。把返回类型改成需要推导的 `auto`，可能让检查表达式时就触发函数体实例化。不能把“表达式能形成”理解成“所有被调用实现均已验证”。

SFINAE 在对应替换的直接上下文中把不适用的模板排除，例如检测 `decltype(q.bid_ticks())` 的偏特化无法匹配没有该成员的类型。它不会普遍隐藏函数体错误，替换过程中触发的其他模板实例化也可能产生硬错误。[模板实参推导](https://timsong-cpp.github.io/cppwp/n4861/temp.deduct) 明确区分直接上下文与这些副作用。concepts 提供可命名的约束和约束排序，但相同的工程要求仍然存在：检测接口，同时编译实际调用。

### 短路有用，但受检查上下文限制

`HasPriceType<T> && std::integral<typename T::price_type>` 先检查左边。若类型没有 `price_type`，合取不满足，不再检查右侧的满足性。析取在左侧满足时也不再检查右侧。requires 需求序列按词法顺序进行替换与检查，一旦确定结果便停止。可以把便宜的存在性检查放在依赖它的后续检查之前，规则见 [约束逻辑操作](https://timsong-cpp.github.io/cppwp/n4861/temp.constr.op)。

这类行为需要放在适当的模板上下文中理解。直接在普通作用域写 `requires(int value) { value.bid_ticks(); }`，其中的非法表达式是编译错误，不会得到一个可供使用的 false。对于任何模板实参替换都必然失败的需求，C++20 还规定了 ill-formed, no diagnostic required（IFNDR）的情形；编译器不拒绝也不能证明程序合法。

约束规范化发生在满足性检查之前。若展开 concept 时，参数映射自身形成了非法类型，例如“指向引用的指针”，不能用右侧 `|| true` 来补救。相关规则见 [constraint normalization](https://timsong-cpp.github.io/cppwp/n4861/temp.constr.normal)。本章的预期失败测试只使用要求诊断的错误，不把 IFNDR 当作必须报错的测试。

### 约束重载按原子来源排序

示例先定义 `QuoteLike`，再定义 `NoThrowQuote = QuoteLike<T> && ...`。两个 `route` 重载都能接受 `Quote`，但第二个沿用第一个的共同约束并增加无异常调用要求，在相同参数匹配条件下更受约束。`PotentiallyThrowingQuote` 只满足前者，因此选择一般版本。

这种 subsumption 关系建立在规范化后的原子约束及其参数映射上。两个原子要有相同的表达式来源和符合规则的参数映射，不能只看它们算出的 bool 值。若两个重载分别手写 `std::is_integral_v<T>` 与 `std::is_integral_v<T> && std::is_signed_v<T>`，重复拼写产生不同来源的原子；即使对 `int` 都为 true，也可能无法按约束排序，示例因此出现歧义。

命名 concept 能让复用处共享定义里的原子来源。它不会把任意等价布尔公式变成同一约束，编译器也不负责证明一般数学蕴含。准确规则见 [原子约束](https://timsong-cpp.github.io/cppwp/n4861/temp.constr.atomic) 与 [约束偏序](https://timsong-cpp.github.io/cppwp/n4861/temp.constr.order)。重载选择还受实参转换等其他规则影响，不能概括成“concept 写得越长就越优先”。

## 数据结构/系统内部实现

理解编译过程时，可以把每个候选看作“声明、待替换的模板参数、关联约束及必要时才检查的定义”。调用点推导参数，检查相应候选的约束满足性，再依据重载规则比较可行候选；需要函数定义或返回类型推导时，继续实例化实现。这个顺序描述依赖关系，不规定编译器必须使用某种内部容器或缓存布局。

约束规范化保留合取、析取和原子条件；concept 的名字会展开到定义中的约束及参数映射。复合需求里的 `noexcept` 检查调用表达式的异常性质，不会执行 getter 验证它实际是否抛出，也不会自动给外层函数添加 `noexcept`。示例 `checked_spread` 可以接受潜在抛出的 getter，相应异常按普通调用传播。

满足编译器检查的条件，不等于履行所有语义约定。标准库对部分概念另有等价保持、稳定性等要求，[equality preservation](https://timsong-cpp.github.io/cppwp/n4861/concepts.equality) 也给出“satisfies 但不 models”的区别。本文自定义 `QuoteLike` 没有宣称自动获得这些性质：两个 getter 可能来自不同快照，返回值可能倒挂，方法也可能借助 `mutable` 改动状态。程序必须另行规定哪些行为被允许。

## C++ runnable demo

示例为一个本地报价检查器。`QuoteLike` 要求价格类型和两个 const getter 的结果都是 `int64_t`；`NoThrowQuote` 增加无异常调用要求。静态断言覆盖缺成员、错误结果类型、仅非 const 成员、潜在抛出 getter，以及正反重载选择。运行时才检查具体价格是否符合本例约定。

```cpp include=examples/templates-concepts.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/templates-concepts.cpp -o /tmp/concepts-demo
/tmp/concepts-demo
```

正常输出 `concept checks passed; total spread=3`。本例人为限定价格为 `0` 到 `1,000,000,000` 的整数 ticks，先验证范围和顺序，再计算差值，以免产生有符号溢出；这个范围和非负假设不代表所有市场。三条固定样本的总和有界，断言检查零价差、允许的最大价差、倒挂、负值和超过上限。

文件里的四个宏仅供独立的失败编译实验。默认未定义这些宏，所以普通构建和 CodeLAB 可正常运行。以下命令均预期编译器非零退出，不生成待运行程序：

```bash
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -fsyntax-only -DCONCEPTS_FAIL_CONSTRAINT examples/templates-concepts.cpp
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -fsyntax-only -DCONCEPTS_FAIL_SUBSUMPTION examples/templates-concepts.cpp
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -fsyntax-only -DCONCEPTS_FAIL_NON_TEMPLATE examples/templates-concepts.cpp
g++ -std=c++20 -Wall -Wextra -Wpedantic -Werror -fsyntax-only -DCONCEPTS_FAIL_BODY examples/templates-concepts.cpp
```

| 宏                           | 应核对的失败原因                                               |
| ---------------------------- | -------------------------------------------------------------- |
| `CONCEPTS_FAIL_CONSTRAINT`   | `MissingAsk` 不满足 `QuoteLike`，`checked_spread` 没有匹配调用 |
| `CONCEPTS_FAIL_SUBSUMPTION`  | 手写条件的原子来源不同，`ambiguous(1)` 调用有歧义              |
| `CONCEPTS_FAIL_NON_TEMPLATE` | 模板外需求直接访问 `int` 的不存在成员                          |
| `CONCEPTS_FAIL_BODY`         | 真正实例化 `body_checked_later<int>` 时，函数体访问不存在成员  |

这些诊断已用 GCC 13.3 验证，其他编译器的措辞和回溯长度可以不同。检查失败原因，而不要只把任意非零退出都算通过。程序是可移植 C++20 教学例，不含网络输入、序列恢复或并发快照协议；`nullopt` 只表示本例价格验证失败，不是完整的交易错误模型。

## 高频追问

“约束和 `if constexpr` 该选哪一个？”用约束描述函数接受什么接口，用 `if constexpr` 在已接受的类型上选择实现分支。后者本身不让外层函数退出重载候选。模板实例化时，依赖条件选定后丢弃的分支可不实例化；这不允许随意写非依赖语法错误。规则见 [`constexpr if`](https://timsong-cpp.github.io/cppwp/n4861/stmt.if)。

“没有 `noexcept` 的 getter 必须被拒绝吗？”由接口决定。若调用者允许异常传播，`QuoteLike` 即可描述本例读取操作。只有算法或调用边界需要非抛出操作时才使用 `NoThrowQuote`。即使 getter 是 `noexcept`，也不能由此推断它不分配、不阻塞或执行时间有上界。

“requires 里写 `std::is_integral_v<T>;` 为什么没筛掉 double？”简单需求只要求表达式合法。示例 `IntegralSpelling<double>` 为 true，而含嵌套需求的 `ReallyIntegral<double>` 为 false。检查布尔条件的值与检查表达式能否写出来，是两种需求。

## 容易答错的点

- 对 concept 的一次正面断言，只覆盖一个模板实参组合。它没有测试完整类型集合，更没有执行业务操作。
- `requires(const T& x)` 没有创建 `T` 对象，不自动要求默认构造；`x` 的 const 和引用性质却会影响成员查找与重载。
- 类型需求只检查类型名；检查 `typename Wrapper<T>;` 不等于已经编译 `Wrapper<T>` 的全部成员实现。
- `same_as` 的精确匹配包括引用性质。把可转换返回值直接当成精确返回值，会误判适配器是否满足约束。
- `noexcept` 约束不验证线程安全、内存分配或行情一致性，也不会自动改变调用者的异常规格。
- 把同一段表达式复制到不同 concept 或重载处，可能改变原子约束身份。修改约束时应重新编译重载调用矩阵。

## 性能分析

约束满足性和重载选择在编译期完成，本身不增加逐条行情消息的运行时类型检查。实际函数仍要读取数据、验证价格并计算结果；模板参数已知给编译器提供了静态选择和内联机会，但不保证最终必定内联或比虚函数快。

对本例单条报价，调用次数和算术次数为常数；处理 `n` 条报价为 `O(n)`。这个成本还依赖 getter 的实现。concept 没有声明 getter 必须常数时间，不能从它的签名推出任意适配器的端到端复杂度。

编译成本应分别看源文件解析、约束检查、实例化与优化。比较 SFINAE 和 concepts 两种接口时，固定编译器、优化级别、类型集合、翻译单元与缓存条件，记录全量和增量构建时间、峰值内存、目标文件大小。GCC 的 [`-ftime-report`](https://gcc.gnu.org/onlinedocs/gcc/Developer-Options.html) 可辅助查看编译阶段耗时；工具版本影响报告分类，应保存原始输出。本文没有报告编译性能实测，也不承诺改用 concepts 会缩短构建时间。

类型种类很多时，重复实例化和代码体积值得检查。把不依赖具体类型的算法提取到普通函数，或在模块边界转换成统一数据结构，可以减少部分重复；类型擦除还可以在运行时选择实现，但需要评估间接调用、存储与销毁约定，某些实现也会分配内存。对比时同时观察真实调用分布、吞吐与 p99/p99.9，以及代码体积对指令缓存的影响。

## Quant/Low-Latency 场景

同一策略可能读取回放报价、模拟报价和实时接收后的本地快照。模板适配器可以要求它们提供相同的 const getter 与价格类型，在编译期发现某个适配器缺少字段或返回了浮点结果。约束应贴近算法实际调用；只读取引用时，额外要求默认构造或复制会排除本来可用的资源拥有者。

进入计算前仍须明确 tick 单位、价格边界、快照有效性和借用寿命。两个 getter 分别从会变化的共享对象读取，即使类型和 `noexcept` 都符合要求，也可能混合不同版本的 bid/ask。可在上游先形成一致快照再调用算法；缺口、过期或非法数据按链路约定拒绝或恢复，不能因为通过 concept 就继续用于下单。

若适配器集合在构建时已知且数量有限，模板能保留静态接口。若要加载运行时插件，可将受约束的模板用于适配层内部，在外部暴露有明确所有权和 ABI 约定的接口。类型擦除不会自行保证跨编译器二进制兼容，接口版本和对象销毁方仍要单独设计。

## 相关专题

- [move 与值类别](move-value-categories.md)：约束中的 const、引用和表达式结果类型会影响可调用性。
- [RAII 与异常安全](raii-exception-safety.md)：区分非抛出接口、失败回滚和资源回收。
- [vector 失效边界](vector-invalidation.md)：容器对元素操作的要求，还需要结合实例化路径和异常保证分析。
- [Order Book](../trading/order-book.md)：接口类型检查之后，继续检查行情数据和状态更新语义。

## 分层面试题

L1 要能把约束语法对应到具体检查；L2 沿模板替换、重载与函数体实例化解释成功或失败；L3 用调用类型矩阵、失败编译实验和业务输入验证接口演进。分析报错时说明它发生在哪个阶段，以及哪些错误必须被诊断。
