# Feed Handler 专题自审

日期：2026-09-19。类型：作者自审。主 agent 已逐行审读代码，并独立严格编译及运行 ASan/UBSan；正文等待最终验收。本记录不替代主 agent 的发布 hash。

## 交付与范围

- `content/topics/trading/feed-handler.md`：11 节正文，15 题，L1/L2/L3 各 5 题。
- `examples/feed-handler.cpp`：C++20 单文件，同步、固定上界的 FH1 解码与发布路径。
- 本文件：来源、技术边界、实际测试与修正记录。

roadmap F 包含 Feed Handler，taxonomy 将 Market Data / Feed Handler 放在 trading，采用 slug `feed-handler`。已应用 cpp-quant-writing、WRITING_STYLE、CONTENT_STANDARD 和贡献指南。没有改共享 manifest、roadmap、已冻结章节、skill 或测试，没有提交 commit。

FH1 是本章明确虚构的教学协议，不是任何交易所 wire 实现。其消息是绝对顶层报价替换，只表达每证券每侧的价格和数量，不表达 MBO 订单、深度队列或撮合。输入路由、session 与 next 锚点由调用方预先建立，示例没有证明启动快照已取得。

## 资料核查

六条 references 均实际打开：

| 核查内容                                            | 资料与适用范围                                                                                                                                                                           |
| --------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 行情消息与外层交付、按日 stock locate、整数价格精度 | [Nasdaq TotalView-ITCH 5.0](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf)，已核对 Architecture、Data Types 及相关身份说明 |
| session、首条消息序号、消息数量及控制消息           | [MoldUDP64 v1.00](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/moldudp64.pdf)，已核对包头、隐式连续编号、heartbeat 与结束会话区别                   |
| span 借用、不拥有存储与子视图前提                   | [C++20 N4861 views.span](https://timsong-cpp.github.io/cppwp/n4861/views.span)                                                                                                           |
| 对象类型访问及字节访问边界                          | [C++20 N4861 basic.lval](https://timsong-cpp.github.io/cppwp/n4861/basic.lval)                                                                                                           |
| 数据竞争与同步关系                                  | [C++20 N4861 intro.races](https://timsong-cpp.github.io/cppwp/n4861/intro.races)                                                                                                         |
| 数据报接收截断信息                                  | [Linux recv(2)](https://man7.org/linux/man-pages/man2/recvmsg.2.html)，用于解释接收层前提，demo 不调用该 API                                                                             |

真实规范仅用于具体层次和边界说明，没有将 FH1 字段、数量限制、序号范围或 session 过滤策略归给交易所。尝试打开 GLIMPSE 的候选链接失败，正文未引用它，也未据此写恢复协议结论。

## 技术自审

1. Header 固定 12 字节，读取任何头字段前先检查长度；count 限制为 1–4 后才计算总长度 12+count*12，要求输入恰好匹配。最大帧 60 字节，没有按输入数量动态分配。
2. 所有记录读取都在总长度检查后，be16/be32 的调用前提写在代码及正文中。通过逐字节组合读取大端无符号值，不依赖结构体布局、未对齐加载或未经证明的类型转换访问。
3. 版本必须为 1，session 必须非零，record 保留位为零，locate 仅接受 1、2，side 仅接受 1、2。零数量要求零价格，非零数量要求非零价格，没有将未知值静默当作默认值。
4. 固定映射为 locate 1→1001、2→1002，wire 价格单位 10^-4，目标 10^-8。先提升到 uint64_t 再乘 10000，最大 uint32_t 输入仍在目标类型范围内。没有声称支持真实价格 band、tick 或证券状态。
5. Handler 为一个外部已区分的通道；Event 未重复保存 channel ID，多通道身份需由路由或事件字段补足。session 和 next 由构造参数提供，不会根据首个包自动切换。
6. 错误检查顺序已与正文逐项对齐：先 valid；再头部、长度、零 session、序号范围；再过滤不同 session；目标 session 才检查记录并标准化。wrong_session 不意味着任何其他 session 的任意报文都被无害丢弃。
7. 目标 session 的完整记录校验先于 duplicate/gap 判断。旧包若记录格式非法仍会失效，这是本例明确选择的策略，不是所有行情协议的要求。
8. end 在 uint64_t 中计算，要求不大于 UINT32_MAX。允许发布的最大消息序号为 UINT32_MAX−1，UINT32_MAX 可以保存为耗尽后的 next；再来一条该序号的消息会明确报 sequence_range，不进行模回绕比较。
9. end<=next 为完全旧区间；first>next 为 gap；其余路径有 first<=next<end，因此 skip=next-first 满足 skip<count，子 span 在局部 staged 数组范围内。
10. 按序号跳过依赖同一通道/session/sequence 对应内容不可变。代码不保留已发布 payload 历史，不验证重复或重叠前缀与原内容一致，冲突副本检测明确不在范围内。
11. 先把最多四条规范化到局部数组，任何记录失败都没有发布前缀或推进 next。gap 同样没有将 next 改成当前帧，以免把跳过缺失区间误作恢复。
12. Publisher 容量为八个 Event，先校验整批剩余容量，再逐项复制，最后修改可见 count。Event 复制赋值不抛出的性质由 static_assert 核对；成功后 Handler 才推进 next。单线程同步调用没有中间回调，这种提交边界不是并发原子事务。
13. Publisher 只保存历史，没有 drain 和并发消费者。剩余一槽接收两条新后缀会整体失败，原历史和 next 保持不变，valid 变为 false。所谓 backpressure 结果是有界失败通知，不是完整反馈控制或重放机制。
14. Event 拥有其标量值，Publisher 不保存输入 span 或 staged 指针；覆盖输入的测试核对已发布事件不随 buffer 复用改变。span 本身的借用性质没有被混淆为所有权转移。
15. invalid 后调用 ingest 直接 blocked。已发布历史仍保留，但调用方不能将其继续当作当前有效行情；同步返回值和普通 bool valid 的使用条件写明，跨线程控制消息交付需要另行设计。
16. 多线程发布还需明确同步和复用；普通 count 或 valid 不能建立 happens-before。正文未声称当前代码支持 SPSC、MPSC、双路仲裁或多个下游的一致事务。
17. 标准化规则及固定字典不包含动态参考数据 epoch；真实 ITCH locate 的按日条件仅作为不应跨会话沿用映射的具体依据，没有外推所有市场相同。
18. 网络层数据报截断必须在上游识别；本例的完整长度检查不取代接收元数据。没有实现 CRC、认证、网络 I/O 或真实交易协议。
19. 15 题各层 5 题，均为 derived 且 companies 为空。L3 涵盖健康通知交付、冗余源一致性、协议演进、完整路径测量和恢复衔接。不会凭序号变连续就自动恢复 valid。
20. 没有性能实测或加速承诺。O(m) 分析指帧中 m 条记录的解析与复制，当前 m<=4；固定存储只说明分配来源受控，不保证固定延迟。

## Reference oracle 与故障覆盖

golden 为源码中手写的 36 字节帧，不经辅助 packet 编码器生成；期望事件也独立列出：

```text
session 7, sequence 100, instrument 1001, bid, price_e8 125000000, quantity 10
session 7, sequence 101, instrument 1002, ask, price_e8 250010000, quantity 20
session 7, sequence 102, instrument 1001, bid, price_e8 0, quantity 0
```

前两条来自 golden，第三条来自含 101 和 102 的重叠帧，最终历史恰好三条，next=103。首次发布后把输入数组全部填零，结果仍等于 oracle；重收 golden 返回 duplicate，不重发。

实际覆盖：

- golden 的每个 0–35 字节短前缀，以及额外一字节尾部。
- magic、version、count 为 0 或 5、零 session、未知证券。
- 第二条的非法 side、非零 flags、价格与数量不匹配；第一条合法也不会提前发布。
- 零 first、范围越界，以及最大允许消息序号后不能再回绕。
- 不同合法 session 保持当前状态，gap 导致 invalid，后续合法帧仍 blocked。
- 发布区先保存七条，只有一项剩余时，两条新事件被整体拒绝；count=7、next=107 均不变，valid=false。

最大价格和数量转换的实际运行，以及重叠后缀恰好填满最后一槽，留作正文练习；本文没有将它们标为已执行测试。没有做随机 fuzz、其他架构验证、双路重传冲突或协议兼容认证。

## 实际编译与运行

环境为 WSL2 x86-64，Linux 6.18.33.2-microsoft-standard-WSL2，GCC 13.3。常规编译：`-std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`。ASan/UBSan：`-std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie`。

初次严格编译指出 range-loop 结构化绑定复制 pair，已改为 `const auto&`，并显式包含声明 pair 的 `<utility>`。随后常规构建和 sanitizer 构建均退出 0，主 agent 对同一代码独立严格编译与 ASan/UBSan 运行也通过。所有测试保持断言启用，正文明确不使用 `-DNDEBUG`，因为 ingest 在断言表达式中执行。

两种构建的实际输出一致：

```text
normalization/ownership/duplicate/overlap: 3 oracle events passed
truncation/fields/late-record/range failures: no partial publication
session/gap/backpressure/sequence exhaustion: state checks passed
FH1 teaching protocol only; no transport, recovery, or trading implemented
```

sanitizer 无报告只支持本次有界路径的检查，不证明任意输入都正确或可用于真实行情接入。代码供父级预审后未再修改。

## 格式与发布交接

最终检查覆盖 Prettier、Markdownlint、内容 schema、11 节、题目分层、canonical include、内部链接及 GFM 表格列数。检查结果与最终三个 SHA-256 随交付消息发送。发布验收与共享审查 manifest 由主 agent 完成。
