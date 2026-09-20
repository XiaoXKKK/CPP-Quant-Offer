# 交易网关自审交接

日期：2026-09-19。类型为作者自审，主代理负责独立技术、构建与页面验收。本次只新增 `content/topics/design/trading-gateway.md`、`examples/trading-gateway.cpp` 和本报告，没有修改旧章、共享 manifest、roadmap 或 harness。

## 内容与资料

核对 roadmap G 的交易网关条目，taxonomy 选择 design / System Design。本章再次读取 cpp-quant-writing 的工作流程及 WRITING_STYLE、CONTENT_STANDARD，保留会话、身份、查询前提等限定。正文 11 节、15 道 derived 题，L1/L2/L3 各 5 道，全部有答案及评分点，companies 为空。

6 条 references 均实际打开。RFC 9293、send 和 recv 用于字节流、发送返回值与接收边界；FIX Session Layer June 2020 PDF 实际阅读第 3.1 和 4.9 节关于 retransmission/resend、业务重复处理及会话跨连接的区分；Coinbase Exchange FIX Order Entry 5.0 与 Drop Copy 用于具体接口中的消息、请求、订单与执行报告身份及取消确认边界。

最初的 FIX Session Layer 页面跳到 technical proposal 下载页，在线规范路径返回 404；随后实际打开 83 页的 June 2020 官方 PDF，不把搜索摘要当作正文。Coinbase 简化 order-entry-messages 路径不可访问，改用完整 order-entry-messages5 页面并核对 ClOrdID、OrderStatusRequest、ExecutionReport 和取消提示。未访问任何交易服务端点，未登录或产生真实交易。

文中五字节协议、业务 session=7、quantity=8 拒绝、ID 范围与恢复证据均明确为虚构教学规则，未宣称实现 FIX 或任何场所的通用重试保证。

## 状态与所有权

Gateway、Peer、ScriptTransport 都是单 owner 值类型，无线程、socket、外部回调或真实计时器。Gateway 固定 4 条记录、16 个业务身份、16 个连接 generation；completed 保留消费后结果。队列满不覆盖旧请求，也不消耗被拒 ID；已完成但未消费的结果继续占用队列，显式传播消费者背压。

frame_size=5，offset 只增加脚本实际写入数量，write 返回量不超过提供 span 长度，subspan 的偏移始终在 0..5。ScriptTransport 的内部复制先核对剩余容量，不会写过固定帧尾。量为零只表示脚本无进展，不模拟真实系统调用完整 errno 集。

完整帧后才能对 Peer 应用。Peer 校验合法 session/id/quantity 后访问固定结果数组，同 ID 同内容返回已存结果，内容不同返回 conflict，不增加 applications。applications 表示首次处理请求次数，包含业务拒绝，不表示成交或资产变化。

断线时 offset=0 保持 queued，任何非零进展且未 final 的队头变 unknown。unknown 不再发送且阻塞后续；final 只等待消费者取结果。旧 transport 的 generation 不匹配时不能用于 pump，旧应用确认与查询响应不会修改新连接状态。

connect 只允许同业务会话，并在 generation 到 16 后冻结，避免代次回绕。业务 ID 成功本地入队后在实例内不复用；程序没有持久化身份分配器，因此全部 16 个身份用完后不再支持新身份。对象重建不是恢复协议。

## 查询与回调前提

本模型同一 Peer 在完整帧写入时同步处理，ScriptTransport 关闭即丢弃不完整帧且不再存在后台处理。因此 query_after_close 可以在同一结果历史上提供 absent_fenced。这个前提显著强于真实 TCP close，正文、题目和注释均明确：本地关闭不能证明远端已清空待处理工作，普通 NotFound 也不能证明请求以后不会生效。

resolve 检查回调 generation、连接状态、当前队头 unknown、业务会话、请求 ID、quantity 与合法枚举。generation 必须在查询发起时捕获，不能在处理响应时重新读取当前代次；正文还指出同一连接上的多次查询需额外 query_id/阶段版本，本例串行模型没有覆盖所有查询并发。

主代理预审指出未知 Verdict 和 EvidenceKind 的防御不足。修订后 acknowledge/resolve 只允许 accepted 或 rejected；未知或 conflict 使网关冻结。EvidenceKind 明确只接受 known/absent_unfenced/absent_fenced，其余冻结。非法状态不会 finalize，也不会产生可消费结果。

真实网络适配器仍需完成帧解析、认证、协议消息顺序与失败分类。本例所有 Receipt/Evidence 来自测试值，白名单检查不能替代真实协议完整校验。XOR 校验只验证脚本组帧，不提供安全认证。

## 独立期望与测试

本章使用按失效位置编写的独立预期断言，没有第二份相同 Gateway 充当 oracle。对端首次应用次数、每段 offset、具体 Receipt、queue size 与合法 phase 分别核对。

- 16 种五字节帧分段组合，段前注入零进展；完整帧前应用次数为零，写满后仍待应用确认。
- 已写 0..5 字节共 6 个断线位置。零进展可直接重连发送；非零进展保留 unknown，禁止自动重发。
- 部分帧场景下普通 absent_unfenced 不推进，关闭旧脚本后的 absent_fenced 才重新发送完整原帧。
- 完整帧确认丢失时查询已存结果，不重新处理；旧代迟到确认被忽略。
- 每条断线路径都验证第一个请求只处理一次，取走结果之后才推进第二条请求。
- ID 0/17、quantity 0/9，4 槽背压，发送前错误确认，零字节 fatal，quantity=8 的业务拒绝。
- 未消费 final 保持队列满，消费后允许原先被 full 拒绝的 ID 入队。
- 重复相同结果、同 ID 不同 payload、矛盾终态与业务会话变化。
- generation 1..16 成功，下一次连接冻结，不回绕。
- 非法 ack Verdict、非法 known Verdict、非法 EvidenceKind 均冻结且不产出结果；旧 generation 的查询回调保持 unknown，不冻结、不推进。

脚本强假设使这些有限案例可确定执行；它们不覆盖远端异步排队、普通查询的历史缺失、网络系统调用、TLS、日志崩溃或真实重连风暴。没有测试框架睡眠、无限重试或外部流量。

## 实际编译与运行

平台为 x86-64 WSL2，GCC `13.3.0`。通过 PowerShell 调用 WSL bash，编译产物在独有临时目录。实际执行如下：

```bash
mkdir -p /tmp/cpp-quant-gateway-review
src=/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/trading-gateway.cpp
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  "$src" -o /tmp/cpp-quant-gateway-review/demo
/tmp/cpp-quant-gateway-review/demo
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  "$src" -o /tmp/cpp-quant-gateway-review/demo-sanitize
/tmp/cpp-quant-gateway-review/demo-sanitize
```

初版普通与 ASan/UBSan 均通过；补齐枚举和旧查询回调断言后，对最终代码再次严格普通及 sanitizer 编译执行，均通过、无 sanitizer 报告。输出：

```text
16 frame partitions, 6 disconnect positions, session/ID/backpressure: OK
Script transport only; no network or real orders.
```

最终代码 SHA-256 为 `d56ab5d45dc90e2727fe91090a9fe930e675084f546f29c624228ffbbad64069`。主代理反馈对该代码独立严格 warning 与 ASan/UBSan 复跑通过，并完成修订路径审读；此处将它标为主代理反馈，不冒充作者测试。

## 文字与题目复核

逐题检查发送返回值、TCP 确认、业务接受、成交等边界，避免将这些事件合并成“成功”。L3 覆盖业务会话变化、FIX 两层恢复、持久化、延迟边界与模型缺失；不会把同一状态定义重复十五遍。

性能小节限定单次有限操作与推广后的 O(N) 队列移动，未发布任何延迟或吞吐数值。正确性输出中的应用次数不解释为实际恰好一次执行保证。内部关联指向 TCP framing、connection lifecycle、matching engine 与 SPSC，未加入未完成章节。

按写作 skill 通读，保留未知结果与强缺失证据的必要条件，减少空泛强调和重复收尾。格式、schema 与 lint 在冻结前检查，正文和报告最终 hash 通过交付消息提供；主代理维护全量与页面记录。
