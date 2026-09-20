# UDP 组播与序列号自审交接

日期：2026-09-19。本报告为作者自审，主代理负责独立技术与页面验收。本次只新增 `content/topics/network/udp-multicast-sequencing.md`、`examples/udp-multicast-sequencing.cpp` 和本文件，未修改共享 manifest、roadmap 或测试。

## 范围与内容契约

开始本章前重新读取了 cpp-quant-writing skill、WRITING_STYLE 和 CONTENT_STANDARD，按成稿要求写入中文内容。正文 11 节，18 道 derived 题，L1/L2/L3 各 6 道，所有题目均有答案与评分点、companies 为空。

示例为 C++20 / Linux，固定 12 字节自定义协议，一报文一事件，不使用任何交易所线格式。组播配置在正文说明，本机 demo 只用 AF_UNIX SOCK_DGRAM socketpair，未产生 IP 或外部流量，未加入实际组播组。

## 来源核对

18 条 references 均于 2026-09-19 用浏览工具实际打开。RFC 768、8085 支持 UDP 交付边界、分片与应用使用条件；RFC 1112 和 3678 用于组与接口、发送接口和源过滤。RFC 1982 的半空间序列比较明确标为原本面向 DNS serial 的数学规则，没有将其宣称为所有 UDP 协议的通用格式。

Linux man-pages 的 udp、recvmsg、socketpair、unix、poll、send、close 用于区分数据报接收与本机测试、输入/输出 MSG_TRUNC、零长度报文、非阻塞错误、有限等待和 fd 清理。

本次 man7 的 ip(7) 已把部分选项拆到独立页面，因此实际继续打开 IP_ADD_MEMBERSHIP、ip_mreqn、IP_MULTICAST_IF、IP_PKTINFO 和 IP_MULTICAST_ALL 的页面，并核对接收接口、发送接口和交付范围。socket(7) 支持 SO_RXQ_OVFL 的 socket 累计丢弃计数范围；正文未将它视为整条网络的丢包统计。

## 解析与状态机自审

decode 先检查长度恰好为 12，再读取各偏移；短路求值确保短报文不会先访问 magic。session 逐字节移位组装，sequence/delta 显式按大端解码，没有不对齐结构体转换。delta 上限 1000，业务 value 加法前检查 uint64_t 剩余容量。

序列差计算在 uint32_t 中进行，最大中间值不超过 131071，不依赖有符号溢出。期望相等才应用，未来值使状态无效而不推进 next，半空间距离 32768 单独作为歧义。大于半空间归旧范围，前提是协议限制旧包寿命和真实前进跨度；跨半圈或整圈无法单凭 16 位数值识别。主代理预审提出这点，正文、q07 和练习均明确保留前提。

初始化与恢复必须显式 install_baseline。该函数接受调用者已确认的 session、next 与完整 snapshot value；没有根据普通数据报自动切换 session，也没有从第一包猜测初始状态。缺口后即便缺失报文到达仍保持冻结，本章没有实现乱序缓存、补发服务或快照与实时流衔接。

ingest 在 channel 分流前检查截断/格式，错误会冻结这个 receiver，正文明确为单流输入的保守策略，多路复用场景需要先用目的组/源/接口等信息分发。异 session 报文仅忽略，不自动使旧会话状态获得新鲜度保证；超时与会话授权在外层。旧范围报文未做历史 payload 一致性比对，不能用于证明双线同号内容一致。

## I/O 与异常路径自审

socketpair 使用 SOCK_NONBLOCK 和 SOCK_CLOEXEC。创建成功后两个 fd 的接管不抛出；后续任何异常由 UniqueFd 析构关闭。没有移动或复制 fd 所有者，未出现双重拥有。

send/recvmsg 都为非阻塞调用，send 检查返回错误和完整长度。接收前只执行一次 1000ms poll，检查超时及 POLLERR/POLLHUP/POLLNVAL，再做非阻塞 recvmsg。示例对 EINTR 也直接抛出并清理，无无限重试。调度延迟可能超过 poll 参数，正文没有给出硬实时墙钟上界。

recvmsg 不把 MSG_TRUNC 作为输入参数，返回 copied 因而按不超过用户缓冲校验；输出位独立保留。第一报文截断后，下一次读取确认为第二份完整报文。零长度报文确实被消费并判为应用格式错误，没有当成流式 EOF。

析构中的 close 不重试 EINTR，限定为 Linux 行为。异常清理测试抛自定义标记，避免把 socketpair 本身的 system_error 误当成预期注入。两个 fd 的 fcntl 检查发生在作用域销毁之后、没有其他打开 fd 的操作之间，均得到 EBADF。

## 实际测试

环境：WSL Ubuntu，GCC 13.3.0。仅编译本章代码到独立 `/tmp/cpp-quant-udp-review/`，没有运行与其他代理争用目录的全量脚本。

严格普通构建和运行通过，退出码 0：

```bash
mkdir -p /tmp/cpp-quant-udp-review
src=/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/udp-multicast-sequencing.cpp
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror "$src" -o /tmp/cpp-quant-udp-review/demo
/tmp/cpp-quant-udp-review/demo
```

ASan/UBSan 构建和运行通过，退出码 0，无报告：

```bash
mkdir -p /tmp/cpp-quant-udp-review
src=/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/udp-multicast-sequencing.cpp
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie "$src" -o /tmp/cpp-quant-udp-review/demo-sanitize
/tmp/cpp-quant-udp-review/demo-sanitize
```

两次均输出：

```text
Sequence boundaries, recovery gate, local datagram truncation and fd cleanup: OK
```

状态验证包括：缺基线、正常递增、65535→0、重复旧报文、gap 冻结、冻结时迟到或更高报文不应用、显式快照恢复、不同 channel/session、会话切换后的旧包、半空间两侧与正中间、跨回绕缺口、delta 为 0、uint64_t 溢出。

解析验证包括长度 0–11、12、13，错误 magic/version，最大 delta 与超限 delta；三个连续序号的全部六种排列检查缺口前完整前缀。网络验证包括输出 MSG_TRUNC、余部丢弃、后一报文内容、空报文、队列空 EAGAIN/EWOULDBLOCK。异常注入后两个 fd 均关闭。

未人为注入 poll 超时、EINTR、内核内存不足或 close 失败；这些分支做了代码审查，不冒充实际覆盖。没有实际 UDP/组播链路、网卡统计、补发服务器或快照交接验证，没有性能基准。

主代理另行告知已独立复跑严格编译、ASan/UBSan，且包含本章的当次全量普通 C++ harness 22 例通过；这不作为作者自己运行全量 harness 的记录。

## 内容检查与留给主验收的边界

首次 schema 检查指出 standard 与 platform 误写为带平台说明的字符串及大写 Linux，已修正为合法枚举 C++20 / linux，随后本章 readTopics/validateTopics 检查通过。正文明确 Linux 限制，没有修改 schema 来迁就内容。

逐题复核覆盖数据报边界、序列域、半空间前提、基线、组播接口、快照衔接、冗余线路、跨层丢弃证据、缓冲过载与下游状态失效。正文与参考资料不声称某交易所采用本例格式。

最终格式检查随冻结消息交付，未运行全站构建或浏览器桌面/移动端渲染，由主代理统一验收。18 条参考来源、11 节、18 题分层及内部链接已通过定向内容校验。
