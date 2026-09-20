---
{
  'schemaVersion': 1,
  'id': 'vtable-devirtualization',
  'title': '虚函数与去虚拟化：从对象语义到调用指令',
  'description': '区分 C++20 虚函数语义和 GCC Itanium ABI 的 vtable 实现，用构造析构轨迹、合法多继承调用与汇编解释去虚拟化的条件。',
  'category': 'cpp',
  'areas': ['STL / C++ Object Model', 'Compiler / Linker / ABI'],
  'tags': ['virtual-functions', 'vtable', 'devirtualization', 'itanium-abi'],
  'difficulty': 'L2',
  'roles': ['C++ Developer', 'Quant Developer', 'Low-Latency C++ Developer'],
  'companyTypes': ['高频交易', '量化私募', 'Trading Firm'],
  'status': 'published',
  'updated': '2026-09-19',
  'reviewed': '2026-09-19',
  'standard': 'C++20',
  'estimatedMinutes': 50,
  'prerequisites': ['继承、基类引用与函数重载', '对象构造和析构顺序', '汇编中的直接与间接调用'],
  'related': ['object-lifetime-layout', 'raii-exception-safety', 'templates-concepts'],
  'demo':
    {
      'file': 'examples/vtable-devirtualization.cpp',
      'platform': 'portable',
      'exercise': '为 Label 增加第二个派生实现，保持现有断言并增加新对象调用；重新生成 -O2 汇编，比较 dispatch_label 的目标检查和回退路径是否改变，不把指令差异写成标准保证。',
    },
  'references':
    [
      {
        'title': 'C++20 draft N4861: virtual functions',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/class.virtual',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: construction and destruction',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/class.cdtor',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: final classes',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/class.pre',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: abstract classes',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/class.abstract',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: delete expressions',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/expr.delete',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: dynamic_cast',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/expr.dynamic.cast',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Itanium C++ ABI: virtual tables and virtual calls',
        'url': 'https://itanium-cxx-abi.github.io/cxx-abi/abi.html',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Itanium C++ ABI implementation examples',
        'url': 'https://itanium-cxx-abi.github.io/cxx-abi/abi-examples.html',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC 13.3 optimization options',
        'url': 'https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Optimize-Options.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC 13.3 C++ ABI overview',
        'url': 'https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/G_002b_002b-and-GCC.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'vtable-devirtualization-q01',
        'level': 'L1',
        'prompt': '通过基类引用调用虚函数，通常由哪个实现处理？',
        'answer': '对正常生命周期中的对象，虚调用选择相应基类子对象的最终覆盖函数。派生类覆盖基类虚函数后，即使省略 virtual 仍然是虚函数。显式作用域限定会抑制虚分派，构造析构期间还有当前阶段的分派规则。',
        'rubric': ['最终覆盖函数', '省略 virtual 不取消覆盖', '限定调用和生命周期例外'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据虚函数分派与覆盖规则推导，区分常规调用和限定场景。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q02',
        'level': 'L1',
        'prompt': 'override 能发现哪类错误，是否负责开启虚分派？',
        'answer': 'override 要求该声明确实覆盖基类虚函数；遗漏 const、改变参数或引用限定等可能导致编译报错。覆盖关系本身决定它是虚函数，override 用来核查意图，不负责在运行时开启某种开关。',
        'rubric': ['编译期覆盖检查', '签名和限定符', '覆盖关系本身决定虚函数'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从覆盖签名与 override 诊断要求推导，解释常见接口修改错误。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q03',
        'level': 'L1',
        'prompt': '类上的 final 与虚成员函数上的 final 分别限制什么？',
        'answer': '类上的 final 禁止继续派生；虚成员函数上的 final 禁止后续覆盖该函数，但类本身仍可被继承。两者都提供编译期限制，不能据此承诺编译器一定内联，也不表示整个类没有虚函数机制。',
        'rubric': ['禁止派生与禁止覆盖', '不承诺内联'],
        'source':
          { 'kind': 'derived', 'rationale': '对照 final 两种声明位置，区分语言约束和优化结果。' },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q04',
        'level': 'L1',
        'prompt': '为什么本例 unique_ptr<LifecycleBase> 需要虚析构函数？',
        'answer': '它使用默认删除器通过基类指针执行普通单对象 delete，实际对象是派生类，需要虚析构函数完成正确的派生和基类析构。若这种删除路径没有虚析构且不涉及 C++20 destroying delete 的特殊机制，行为未定义，不能简化为只漏调派生析构。',
        'rubric': ['删除器的静态指针类型', '动态类型不同的删除规则', '普通路径下的 UB'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据单对象 delete 条件推导多态所有权接口，保留 C++20 例外限定。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q05',
        'level': 'L1',
        'prompt': '为什么基类构造函数里调用 kind() 得到基类结果？',
        'answer': '对正在构造的对象，构造函数直接或间接发起的虚调用选择该构造阶段所属类的最终覆盖函数，不会分派到尚未进入构造阶段的更派生类。析构有对应规则：进入基类析构阶段后，不再分派到已结束该阶段的派生实现。',
        'rubric': ['按当前构造析构阶段分派', '不能访问尚未就绪的派生行为'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由构造析构期的分派规则推导示例轨迹，避免套用完整对象规则。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q06',
        'level': 'L1',
        'prompt': 'C++ 标准要求每个多态对象首地址存一个 vptr 吗？',
        'answer': '没有这种可移植布局保证。vptr 和 vtable 是具体 ABI 的实现机制；多继承可以涉及多个多态基类子对象和相应分派信息。应通过编译器布局或汇编核查具体目标，不能把本机偏移写成所有实现的规则。',
        'rubric': ['语义与 ABI 布局分开', '多继承子对象', '目标相关证据'],
        'source':
          {
            'kind': 'derived',
            'rationale': '对照语言语义与具体 ABI，检查对象布局结论的适用范围。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q07',
        'level': 'L2',
        'prompt': 'object->LifecycleBase::kind() 为什么返回 1 而不是派生类的 2？',
        'answer': '显式作用域限定指定调用基类实现，抑制这次调用的虚分派；对象仍然是同一个派生对象，没有发生切片。这个语义与编译器在保持虚调用结果的前提下去虚拟化不同。',
        'rubric': ['作用域限定抑制分派', '不改变对象身份', '区别于优化'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据限定调用与普通虚调用的差别推导，防止混淆语言行为和优化。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q08',
        'level': 'L2',
        'prompt': 'Itanium ABI 中的 vtable address point 为什么不等于整张表的起点？',
        'answer': '对象中的虚表指针指向 ABI 规定的 address point，表中还可以有位于它之前的 offset-to-top 和 typeinfo 等字段。本例 GCC x86-64 汇编把部分 vptr 设置为对应虚表符号加 16；这是本例布局证据，不能直接推广到任意继承结构和平台。',
        'rubric': ['地址点与表起点', '前置元数据', '偏移仅适用于观察目标'],
        'source':
          {
            'kind': 'derived',
            'rationale': '结合 ABI 地址点定义和实际汇编记录，解释符号地址与对象指针的差别。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q09',
        'level': 'L2',
        'prompt': '通过 Combined 的 Label 基类调用 label()，thunk 可能做什么？',
        'answer': '调用者持有 Label 子对象地址，而 Combined::label 实现可能按完整 Combined 的 this 访问成员，调整入口可修正 this 再进入实现。本例 O0 thunk 减去 8 字节后跳转；O2 把成员读取合并到调整入口。具体偏移和指令都由该 ABI 布局与优化决定。',
        'rubric': ['基类子对象与实现期望的 this', '调整入口', '不能固定指令形态'],
        'source':
          {
            'kind': 'derived',
            'rationale': '以真实多继承汇编解释调整入口，避免用非法对象内存读取验证。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q10',
        'level': 'L2',
        'prompt': '去虚拟化和内联之间有什么关系？',
        'answer': '去虚拟化在足够信息下把动态目标解析成直接目标，或建立有回退的快速路径。内联则把函数体放进调用者，可能利用已知目标继续优化。直接调用仍可保留为 call；本例 final 路径在 O0 直调，在 O2 才成为字段读取。',
        'rubric': ['目标确定与函数体展开', '直接调用未必内联', '对应汇编证据'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据两级编译输出推导优化阶段的区别，避免把术语混为一谈。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q11',
        'level': 'L2',
        'prompt': '某个派生类标记 final，为什么任意 Value& 调用仍可能间接分派？',
        'answer': 'final 只限制该派生类不能继续被继承。Value& 仍可能绑定到其他派生类对象；如果调用点不知道是哪一种，就不能凭其中一个派生类的 final 唯一确定目标。反过来，局部对象动态类型已知时，即使类未 final 也可去虚拟化。',
        'rubric': ['基类可对应其他派生类型', '调用点知识', 'final 不是必要条件'],
        'source':
          {
            'kind': 'derived',
            'rationale': '对比未知基类引用和局部完整对象，分析优化所需的类型信息。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q12',
        'level': 'L2',
        'prompt': '汇编中既有目标地址比较又有间接跳转，是否说明去虚拟化失败？',
        'answer': '不一定，这可能是推测性去虚拟化。编译器为某个已知或可能目标建立直接执行路径，检查不匹配时保留通用间接调用。本例 dispatch_label 在 O2 比较 Combined 的 thunk 地址，命中后直接读成员，未命中则跳转真实目标。',
        'rubric': ['目标检查与快速路径', '间接回退保持一般语义', '实际汇编对应'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据观察到的保护分支与 GCC 优化说明推导，避免只搜索 call 指令。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q13',
        'level': 'L3',
        'prompt': '怎样构造虚函数调用性能实验，避免整个循环被优化掉？',
        'answer': '让输入和结果可观察，检查最终汇编确认待测路径仍存在，并记录动态类型分布。分别测试单一目标、混合目标与不同对象局部性，保持函数工作量和分配策略一致。计时包含哪些步骤要明确，报告吞吐和延迟分位数，不能把 O0/O2 总耗时差归因于虚调用。',
        'rubric': ['观察结果和核查汇编', '目标分布与局部性', '控制工作量和测量边界'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从常量折叠和多种分派路径设计实验，考查性能归因是否可靠。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q14',
        'level': 'L3',
        'prompt': '跨翻译单元调用为什么可能在启用 LTO 后改变？',
        'answer': 'LTO 可以让优化器看到参与链接的更多定义和类型流，提供跨文件内联或去虚拟化机会。但动态库、可外部访问符号及运行时扩展仍限制它能作出的假设。需要检查编译和最终链接选项，以及最终二进制，不能把启用 LTO 等同于已知全程序。',
        'rubric': ['跨文件可见信息', '外部扩展和可见性约束', '最终链接验证'],
        'source':
          {
            'kind': 'derived',
            'rationale': '依据 GCC LTO 的信息范围推导跨模块优化条件，避免封闭世界假设。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q15',
        'level': 'L3',
        'prompt': '插件暴露 C++ 虚接口时，除了函数名还要维护哪些兼容性条件？',
        'answer': '需要维护编译器与 ABI 约定、继承结构、虚函数签名和布局，并约定对象创建与销毁方、异常和 RTTI 边界。插入虚函数或改变基类可能改变旧调用者依赖的槽位和调整规则。应使用接口版本与兼容性测试，不能靠导出同一个类名保证二进制兼容。',
        'rubric': ['槽位和继承布局', '生命周期与运行库边界', '版本化接口'],
        'source':
          {
            'kind': 'derived',
            'rationale': '把虚表实现约束用于运行时插件设计，考查 ABI 演进风险。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q16',
        'level': 'L3',
        'prompt': '遍历 vector<unique_ptr<Base>> 较慢，为什么不能直接归因于虚函数？',
        'answer': '每个指针可能指向分散分配的对象，缓存未命中和指针追踪可能占较大成本，动态目标分布也影响间接分支预测。应分别控制存储布局与分派方式，测量后再考虑按类型批处理、连续存储或静态接口，同时保留原有所有权和处理顺序。',
        'rubric': ['分派与数据局部性分开', '目标分布', '控制变量与业务顺序'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从多态容器的数据访问方式设计性能排查，避免只替换调用语法。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q17',
        'level': 'L3',
        'prompt': '交易组件在基类构造期间注册 this 给回调系统，有哪些问题？',
        'answer': '派生状态尚未构造完整，同步重入也只能依赖当前构造阶段；若其他线程开始回调，还需考虑发布同步和数据竞争。应在完整构造后通过明确启动步骤注册，关闭时先停止并排空回调，再销毁对象，不能依赖虚表切换解决生命周期。',
        'rubric': ['未完成构造与重入', '跨线程发布', '停止和排空回调'],
        'source':
          {
            'kind': 'derived',
            'rationale': '将构造析构分派规则用于异步组件生命周期，考查关闭协议。',
          },
        'companies': [],
      },
      {
        'id': 'vtable-devirtualization-q18',
        'level': 'L3',
        'prompt': '如何验证一次去虚拟化结论既有证据又不过度推广？',
        'answer': '保存源文件、编译器版本、目标平台、选项和对应符号的汇编，区分间接 call/jmp、直接调用、内联和常量折叠。核对所有回退分支及最终链接产物，再用行为断言确认语义。只把结果描述为该构建的观察，不写成 final 或 C++ 标准的指令承诺。',
        'rubric': ['可复现编译条件', '调用路径和链接阶段', '语义验证及证据边界'],
        'source':
          {
            'kind': 'derived',
            'rationale': '从汇编实验的可复现性推导验收流程，防止把局部指令推广为标准。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

虚函数通过对象的动态类型选择最终覆盖函数，`override` 检查覆盖意图，`final` 限制继续派生或覆盖。vptr/vtable 是具体 ABI 的实现方式，C++ 不规定其固定内存布局。编译器若能确定调用目标，可以去虚拟化，随后还可能内联；`final` 只提供类型信息，不保证内联，也不消除构造析构期的特殊分派规则。

## 核心概念

静态类型来自表达式声明，例如 `const Value&`；动态类型描述引用所指完整对象的类型，例如 `Fixed`。正常生命周期内，通过该基类引用发起虚调用会到达相应的最终覆盖函数。派生覆盖即使省略 `virtual`，仍然是虚函数，规则见 [C++20 虚函数](https://timsong-cpp.github.io/cppwp/n4861/class.virtual)。

| 写法或术语     | 保证或含义                                 | 不应据此推断                     |
| -------------- | ------------------------------------------ | -------------------------------- |
| `override`     | 声明确实覆盖基类虚函数，否则编译失败       | 运行时额外检查一次覆盖关系       |
| 虚函数 `final` | 后续派生类不能再覆盖该函数                 | 类本身不能继续派生               |
| 类 `final`     | 该类不能再作为基类                         | 所有基类引用都必定指向该类       |
| 去虚拟化       | 优化器确定直接目标，或建立带检查的直接路径 | 一定把函数体内联                 |
| 内联           | 调用处纳入被调用代码，可能继续简化         | 所有标为 inline 的函数都必须展开 |

类 `final` 的限制见 [class.pre](https://timsong-cpp.github.io/cppwp/n4861/class.pre)。`override` 对签名失配很有用：基类的 `value() const` 与派生类的 `value()` 不形成同一个覆盖关系。参数列表、cv 和引用限定都应核对；符合条件的协变返回类型另有规则。

## 原理深入

### 先确定语言要求调用谁

示例 `Value` 有纯虚 `value()`，`Fixed` 与 `Flexible` 都提供实现。`dispatch_unknown(const Value&)` 必须对两种对象都返回各自字段。编译器可以改变调用方式，但必须保留这些结果。

显式限定的 `object->LifecycleBase::kind()` 指定基类实现，抑制这次调用的虚分派，因此得到 1。对象没有被切片。普通 `object->kind()` 则得到派生实现的 2。去虚拟化是在不改变原调用语义的前提下优化，不能把源代码加上基类限定后得到的行为变化称为等价优化。

### 构造与析构遵循当前阶段

基类构造函数调用正在构造对象的虚函数时，选择基类阶段的最终覆盖函数。到了派生构造函数体，才使用派生阶段的覆盖；析构按相反方向经历这些阶段。示例记录 `101, 202, 302, 401`：百位表示基类构造、派生构造、派生析构和基类析构，末位是本次 `kind()` 的结果。依据见 [construction and destruction](https://timsong-cpp.github.io/cppwp/n4861/class.cdtor)。

这套规则同样约束构造或析构函数间接发起的调用，不能通过多绕一层 helper 调到尚未就绪的派生实现。从构造或析构函数直接或间接对该对象发起虚调用，若落到纯虚函数则行为未定义。纯虚函数可以有类外定义，对该定义进行显式限定调用是另一种情况：限定调用抑制虚分派。本例的生命周期类提供了普通虚函数定义，没有执行纯虚调用。相关限制见 [abstract classes](https://timsong-cpp.github.io/cppwp/n4861/class.abstract)。

本例由 `unique_ptr<LifecycleBase>` 的默认删除器释放实际的派生对象，因此基类提供虚析构。普通单对象 delete 在静态类型与动态类型不同时，需要满足相应的基类与虚析构要求；不满足时是 UB，不能只解释为“少执行一个析构”。C++20 的 destroying delete 有专门条件，本文不使用这一机制。见 [delete 表达式](https://timsong-cpp.github.io/cppwp/n4861/expr.delete)。

### 再判断优化器知道多少

`dispatch_final(const Fixed&)` 的参数类型已经是 `final` 类，调用目标受到限制；`dispatch_known_local()` 自己构造一个 `Flexible`，即使类未标 `final`，局部对象的实际类型仍然已知。相反，任意 `Value&` 可以指向不同派生对象，知道其中某个类是 `final` 不能直接确定它的目标。

目标已知也不要求编译器内联。它可以保留一个直接 `call`，也可以把字段读取融入调用者。如果对象内容和调用结果进一步可推断，整条路径还可能被折叠成常量。判断时应检查对应函数的生成代码，而不只查有没有 `call` 字样，尾调用常表现为 `jmp`。

## 数据结构/系统内部实现

以下布局说明限定于 Itanium C++ ABI 及本例 Linux x86-64 GCC 实验。GCC 文档介绍了其 [C++ ABI](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/G_002b_002b-and-GCC.html) 背景；这不表示 x86-64 使用 Itanium 处理器的指令或函数描述符格式。

对象或多态基类子对象中的 vptr 指向虚表的 address point，未必是虚表符号的起点。该地址点之前可有 offset-to-top 与 typeinfo 等元数据；函数项可以指向实际实现，也可以指向调整入口。含虚继承时还会涉及其他偏移信息。具体布局见 [ABI §2.5](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#vtable)。

`Combined` 同时有 `Value` 和 `Label` 基类。经 `Label&` 调用时，调用者传入的是该基类子对象地址；调整入口（thunk）负责让实现按正确的 `this` 访问 `Combined` 成员。调整可以被优化进函数体，不能要求每个 thunk 都是一条减法加跳转。多入口机制见 [ABI §3.2](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#vcall) 和 [实现示例](https://itanium-cxx-abi.github.io/cxx-abi/abi-examples.html)。

本例的 `-O0` 汇编在 `Combined` 虚表组中显示一个主表部分和一个 `Label` 对应的次表部分，后者带有 `-8` 的 offset-to-top，并引用 `label()` 的调整入口。`Fixed` 的虚表还显示成对的析构入口：完整对象析构入口与负责释放存储的 deleting destructor。这些是本次编译产物中的 ABI 证据，不是应用代码可随意访问的 C++ 数据成员。

程序通过合法基类转换和 `dynamic_cast` 检查对象身份；`dynamic_cast<const void*>` 可从多态子对象指针得到完整对象地址，转换规则见 [expr.dynamic.cast](https://timsong-cpp.github.io/cppwp/n4861/expr.dynamic.cast)。示例没有把对象首地址转成任意指针类型来读取 vptr，也没有按猜测槽位调用函数。分析布局使用编译器输出，不依赖越过对象模型的访问。

## C++ runnable demo

程序检查未知基类引用、`final` 参数和局部完整对象的调用结果，用双基类对象检查次基类分派，并通过基类拥有者验证完整析构轨迹。代码是可移植 C++20；后面的汇编与符号命令限定于 Linux GCC/binutils，本机在 WSL 中执行。

```cpp include=examples/vtable-devirtualization.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/vtable-devirtualization.cpp -o /tmp/vtable-demo
/tmp/vtable-demo
g++ -std=c++20 -O0 -Wall -Wextra -Wpedantic -Werror -S -masm=intel examples/vtable-devirtualization.cpp -o /tmp/vtable-O0.s
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -S -masm=intel examples/vtable-devirtualization.cpp -o /tmp/vtable-O2.s
g++ -std=c++20 -O0 -Wall -Wextra -Wpedantic -Werror examples/vtable-devirtualization.cpp -o /tmp/vtable-O0
nm -C /tmp/vtable-O0
objdump -d -C -Mintel /tmp/vtable-O0
```

正常输出 `dispatch checks passed; lifecycle=101,202,302,401`。断言同时覆盖不同派生对象、正负整数值、成功的完整对象转换和失败返回空指针的类型转换。程序不测量耗时，不在析构完成后访问对象。

以下是 GCC `13.3.0`、目标 `x86_64-linux-gnu`、上述普通优化选项、未启用 LTO 时的实际观察。表中关注相关调用路径，不列栈保护和函数入口等无关指令。

| 函数或入口                 | `-O0`                                  | `-O2`                                              |
| -------------------------- | -------------------------------------- | -------------------------------------------------- |
| `dispatch_unknown`         | 经虚表项取地址，再 `call rdx`          | 读取 vptr 后间接尾跳转                             |
| `dispatch_final`           | 直接调用 `Fixed::value()`              | 读取字段后返回，无函数调用                         |
| `dispatch_known_local`     | 经基类引用间接调用，并执行局部对象析构 | 返回常量 7                                         |
| `dispatch_label`           | 经虚表项间接调用                       | 比较目标与已知 thunk；命中直接读字段，否则间接跳转 |
| `Combined::label` 调整入口 | `sub rdi, 8` 后跳到实现                | 从传入子对象地址的适当偏移读取字段并返回           |

`dispatch_unknown` 的 `-O2` 相关指令如下，Intel 语法中的寄存器约定和 16 字节槽位偏移属于本次目标：

```asm
mov rax, QWORD PTR [rdi]
jmp [QWORD PTR 16[rax]]
```

同一构建的 `dispatch_final` 为 `mov eax, DWORD PTR 8[rdi]` 后 `ret`，`dispatch_known_local` 为 `mov eax, 7` 后 `ret`。`nm -C` 还能看到 `vtable for vtable_demo::Combined` 和 `non-virtual thunk to vtable_demo::Combined::label() const`。这些结果证明本次构建采用了哪些路径，没有证明某条路径在真实负载下快多少。

## 高频追问

“为什么 `dispatch_label` 还有间接跳转？”这次 GCC 为编译时可见的候选目标建立了带条件的快速路径，未匹配时仍需要保持一般接口行为。本实验没有使用 PGO。其 [推测性去虚拟化选项](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Optimize-Options.html) 允许这样的转换。看到回退路径不代表优化完全没有发生；只看快速路径也会漏掉最坏路径。

“LTO 一定能消除跨文件虚调用吗？”LTO 可以让优化器看到更多参与链接的定义和类型流，但外部可访问符号、动态库和运行时扩展会限制可作出的假设。编译和最终链接阶段的选项都应核对，再检查最终产物。本文的单文件实验没有验证跨翻译单元的 LTO 效果。

“多态基类是否一律需要 public virtual 析构？”要看销毁协议。允许使用者通过基类指针 delete 时，通常提供 public virtual 析构；明确禁止这种销毁时可以使用 protected 非虚析构，并通过受控接口管理对象。本例选择前者，使 `unique_ptr<Base>` 的默认删除路径合法。

## 容易答错的点

- 派生类省略 `virtual` 不会取消对虚函数的覆盖。漏写 `const` 却可能产生另一个函数，`override` 能帮助发现这种错误。
- `final` 不取消已存在的多态语义，也不保证对象没有 vptr 或编译器一定内联。
- vtable 的地址点可以位于整组表项内部。将它当作所有元数据的起点，会错误解释汇编偏移。
- 构造析构期间不应用完整派生对象的分派预期；间接调用 helper 也不能绕过生命周期规则。
- 通过非虚基类析构进行不符合 delete 条件的多态删除是 UB，不能用一次运行没有崩溃证明安全。
- 有效的基类指针转换和 `dynamic_cast` 与手工改写 vptr 是不同操作。后者不属于本例的对象模型或测试范围。

## 性能分析

本例的 getter 只返回一个字段。生成代码显示未知目标路径需要读取分派信息并间接转移控制，已知目标路径则能进一步简化。实际性能还受对象局部性、目标分布和被调用函数工作量影响；一次表查询的指令数量不能代表端到端延迟。

设计对照实验时，先固定函数工作量、对象数量与存储位置，改变动态目标分布，例如全部同类或交错多类。然后固定分派方式，比较连续存储与分散分配。将所有收益归因于去掉 `virtual`，可能掩盖缓存未命中和指针追踪的变化。按类型批处理还可能改变事件顺序，需要先证明业务允许。

保持编译器、优化参数、链接选项和输入一致，核对最终汇编中的调用路径仍存在，防止已知结果被常量折叠。计时范围应区分纯调用、对象构造与分配、排队和业务处理；报告吞吐及 p50/p99/p99.9，保存原始样本和目标类型分布。短调用容易被计时开销影响，需校准或用有可观察结果的批量测量补充。本文只报告汇编实验，没有性能数字。

静态模板接口可能提供更多内联机会，也可能增加实例化和代码体积；多态接口方便运行时替换实现，但涉及间接访问与生命周期管理。是否采用 LTO、按类型批处理或静态分派，应以实际热路径测量和接口扩展需求决定。

## Quant/Low-Latency 场景

交易网关可以用虚接口接入不同交易场所的会话实现，策略启动时选择具体对象。如果每个事件都会经过同一实现，目标稳定性与缓存状态可能和随机混合多种实现的微基准不同。先确认分派成本在端到端路径中的占比，再决定是否在批次或连接边界完成一次选择，并在内部使用具体类型处理。

组件构造期间不要把尚未完成初始化的 `this` 提前交给可重入或跨线程回调。应在完整构造后启动和注册；关闭时先停止新回调、等待已有回调退出，再销毁对象。vtable 分派无法替代对象寿命、发布同步或回调排空协议。

运行时插件还要维护 ABI：继承结构、虚函数签名、槽位约定以及对象创建与销毁边界。增加虚函数、调整基类或更换工具链，可能改变旧调用方依赖的布局。接口版本、异常与 RTTI 约定应明确；仅保留相同类名不构成兼容性保证。

## 相关专题

- [对象生命周期与布局](object-lifetime-layout.md)：先判断对象是否处于可访问阶段，再讨论多态调用。
- [RAII 与异常安全](raii-exception-safety.md)：多态拥有者如何完成析构和资源清理。
- [模板与 concepts](templates-concepts.md)：比较静态接口约束、实例化成本和运行时扩展边界。

## 分层面试题

L1 先说明 C++ 要求调用哪个实现；L2 对照 ABI 和实际汇编解释目标解析与 this 调整；L3 设计保持业务顺序、对象寿命和可复现测量的接口方案。布局偏移、指令和性能结论都应带上各自的适用条件。
