---
{
  'schemaVersion': 1,
  'id': 'branch-prediction',
  'title': '分支预测：从输入序列到实际机器代码',
  'description': '区分源码条件、条件跳转与间接目标预测，检查 if-conversion 和向量化，讨论 branchless、likely 与 PGO 的适用前提，并用相同直方图输入核对语义。',
  'category': 'performance',
  'areas': ['Performance Engineering', 'Low-Latency Programming'],
  'tags': ['branch-prediction', 'branchless', 'pgo', 'compiler', 'assembly'],
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
  'prerequisites': ['C++ 条件求值与无符号运算', '优化编译和基本汇编阅读', '受控性能实验'],
  'related':
    [
      'perf-flamegraph',
      'vtable-devirtualization',
      'cpu-cache-false-sharing',
      'udp-multicast-sequencing',
    ],
  'demo':
    {
      'file': 'examples/branch-prediction.cpp',
      'platform': 'linux',
      'exercise': '保持输入直方图与 oracle 不变，增加交替和分块排列；先检查目标优化构建中的数据条件是否仍为跳转，再设计独立的采集区间。若只有 cmov 或向量掩码，解释为什么不能从有序/打乱耗时直接推导该条件的错预测成本。',
    },
  'references':
    [
      {
        'title': 'Intel: Hardware Features and Behavior Related to Speculative Execution',
        'url': 'https://www.intel.com/content/www/us/en/developer/articles/technical/software-security-guidance/technical-documentation/hardware-behavior-related-to-speculative-execution.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC 13.3 optimization options',
        'url': 'https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Optimize-Options.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'GCC 13.3 instrumentation options',
        'url': 'https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Instrumentation-Options.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Clang likely and unlikely attributes',
        'url': 'https://clang.llvm.org/docs/AttributeReference.html#likely-and-unlikely',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Clang profile-guided optimization',
        'url': 'https://clang.llvm.org/docs/UsersManual.html#profile-guided-optimization',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'LLVM auto-vectorization',
        'url': 'https://llvm.org/docs/Vectorizers.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861 likelihood attributes',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/dcl.attr.likelihood',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861 conditional operator',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/expr.cond',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861 logical AND',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/expr.log.and',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'branch-prediction-q01',
        'level': 'L1',
        'prompt': 'C++ 中出现 if，是否意味着 CPU 必须执行一个条件跳转？',
        'answer': '不一定。编译器可以消除条件、生成条件移动、用掩码选择，或者把循环向量化。C++ 约束程序的可观察语义，不规定每个 if 对应一条跳转。本例 GCC 13.3 的 O2 构建将数据条件编译为 cmov，循环回边仍有跳转，必须区分它们。',
        'rubric': ['区分语言语义与指令实现', '识别数据条件和循环回边'],
        'source':
          { 'kind': 'derived', 'rationale': '由源码条件与实际 if-conversion 编译产物差异推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q02',
        'level': 'L1',
        'prompt': '条件分支预测和间接分支预测主要预测什么？',
        'answer': '条件分支需要预测是否沿跳转方向执行；间接跳转或间接调用需要预测尚未解析出的目标地址。函数指针和虚调用可能产生间接调用，但编译器也可能将其去虚拟化为直接调用。返回目标还可由专门机制预测，不能把所有控制转移简化为同一个布尔条件预测器。',
        'rubric': ['区分方向和目标地址', '保留编译优化与返回预测边界'],
        'source': { 'kind': 'derived', 'rationale': '由处理器控制流预测对象与间接调用形式推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q03',
        'level': 'L1',
        'prompt': '两个输入的条件成立比例都是 50%，可预测性就相同吗？',
        'answer': '不相同。长段连续成立、交替成立和伪随机排列具有不同的时序相关性，现代预测器可能利用历史。相同比例只约束总次数，不能确定错预测率。还要先确认目标机器代码保留了这个条件跳转，否则这个比较没有直接测量该条件的分支预测。',
        'rubric': ['指出时序相关性', '先确认真实条件跳转'],
        'source': { 'kind': 'derived', 'rationale': '由相同直方图不同排列的受控输入设计推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q04',
        'level': 'L1',
        'prompt': 'C++20 的 likely 属性是否要求 CPU 总是预测该路径？',
        'answer': '不是。likely 表达供实现优化使用的路径倾向，不规定概率数值，也不要求生成某条指令或保证速度。编译器可能据此调整布局等决策，具体效果依赖实现和优化配置。路径很少出现也必须保持正确处理，属性不等于 assume 或 unreachable。',
        'rubric': ['说明只是优化倾向', '保留冷路径的完整语义'],
        'source': { 'kind': 'derived', 'rationale': '由 C++20 likelihood 属性的规范边界推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q05',
        'level': 'L1',
        'prompt': '本例三种求和写法为何可以用同一个 oracle 校验？',
        'answer': '它们都只累加大于等于 threshold 的元素，输入为固定次数重复的 0..255，排列不影响整数加法结果。oracle 用每个值的重复次数和等差数列公式计算，独立于三个逐元素选择实现。空输入和域外边界另用显式期望值核对。',
        'rubric': ['说明语义与排列不变性', '解释闭式 oracle 与边界检查'],
        'source':
          { 'kind': 'derived', 'rationale': '由固定直方图和独立数学 oracle 的验证方法推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q06',
        'level': 'L2',
        'prompt': '把短路检查改成先算两边再选择，可能引入哪些错误？',
        'answer': '原本未求值的一边可能包含越界解引用、除零、异常、I/O 或状态修改。预先计算两边会改变语义，之后用掩码丢弃结果也无法撤销这些行为。内建逻辑与及条件运算符具有相应的条件求值规则；安全的改写需要证明新增求值总是有效且不会改变可观察行为。',
        'rubric': ['列出新增求值的安全或副作用风险', '不能用事后掩码修复已发生行为'],
        'source':
          { 'kind': 'derived', 'rationale': '由条件求值规则与 eager branchless 改写差异推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q07',
        'level': 'L2',
        'prompt': '消除条件跳转后，为什么执行仍可能变慢？',
        'answer': '选择可能增加指令数、执行本可跳过的工作，或者让结果依赖比较与两侧数据，延长关键依赖链。原跳转若预测准确，能够先沿正确路径执行，收益未必小于无分支版本。还要检查寄存器压力、内存访问和向量化，最终以同等语义负载下的实际指标比较。',
        'rubric': ['说明额外工作或依赖成本', '按实际机器代码与工作负载测量'],
        'source': { 'kind': 'derived', 'rationale': '由控制依赖转换为数据选择后的成本变化推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q08',
        'level': 'L2',
        'prompt': '检查汇编时，如何避免把循环回边当成被研究的条件分支？',
        'answer': '沿比较操作数追踪标志使用，确认跳转受数据阈值还是指针/计数终止条件控制。本例 O2 的数据比较紧接 cmov，而 jne 跟随输入指针与结束地址比较，控制下一轮迭代。仅搜索 j 开头指令或统计跳转数量，无法确定要研究的条件是否还存在。',
        'rubric': ['追踪比较和标志消费者', '区分数据选择与迭代控制'],
        'source': { 'kind': 'derived', 'rationale': '由本例 O2 汇编中 cmov 与回边共存的观察推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q09',
        'level': 'L2',
        'prompt': 'PGO 使用一份训练输入后，编译器获得了什么，又没有保证什么？',
        'answer': 'PGO 可提供路径、调用等执行频率，帮助布局、内联及其他优化。训练数据描述的是被观测负载，不保证未来输入有同样分布；缺少恢复或异常流量可能导致这些路径被视为冷路径。应保存 profile 与构建匹配关系，并在独立代表负载上检验功能与性能。',
        'rubric': ['解释 profile 的作用', '说明代表性与独立验证需求'],
        'source': { 'kind': 'derived', 'rationale': '由编译器 PGO 数据使用与训练分布条件推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q10',
        'level': 'L2',
        'prompt': '有序和打乱数据比较时，为何先核对直方图？',
        'answer': '核对直方图可确保值集合、每个值次数和阈值命中总数相同，让排列成为明确变化。本例还保持连续扫描同样大小的数组。不过这只能减少混杂变量，不能保证预测器初始状态、缓存或调度完全相同，也不能证明任意伪随机排列代表真实输入。',
        'rubric': ['控制数值集合和命中率', '保留缓存、历史和代表性的限制'],
        'source': { 'kind': 'derived', 'rationale': '由直方图检查对排列实验控制能力的边界推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q11',
        'level': 'L3',
        'prompt': '能否为了提高预测命中率，把到达的行情消息按类型排序？',
        'answer': '要先证明协议和业务允许重排。消息可能依赖序列号、前序状态或快照边界，排序可改变状态重建结果；即使类型独立，等待成批也可能增加早到消息的延迟。若允许按独立域分组，应保留域内顺序、跨域一致性要求和失效恢复规则，再测量端到端收益。',
        'rubric': ['先检查业务与协议顺序语义', '评估批次等待与恢复边界'],
        'source': { 'kind': 'derived', 'rationale': '由输入重排优化在行情状态重建中的约束推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q12',
        'level': 'L3',
        'prompt': 'branches 和 branch-misses 下降是否足以证明请求延迟改善？',
        'answer': '不足。事件覆盖范围可能包括循环、调用和其他线程，具体映射还依赖 PMU。降低 miss 可能伴随更多指令、访存或排队。需要核对事件定义和测量区间，记录绝对计数及复用比例，再比较相同到达负载下的吞吐与延迟分布，不能只使用一个比例。',
        'rubric': ['检查计数范围与 PMU 语义', '验证指令/内存成本及端到端结果'],
        'source':
          { 'kind': 'derived', 'rationale': '由分支事件变化与业务性能目标之间的非等价关系推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q13',
        'level': 'L3',
        'prompt': '关闭 if-conversion 后观察到数据跳转，如何解释这份结果？',
        'answer': '它是指定诊断选项下的编译产物，可帮助识别条件跳转和比较机制，但不能代表原目标构建。关闭转换可能同时阻止其他优化，性能差异需要逐项解释。本章只记录诊断汇编，不运行它来宣称某种分支成本，更没有把其中的跳转说成 O2 默认构建的行为。',
        'rubric': ['区分诊断与目标构建', '不从存在跳转推导成本'],
        'source': { 'kind': 'derived', 'rationale': '由限制编译优化的诊断实验与生产归因边界推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q14',
        'level': 'L3',
        'prompt': '正常时段训练的 PGO 版本在恢复期间变慢，应如何验证原因？',
        'answer': '保留同一恢复输入和构建条件，对照无 PGO 与 PGO 版本的调用路径、代码布局、CPU 时间及尾延迟，检查恢复路径是否在训练中缺席。补充有代表性的训练集后再用独立恢复轨迹验证，不能把验证数据全部并入训练后仍称独立评估。正确性与错误处理先保持一致。',
        'rubric': ['检查分布漂移与缺席路径', '保留独立验证及完整语义'],
        'source': { 'kind': 'derived', 'rationale': '由 PGO 训练分布与交易恢复负载的差异推导' },
        'companies': [],
      },
      {
        'id': 'branch-prediction-q15',
        'level': 'L3',
        'prompt': '一个分支极少触发但承担长度校验，如何优化且保留边界？',
        'answer': '保留检查及失败处理，可先测量它是否形成实际开销，再考虑布局提示、批量预检查或安全的解析结构。不能用极少发生为理由删掉检查、声明失败路径不可达，或先读越界数据再掩码。任何改写都要覆盖短输入、最大长度、溢出与恢复路径，并核对目标构建。',
        'rubric': ['冷路径仍须正确处理', '将安全边界纳入优化验证'],
        'source': { 'kind': 'derived', 'rationale': '由低频校验路径的功能职责与优化条件推导' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

处理器预测控制流，让后续指令在条件或目标解析前开始执行；预测错误时需要丢弃错误路径的工作并恢复。条件分支主要涉及方向预测，间接调用还需要目标地址预测。优化时先看实际机器代码：源码 `if` 可能已变成条件移动或向量掩码。branchless、`[[likely]]` 和 PGO 都有适用条件，收益取决于输入序列、额外工作和目标构建，不能只数源码里的分支。

## 核心概念

条件跳转有跳转与继续顺序执行两个方向；间接调用或跳转的目标来自运行时值。虚调用可以形成间接调用，也可以被编译器去虚拟化。预测器利用执行历史等信息提前选择路径，返回地址也可能由专门机制预测。Intel 资料分别说明了方向预测、目标预测与错误推测后恢复的行为，不同处理器的内部结构不必相同。[Intel 控制流推测说明](https://www.intel.com/content/www/us/en/developer/articles/technical/software-security-guidance/technical-documentation/hardware-behavior-related-to-speculative-execution.html)

| 观察对象          | 它说明什么             | 不能直接推出什么           |
| ----------------- | ---------------------- | -------------------------- |
| 源码中的 `if`     | 哪些语句有条件地执行   | 存在某条硬件条件跳转       |
| 条件跳转指令      | 控制流依赖比较或标志   | 错预测率与固定周期惩罚     |
| `cmov` 或向量掩码 | 用数据选择实现某个条件 | 整个函数没有跳转或一定更快 |
| taken 比例        | 某方向出现的总比例     | 相同输入顺序或预测难度     |

`branchless` 通常描述某个选择没有使用数据相关跳转，必须说明范围。循环回边、长度检查和函数返回仍然存在。条件移动把控制流选择转成数据依赖，不等于处理器不再需要等待条件或操作数。

## 原理深入

对于阈值判断，相同数量的真和假可以排列成长段、交替段或伪随机序列。总比例相同，历史相关性却不同。一个简单的“总猜多数方向”模型无法描述现代预测器的全部行为，因此 50% 命中条件不能等同于 50% 错预测；重复播放同一短输入还可能形成特殊的可学习模式。

预测正确时，处理器能够提前在预计路径工作。预测错误造成已经投入的部分工作失效，并需要重新供应正确路径的指令。损失随微架构、分支解析时间及周围执行状态变化，本章不使用一个固定周期数作为通用惩罚。即使 miss 数减少，额外计算或更长依赖链仍可能抵消收益。

编译器也会改变问题本身。GCC 的 if-conversion 尝试把条件跳转转换为条件移动、置标志或其他等价算术。循环向量化可一次处理多个值，并用向量比较和掩码表示选择。源代码三元表达式不保证无跳转，手写 mask 也不阻止编译器再将它转换成另一形式。[GCC 13.3 优化选项](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Optimize-Options.html)、[LLVM 向量化文档](https://llvm.org/docs/Vectorizers.html)

安全前提要先于指令选择。内建 `&&` 在左侧为假时不求值右侧，`?:` 只求值被选择的操作数。如果为了“去分支”先执行两边的读取或计算，再按条件挑结果，原来不会访问的地址、不会触发的异常或副作用也可能发生。无效结果被乘零或按位清零，不能消除已经发生的越界读取。[N4861 逻辑与](https://timsong-cpp.github.io/cppwp/n4861/expr.log.and)、[条件运算符](https://timsong-cpp.github.io/cppwp/n4861/expr.cond)

## 数据结构/系统内部实现

示例输入包含 0..255，每个数出现 64 次，总计 16384 个 `uint16_t`。一份按值递增排列，另一份通过固定种子的 xorshift32 和逐步交换生成排列。两份都连续扫描相同大小的数组，校验完整直方图，保证变化的是顺序。取模选择交换位置会有统计偏差，本例只要求可复现置换，不把它当作均匀随机样本或生产分布。

三个函数分别写成 `if`、三元选择和无符号掩码。掩码由 `uint64_t{0} - bool值` 形成 0 或全 1，采用定义明确的无符号回绕；值提升到 uint64_t 后按位与并累加。数据值和输入长度有界，总和远低于 uint64_t 上限。函数只处理传入的有效 span，不额外读取未选中分支的地址。

独立 oracle 根据重复次数与等差和计算：阈值 t 在 0..255 时，结果为 `64 × (t + 255) × (256 - t) / 2`；t 大于等于 256 时为零。公式的乘法先使用足够宽的整数，避免中间溢出。三个逐元素实现不互相充当唯一真值，空输入与 65535 单元素等边界另有显式期望值。

实际代码使用 GNU `noinline` 保留命名函数，便于定位汇编。它不会禁止 if-conversion 或向量化。栈、容器分配和测试循环只服务于功能校验，没有被包装成独立性能测量区间。

## C++ runnable demo

```cpp include=examples/branch-prediction.cpp

```

本次在 Linux/GCC 13.3.0、x86-64 WSL2 内核 `6.18.33.2-microsoft-standard-WSL2` 执行，未加 `-march=native`。C++20 与 GNU 属性适用于这里的工具链，其他编译器需要重新检查属性支持和生成代码。

```bash
g++ -std=c++20 -O2 -g -Wall -Wextra -Wpedantic -Werror \
  examples/branch-prediction.cpp -o /tmp/branch-prediction
/tmp/branch-prediction
g++ -std=c++20 -O2 -S -masm=intel \
  examples/branch-prediction.cpp -o /tmp/branch-O2.s
g++ -std=c++20 -O3 -S -masm=intel \
  examples/branch-prediction.cpp -o /tmp/branch-O3.s
```

程序对全部 257 个阈值检查两种排列的三种实现，并覆盖空输入、单个最大 uint16_t、阈值高于数据域和 uint32_t 最大阈值。普通 O2/O3 与 ASan/UBSan 执行通过，末尾明确输出未测量耗时或 branch-miss。阈值 128 的求和结果是 `1568768`。

实际 O2 汇编中，`sum_if` 与 `sum_select` 的核心相同，阈值选择使用 `cmovnb`。下面摘录保留了区分两次比较所需的指令，寄存器和标签来自本次产物：

```asm
movzx esi, WORD PTR [rdi]
mov   rax, rsi
add   rax, rcx
cmp   esi, edx
cmovnb rcx, rax
add   rdi, 2
cmp   r8, rdi
jne   .L4
```

第一处 cmp 比较元素与 threshold，cmovnb 选择是否更新和；后一处 cmp 比较结束指针与当前位置，jne 控制下一轮。`sum_mask` 则生成 `cmp` 加 `cmovb`，将不满足条件的值替换为零。因而默认 O2 产物没有针对该阈值条件的跳转，不能把有序和打乱数据的潜在时间差直接命名为该分支的错预测开销。

O3 产物中，三者主循环出现 SSE 的 `pcmpgtd`、`pand` 或 `pandn`、`paddq` 等比较、选择和累加指令，标量尾部仍可见 cmov。这个结论来自本次编译产物，不是所有 GCC、优化级别或目标架构的保证。

为观察数据跳转形态，还实际生成了下列诊断汇编：

```bash
g++ -std=c++20 -O2 -fno-if-conversion -fno-if-conversion2 \
  -fno-tree-vectorize -S -masm=intel examples/branch-prediction.cpp \
  -o /tmp/branch-diagnostic.s
```

该产物的 `sum_if` 在数据比较后使用 `jb` 跳过累加。没有运行这个诊断版本去报告耗时，也没有获取硬件 branch-miss 计数。本机未安装 perf CLI，本章未尝试把这个事实推导成所有 PMU 都不可用；没有生成对应 PMU 测量结果。

## 高频追问

### likely 与 PGO 应怎样使用？

C++20 的 `[[likely]]`/`[[unlikely]]` 表达路径倾向，允许实现据此优化，不规定概率常量、某条机器指令或速度。写错倾向可能使布局和其他选择更差。Clang 文档还说明了它在 O0、PGO 等配置下的具体处理，这属于实现规则。[N4861 likelihood 属性](https://timsong-cpp.github.io/cppwp/n4861/dcl.attr.likelihood)、[Clang 属性说明](https://clang.llvm.org/docs/AttributeReference.html#likely-and-unlikely)

PGO 从插桩或采样 profile 获得执行信息，供编译器优化。训练输入应覆盖实际常态以及有业务意义的突发、恢复和错误路径。只训练最顺畅的处理流，可能让未来重要路径被视为不重要。保留 profile 来源、构建匹配与独立验证输入；本章没有实际生成或使用 PGO profile。[GCC 插桩选项](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Instrumentation-Options.html)、[Clang PGO 指南](https://clang.llvm.org/docs/UsersManual.html#profile-guided-optimization)

### branchless 什么时候可能吃亏？

如果原分支可预测且某一侧工作很重，选择式改写可能增加本可省略的计算。即使两侧都便宜，新增的比较、掩码、操作数读取和依赖也有成本。向量化、代码体积及寄存器压力还会一起变化，所以应比较最终生成代码，检查工作量与吞吐或延迟，而不把“没有跳转”单独当作成功条件。

### 间接调用的优化是否只是给常见类型加 likely？

间接调用涉及目标地址、代码局部性及是否能内联。编译器可能根据类型证明或 profile 产生带检查的快速直接调用路径，再为其他目标保留间接路径。需要观察调用点与目标分布，并验证所有类型语义；单个 likely 标注不能代替类型证明，也不能消除所有间接目标预测成本。

## 容易答错的点

- “条件一半真一半假，所以一定难预测。” 比例不包含顺序信息，历史规律和目标预测器也影响结果。
- “三元表达式就是 cmov。” C++ 只定义条件求值语义，具体指令由编译器选择。
- “mask 写法没有 if，整个函数就没有分支。” 循环和边界处理仍可有跳转，编译器也可能改写选择形式。
- “unlikely 的错误路径可以先不测。” 属性不削弱功能契约，恢复、异常和短输入仍需覆盖。
- “把两边都算出来更适合流水线。” 要先证明新增计算安全且语义等价，再评估额外工作与依赖。

## 性能分析

三个选择实现都是 O(n) 扫描、O(1) 额外空间；输入生成和置换为 O(n)，测试本身还遍历 257 个阈值。直方图验证与生成不属于求和工作，但当前可执行程序把它们放在同一进程，因此直接对整个程序做 perf stat 会把测试框架的循环和分配一同统计。

要形成分支性能结论，先固定实际目标构建，确认数据分支的地址和形态。将生成、预热及验证移出被比较区间，重复测量有序、分块、交替和若干置换输入，交错执行次序并保留每次原始结果。记录硬件、内核、编译器、参数、CPU 放置、输入大小和序列；伪随机种子固定只保证可复现，不保证代表性。

若 PMU 采集可用，核对本处理器的事件定义和支持范围，记录 branches、branch-misses 的绝对计数、测量区间以及 time_enabled/time_running。通用比例可能包含其他跳转，不能直接归给源码的一处 if；使用具体位置归因时还要核对采样精度。诊断关闭优化后的结果只能解释该诊断构建，不能替代默认发布构建测量。

本章没有计时和 branch-miss 实测，因此没有得出有序输入更快、mask 更快或每次错预测损失多少周期的结论。下一步应围绕真实热点准备区间与计数证据，同时观察每条业务消息的 CPU 时间、吞吐和 p50/p99/p99.9；错预测减少但总指令或排队增加，仍可能没有业务收益。

## Quant/Low-Latency 场景

行情解码中的长度、消息类型和字段有效性检查常有倾斜分布。优化可以从代码布局或已有 profile 开始，但必须保留短报文、未知类型与恢复消息的正确行为。把原来受长度条件保护的读取提前，再按掩码丢弃，会破坏解析边界，不能作为低延迟写法。

按消息类型分组可能改变输入规律，也可能破坏序列或状态依赖。只有业务允许的独立域才能考虑重排，域内顺序、快照衔接和失效恢复仍要保持。批量形成的等待时间也应计入端到端延迟；一次纯求和在排列下等价，不能用来证明行情重排等价。

正常时段的 profile 与开盘突发、快照恢复时的分布可能不同。优化版本需要在这些场景分别验证，尤其关注冷路径是否因布局、内联或代码体积变化而影响尾延迟。若目标只是一个阈值过滤，先检查它是否早已被向量化，避免围绕不存在的数据跳转设计实验。

## 相关专题

- [perf / Flame Graph](perf-flamegraph.md)：设计计数和采样区间，区分归因与因果验证。
- [虚调用与去虚拟化](../cpp/vtable-devirtualization.md)：理解间接调用如何变成直接调用和内联。
- [CPU cache / false sharing](cpu-cache-false-sharing.md)：控制数据访问与缓存成本，避免把它们误归为预测开销。
- [UDP 组播与序列号](../network/udp-multicast-sequencing.md)：检查输入重排、边界读取和状态恢复的业务前提。

## 分层面试题

15 道题按 L1/L2/L3 各 5 道组织。练习时从源码语义和实际跳转的区别开始，再解释编译转换、条件求值与 PGO，最后设计兼顾业务顺序和错误路径的验证方案。
