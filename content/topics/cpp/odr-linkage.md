---
{
  'schemaVersion': 1,
  'id': 'odr-linkage',
  'title': 'ODR 与链接：从头文件定义到静态库提取',
  'description': '区分声明、定义、翻译单元与链接属性，用多翻译单元构建检查 inline 实体身份、内部状态和静态库顺序，并说明链接诊断的边界。',
  'category': 'cpp',
  'areas': ['Modern C++', 'Compiler / Linker / ABI'],
  'tags': ['odr', 'linkage', 'inline', 'translation-unit', 'static-library'],
  'difficulty': 'L2',
  'roles': ['C++ Developer', 'Quant Developer', 'Low-Latency C++ Developer'],
  'companyTypes': ['高频交易', '量化私募', 'Trading Firm'],
  'status': 'published',
  'updated': '2026-09-19',
  'reviewed': '2026-09-19',
  'standard': 'C++20',
  'estimatedMinutes': 55,
  'prerequisites': ['头文件与源文件的基本使用', '函数和命名空间', '编译与链接的基本命令'],
  'related':
    [
      'abi-name-mangling',
      'templates-concepts',
      'raii-exception-safety',
      'cpp-memory-model',
      'shared-mutex',
    ],
  'demo':
    {
      'file': 'examples/odr-linkage.cpp',
      'platform': 'portable',
      'exercise': '按文中命令把同一源文件编译成 A、B、MAIN 三个对象，再分别直接链接与放入静态库。比较两条路径的实体地址断言，并解释反向静态库顺序为什么失败；不要运行故意违反 ODR 的变体。',
    },
  'references':
    [
      {
        'title': 'C++20 draft N4861: implementation compliance',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/intro.compliance',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: declarations and definitions',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/basic.def',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: one-definition rule',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/basic.def.odr',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: program and linkage',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/basic.link',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: inline specifier',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/dcl.inline',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: separate translation',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/lex.separate',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC 13.3: overall options',
        'url': 'https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Overall-Options.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC 13.3: link options',
        'url': 'https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Link-Options.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GNU Binutils: nm',
        'url': 'https://sourceware.org/binutils/docs/binutils/nm.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GNU ld: command line options',
        'url': 'https://sourceware.org/binutils/docs/ld/Options.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GNU Binutils: ar',
        'url': 'https://sourceware.org/binutils/docs/binutils/ar.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'odr-linkage-q01',
        'level': 'L1',
        'prompt': 'extern int x、int x 和带初始化的 extern 分别是否定义变量？',
        'answer': '在普通命名空间作用域，extern int x; 只声明，int x; 已经是定义，extern int x = 7; 也因带初始化成为定义。不能把 C 的暂定定义习惯直接搬到 C++；头文件里的非 inline 外部变量定义会在每个包含它的翻译单元出现。',
        'rubric': ['区分三种写法', '初始化的 extern 是定义', '说明头文件重复定义风险'],
        'source': { 'kind': 'derived', 'rationale': '由变量声明的定义例外和头文件展开方式推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q02',
        'level': 'L1',
        'prompt': '翻译单元与一个 cpp 文件是否完全相同？',
        'answer': '传统文本包含模型中，翻译单元由源文件及包含的头文件内容组成，并排除条件编译跳过的行。两个 cpp 包含同一个头文件，会各自得到其中的声明与定义；头文件通常不作为独立翻译单元参与这次普通构建。',
        'rubric': ['包含展开后的内容', '条件编译影响', '不同源文件各有翻译单元'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由 separate translation 的定义与文本包含构建模型推导。',
          },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q03',
        'level': 'L1',
        'prompt': '命名空间作用域 static 变量的链接属性有什么影响？',
        'answer': '其名字具有内部链接，不同翻译单元中同名的此类变量是各自的实体。把它定义在头文件中会形成每个翻译单元一份状态。链接属性决定名字能否跨翻译单元指向同一实体，不能与对象保存多久的存储期混为一谈。',
        'rubric': ['内部链接', '跨翻译单元各自实体', '区分链接与存储期'],
        'source':
          { 'kind': 'derived', 'rationale': '由内部链接范围与头文件重复包含产生的实体关系推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q04',
        'level': 'L1',
        'prompt': 'include guard 为什么不能解决跨翻译单元重复定义？',
        'answer': '它阻止头文件在一次预处理过程中被重复展开。每个 cpp 有自己的预处理过程，仍可各自包含该定义。外部非 inline 实体通常应在头文件声明、在一个源文件定义；适合多定义的实体则需要满足对应 ODR 条件。',
        'rubric': ['保护范围在单次预处理', '不同 TU 仍各有定义', '正确组织声明和定义'],
        'source': { 'kind': 'derived', 'rationale': '由预处理范围和跨翻译单元定义规则推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q05',
        'level': 'L1',
        'prompt': 'inline 是否保证调用处展开，或者每个 cpp 各有一份对象？',
        'answer': 'inline 不保证机器码在调用处展开。在满足 ODR 的普通外部链接情形下，inline 函数或变量是一个实体，跨翻译单元地址相同。若另外使用 static 使名字具有内部链接，则需要按各翻译单元自己的实体理解。',
        'rubric': ['不保证调用展开', '外部 inline 实体唯一身份', '内部链接情形另算'],
        'source':
          { 'kind': 'derived', 'rationale': '由 inline 的语义和其不改变函数链接属性的规则推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q06',
        'level': 'L1',
        'prompt': '只声明 extern int x 后使用 sizeof(x)，一定需要 x 的定义吗？',
        'answer': '这里 sizeof 的操作数不求值，不会 odr-use 这个 int 对象，因此仅这一用途不要求提供定义。取地址或普通运行时读取通常需要定义；ODR-use 还包含常量表达式等例外，不能只按源代码里有没有写出变量名判断。',
        'rubric': ['sizeof 不求值', '该用途不 odr-use', '不能按名字出现与否判断'],
        'source': { 'kind': 'derived', 'rationale': '由潜在求值表达式与对象 ODR-use 的条件推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q07',
        'level': 'L2',
        'prompt': '不同翻译单元里的 inline 函数定义文本相同就够了吗？',
        'answer': '还不够。普通非模块情形下，多定义需满足相同 token 序列及对应名字查找等 ODR 条件。同一段函数体若在不同翻译单元查找到不同对象或重载，仍可能违规；标准列出的某些内部常量例外也不能推广到可变状态。',
        'rubric': ['同 token 序列', '对应名字查找约束', '不泛化有限例外'],
        'source': { 'kind': 'derived', 'rationale': '由多定义实体的词法与名字查找条件推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q08',
        'level': 'L2',
        'prompt': '外部 inline 函数中的局部 static 在多个翻译单元共享吗？',
        'answer': '当这些定义满足 ODR 且函数具有外部链接时，函数体中的局部 static 是同一个对象。demo 从 A、B 返回它的地址并比较，再分别递增得到总数 2。函数若具有内部链接，则每个函数实体可以拥有自己的局部 static。',
        'rubric': ['外部 inline 函数前提', '同一局部 static 对象', '内部链接改变实体边界'],
        'source':
          { 'kind': 'derived', 'rationale': '由 inline 实体身份与函数体内静态对象规则推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q09',
        'level': 'L2',
        'prompt': '命名空间 const 与 inline constexpr 变量默认有何链接差异？',
        'answer': '在本文普通非模块、非模板且无先前声明改变链接的情形，非 volatile 的命名空间 const 变量默认内部链接，单独 constexpr 变量也通常如此。加入 inline 后不再因这条 const 规则获得内部链接，可用 inline constexpr 定义共享实体；仍需检查是否另有 static。',
        'rubric': ['限定普通命名空间情形', 'const 默认内部链接及例外', 'inline 与 static 分别判断'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由命名空间 const 变量的内部链接规则及 inline 例外推导。',
          },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q10',
        'level': 'L2',
        'prompt': '重复定义和缺少定义是否都要求编译器或链接器报错？',
        'answer': '同一翻译单元内重复定义违反需要诊断的规则。本文普通跨翻译单元的外部非 inline 重复定义，以及 odr-use 后缺少定义，则不能一概宣称标准要求诊断。GNU ld 在示例中报告 multiple definition 和 undefined reference，这是本工具链的观察。',
        'rubric': ['区分同 TU 与跨 TU', '说明无需诊断的情形', '区分标准规则和 GNU 诊断'],
        'source':
          { 'kind': 'derived', 'rationale': '由 ODR 的诊断条件与实际链接器错误类型对照推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q11',
        'level': 'L2',
        'prompt': 'nm 看到 U 或 W 后，能直接判断程序是否违反 ODR 吗？',
        'answer': 'U 表示这个目标文件有未定义符号，其他链接输入仍可能提供它；W 是 GNU nm 的弱符号标记，可出现在 inline 实现中。符号表帮助定位提供方和引用方，却不验证各翻译单元里的 token、类型和名字查找一致性，不能据此证明 ODR 合法。',
        'rubric': ['U 需看最终提供方', 'W 是工具链符号属性', '符号表无法证明语言层一致性'],
        'source':
          { 'kind': 'derived', 'rationale': '由目标文件局部信息与跨翻译单元 ODR 条件的差异推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q12',
        'level': 'L2',
        'prompt': '为什么 MAIN.o libparts.a 能链接，反向顺序却可能失败？',
        'answer': '在本文 GNU ld 的普通静态库搜索中，链接器到达库的位置时，为此前未解决的引用提取成员。库出现在 MAIN.o 前面时，还没有这些引用，后面的对象也不会自动让该库重扫。应按引用关系放置库，循环依赖则需要重整边界或使用工具链的库组机制。',
        'rubric': ['当前位置按未解决引用提取', '后续对象不自动触发重扫', '修复顺序或依赖结构'],
        'source':
          { 'kind': 'derived', 'rationale': '由 GNU ld 静态归档搜索规则和对象引用关系推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q13',
        'level': 'L3',
        'prompt': '头文件中的宏让两个 TU 看到不同 inline 函数体，链接成功能发布吗？',
        'answer': '不能以链接成功作为放行证据。不同 token 序列会破坏相关多定义条件，普通跨翻译单元场景可属于 IFNDR。应统一影响公共定义的构建配置，比较预处理输出并重建依赖对象；测试某次选中了哪个函数体不能修复语言层问题。',
        'rubric': ['指出 token 不一致', '链接成功不证明合法', '统一配置并检查重建边界'],
        'source':
          { 'kind': 'derived', 'rationale': '由条件编译对定义的影响与无需诊断的 ODR 违规推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q14',
        'level': 'L3',
        'prompt': '行情接收与下单组件读取到不同的头文件 static 风控计数，该如何定位？',
        'answer': '先确认两个访问是否来自不同翻译单元，以及计数是否具有内部链接。若每个 TU 各有对象，这可能完全符合语言规则却不符合业务要求。通过对象地址和符号定位实体，再选择集中所有者或明确共享定义；共享后还要单独设计并发同步，inline 本身不提供同步。',
        'rubric': ['合法的多实体也可违背业务语义', '地址与符号定位', '共享所有权及同步另行设计'],
        'source':
          { 'kind': 'derived', 'rationale': '由内部链接的独立状态与跨组件风控一致性要求推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q15',
        'level': 'L3',
        'prompt': '只重编部分源文件后开始出现异常，怎样检查公共类型定义不一致？',
        'answer': '比较出问题对象各自的编译选项、宏和预处理后的公共类型定义，检查构建依赖是否漏掉头文件或生成配置。旧对象可能保留旧布局，新对象已使用新布局，符号名相同也不能保证 ODR 合法。修复依赖后全量重建受影响产物，再执行相关接口与回放测试。',
        'rubric': ['检查宏和预处理定义', '旧新对象布局可能不一致', '修复依赖后重建验证'],
        'source':
          { 'kind': 'derived', 'rationale': '由类多定义要求和增量构建遗漏依赖的后果推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q16',
        'level': 'L3',
        'prompt': '静态库里的注册对象构造函数没有执行，应先检查什么？',
        'answer': '先确认包含注册对象的归档成员是否被提取进最终程序。普通按需提取不会仅因成员含有初始化副作用就必然选中它。可设置明确的注册入口或引用锚点；使用 whole-archive 时要限定范围并检查体积、重复符号和初始化依赖，它不是通用 ODR 修复开关。',
        'rubric': ['先确认成员提取', '显式入口或引用锚点', 'whole-archive 的边界与代价'],
        'source':
          { 'kind': 'derived', 'rationale': '由归档按需提取方式与静态注册副作用之间的边界推导。' },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q17',
        'level': 'L3',
        'prompt': '收到 undefined reference 后，应怎样缩小问题范围？',
        'answer': '先核对完整符号及其反修饰结果，再用 nm 找引用对象与定义所在对象，检查签名、命名空间、条件编译和是否误设内部链接。若定义在静态库，继续查成员是否入库、库顺序与提取过程。最后核对实际链接命令，避免只看 IDE 中显示的源码是否存在。',
        'rubric': ['匹配完整符号身份', '定位定义和链接输入', '检查归档顺序与实际命令'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由符号引用解析的各阶段和静态库提取规则推导排查步骤。',
          },
        'companies': [],
      },
      {
        'id': 'odr-linkage-q18',
        'level': 'L3',
        'prompt': '为降低调用开销把函数移进头文件后，应怎样评估收益和成本？',
        'answer': '保持算法、输入和优化选项可比，检查生成代码是否真的改变，再测完整路径吞吐和尾延迟。同时记录最终可执行文件代码体积、编译和链接时间；头文件改动还会扩大重编范围。inline 关键字不保证展开，单看目标文件出现几份弱符号也不能推断最终运行成本。',
        'rubric': ['确认代码生成变化', '测运行与构建成本', '不从 inline 或弱符号数量推导性能'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由 inline 语义、链接处理和构建依赖范围推导测量方案。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

ODR 约束同一实体可以在哪里定义、不同翻译单元的定义必须怎样一致。普通外部非 inline 函数或变量在弃置语句之外被 odr-use 时，需要程序中有唯一的定义；类、inline 实体和部分模板实体允许在不同翻译单元定义，但要满足相同 token 与名字查找等条件。头文件保护只能防止同一次预处理重复包含。链接器能发现一些重复或缺失符号，却不能完整检查 ODR，链接成功仍可能对应 IFNDR 程序。

## 核心概念

本文采用 C++20 的传统源文件与文本头文件模型，示例默认可独立编译。多翻译单元、符号和静态库实验使用 Linux 上 GCC 13.3 与 GNU Binutils 2.42；命令与符号字母属于该工具链。命名模块、动态库装载和跨动态库符号可见性需要额外规则，不在此例范围内。

[声明与定义](https://timsong-cpp.github.io/cppwp/n4861/basic.def) 的关系决定了头文件应该放什么。函数原型 `int price();` 可以只声明，函数体通常构成定义。普通命名空间作用域的 `extern int limit;` 只声明；`int limit;` 和 `extern int limit = 7;` 都是定义。`extern` 不是一个无条件取消定义的开关。

[翻译单元](https://timsong-cpp.github.io/cppwp/n4861/lex.separate) 包含源文件及其包含的文件内容，并排除条件编译跳过的行。把同一个头文件分别包含到 `a.cpp` 与 `b.cpp`，会让其内容分别出现在两个翻译单元。include guard 的宏状态属于各次预处理，不能在整个程序范围内只保留一次定义。

链接属性描述名字能否跨作用域或翻译单元指向同一实体。普通命名空间函数及许多变量名字具有外部链接；命名空间作用域的 `static` 和匿名命名空间可提供内部链接。函数局部变量一般没有链接，局部 `static` 仍可有静态存储期。判断链接属性与判断生命周期是两件事，规则见 [basic.link](https://timsong-cpp.github.io/cppwp/n4861/basic.link)。

## 原理深入

### 一个头文件可以让哪些定义出现多次？

普通非 inline 外部函数或变量常用“头文件声明、一个源文件定义”的组织方式。命名空间的变量声明只写 `extern` 且不带初始化，函数声明只给签名。调用者编译时有声明即可；定义所在对象必须进入最终链接。

类定义、inline 函数或变量，以及部分模板实体，允许在多个翻译单元中定义。[N4861 basic.def.odr](https://timsong-cpp.github.io/cppwp/n4861/basic.def.odr) 为这类多定义列出条件，包括定义出现在不同翻译单元、未附属于命名模块、相同 token 序列，以及对应名字查找等要求。这里不能简化成“函数体长得一样就行”。模板还涉及实例化点等约束，显式特化也不能一律当成普通模板定义处理。

例如一个 inline 函数体在两个翻译单元都是 `return limit;`，若它分别读到不同的可变内部链接对象，就不能仅凭相同文本判定合法。标准对某些未被 odr-use 的内部常量等情况有有限例外；它们不允许把任意 per-TU 可变状态藏进外部 inline 定义。

头文件中的命名空间 `static` 变量则是另一种情况：每个翻译单元得到自己的实体，可以合法存在，却可能不符合“所有组件共享一个计数”的需求。普通非模块、非模板情形下，未被先前声明改变链接的非 volatile 命名空间 `const` 变量也默认内部链接；`inline`、`extern` 等是该规则的例外。因而单独 `constexpr` 的命名空间变量通常仍是内部链接，`inline constexpr` 则适合表达共享常量实体，前提是没有另加 `static` 等改变链接。

### ODR-use 决定何时需要实体定义

demo 的 `declared_only` 只有 `extern int` 声明，却用于 `sizeof`。该操作数不求值，计算大小只需要已知类型，不需要这个对象的定义。相反，demo 取得 `external_value` 的地址并递增它，这些操作需要定义。

ODR-use 的完整规则涉及潜在求值、常量表达式、左值到右值转换等条件，不能写成“使用名字就是 odr-use”或“只有取地址才是 odr-use”。函数被潜在求值的表达式命名时通常也会被 odr-use。优化器删除一次运行时工作，不会自动取消源程序必须满足的定义要求。

### 外部 inline 仍有统一的实体身份

按照 [dcl.inline](https://timsong-cpp.github.io/cppwp/n4861/dcl.inline)，符合条件的外部链接 inline 函数或变量跨翻译单元是同一实体，具有同一地址；这种 inline 函数体内的局部 static 也指向同一对象。inline 定义需要在相应使用它的定义域中可达，在本文构建模型中通常通过头文件提供。

demo 比较 A、B 返回的三个地址：外部非 inline 变量、外部 inline 变量、外部 inline 函数中的局部 static。三组都相等。命名空间 `static internal_value` 的地址则不同，分别递增后各为 1。`inline` 不改变函数的链接属性，若函数本身是 `static inline`，应按各翻译单元各自的函数实体分析。

### 诊断要求与链接结果分别判断

同一翻译单元中重复定义同一变量会违反需要诊断的规则，demo 用 `ODR_BAD_SAME_TU` 检查编译器报告重定义。[intro.compliance](https://timsong-cpp.github.io/cppwp/n4861/intro.compliance) 要求符合条件的实现至少给出一条诊断，并不要求所有诊断都表现为拒绝生成文件。普通跨翻译单元的非 inline 外部定义冲突、odr-use 后缺少所需定义，以及不满足条件的外部 inline 多定义，不能统称为“标准要求链接器报错”。相关情形可能是 ill-formed, no diagnostic required，简称 IFNDR。

GNU ld 在本例能报告前两种跨翻译单元问题。若两个 inline 定义因构建宏不同而产生不同 token，链接器仍可能选择或合并符号，让链接完成。这样的程序没有因此变得合法；本章不运行故意制造的 IFNDR 产物，也不把某次观察到的函数体当作语言保证。

## 数据结构/系统内部实现

在本实验中，GCC 驱动预处理、编译、汇编及链接步骤。`-E` 停在预处理，`-S` 停在汇编文本，`-c` 产生对象而不链接，见 [GCC Overall Options](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Overall-Options.html)。因此一个 cpp 编译成功只说明当前翻译单元走到了对象文件阶段，其他对象中的定义是否存在尚未确定。

对象文件携带符号及相应代码、数据或重定位信息。使用 `nm -C` 可以反修饰 C++ 名字，帮助查看哪个对象引用符号、哪个对象提供定义。[GNU nm](https://sourceware.org/binutils/docs/binutils/nm.html) 的常见标记及本例观察如下：

| 标记 | 本例中的符号                         | 读取方式                                |
| ---- | ------------------------------------ | --------------------------------------- |
| `D`  | A 中的 `external_value`              | 已初始化数据区中的定义                  |
| `U`  | B 中的 `external_value`              | 本对象没有定义，需要链接解析            |
| `T`  | `view_a`、`touch_b` 等               | 代码区中的定义                          |
| `b`  | 各对象的 `internal_value`            | 局部的零初始化存储符号                  |
| `W`  | `function_counter` 等                | 此 GNU 实现发出的弱符号                 |
| `u`  | `shared_visits`、局部 static `count` | GNU unique 符号，不是普通的局部符号标记 |

这些字母不属于 ISO C++ 对符号编码的规定，也不能用“所有小写都内部链接”概括。工具链和优化选项改变后，符号可能不同或不再独立出现。不同对象文件中相同的节内偏移不表示运行时地址相同；本例通过正常程序中的指针比较验证实体身份。

静态库是对象等文件的归档。`ar rcs` 收集成员并建立符号索引，见 [GNU ar](https://sourceware.org/binutils/docs/binutils/ar.html)。归档创建成功不表示成员的外部引用已解析，也不表示把所有成员放入同一个程序一定合法。

在 [GNU ld 的普通归档搜索](https://sourceware.org/binutils/docs/ld/Options.html) 中，链接器在命令行遇到库时，按此前尚未解决的引用提取需要的成员；同一归档内部会利用成员间的引用，但后面才出现的普通对象不会自动使已经处理的库重新搜索。因此本例把 `MAIN.o` 放在 `libodr-parts.a` 前。库之间存在循环引用时，可以调整组件依赖、重复列库，或采用 GNU 的 `--start-group` / `--end-group` 重复搜索；组搜索有额外链接成本。GCC 的 [Link Options](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Link-Options.html) 也说明了库顺序对解析的影响。

## C++ runnable demo

单个源文件保留默认独立运行方式。多翻译单元模式用 `ODR_TU_A`、`ODR_TU_B`、`ODR_TU_MAIN` 分别选择一个实现区域，相当于把公共声明区放进头文件、把三段实现放进三个 cpp。公共 inline 定义保持相同，且不读取 per-TU 的 `internal_value`；只有各自唯一的 `view_a/view_b` 与 `touch_a/touch_b` 访问本翻译单元的内部对象。

```cpp include=examples/odr-linkage.cpp

```

默认模式只使用标准 C++20，可在支持它的工具链运行。保持断言启用，不要定义 `NDEBUG`：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/odr-linkage.cpp -o /tmp/odr-single
/tmp/odr-single
```

预期输出 `single-TU: entity identity and bounded updates OK`。此时 A、B 函数在同一个翻译单元，`internal_value` 也只有一个，最终值为 2。

以下 Bash 命令从仓库根目录执行，使用 GCC 与 GNU Binutils。保留同一个 shell 中的变量，以便继续执行后面的诊断实验：

```bash
set -eu
src="$PWD/examples/odr-linkage.cpp"
work="$(mktemp -d /tmp/odr-linkage.XXXXXX)"
flags=(-std=c++20 -O0 -g -Wall -Wextra -Wpedantic -Werror)
for unit in A B MAIN; do
  g++ "${flags[@]}" -DODR_MULTI_TU -DODR_TU_$unit -c "$src" -o "$work/$unit.o"
done
g++ "$work/MAIN.o" "$work/A.o" "$work/B.o" -o "$work/multi"
"$work/multi"
ar rcs "$work/libodr-parts.a" "$work/A.o" "$work/B.o"
g++ "$work/MAIN.o" "$work/libodr-parts.a" -o "$work/archive"
"$work/archive"
nm -C "$work/A.o" "$work/B.o"
```

直接对象链接和静态库链接都应输出 `multi-TU: shared entities and separate internal state OK`。断言同时验证共享对象地址、内部对象地址、共享计数总和与分离计数；多翻译单元模式并不依赖打印地址再人工猜测。`declared_only` 没有定义且未被 odr-use，本次 GNU 对象符号输出也没有它。

错误变体只用于检查构建诊断。下面的帮助函数要求命令失败，并打印日志；阅读日志时核对表中原因，不能把任意失败都当作成功验证：

```bash
expect_failure() {
  name="$1"
  shift
  if "$@" >"$work/$name.log" 2>&1; then
    echo "unexpected success: $name" >&2
    exit 1
  fi
  cat "$work/$name.log"
}
expect_failure same-tu g++ "${flags[@]}" -DODR_BAD_SAME_TU -c "$src" -o "$work/same-tu.o"
g++ "${flags[@]}" -DODR_MULTI_TU -DODR_TU_B -DODR_BAD_DUPLICATE -c "$src" -o "$work/duplicate-B.o"
expect_failure duplicate g++ "$work/MAIN.o" "$work/A.o" "$work/duplicate-B.o" -o "$work/duplicate"
g++ "${flags[@]}" -DODR_MULTI_TU -DODR_TU_A -DODR_BAD_MISSING -c "$src" -o "$work/missing-A.o"
expect_failure missing g++ "$work/MAIN.o" "$work/missing-A.o" "$work/B.o" -o "$work/missing"
expect_failure order g++ "$work/libodr-parts.a" "$work/MAIN.o" -o "$work/wrong-order"
```

| 变体        | 本次工具链观察                                               | 检查的边界                           |
| ----------- | ------------------------------------------------------------ | ------------------------------------ |
| `same-tu`   | 编译时报 `external_value` redefinition                       | 同一翻译单元重复定义，需要诊断       |
| `duplicate` | 链接时报 `external_value` multiple definition                | 跨 TU 冲突，本次 GNU ld 检出         |
| `missing`   | 链接时报 `external_value` undefined reference                | odr-use 后缺少定义，本次 GNU ld 检出 |
| `order`     | 链接时报 `view_a/view_b/touch_a/touch_b` undefined reference | 库先于引用对象，成员未按预期提取     |

错误变体没有运行步骤。最后一项演示链接输入顺序，不应归因于源文件里的两个正常定义区域违反 ODR。对正常程序还可启用 ASan/UBSan；它们检查实际执行中的相应错误，不会替代跨翻译单元 ODR 审查。该 demo 为有界单线程程序，没有动态库、模块、并发初始化竞争或性能基准。

## 高频追问

### 给头文件中的函数加 inline 就一定合法吗？

还要检查函数在各翻译单元的定义与名字查找是否满足 ODR，以及每个需要定义的地方能否看到它。比如调试宏改变函数体、包含顺序改变可见重载、生成配置改变公共类型，都可能破坏条件。用 `-E` 比较预处理输出有助于发现 token 差异，但相同 token 仍需检查名字查找。

### 把变量改成 static 能消除重复定义问题吗？

它可以让各翻译单元各有一个内部实体，从而改变了对象的共享语义。若原需求是统一风险阈值或累计计数，分离状态会造成业务错误。应该先决定所有权，再选择外部声明加单一定义，或符合条件的外部 inline 定义；线程安全需要额外设计。

### 为什么静态库里有函数，仍然 undefined reference？

“库里有”还要匹配实际需要的完整符号。参数类型、命名空间或条件编译不同，可能得到另一个符号；内部链接定义也不能作为跨 TU 的对应定义。确认名字后，再检查实际链接输入、归档成员索引与库顺序。可用 GCC `-v` 查看驱动执行的命令，用 `nm -C` 对比提供方和引用方。

### whole-archive 可以作为修复开关吗？

GNU `--whole-archive` 会要求包含指定归档中的全部成员，适合一些需要显式保留成员的构建安排。它可能增加体积，也可能让原来未提取成员中的重复定义暴露出来。该选项不会让不满足 ODR 的定义变得合法；静态注册优先考虑清楚的注册入口和生命周期。

## 容易答错的点

- 把 `int x;` 当成 C++ 的纯声明。命名空间作用域下它已是定义；纯外部声明通常写 `extern int x;`。
- 认为头文件有 include guard 就不会跨 cpp 重复定义。不同翻译单元有各自的预处理过程。
- 把 inline 理解成编译器必须展开调用。关键字有语言层定义与身份规则，实际展开由实现决定。
- 认为每个 TU 的外部 inline 变量各有一份。满足 ODR 时它们是同一实体；内部链接对象才需要按各 TU 分开分析。
- 认为 `U` 表示程序已经缺少定义。它只描述当前对象，定义可以来自其他链接输入。
- 把弱符号合并当成 ODR 正确性的证明。符号解析不核对所有定义的 token、类型与名字查找。
- 把链接成功的 IFNDR 程序归类为“标准允许任取一个结果”。程序已不符合语言要求，不能从某次运行挑选可依赖行为。
- 给链接器允许重复定义的选项后就宣布修复。工具选项改变解析方式，源程序的语言义务仍然存在。

## 性能分析

ODR 本身没有可报告的运行时 O(1) 或 O(n) 操作；它是程序合法性约束。符号搜索、归档扫描及优化的成本属于具体工具链，受对象数、符号数、依赖结构与构建参数影响。GNU 库组重复搜索可能增加链接时间，不能只为掩盖依赖混乱而无条件把所有库放进组中。

将定义移入公共头文件会扩大修改后的重编范围。即使最终程序只保留一种实体表示，各翻译单元仍要处理相关源代码。评估时分别记录干净构建、无改动构建、修改公共头文件后的增量构建耗时，并核对实际重编对象；不能把一次缓存命中与另一次完整编译直接比较。

运行时实验应先检查最终代码是否真的发生调用展开或其他优化，再测完整调用路径的吞吐与 p50/p99/p99.9。最终可执行文件的代码体积、指令缓存压力和编译时间都可能改变；目标文件里出现多个弱定义不直接等于最终有多份运行时代码。保持输入、编译器版本和优化选项可比，并多次重复测量。本文没有性能实测，demo 的断言只验证实体身份和有限更新。

## Quant/Low-Latency 场景

行情解析、风控和发单路径经常共用头文件中的配置与结构定义。若不同目标使用不同宏，让同一订单结构的字段或 inline 校验逻辑发生变化，即使最终符号能解析，也可能已经违反 ODR。构建系统应让影响公共定义的配置成为明确依赖，生成头文件更新后重编全部受影响对象，发布前用同一套接口与回放检查产物。

头文件里的 `static` 计数可能让接收端与下单端各自记录“累计拒单数”。这种分离可能符合 C++ 规则，却让监控或限额判断拿到不完整状态。先确定计数属于某个线程、组件还是整个进程，再设计访问入口；若改为共享 inline 变量，多线程递增仍需同步。此 demo 的两个调用顺序执行，不能作为并发共享计数方案。

策略或协议处理器若靠静态库成员中的注册对象自动登记，成员未被提取就可能没有注册效果。构建时检查最终成员与符号，运行时验证应有的处理器集合；对于订单路径，缺失处理器要进入明确失败流程，不能静默回退成一个看似成功的空处理。显式注册入口通常更容易安排启动顺序和错误报告。

## 相关专题

- [RAII 与异常安全](raii-exception-safety.md)：实体的链接身份确定后，继续检查对象初始化、销毁和失败路径。
- [ABI 与名字改编](abi-name-mangling.md)：继续排查符号身份、调用边界和二进制兼容，链接成功也不能替代接口一致性检查。
- [模板与 concepts](templates-concepts.md)：补充模板定义可见性、实例化与约束规则，不能只套用普通函数的组织方式。
- [C++ 内存模型](../concurrency/cpp-memory-model.md)：多个线程访问同一共享实体时，继续分析数据竞争和 happens-before。
- [shared_mutex](../concurrency/shared-mutex.md)：对共享配置安排同步与结果生命周期，inline 变量本身不提供锁保护。

## 分层面试题

L1 从一段声明能否成为定义、定义进入哪些翻译单元开始；L2 结合对象地址与符号表解释各实体的关系；L3 检查配置、增量构建和静态库边界。回答构建失败时，说明错误发生在编译还是链接，并指出当前证据能证明什么：发现一个具体错误与证明整个程序满足 ODR，需要不同程度的检查。
