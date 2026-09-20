# 连接生命周期专题自审交接

- 日期：2026-09-19。
- 类型：作者自审，主 agent 独立验收。
- 范围：`content/topics/network/connection-lifecycle.md`、`examples/connection-lifecycle.cpp`。
- 结论：应用状态机与实际非阻塞连接验证通过；模型动作与真实 syscall 证据分别记录。

## 内容与写作

已应用项目 `cpp-quant-writing`、写作约定、质量标准和贡献指南。正文有 11 节、18 道题，L1/L2/L3 各 6 道，全部 `derived`，`companies` 为空。主题集中于拥有者、阶段、截止和迟到事件，没有重写 TCP framing 的解析器。

技术叙述区分内核 TCP 状态、应用连接阶段、对象内存寿命与业务结果。明确 output pending 归零只是本地发送进度，超时不等于远端未执行。去掉无条件时限和性能承诺，保留模型范围及真实操作条件。

## 资料核查

2026-09-19 实际打开并核对 frontmatter 中的全部来源：

| 来源                                                                     | 核对结论                                              |
| ------------------------------------------------------------------------ | ----------------------------------------------------- |
| [connect(2)](https://man7.org/linux/man-pages/man2/connect.2.html)       | EINPROGRESS、完成就绪后读 SO_ERROR、失败后新建 socket |
| [socket(7)](https://man7.org/linux/man-pages/man7/socket.7.html)         | SO_ERROR 读取并清除挂起错误                           |
| [getsockopt(2)](https://man7.org/linux/man-pages/man2/getsockopt.2.html) | 调用返回状态与输出参数、选项长度分开                  |
| [poll(2)](https://man7.org/linux/man-pages/man2/poll.2.html)             | 就绪与错误、POLLHUP 残留数据、等待粒度及调度影响      |
| [epoll(7)](https://man7.org/linux/man-pages/man7/epoll.7.html)           | 事件缓存中同批关闭导致的陈旧条目与清理问题            |
| [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)           | Linux fd 释放、复用与不可盲目重试                     |
| [shutdown(2)](https://man7.org/linux/man-pages/man2/shutdown.2.html)     | SHUT_WR 与双向关闭区别                                |
| [recv(2)](https://man7.org/linux/man-pages/man2/recv.2.html)             | 非零长度流读取 EOF 的适用条件                         |
| [RFC 9293](https://www.rfc-editor.org/rfc/rfc9293.html)                  | TCP 两个方向的独立关闭及协议范围                      |

代次方案和单 owner 是本章设计，不归为 Linux 自动提供的机制。真实测试没有声明覆盖操作系统全部关闭竞态。

## 模型自审

- `model::Owner` 单线程串行派发，Connection 状态不会被工作线程直接访问。接口只返回 Snapshot 值，不输出 Connection 裸指针。
- 成功连接结果由单独事件表示；拒绝建立前的应用输出。实际就绪后 SO_ERROR 的读取由后述系统调用测试核查。
- pending 上限 32，增加前检查剩余容量，sent 事件拒绝超过当前数量；纯计数不访问虚构缓冲区。
- 读 EOF 可先于写半关闭，本例协议允许最后响应。draining 后拒绝新输出，pending 归零才调用模型 shutdown；成功后等待另一方向 EOF。
- 两个方向按两种顺序结束都有断言。重复关闭请求不会重复 shutdown，也不会把截止向后推。
- 模型 close 先标记 closed，再将 fd 置为无效并记录一次。apply 返回后 optional 才 reset，析构不会二次记录 close。
- 假 fd 42 的复用完全在 model 内，不向 Linux 使用这一编号。迟到事件以旧 Handle 派发，不能修改新对象；错误槽位也被拒绝。
- uint64_t 最大代次的对象关闭后退休槽位，新建失败，旧 Handle 也失败；没有隐藏代次回绕。
- 连接与关闭截止由绝对 tick 驱动。重复请求不能延长，截止到达关闭；模型不声称在没有调度 tick 时自动保证清理时刻。
- 结束原因和未提交字节记录在 owner。十字节输出已推进三字节后超时，剩余七字节被记录为丢弃；不据此判定业务是否执行。

## 实际 syscall 范围

真实部分单线程创建 loopback listener，绑定 `127.0.0.1:0`，listen 后查询临时端口；创建非阻塞 client 发起 connect。立即成功可继续，EINPROGRESS 则 poll 等待完成提示，其他错误抛出并由 RAII 清理。

完成时先检查 getsockopt 调用，再检查 SO_ERROR 值及返回长度。poll 检查 POLLNVAL，EINTR 保留绝对截止并重新计算等待。steady_clock 两秒只限制用户态等待预算，系统调度或定时粒度可能让返回更晚，不作硬实时声明。

Fd 不可复制，构造成功后独占资源，异常路径析构一次 close，不跨线程关闭。没有实际 accept 后业务、数据发送、真实排空、真实 fd 编号复用或 epoll 批内销毁测试。上述关闭、复用及错误场景来自模型，正文已明确。

## 实际构建与运行

环境：Windows 宿主上的 WSL，GCC `13.3.0`，目标 `x86_64-linux-gnu`，断言启用，未定义 NDEBUG。未运行全量 C++ 测试脚本。

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/connection-lifecycle.cpp -o .artifacts/connection-lifecycle-review/demo
.artifacts/connection-lifecycle-review/demo
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fno-omit-frame-pointer -fsanitize=address,undefined -fno-pie -no-pie examples/connection-lifecycle.cpp -o .artifacts/connection-lifecycle-review/demo-san
.artifacts/connection-lifecycle-review/demo-san
```

实际调用使用 `/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/` 绝对路径，两个构建及运行成功，ASan/UBSan 无报告，输出均为：

```text
lifecycle checks passed; stale handles rejected; generation retired; SO_ERROR=0
```

模型覆盖正常排空、输出上限、部分发送、超额进度拒绝、读先关和写先关、旧事件、错误槽位、重复关闭、建立失败、建立超时、排空超时、写半关闭失败和代次耗尽。时间边界直接注入 49、50 等绝对 tick，不依赖 sleep 或特定线程调度。真实连接成功结果为本次 loopback 观察，不推广为所有系统环境都可无条件建立连接。

## 结构与交接

- 项目内容校验通过：11 节、18 题、每层 6 题，schema、demo、内部链接及 related 无本章错误。
- 正文与交接完成后统一执行 Prettier 与 markdownlint，最终结果另附交接消息。
- 未修改旧章、共享 manifest、roadmap、配置或测试，未提交或创建 PR。
- 全站构建和页面验收由主 agent 汇总；本章没有性能数据，也没有把模型当作完整 reactor 实现。
