# TCP framing 专题自审交接

- 日期：2026-09-19。
- 类型：作者自审，主 agent 独立验收。
- 范围：`content/topics/network/tcp-framing.md`、`examples/tcp-framing.cpp`。
- 结论：有界解析、分片等价性、EOF、发送进度与半关闭验证完成；不声称测试了真实 TCP 分段或性能。

## 范围与写作

已应用项目 `cpp-quant-writing`、写作约定、内容质量标准和贡献指南。正文有规定的 11 节，18 道题按 L1/L2/L3 各 6 道分布，全部为 `derived`，`companies` 为空。题目分别检查边界定义、状态转换和连接/业务层恢复，没有新增岗位、公司筛选。

协议只定义四字节大端正文长度、允许零长、最大正文 16 字节。明确这是教学选择。正文把应用帧、TCP segment 和系统调用边界分开，也把已发字节、对端 TCP 接收和业务执行确认分开。性能部分只给实验控制与复杂度，没有数字或加速承诺。

## 资料核查

2026-09-19 实际打开并核对了 frontmatter 的全部来源：

| 来源                                                                     | 核对内容                                                        |
| ------------------------------------------------------------------------ | --------------------------------------------------------------- |
| [RFC 9293](https://www.rfc-editor.org/rfc/rfc9293.html)                  | §2.2 字节流；§3.7 分段；§3.6.1 独立方向关闭；PSH 不提供记录边界 |
| [recv(2)](https://man7.org/linux/man-pages/man2/recv.2.html)             | 短读、非阻塞、EINTR、零长读取与流 EOF、MSG_WAITALL 限制         |
| [send(2)](https://man7.org/linux/man-pages/man2/send.2.html)             | 本地发送结果、EINTR、EAGAIN/EWOULDBLOCK、MSG_NOSIGNAL 与 EPIPE  |
| [shutdown(2)](https://man7.org/linux/man-pages/man2/shutdown.2.html)     | SHUT_WR 不同时关闭接收方向                                      |
| [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)           | Linux 下错误后盲目重试 close 的 fd 复用风险                     |
| [socketpair(2)](https://man7.org/linux/man-pages/man2/socketpair.2.html) | 本机相连 socket pair 的创建与类型选项                           |
| [unix(7)](https://man7.org/linux/man-pages/man7/unix.7.html)             | AF_UNIX 的 stream 类型及适用范围                                |
| [byteorder(3)](https://man7.org/linux/man-pages/man3/byteorder.3.html)   | 网络字节序与 htonl/ntohl 约定                                   |

没有把 Linux close 策略写成所有 POSIX 平台通用，也没有从 AF_UNIX 功能测试推导 TCP_NODELAY、Nagle、拥塞控制或真实 TCP 分段效果。

## 状态与边界自审

- 长度逐字节组装到 uint32_t，最多接收四个头部字节，不执行未对齐整数解引用。收齐头部后，先拒绝长度大于 16，再接收正文。
- 正文写入次数受已校验长度限制，数组容量为 16。零长度帧在完整头部到达时直接变为 ready，合法完整帧始终可交付。
- feed 报告实际 consumed。ready 时不消费新字节，调用者保留后缀，取帧后继续。固定单帧容量不足是暂停，非法长度则为 terminal failed，两者不混淆。
- take 返回拥有数组的值，不把 span 或内部引用交给异步下游。frame 取出后内部数据归零，测试比较不会读取未初始化内容。
- finish 的前提是所有 recv 返回字节都已消费。零残留干净结束，半头部和半正文报 truncated，完整待取帧在 EOF 后保留到 take，错误后不继续猜测边界。
- Wire 编码夹具先检查最大正文，再用剩余容量检查追加；64 字节预算拒绝追加时原大小不变。它只用于构造有界测试输入。
- flush 仅按正返回推进 offset；EINTR 不推进，EAGAIN/EWOULDBLOCK 保留后缀并返回。非空发送零进度被归为失败，每次至多 16 次调用，连续中断可返回 retry_later。
- 所有实际 socket 均为非阻塞，读缓冲非零，程序交替推进发送与接收。每段集成循环最多 1024 轮，未完成即错误退出，不阻塞等待大于 socket 缓冲的数据写完。
- fd 接管与移动无抛出、不可复制，先置无效后 close 一次。MSG_NOSIGNAL 保留 EPIPE 返回而抑制 SIGPIPE。示例析构不报告 close 错误，正文注明诊断省略。

## 实际验证

环境为 Windows 宿主的 WSL，GCC `13.3.0`，目标 `x86_64-linux-gnu`。开启 assert，未定义 NDEBUG，未调用全量 C++ 测试脚本。

正常构建与运行：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/tcp-framing.cpp -o .artifacts/tcp-framing-review/demo
.artifacts/tcp-framing-review/demo
```

ASan/UBSan 构建与运行：

```bash
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fno-omit-frame-pointer -fsanitize=address,undefined -fno-pie -no-pie examples/tcp-framing.cpp -o .artifacts/tcp-framing-review/demo-san
.artifacts/tcp-framing-review/demo-san
```

实际执行使用 `/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/` 下的绝对路径。两种构建运行均成功，sanitizer 无报告，输出为：

```text
framing checks passed; split points=32; frames=3; half-close=ok
```

| 测试             | 实际验证内容                                                                   |
| ---------------- | ------------------------------------------------------------------------------ |
| 31 字节三帧流    | 正文长度 3、0、16，包含二进制零字节                                            |
| 32 个切分点      | 在每个位置拆成两个输入，恢复帧与基线一致                                       |
| 逐字节输入       | 头部和正文跨多个 feed 保持正确状态                                             |
| ready 容量       | 后续 feed 消费零字节，取走帧后保留后缀可继续                                   |
| 非法长度         | 17 与 0xffffffff 都在头部完成时拒绝，之后消费零字节                            |
| EOF 前缀         | 第一帧前缀 0 到 7，覆盖干净结束、所有部分头部/正文和完整待取帧                 |
| 编码缓冲预算     | 三个最大帧占 60 字节，追加下一最大帧失败；正文 17 也失败，大小仍为 60          |
| 发送脚本         | EINTR、成功 2 字节、EAGAIN、成功 1 字节、成功 3 字节；偏移和最终六字节完全一致 |
| 连续中断与零进度 | 16 次 EINTR 后返回 retry_later；零进度返回 failed，无无限循环                  |
| 真实 socketpair  | 初始 recv EAGAIN；发送后接收相同三帧；SHUT_WR 导致接收 EOF，反向一个字节仍到达 |
| 真实 EPIPE       | 关闭 peer 后带 MSG_NOSIGNAL 发送，返回 EPIPE，进程继续                         |

实际 socket 每次 send 主动限制最多提交 3 字节，不把这个限制称为内核发生短写。recv 缓冲为 5 字节，没有断言实际读取边界；短写和 EINTR 的确定性覆盖来自脚本注入。没有运行真实 TCP、跨主机网络或性能测试。

## 结构与交接

- 11 节、18 题、各层 6 题，以及 schema、demo 路径、内部链接和 related 的项目校验通过。
- 正文与交接已 Prettier 格式化；最终 markdownlint 检查 47 个 Markdown 文件，0 issues。
- 代码 SHA256：`1730e010c99322b73e056c911f62c4c6d607af83422db388026f5d6905d70e70`。
- 未修改旧章、共享 manifest、roadmap、配置或测试，未提交或创建 PR。
- 全站构建和页面验收由主 agent 汇总；这份交接只报告上述本章检查。
