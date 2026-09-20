# Roadmap 内容验收记录

日期：2026-09-19；下文时刻均为北京时间（UTC+8）。每章由一位作者 subagent 编写，主代理负责逐章验收、退回修改和集成检查。所有作者均应用项目 `cpp-quant-writing` 技能。这里记录同一 AI 工作流内的作者自审与交叉复核，不声称有外部独立审查者。

## 本次验收结果

Roadmap A–H 新增 32 章、528 道题，每章包含正文、canonical C++ 示例与作者自审记录；全站合计 37 篇、603 道题。主代理完成逐章全文、答案、代码和来源边界复核，发现的问题退回作者修正。正文及代码最终 SHA-256 记录于 [content-review.json](../reviews/content-review.json)。

最终检查：内容结构、来源字段、hash 与 42 个 HTML 页链接通过；Astro 检查 37 个文件零错误、警告和提示；ESLint、83 个 Markdown 文件与 Prettier 通过；8 项逻辑测试和 22 项桌面/移动浏览器测试通过。37 个 C++ 示例普通严格编译运行通过，新增示例逐章完成 ASan/UBSan 复验；测试范围和未覆盖环境见文末。

32 篇新增专题均检查 1440px 与 390px 视口的章节、题数、答案折叠、整页溢出和页面错误，并查看各批代表截图。末批三章和知识地图完成最终视觉检查，知识地图链接覆盖全部 37 篇；过期“后续发布”文案改为阅读与复查顺序。没有执行合并或部署。

## 已完成技术复核

| 专题               | 题数 | 验收重点                                                 | 作者记录                                            |
| ------------------ | ---- | -------------------------------------------------------- | --------------------------------------------------- |
| RAII 与异常安全    | 18   | 构造失败清理、批量强保证、分配器交换前提、外部副作用边界 | [自审](../reviews/raii-exception-safety-review.md)  |
| move 与值类别      | 18   | const、转发、NRVO、移动后状态、失败时所有权与自引用      | [自审](../reviews/move-value-categories-review.md)  |
| vector 失效        | 18   | 扩容与位置条件、旧 end、异常保证、span 借用期与业务身份  | [自审](../reviews/vector-invalidation-review.md)    |
| 对象生命周期与布局 | 18   | 存储与对象、原位重建、隐式创建、布局性质与对象表示       | [自审](../reviews/object-lifetime-layout-review.md) |
| 模板与 concepts    | 18   | 约束满足、原子约束来源、实例化边界、语法与语义要求       | [自审](../reviews/templates-concepts-review.md)     |
| LRU Cache          | 18   | 跨容器句柄、异常回滚、淘汰顺序、容量与复杂度             | [自审](../reviews/lru-cache-review.md)              |

主代理已通读上述正文、108 道题的答案和评分点，并逐行检查六个 canonical demo。正文采用 C++20 固定版本 N4861 资料；没有把某次构造次数、具体布局或分配策略写成标准保证。

| 工具链专题           | 题数 | 验收重点                                       | 作者记录                                             |
| -------------------- | ---- | ---------------------------------------------- | ---------------------------------------------------- |
| ODR 与链接           | 18   | 实体身份、名字查找、诊断义务与归档提取         | [自审](../reviews/odr-linkage-review.md)             |
| ABI 与 name mangling | 15   | 语言链接、符号可见性、调用约定与版本接口       | [自审](../reviews/abi-name-mangling-review.md)       |
| 虚函数与去虚拟化     | 18   | 构造析构分派、合法对象访问、ABI 调整与优化证据 | [自审](../reviews/vtable-devirtualization-review.md) |

工具链三章的全文、51 道题、示例与交接也已由主代理通读，累计验收新增 9 章、159 道题。ODR 保留同 TU 诊断与跨 TU IFNDR 的区别；vtable 修订了纯虚调用条件和编译时候选目标的表述，未将本例解释为 PGO 实验。

工具链批次中，ABI 与 name mangling 已完成技术验收：11 节、15 题，每层 5 题。主代理通读全文、问答、示例及[作者记录](../reviews/abi-name-mangling-review.md)，复查语言链接、ELF gABI 和 libstdc++ dual ABI 的原始资料。版本长度不代替指针有效性，示例也没有宣称提供 C 公共头或通用跨版本兼容。

并发批次已完成 mutex / condition_variable 的全文、18 道题、示例和[自审记录](../reviews/mutex-condition-variable-review.md)复核。重点检查同锁谓词、两个等待组、关闭后排空、绝对 deadline 与实际业务截止的区别，以及通知之后对象仍须存活。生产者失败与消费者失败的数据处置保证分别说明。

独立 SPSC 章的全文、18 道题、非平凡负载示例和[自审记录](../reviews/spsc-queue-review.md)也已复核。两条同步链分别覆盖 optional 构造发布及移出、销毁后的复用；有限 N 传输与正文讨论的在线 done 协议分别表述。退回修改了“循环均有界”的说法，明确外部重试次数没有固定上界。累计技术验收新增 11 章、195 道题。

ABA 与回收的全文、18 道题和[自审记录](../reviews/aba-reclamation-review.md)已复核。示例用活着的索引模型复现错误状态，用两位标签展示回绕，再检查 atomic shared_ptr 的所有权比较与不可变节点栈。没有运行释放后访问，也没有把 shared_ptr 宣称为通用 ABA 解法。生产者全部 join 后才启动消费者，验证范围如实保留。

NUMA 与 first-touch 的全文、15 道题和代码已复核。CPU 许可、继承内存策略、首次写入、节点查询和页面迁移分别解释；单节点观察未被写成本地与远端性能对比。累计技术验收新增 13 章、228 道题。

TCP framing 的全文、18 道题、解析器与 socket 示例，以及[作者自审](../reviews/tcp-framing-review.md)已验收。逐项检查 consumed 与输入后缀、ready 背压、非法长度、EOF、发送偏移和 fd 所有权；退回修订验证表格，明确零长与最大合法正文接受，两个超限长度拒绝。累计技术验收新增 14 章、246 道题。

UDP 组播与序列号的全文、18 道题及示例已完成技术复核。状态机只接受连续前缀，半空间比较保留旧包寿命与最大跨度前提；会话、快照交接和状态新鲜度由外层控制。数据报截断、零长度报文和坏输入的单流冻结策略均与代码一致。累计技术验收新增 15 章、264 道题。

连接生命周期的全文、18 道题、最终示例和[作者自审](../reviews/connection-lifecycle-review.md)已验收。应用阶段与 TCP 内核状态分别定义，稳定句柄先验证身份再访问对象；apply 返回后回收，代次耗尽退休槽位。假时间仅在 tick 时检查截止，单线程拥有者也没有被解释为自动消除异步借用风险。累计技术验收新增 16 章、282 道题。

io_uring 的全文、15 道题、示例和[作者自审](../reviews/io-uring-review.md)已验收。固定 Linux v6.8 UAPI 与 liburing 2.8 资料边界，区分 SQE 消费、数据 I/O 完成和资源回收。取消小节按原始文档修正 EALREADY，成功取消后的原完成也要求消费并结清状态。累计技术验收新增 17 章、297 道题；A、B、C、D 批次及 H 中的 LRU 均已完成。

perf / FlameGraph、benchmark methodology、尾延迟与背压三章的全文、48 道题、示例和作者自审已完成技术验收。累计新增 20 章、345 道题，全站 25 篇、420 道题。perf 章依据 Linux v6.8 源码修正了 task-clock 的权限过滤解释；benchmark 保留全部原始样本与汇编观察；尾延迟明确区分准入拒绝与上游反馈背压。

分支预测的全文、15 道题、示例及[自审](../reviews/branch-prediction-review.md)已验收。相同直方图的两种排列使用等差和 oracle 校验，257 个阈值及边界通过。主代理复跑严格 warning 和 ASan/UBSan，并独立读取 GCC 13.3 O2 汇编，确认数据条件的 cmov 与循环回边的区别。本章没有计时、PMU 或 PGO 实验。累计新增 21 章、360 道题，A–E 批次及 LRU 完成。

Feed Handler 与快照恢复的全文、33 道题、示例及[Feed Handler 自审](../reviews/feed-handler-review.md)、[快照恢复自审](../reviews/snapshot-recovery-review.md)已验收。前者明确固定 FH1 教学协议、不可变消息身份假设与整批发布失败；后者明确可信快照输入、原始 attempt 上下文、连续 cut 与旧句柄不能撤销。主代理另核对 Nasdaq ITCH/MoldUDP64、Coinbase Full Channel 和 Binance diff-depth 的原始协议边界，没有将教学格式称为真实协议实现。累计新增 23 章、393 道题。

Matching Engine 的全文、15 道题、最终代码和[自审](../reviews/matching-engine-review.md)已验收。固定 price-time/maker 定价及 GTC/IOC 边界与公开规则对照，未泛化到所有场所。主代理复核单位数量 oracle 的独立表示、16384 条命令逐步比较，以及预审后新增的八笔成交、槽位复用 FIFO、64 身份耗尽和两类提交前回滚测试；最终严格 ASan/UBSan 再次通过。累计新增 24 章、408 道题。

OMS 与交易前风控的全文、15 道题、修正版代码和[自审](../reviews/oms-pre-trade-risk-review.md)已验收。权威累计报告及每订单单调身份是虚构协议前提，未冒称 FIX ExecID 语义。逐笔成交限价为外部假设，累计金额检查不替代逐笔证明；unknown、取消竞争和保留预留均与代码一致。累计新增 25 章、423 道题，A–F 批次及 LRU 已完成。

Market-data fanout 全文、15 道题、示例与[自审](../reviews/market-data-fanout-review.md)已验收。逐端整批接受或失效，与源提交分别报告；完整状态邮箱不冒充增量历史。主代理审读代码并严格 ASan/UBSan 复跑通过，核对六个独立中间状态、慢端 seq5 失效、快端继续、满队列外的健康门禁及值副本寿命。序号耗尽只有代码推理，没有伪称运行覆盖。累计新增 26 章、438 道题。

## 已执行检查

- 原有 5 篇的结构、内部链接、审查 hash、静态构建和 8 项逻辑测试通过。
- 原有页面的桌面及移动视口 Chromium 共 22 项浏览器测试通过。
- 主代理在 WSL Ubuntu、GCC 13.3 下执行 ASan/UBSan，包含原有 5 个和新增 3 个示例，8 个程序均退出 0。
- vector 示例另以 `-D_GLIBCXX_DEBUG` 编译运行通过。
- move 示例另以 `-fno-elide-constructors` 编译运行通过；具名返回观察到一次移动，同类型 prvalue 直接构造仍合法。
- 作者各自完成严格 warning 编译、内容契约、Markdownlint 和 Prettier 检查，详情见逐章记录。

首批集成后，内容校验通过：8 篇、129 道题；静态构建生成 13 个 HTML 页面，子路径与 fragment 检查通过。更新后的桌面/移动浏览器测试 22 项全部通过。

新增三篇各在 1440px 与 390px 视口检查：11 节完整、每篇 18 题、答案默认折叠、无整页横向溢出、无页面脚本错误。主代理查看正文与题库截图，文字、表格和答题区可读。截图与检查输出保存在本地 `.artifacts/roadmap-review/`。审查 hash 绑定主代理完成技术复核后的源文件与示例字节。

第二批的对象生命周期、concepts、LRU 示例均由主代理以 GCC 13.3 严格 warning 和 ASan/UBSan 重新编译运行，退出 0。生命周期示例覆盖构造失败后的空状态与构造/析构计数；LRU 示例使用独立顺序表 oracle，覆盖容量 0–4、10,000 步操作、rehash 和分配失败。concepts 的四个失败宏由作者逐一核对诊断，主代理另行复查 subsumption 歧义与函数体实例化失败两个宏，均因预期原因编译失败。失败分支不参与默认正常构建。

第二批集成后的内容校验通过：11 篇、183 道题，16 个 HTML 页的子路径和 fragment 检查通过，桌面/移动浏览器测试 22 项通过。三篇新专题及修正后的 shared_mutex 均在 1440px 与 390px 视口检查通过；新专题各有 18 题，shared_mutex 保持 15 题，均为 11 节、答案默认折叠、无整页溢出或页面脚本错误。主代理查看了正文表格和答题区截图。

ABI 示例由主代理以严格 warning、ASan/UBSan 复跑通过。另以 O0、关闭内联生成目标文件，实际 readelf 符号表确认两个重载为 GLOBAL DEFAULT、C 入口为 GLOBAL DEFAULT、hidden 探针为 GLOBAL HIDDEN、匿名命名空间函数为 LOCAL DEFAULT。没有创建共享库或将目标文件符号表当成动态导出测试。

ODR 由主代理复跑单 TU 和三 TU 的 ASan/UBSan，以及普通静态库正确顺序的链接运行；四个错误变体分别核对重定义、缺定义与库顺序的实际诊断，没有运行错误产物。vtable 由主代理复跑 ASan/UBSan，并读取 O0/O2 汇编，确认未知引用的间接尾跳转、final 参数的字段读取、局部对象的常量返回，以及次基类 thunk 与虚表组的布局记录。

工具链批次集成后，内容校验为 14 篇、234 道题，19 个 HTML 页的子路径与 fragment 检查通过。新增三章在 1440px 和 390px 视口均检查到 11 节、正确题数、答案折叠，无整页溢出及页面脚本错误；主代理查看了桌面正文、移动正文表格及答题区截图。全站 ESLint、Markdownlint、Prettier、8 项逻辑测试和 22 项浏览器测试通过。

mutex 示例由主代理以严格 warning、pthread 和 ASan/UBSan 复跑，正常传输 128/128，生产者注入失败后排空 37/37。关闭竞争测试不以 sleep 猜测线程已经阻塞；30 秒仅作 watchdog，不被解释为实时完成保证。作者的 TSan 版本编译成功，但因 unexpected memory mapping 启动失败，未完成 TSan 竞争检测。

SPSC 由主代理以严格 warning、pthread 和 ASan/UBSan 复跑，九轮并发传输共 54,000 条，连同边界与静止析构测试共 55,282 个资源全部释放。作者的独立 TSan 也在启动时遭遇映射错误，未得到竞争检测结果。主代理正常全量 C++ harness 此时执行 16 个示例全部通过，其中包括已审读代码、尚在完成正文的 mutex 和 SPSC；该单文件运行没有替代 ODR 的多 TU 检查。

mutex 与 SPSC 集成后，结构及 hash 校验通过：16 篇、270 道题，21 个 HTML 页的内部链接通过。两篇在 1440px、390px 视口均为 11 节、18 题，答案折叠且无整页溢出或脚本错误。主代理查看截图，确认条件变量表格的两行谓词完整显示在第三列。

ABA 示例由主代理以严格 warning、pthread 和 ASan/UBSan 复跑通过。检查索引 ABA、标签回绕、相同地址不同所有权的 CAS、注入分配失败后的状态保留、延迟释放，以及 128 个值恰好消费一次。atomic shared_ptr 在本机不是 lock-free；有限测试不证明任意并发历史正确。

NUMA 示例由主代理以严格 warning、pthread 和 ASan/UBSan 复跑通过。本机 32 个页地址查询结果均为节点 0，写读、worker 亲和性恢复及映射释放通过。作者另在仅允许 CPU 15 的进程中确认选择 CPU 15；这些结果只覆盖本机单节点环境。

ABA 与 NUMA 集成后，内容及 hash 校验为 18 篇、303 道题，23 个 HTML 页的链接检查通过。两篇在 1440px、390px 下的章节、题数、答案折叠、整页溢出及脚本错误检查通过，主代理查看了正文和题库截图。桌面与移动浏览器回归 22 项通过。

TCP 示例由主代理以严格 warning 和 ASan/UBSan 复跑通过。31 字节流的 32 个切分位置、逐字节输入、三帧结果、EOF 前缀、编码预算和发送脚本断言通过；真实 AF_UNIX 流 socket 验证初始 EAGAIN、单向 EOF、反向回复与 MSG_NOSIGNAL 下的 EPIPE。未把主动限制每次 send 为三字节称为内核短写，未以本例支持真实 TCP 性能结论。

TCP 集成后的校验为 19 篇、321 道题，24 个 HTML 页的链接通过。Astro 检查 37 个文件无错误、警告或提示；ESLint、Markdownlint、Prettier 和 8 项逻辑测试通过。新章在 1440px、390px 下均有 11 节、18 题，答案默认折叠，无整页溢出或脚本错误；主代理查看正文及移动题库截图。

UDP 示例由主代理以严格 warning 和 ASan/UBSan 复跑通过。检查模序号边界、完整前缀、缺口后的冻结、可信基线安装、累加溢出、格式边界及六种排列；本机 SOCK_DGRAM 检查 MSG_TRUNC 输出位、余部丢弃、下一数据报、零长度报文和异常后的 fd 释放，没有实际组播流量。

网络示例完成代码预审时，主代理再次执行普通全量 C++ harness，22 个示例全部通过，其中 io_uring 与连接生命周期的正文仍在作者完成中。两者的独立 ASan/UBSan 也通过：前者 NOP 的 user_data 匹配、res=0、overflow=0，后者状态模型断言与本进程 loopback connect 的 SO_ERROR=0。最终正文验收和 hash 登记单独完成。

UDP 集成后为 20 篇、339 道题，25 个 HTML 页链接通过；新章在 1440px、390px 下均有 11 节、18 题，答案折叠、无整页溢出和脚本错误。主代理查看正文和题库截图，表格列数检查通过。

连接生命周期在预审后补入“本地先写半关闭，再收到 peer EOF”的确定性分支，主代理审读新增代码并再次以严格 warning 和 ASan/UBSan 编译运行最终示例，通过。两种关闭顺序、部分输出、超时丢弃计数、错误关闭和代次退休均覆盖；真实 loopback 仍只验证 connect 和 SO_ERROR。

网络批次最终集成为 22 篇、372 道题，27 个 HTML 页的链接检查通过。io_uring、连接生命周期在 1440px、390px 下均有 11 节及对应题数，答案折叠、无整页溢出或脚本错误；主代理查看了正文表格和移动题库截图。全站 22 项浏览器回归通过，表格列数检查通过。

性能三例均经主代理严格 warning 和 ASan/UBSan 复跑。perf 的四组 checksum 另由独立计算核对；本机直接 task-clock 计数可用，但未安装 perf CLI，也未实际生成 FlameGraph。benchmark 的 GCC 13.3 O2 汇编保留测量区间内的调用循环，B 实现已向量化，不能将观察差异单独归因于四条标量累加链；自审中的完整 20 对原始样本与 nearest-rank 数值一致。尾延迟的开放、闭环和截止轨迹由主代理手算核对，64 条 FIFO oracle 与边界断言通过，数字是抽象 tick。

性能三章集成后内容及 hash 校验为 25 篇、420 道题，30 个 HTML 页链接通过。三章在 1440px、390px 下的 11 节、题数、答案折叠、整页溢出与脚本错误检查通过；主代理查看桌面正文、移动表格和题库截图。逐章记录见 [perf](../reviews/perf-flamegraph-review.md)、[benchmark](../reviews/benchmark-methodology-review.md)、[尾延迟](../reviews/tail-latency-backpressure-review.md)。

14:55 起的全站回归通过：Astro 检查 37 文件零问题，ESLint、Markdownlint、Prettier 与 8 项逻辑测试通过，桌面/移动 22 项浏览器测试通过。普通全量 C++ harness 的 26 个示例全部通过；其中分支预测此时已完成代码预审，正文与 hash 随后登记。微基准程序在 harness 中的输出只作正确性检查，不与作者保留的原始性能样本混合。

分支预测集成后为 26 篇、435 道题，31 个 HTML 页链接通过；其 1440px/390px 视口检查通过，主代理查看了移动正文截图。

Feed Handler 与快照恢复分别由主代理以严格 warning 和 ASan/UBSan 复跑通过。前者检查手写 golden、独立规范化事件、36 种截断、迟发现坏记录与满发布区不部分提交；后者检查六种排列、补 gap、cut 边界、旧 session/attempt、冲突和容量毒化、数量与身份耗尽。快照测试只使用单线程，未进行并发压力或 OOM 注入。

Feed Handler 与快照恢复集成后为 28 篇、468 道题，33 个 HTML 页链接通过。两章 1440px/390px 下均有正确的章节和题数，答案默认折叠，无整页横向溢出和页面脚本错误。主代理查看桌面正文、移动表格与题库截图；全站 Markdown 表格列数一致。

OMS 最终代码由主代理以严格 warning 和 ASan/UBSan 复跑通过。独立手算轨迹最终成交 6、取消余量 4、spent=54、reserved=0；Fill 早于 Ack、取消期间成交、权威终结补累计量、unknown 保留、容量和整数边界均通过。

撮合、OMS 与行情分发集成后，内容和 hash 校验为 31 篇、513 道题，36 个 HTML 页链接通过。三章在 1440px/390px 下均有 11 节、15 题，答案默认折叠，无整页溢出或脚本错误；主代理查看了桌面正文、移动正文和题库截图。Markdown 表格列数一致，22 项浏览器回归通过。

## 发现与处理

- move 扩容实验补入 `capacity() < max_size()` 前提，正文区分演示类型的移动构造与默认移动赋值。
- 三章增加同批前置与后续链接，避免语言基线只能跳到较远的并发或交易主题。
- 既有浏览器测试把专题数固定为 5，并依赖搜索唯一命中、L1 为空。作者已改为实际专题数与匹配集合；新增类型错误退回作者修正，Astro 检查和浏览器回归均通过。
- 复核 LRU 时发现原有 shared_mutex 正文将 erase 与 rehash 对元素引用的影响混写。作者已区分关联容器种类、迭代器失效、元素删除与解锁后的并发访问风险，并补入 N4861 依据。主代理通读修改和[复核记录](../reviews/shared-mutex-followup-review.md)，该篇示例与题库未改动。
- mutex 章表格中的逻辑或被 Markdown 识别成分栏符，常规 lint 未报错。作者改成自然语言谓词；主代理扫描各篇表格列数，确认修复后全部一致，并在构建页面中完成视觉复核。

benchmark 和 Feed Handler 的教程正文删去了主代理协作过程说明，实际验证保留在自审和本记录中；benchmark 仅改文字，代码和原始性能记录未变，重新登记正文 hash。

OMS 初稿未验证未知报告枚举，可能落入普通确认路径并改变账本。主代理退回后，作者增加五种 Kind 的显式校验及 Kind=99 故障断言，确认原金额保持、halted=true、订单 unknown；主代理复跑修正版通过。

fanout 的积压公式经主代理退回补入显式丢弃量 R：初始积压+A−D−R；拒入不计入 A，出队或清理均不等于业务完成。

交易网关与确定性回放的正文、30 道题、示例及作者记录已全文复核。网关的 16 种分段、6 个断线位置、身份与回调边界通过严格 warning 和 ASan/UBSan；未知 Verdict/EvidenceKind 初稿处理不足已退回修正并复跑。absent_fenced 依赖脚本关闭后无远端待处理工作的强前提，未将 TCP close 或 NotFound 解释为可安全重发证据。详见[网关自审](../reviews/trading-gateway-review.md)。

回放的 4 条手算增量为 7、12、2、10，完整回放与水位 2 快照加后缀均为余额 31、4 条 pending；160 个短前缀及每字节最低位翻转、语义故障和输出权限测试通过严格 warning 与 ASan/UBSan。快照一致性、提交 target 与 epoch 授权是外部可信输入；FNV32 非认证，outbox 没有确认删除，内存 Sink 没有持久化。主代理复查 fsync 与 SQLite WAL 原始资料，未以示例宣称真实灾备完成。详见[回放自审](../reviews/deterministic-replay-recovery-review.md)。

内存池正文、15 道题、代码与[作者记录](../reviews/memory-pool-review.md)已全文复核，主代理严格 warning 和 ASan/UBSan 通过。检查非平凡过对齐对象、构造异常中的成员资源展开、空闲链回滚、默认/跨存活池/陈旧/重复句柄、255 轮后退役与最终资源计数。N4861 的字节存储、construct_at 与对齐原文已复查；裸指针借用、池自身寿命和禁止重入仍是调用方前提。池按槽位顺序析构，Resource 内部仍有堆分配，没有泛化为 allocator 或无分配接口。

网关、回放与内存池集成后为 34 篇、558 道题，39 个 HTML 页链接通过。三章在 1440px/390px 下的 11 节、15 题、答案折叠、整页溢出及脚本错误检查通过，主代理查看桌面正文、手机表格和题库截图；全站 Markdown 表格列数一致。

末批 Ring Buffer、Price Ladder、Timer Wheel 的正文、45 道题、代码和作者记录已全文复核，三例均由主代理以严格 warning 与 ASan/UBSan 编译运行通过。

- [Ring Buffer](../reviews/ring-buffer-review.md)：不可移动非平凡对象、构造失败的成员清理、容量 1/3、2×6561 条轨迹和 10000 步回绕与 deque 一致。明确禁止重入，别名构造副作用不受结构回滚保护；整体析构除 clear 还计入 N 个 optional 槽的处理成本。
- [Price Ladder](../reviews/price-ladder-review.md)：精确小数解析含 int64 两端、固定 tick 的有界价域、bit0/63 与总量溢出检查通过；400 次 map 对照逐步核对 25600 个档位。构造跨度有意限制在 INT64_MAX 内，完整 set 包含 verify 扫描。主代理复查 JPX 官方 tick 表与 N4861 bit.count，未把固定教学网格当成真实产品配置。
- [Timer Wheel](../reviews/timer-wheel-review.md)：24 个 tick 的独立完整事件集合、取消复用、8 项同时到期、关闭和最大 tick 检查通过。新增 8 项测试后已重新运行 sanitizer；其 count/sum 检查与原完整集合 oracle 分开记录。真实时间公式补齐 epoch 偏移，id 明确为实例内身份。未实际跑完 uint64 id 空间；没有真实时钟、线程或回调执行。

末批代码预审完成后的普通全量 C++ harness 执行 37 个示例全部通过，之后 timer 仅新增同 tick 测试，由作者普通严格构建和主代理 sanitizer 复验通过。此轮微基准输出仍只用于功能检查，不替换或混入原始性能样本。A–H 合计新增 32 章、528 道题，连同原有 5 篇共 37 篇、603 道题。

最终阅读复核将 ABI、io_uring、NUMA 三章摘要中的平台版本说明原样移至“核心概念”。主代理将段落移回原位后计算 hash，均与上轮验收字节一致，确认没有附带修改题目、代码、条件或链接。更新三篇正文 hash 后，内容校验、42 页静态构建和链接检查通过；三章的 1440px/390px 检查及代表截图复核通过。

另逐页核对全部 37 个构建产物：CodeLAB 编辑器初始源码与对应 canonical 文件一致，最长示例为 18253 个字符，未超过编辑器上限；9 个 Linux 示例均显示 Linux/WSL 运行说明。37 个对话框初始关闭，运行区显示尚未运行，603 道题答案初始折叠。这项静态检查没有提交代码到在线编译服务；运行按钮、懒加载与草稿交互由前述浏览器测试覆盖。

扩充后的首页搜索索引包含全部 37 篇和 603 道题。在 390px Chromium 视口逐一输入 37 个专题标题，均能显示对应专题；再输入网关、回放、Ring Buffer、Price Ladder、Timer Wheel 各自最后一道题的题干，均能找到所属专题。清空搜索后恢复 37 项，未出现页面脚本错误。该检查只验证检索功能，不代表真机弱网性能测量。

## 验证边界

本机没有 Clang，不能声称本地完成双编译器验证。Sanitizer 与断言仅覆盖有限执行，不替代生命周期和异常保证的推理。benchmark 章仅有注明环境的一次微基准原始观测，没有可推广的性能收益结论。未完成真实内存耗尽注入、Safari 真机或弱内存硬件测试。桌面/手机指 Chromium 的两种视口。

原有五篇的历史审查仍见 [首期记录](REVIEW.md)。用户在任务开始前已修改写作技能，该改动保留。
