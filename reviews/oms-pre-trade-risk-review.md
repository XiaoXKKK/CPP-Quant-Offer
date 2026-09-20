# OMS 与交易前风控专题自审交接

- 日期：2026-09-19。
- 类型：作者自审，主 agent 独立验收。
- 范围：`content/topics/trading/oms-pre-trade-risk.md`、`examples/oms-pre-trade-risk.cpp`。
- 结论：虚构单 owner 状态机的数量与风险边界验证通过；没有真实交易、完整 FIX、持久化恢复或监管模型。

## 内容与资料

已应用项目 cpp-quant-writing、写作约定和质量标准。正文 11 节、15 道题，L1/L2/L3 各 5 道，全部 derived，companies 为空。题目围绕工程规则，不提供投资或法律结论。风险额度使用本实例累计买入成交金额加未决预留，与净仓、亏损和资金余额明确区分。

实际打开并核查的一手资料：

| 来源                                                                                                                        | 核对范围                                                                                                             |
| --------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| [FIX Order State Changes PDF](https://www.fixtrading.org/wp-content/uploads/download-manager-files/Order-State-Changes.pdf) | 实际版本 EP258，July 2020；ExecType 与 OrdStatus 区别、Pending Cancel 不表示已取消、数量字段定义、取消与成交交叉场景 |
| [FIX 4.4 ExecutionReport 字典](https://fiximate.fixtrading.org/legacy/en/FIX.4.4/body_5756.html)                            | 官方旧字典地址重定向至 Orchimate 的 FIX Trading 数据；ExecID、订单链、OrigClOrdID、报告用途与当前状态                |

PDF 文本读取成功；后续页面截图调用失败，没有将截图当作已完成的图表核查。没有使用论坛讨论作为技术论据。模型的每订单单调整数 report_id、累计成本字段、暂停策略和额度规则均为本章虚构契约，不从 FIX 借用未提供的保证。

## 风险与状态审查

- 所有方法都要求单 owner 串行调用，包括 read。固定 4 项订单表和 16 项报告历史；无动态订单容器、无线程或网络操作。
- quantity、limit_price 为正；prepare 先检查乘法，再在 limit-spent-reserved 范围内预留并登记。相同订单 ID 不复用，终结槽位不回收；无界运行需求未伪装成该有限模型能力。
- 未终结预留为 `(quantity-filled)*limit_price`，终结预留为零；每次 verify 重算全局成本和预留总和，校验不超过额度。累计成交金额不会因取消或发送前失败而消失。
- 新增成本受新增数量乘限价限制，成交更新将最坏预留转实际成本，价格改善释放差额。候选验证完成后才写回订单和全局金额。
- 逐笔正价格且不超限价属于外部协议假设。代码只有累计数量和成本，聚合检查不能排除高低成交价相互抵消；正文已明确逐笔校验需要逐笔明细。
- fail_before_send 仅允许 prepared，要求调用方确实尚未把字节交给外部发送路径。dispatch 先记 pending_new，外部发送随后进行；已发送不能走无风险的本地失败路径。
- 发送失败或等待超时进入 unknown，保留剩余执行可能性；未知订单不能再 dispatch。普通 Ack 和部分 Fill 仍保留 unknown，权威终结才解除该订单的不确定状态。
- request_cancel 保持预留；取消处理中仍可成交。全部成交优先成为 Filled，后续 cancel_reject 不回退；非终结取消拒绝仍预留未成交部分。
- 教学报告携带权威累计数量与成本，更高的每订单 report_id 覆盖更早成交，所以允许缺号但不允许累计值倒退。真实 FIX ExecID 不具备这个整数排序契约，成交更正和撤销也未实现。
- 身份由固定 session、order_id、report_id 组成；内容相同的历史重复不再计账，同号冲突停止新业务。未缓存旧编号按模型累计覆盖条件忽略，不推广成普通网络去重规则。
- 显式检查五种 Kind，拒绝其他枚举值；数字转枚举没有被当成输入校验。
- halted 阻止 prepare、dispatch、request_cancel，但 on_report 仍可处理经过校验的已有订单报告。冻结取消是教学简化；正文说明生产可设置独立受控安全取消或 kill 路径。
- 历史满后不再应用新报告，不驱逐旧 ID，保留本地账本并要求外部对账。保持 reserved 只证明没有无证据地释放，不证明异常后本地账本与真实成交已经一致。
- 构造 session 非零是断言前提；示例明确测试必须启用断言。无 session 切换、持久化事务、重启恢复或解除 halted API，不能通过新建空对象清掉真实订单责任。

## 独立轨迹与故障

| 场景                 | 断言证据                                                                                                                           |
| -------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| 手算买单             | Q=10、P=10，先预留 100；成交 3 个成本 27 后预留 70，再成交 2 个后成本 45、预留 50                                                  |
| 超时与取消终结       | unknown 保留 50；取消报告累计成交 6、成本 54，先补一个成交再释放其余 4 个数量，最终 spent=54/reserved=0                            |
| 重复与迟到           | 原 Fill 重复、较旧 Ack、较旧 Fill、取消报告重复均不重复计账                                                                        |
| 确定未发与拒绝       | prepared 本地失败释放其预留；零成交权威拒绝同样释放；已有成交成本 54 保持                                                          |
| Fill/Ack/Cancel 竞争 | Fill 先于 Ack；Ack 不覆盖 pending_cancel；Ack 前发取消，全部成交后才收到取消拒绝仍保持 Filled                                      |
| unknown 进展         | Ack 后仍未知，部分成交后仍未知但准确减少预留，全部成交后终结                                                                       |
| 取消拒绝             | 累计成交 1、成本 9 时拒绝取消，状态 live、预留 40                                                                                  |
| 风险和整数边界       | 满额度拒绝新订单；乘法溢出提前拒绝；最大整数额度支持预留与完整成交且不回绕                                                         |
| 固定容量             | 四个本地失败订单仍占槽，第五个拒绝；第十七个新报告触发 report_full 并保留 50 预留                                                  |
| 非法输入             | 超数量、超价格成本、空 trade、未申请取消的 cancel_ack、未知 Kind=99 均返回 invalid，spent=0/reserved=50、halted=true、订单 unknown |
| 身份冲突             | 外来 session 不更新；同号不同累计值触发 conflict，已有 spent=9/reserved=40 保持                                                    |

独立预期使用具体常量，不调用实现生成 oracle。断言不是对真实撮合、成交价格、费用或交易所保证的证明。

## 执行与修订

作者在 WSL GCC 13.3.0 上运行 C++20、严格 warning 和断言启用的正常构建与初版 sanitizer：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  examples/oms-pre-trade-risk.cpp -o .artifacts/oms-pre-trade-risk-review/demo
.artifacts/oms-pre-trade-risk-review/demo

g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  examples/oms-pre-trade-risk.cpp -o .artifacts/oms-pre-trade-risk-review/san
.artifacts/oms-pre-trade-risk-review/san
```

均退出 0、sanitizer 无诊断。主 agent 预审发现初版没有显式检查未知 Kind，作者增加 switch 与 Kind=99 独立故障断言后复跑正常严格构建通过。主 agent 再对最终代码独立执行严格 warning、ASan/UBSan，反馈全部通过；最终代码 hash 为 `52466f46005f44d80fd0f21168d5b8c5f1009f976901dbd1f593f67e54926031`。没有把初版 sanitizer 结果冒称为最终作者复跑。

`readTopics` 与 `validateTopics` 本章检查零错误，11 个二级标题、15 题、各层 5。正文和交接已格式化，最终 Prettier 与 markdownlint 结果在交接消息报告。没有更改公共 manifest、roadmap、旧章节、测试或其他 agent 文件，独立终验登记由主 agent 完成。
