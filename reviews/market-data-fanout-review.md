# Market-data fanout 专题自审

日期：2026-09-19。类型：作者自审。主 agent 已全文预审代码并独立严格编译、运行 ASan/UBSan；正文等待最终验收。本文件记录证据和范围，不替代主 agent 的发布 hash。

## 交付

- `content/topics/design/market-data-fanout.md`：11 节正文，15 题，L1/L2/L3 各 5 题。
- `examples/market-data-fanout.cpp`：C++20 单文件，两个独立增量端加完整状态邮箱的确定性模型。
- 本文件：来源、技术自审和真实验证。

roadmap G 将 market-data fanout 放在系统设计，taxonomy 使用 design / System Design。已应用 cpp-quant-writing 与写作、质量规范。仅修改本章三个文件，没有修改共享 manifest、roadmap、skill、测试、已冻结章节或提交 commit。

模型是虚构的双证券数量增减，初始状态 `[100,200]`、generation=7、next=1 由教学前提给定。它没有真实行情协议、网络、交易行为、并发、持久化或恢复入口。

## 来源核查

六条 references 已全部实际打开：

| 论点                                                            | 资料                                                                                                                                                              |
| --------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 广播与工作分摊不同，每端序号及 gating 限制槽复用                | [LMAX Disruptor user guide](https://lmax-exchange.github.io/disruptor/user-guide/)，页面标示 4.0.0-SNAPSHOT；只引用机制，没有采用其历史吞吐表或 Java 等待时延数值 |
| pending limits、慢消费者信号及客户端行为差异                    | [NATS slow consumers](https://docs.nats.io/learn/resilient-clients/slow-consumers)，从旧文档路径跳转至当前页面，核对客户端例外，没有把全部客户端写成相同丢弃方式  |
| CONFLATE 只保留最后消息、multipart 限制，HWM 行为依 socket 类型 | [ZeroMQ socket options](https://zeromq.github.io/libzmq/zmq_setsockopt.html)，没有声称其选项自动构成按键完整状态或精确端到端内存上限                              |
| 框架写入接受不等于已上网络，流量控制的适用层次                  | [gRPC flow control](https://grpc.io/docs/guides/flow-control/)，未将传输确认等同于业务处理或持久化确认                                                            |
| 共享所有权、最后持有者释放及对象字段同步的区别                  | [C++20 N4861 shared_ptr](https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.shared)                                                                          |
| 跨线程数据竞争与同步前提                                        | [C++20 N4861 intro.races](https://timsong-cpp.github.io/cppwp/n4861/intro.races)                                                                                  |

没有安装或运行上述中间件，也没有声称示例实现其 API、可靠性或并发保证。容量、交付与恢复部分的设计取舍均围绕本文明示模型分析，不冒充某个交易所的要求。

## 技术自审

1. 广播与共享工作队列区分。两个 DeltaConsumer 各有队列和 Health，不让两个消费者竞争同一条消息；LatestConsumer 的合同另行定义。
2. 增量不可静默覆盖。本例容量不足会使相应订阅者失效，不再交付其旧队列内容；这不是无损或 exactly-once 的实现，也没有保存其可恢复日志。
3. source_accepted 只表示源批次已提交。每消费者的 accepted、invalidated、inactive 独立报告；源拒绝时 delivery 全为 not_attempted。其他端失败不能回滚已提交源和成功端，正文提醒不能盲目重试整批。
4. 输入批次数 1–3，超范围或空批明确拒源。代次固定为 7，序号要求与 next+i 一致，instrument 小于两项，change 非零且在 ±1000 范围内。
5. staged 从源数量按值复制，所有变化在该副本计算，数量检查为 0–1000。中间加法使用 int64_t，既有数量与允许 change 的和只可能在 -1000 到 2000，不会触发 signed overflow。
6. 输入全部通过后才提交 quantities 与 next。第一条合法、第二条会使数量为负的测试确认源状态不部分更新，所有订阅者失效，未尝试投递。
7. next 推进前检查 uint32_t 剩余范围，防止回绕；从 1 开始并保留耗尽 next 的设计由代码检查。没有运行到 UINT32_MAX 附近，正文及本报告都不把这一分支算作实际覆盖。
8. 每端四槽队列在复制前检查整个 batch 的容量。循环索引至多由有界 head、count、i 构成；有效入队时所有偏移均落在 ring 内，pop 后清槽并模容量推进。
9. Delta 和 Snapshot 为拥有标量字段的值类型，复制赋值不抛出由 static_assert 检查。源提交后的各端复制和 optional 状态赋值不依赖动态分配，本模型没有回调观察中间步骤。
10. 一端容量不足不会阻止其他端接受。主测试中快端每批清空，慢端积压四项，在第三批首序号 5 失效，快端仍收到 5、6，并能继续接受 7。
11. Health 在数据队列之外，满队列不会让失效标记无处存放。pop 先检查 valid，慢端四项仍在也不返回。这里仅是单线程同步门禁，没有把普通变量写成跨线程安全。
12. Health 保留第一次失效。已经 slow_consumer 的端不会被之后的 source_invalid 覆盖原因；Fanout.valid 单独表示源状态。它不是完整错误历史，正文保留这个边界。
13. discard_invalid_backlog 清除旧值并释放本地槽位，但不恢复健康。后续 offer 返回 inactive，示例没有重连、重放或重建，因此不能声称清队列即恢复。
14. 门禁不撤销调用方已经取得的 Delta 或 Snapshot 副本。实际跨线程/进程需额外代次与使用时点协议，业务有效性不能由对象仍存活推导。
15. LatestConsumer 合并的是应用所有连续输入后得到的完整双证券数组，并携带 generation 和 through_sequence。它不会只保存最后一个 Delta，也未声称支持审计历史或按请求事件交付。
16. 该邮箱最多保存一份内部状态；外部持有者若任意保存副本，队列容量不限制这些额外内存。正文同时提到网络缓冲、在途对象和重放预算，不把局部容量当全系统上界。
17. 以共享对象替代复制只在讨论中出现，没有引入共享可变对象。正文明确 shared_ptr 不自动同步 payload，慢引用会影响最后回收。
18. 当前固定代次和初始状态是已建立前提，不证明 bootstrap 已完成。快照衔接部分给接口条件，没有伪称示例具备恢复协议或新鲜度检测。
19. 广播成本分析使用 n 个增量端、m 条批次及 k 项完整状态的范围，指出完整快照 O(k)；没有拿双证券复制成本外推全市场固定开销。
20. 按主 agent 意见修正容量模型，待取积压为初始积压加 A−D−R，分别累计成功入队、出队和从队列显式丢弃的数量。discard_invalid_backlog 计入 R，拒绝入队不计入 A。没有把出队或丢弃等同业务完成，也没有用源吞吐不降证明所有消费者健康。
21. 15 题各层 5 题，均为 derived，companies 为空。L3 涵盖共享 ring 的回收、审计无损、恢复边界、容量和观测；没有公司来源或与代码无关的空泛结论。

## Oracle 与实际覆盖

原始六个 Delta 和六个中间状态均在源码中独立列出，状态 oracle 为：

```text
start [100,200]
1: instrument 0 +5  -> [105,200]
2: instrument 1 -10 -> [105,190]
3: instrument 0 -3  -> [102,190]
4: instrument 1 +20 -> [102,210]
5: instrument 0 +8  -> [110,210]
6: instrument 1 -5  -> [110,205]
```

每批两项后，检查源状态等于相应 oracle；快消费者逐条检查事件身份和中间状态，最后应用六项。调用者覆盖原 batch 后结果不变。慢端最终 pending=4、failed_at=5，Health 可见但 pop 返回空。

邮箱在未消费 sequence 2、4 状态后仍能取得 through_sequence=6 的完整 `[110,205]`，随后 take 为空。清理慢端四项积压不使其恢复。追加第七条得到源状态 `[100,205]`、next=8，快端 accepted，慢端 inactive。

另一个实例先保存三项，快端取完、慢端剩三项；随后两项批次只给快端成功，慢端原有三项保持不变，验证“仅剩一槽不能接受半个批次”。

拒源检查覆盖八类输入：第二条造成数量下溢、空批、四项超长批、起始序号不连续、错代次、未知证券、零变化及 change=1001。每次拒绝都保留初始数量和 next，所有 Health 失效，后续合法 batch 也不恢复。数量上溢、change 小于 -1000 及序号耗尽分支经过源码边界分析，未单独实际触发；不标为运行覆盖。

寿命测试在 Fanout 存活期间 take 快照，销毁 Fanout 后仍验证拥有值的快照。它只验证内存所有权，不证明该快照仍是当前有效行情。

## 实际验证

环境：WSL2 x86-64，Linux 6.18.33.2-microsoft-standard-WSL2，GCC 13.3。

常规构建：`-std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`。

ASan/UBSan：`-std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie`。

在增加明确 not_attempted 结果和一槽空余批次测试后，两种最终构建再次退出 0，断言通过，sanitizer 无报告。此版本随后交父级预审，主 agent 独立严格编译和 ASan/UBSan 运行通过；之后源码未修改。

实际输出：

```text
fast: 6 ordered deltas match oracle; slow: invalid at 5 with 4 pending
latest: complete state at 6 is [110,205]; slow reclamation does not reactivate
invalid source batches commit nothing; owned snapshot survives producer lifetime
single-thread model only; no networking, persistence, or recovery implemented
```

测试保持断言启用，正文明确部分操作在 assert 内，不使用 `-DNDEBUG`。没有 TSan 或真实网络实验，未作性能测量，不把 sanitizer 无报告当并发或生产正确性证明。

## 交接检查

最终检查包括 Prettier、Markdownlint、内容 schema、11 节、各层题数、canonical include、内部链接和 GFM 表格列数。结果与三个 SHA-256 随交付消息发送，共享发布审查由主 agent 负责。
