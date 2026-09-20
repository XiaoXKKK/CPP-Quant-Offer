# 快照与增量恢复专题自审交接

- 日期：2026-09-19。
- 类型：作者自审，主 agent 独立验收。
- 范围：`content/topics/trading/snapshot-recovery.md`、`examples/snapshot-recovery.cpp`。
- 归属：roadmap F · 交易链路的 snapshot recovery；taxonomy trading，slug 为 snapshot-recovery。
- 结论：有限恢复模型与故障测试通过；没有真实交易所、网络或完整 live feed 集成。

## 内容和来源

已应用 `cpp-quant-writing`、写作约定、内容质量标准和贡献指南。正文 11 节，18 题，L1/L2/L3 各 6，全部 derived，companies 为空。正文聚焦水位、身份、连续性和发布，不复制 Order Book 的订单容器实现。

2026-09-19 实际打开并阅读：

| 来源                                                                                                                                      | 核对内容                                                                           |
| ----------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| [Coinbase Exchange Full Channel](https://docs.cdp.coinbase.com/exchange/websocket-feed/channels)                                          | 先订阅与缓存，再请求快照，剔除不大于快照序号的消息后重放；不同消息类型不一定改变簿 |
| [Binance 官方 WebSocket Streams 文档源文件](https://raw.githubusercontent.com/binance/binance-spot-api-docs/master/web-socket-streams.md) | U/u 区间、快照水位衔接、缺口重启、数量设置及零量移除、初始快照深度有限             |
| [C++20 N4861 atomic shared_ptr](https://timsong-cpp.github.io/cppwp/n4861/util.smartptr.atomic.shared)                                    | 原子 store/load、适用内存序、构造非原子、lock-free 非通用保证                      |

Binance 官网文档页重定向到新版目录，随后打开其官方 GitHub 原始文档读取完整恢复步骤。正文没有将 U/u 区间强行改写为单事件相等判断。示例的单 session、连续单号 ADD、complete 输入、容量 8、cut 和 attempt 都明确属于教学协议设计。

## 模型与发布审查

- owner 串行调用 begin、buffer、commit，缓存不对并发写者开放。read 使用原子 shared_ptr，Published 通过 const 保持发布后不可变。
- begin 在增加 attempt 和切换轮次前准备 invalid 发布对象；分配完成后清缓存并发布。若准备失败，本轮未启动，调用方仍需对原始 gap 执行业务门禁，不能继续假定数据有效。
- 当前发布版本在恢复期间保留旧 payload 并标 invalid。失败提交不修改发布句柄，也不改变旧不可变对象。读者之前持有的 valid 句柄不会被远程撤销，正文说明业务需要当前有效性、代次或门禁。
- session 来自受信任的外部决定，不能被任意新包驱动切换。Token 包含本地 attempt，随异步上下文携带；重新读取当前 Token 再贴到旧响应上会破坏保护。
- 缓存最多 8 条不同序号；相同载荷重复不增加数量，同号冲突、非法事件或第九条使本轮 poisoned。毒化后即使带来新快照也不能发布，必须 begin 新轮。
- complete 只是可信组装层的输入条件，不是快照真实性、分片完整性或校验和验证实现。
- 重放先复制快照和缓存，在局部候选中跳过已覆盖序号、验证连续性并检查加法。失败候选不会泄漏到发布视图；重试时从原快照重新计算，不叠加之前的部分工作。
- cut 不早于快照水位或同 session 的旧发布水位；缓存有 cut 之后的事件时拒绝提交。成功必须恰好到达 cut，不能把最高收到号当完整性证明。
- watermark 与 cut 等于最大值时拒绝，最大值减一可发布；序号递增路径因此不会溢出。quantity 加法先检查最大值减当前量，attempt 耗尽抛出而不回绕。
- 最终 make_shared 在 release store 前完成；分配抛出保持已有发布句柄。这是代码路径推理，没有注入 OOM 测试。
- 成功只发布截至 cut 的完整版本，随后 buffer 返回 inactive。真实 live 流的 owner 交接、next 更新与未来事件保存尚未实现，正文明确列为集成责任。
- shared_ptr 保证存活不代表 lock-free，旧读者可保留多个历史版本；正文不把固定缓存大小误作进程总内存上限。

## 独立期望和测试

| 场景       | 实际验证                                                                                                |
| ---------- | ------------------------------------------------------------------------------------------------------- |
| 独立手算   | W=10，快照 10/20/30/40；11 给第一项加 2、12 给第二项加 3、13 给第一项加 5；期望常量 17/23/30/40         |
| 全排列     | 11/12/13 全部 6 种排列，加覆盖序号 10 和完全重复 12，全部得到期望常量                                   |
| 保留旧读者 | begin 后当前视图 invalid，旧 shared_ptr 仍保持旧 valid 版本；恢复后旧 payload 仍为 5/6/7/8              |
| 缺口恢复   | 11/13 提交失败且句柄不变；补 12 后正确恢复                                                              |
| cut 范围   | cut=12 时仍有 13，不允许清掉未来事件；cut=14 无 14 时返回 gap；两次均不发布                             |
| 身份       | 同 session 旧 attempt、旧 session 的增量及快照均返回 foreign；受信任 begin 新 session 后可从水位 0 开始 |
| 毒化与重开 | 容量 8 的第九条返回 buffer_full、后续 commit 返回 poisoned；新轮快照成功。重复冲突同样阻止发布          |
| 数量和序列 | 最大数量加一失败、最大序号失败、最大减一水位成功                                                        |
| 身份耗尽   | attempt 最大值时 begin 抛出且发布句柄不变；零 session 抛出                                              |
| 输入       | incomplete 快照、非法零事件序号、越界 instrument 有失败断言                                             |

没有用恢复实现本身来计算期望数组。模型只提供四计数器的确定性恢复证据，不证明实际行情消息解码、订单簿语义或跨频道一致性。

## 执行记录

WSL GCC 13.3.0、C++20、断言启用：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  examples/snapshot-recovery.cpp -o .artifacts/snapshot-recovery-review/demo
.artifacts/snapshot-recovery-review/demo

g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  examples/snapshot-recovery.cpp -o .artifacts/snapshot-recovery-review/san
.artifacts/snapshot-recovery-review/san
```

正常与 sanitizer 构建均退出 0，无 sanitizer 诊断；输出依次确认六排列、缺口及身份、容量与溢出测试通过，末行声明虚构协议。主 agent 另外通读代码并复跑严格 warning 和 ASan/UBSan，已反馈通过。没有跨线程压力测试或 TSan，不将这些有限测试写成并发正确性证明。

初稿使用 std::sort 对固定短数组的有效前缀排序，GCC 13.3 优化内联产生 array-bounds 警告，严格构建因此失败。最终改为明确有界的插入排序，未屏蔽 warning，正文按最终 `O(B²)` 算法描述。

`readTopics` / `validateTopics` 本章检查零错误：11 个二级标题、18 题、每层 6 题。正文及本交接使用 Prettier；最终 markdownlint 和格式结果在交接消息中报告。未改其他章、共享 manifest、roadmap、配置或测试文件，等待主 agent 独立终验登记。
