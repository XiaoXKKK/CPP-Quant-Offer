# 尾延迟与背压专题自审交接

- 日期：2026-09-19。
- 类型：作者自审，主 agent 独立验收。
- 范围：`content/topics/performance/tail-latency-backpressure.md`、`examples/tail-latency-backpressure.cpp`。
- 结论：确定性模型、独立 FIFO oracle 和边界断言通过；全部数字均为模型 tick，不是本机性能测量。

## 内容与写作

应用项目 `cpp-quant-writing`、写作约定、内容质量标准与贡献指南。正文 11 节、18 道题，L1/L2/L3 各 6 道，全部 derived，companies 为空。按输入轨迹说明机制，没有以分位数差异宣称提速。

正文明确区分背压、限流、准入拒绝与取消：demo 满队列拒绝属于 load shedding，没有实现上游反馈控制。credit 与高低水位仅用于解释生产协议责任。闭环输入是串行客户端模型，不能由此声称实现了真实背压。

统计分母覆盖全部 offered。成功分位数带成功样本数，拒绝和超时分开计数；所有分位数仅为小规模模型统计。排队、服务占用与到结果终结的时间各有定义，特别说明超时 active 的服务工作可继续到 terminal 之后，不能套用成功请求的加法。

## 资料核查

2026-09-19 实际打开以下原作者或官方项目资料：

| 资料                                                                            | 实际核对范围                                                                                         |
| ------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| [Little 原论文页面](https://pubsonline.informs.org/doi/10.1287/opre.9.3.383)    | 原始摘要中的有限均值、平稳性等条件与平均关系；未声称阅读全文证明                                     |
| [The Tail at Scale 官方论文页](https://research.google/pubs/the-tail-at-scale/) | 作者、出版信息及原始摘要中的大规模依赖、利用率和尾部影响；正文下载链接访问失败，未将全文细节用作论据 |
| [wrk2 README](https://github.com/giltene/wrk2)                                  | 固定目标吞吐、预定发送时刻、慢响应使测量器停止请求的 coordinated omission 机制                       |
| [HdrHistogram README](https://github.com/HdrHistogram/HdrHistogram)             | 整数范围与精度、兼容计数聚合、预期间隔遗漏校正的假设；未引用项目旧硬件性能数字                       |
| [Google SRE Handling Overload](https://sre.google/sre-book/handling-overload/)  | 拒绝成本、重试预算、多层重试放大及资源限制                                                           |
| [gRPC Deadlines](https://grpc.io/docs/guides/deadlines/)                        | 剩余预算传播、跨主机时钟、应用必须自行停止派生工作                                                   |

队列容量、事件顺序、256 条限制及全部示例结果是本章模型选择，不归为这些资料的标准保证。未虚构来源页中没有的实验数据。

## 代码审查

- 单线程离散事件模型，没有真实网络、线程、睡眠或无限外部等待。有限输入、正服务时间，每个请求最多到达、开始、超时和完成各一次；没有自动重试。
- 等待容量只包含尚未开始的请求；独立服务槽使容量零时仍可直接服务。等待数在每轮结束断言不超过容量。
- 同 tick 顺序固定为完成、过期、旧等待者开始、新到达。完成恰好截止算成功；排队请求恰好截止不能开始；同时到达按照输入编号顺序准入。
- active 超时保持占用，工作完成只记录 work_done，不覆盖 timed_out。等待请求超时立即删除，零预算请求不开始执行。
- `optional` 表示无 active 或无下一事件，最大 uint64 tick 可以作为合法完成时间。截止相加和开始后完成相加都通过 checked_add 检查溢出。
- 输入数及等待容量各最多 256，服务时间大于零，到达非递减。nearest-rank 乘法受样本数上限约束，空样本返回 nullopt，百分位非法时拒绝。
- 所有已开始请求最终都有 work_done；输出中的差值在这些先后关系下计算，没有无符号下溢。结果类别计数断言和记录总数相等。
- 每次扫描或过期移除可能处理等待队列，正文给整体 `O(N(Q+1))`、存储 `O(N+Q)`，没有将该模拟器描述为生产热路径实现。

## 独立预期与边界证据

| 测试           | 实际结果                                                                                                                |
| -------------- | ----------------------------------------------------------------------------------------------------------------------- |
| FIFO oracle    | 4 组间隔，每组 16 条；使用 `max(arrival, previous_finish)` 独立递推，64 条开始、完成、结果断言通过                      |
| 开放轨迹       | 到达 0..7，等待容量 2；完成 id 0/1/2/5/6/7，拒绝 3/4，成功延迟 5/5/5/3/3/3，N=6，p50=3，p95=5；最后完成 tick 10         |
| 闭环轨迹       | 到达 0/5/6/7/8/9/10/11；8 条完成，成功延迟 5/1/1/1/1/1/1/1，p50=1，p95=5；最后完成 tick 12                              |
| 截止轨迹       | 5 offered，1 完成、1 拒绝、3 超时；id 0 在 2 超时而工作至 5，id 1 排队至 4 过期，id 2 在 6 完成，id 4 在 6 过期且不启动 |
| 容量与同刻边界 | 容量零空闲可开始、忙时拒绝；容量一同刻第三条拒绝；完成与下一到达同刻可继续服务                                          |
| 恢复           | 突发后有拒绝；tick 20 的新请求没有遗留排队，21 完成                                                                     |
| 输入与算术     | 空输入、空分位数、nearest-rank 手算、零预算、非法顺序/服务/规模/百分位、截止与完成溢出、最大 tick 合法完成均通过        |

开放与闭环到达时刻、成功数量及结束窗口不同，不把二者作为相同负载下的性能对比。模型服务成本由输入指定，闭环生成时按该成本推导零思考时间的下一到达。

## 执行记录

作者在 WSL 的 GCC 13.3.0、C++20、断言启用环境下执行：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  examples/tail-latency-backpressure.cpp \
  -o .artifacts/tail-latency-backpressure-review/demo
.artifacts/tail-latency-backpressure-review/demo

g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  examples/tail-latency-backpressure.cpp \
  -o .artifacts/tail-latency-backpressure-review/san
.artifacts/tail-latency-backpressure-review/san
```

两次均退出 0，输出结尾为 `oracle_cases=64; boundary checks passed`，sanitizer 无诊断。主 agent 另行通读代码、手算关键轨迹并复跑 ASan/UBSan，已反馈通过。上述检查不能证明真实服务的性能、调度或取消协议正确。

`readTopics` 与 `validateTopics` 检查本章零错误，二级标题为 11，题目为 18，各层 6。Prettier 已格式化正文；markdownlint-cli2 按仓库配置最终实际检查 59 个 Markdown 文件，零问题。未修改共享 manifest、roadmap、旧章或其他 agent 文件；最终独立验收和登记由主 agent 完成。
