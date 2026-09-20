# io_uring 专题自审

日期：2026-09-19。类型：作者自审。主 agent 已通读正文、15 题和示例，并独立复跑严格编译及 ASan/UBSan。本文件记录来源与验证范围，不替代主 agent 的发布 hash。

## 交付

- `content/topics/network/io-uring.md`：正文 11 节，15 题，L1/L2/L3 各 5 题。
- `examples/io-uring.cpp`：Linux UAPI 单文件 NOP 示例，不依赖 liburing 链接。
- 本文件：技术自审、资料对应及实际验证记录。

已应用项目 cpp-quant-writing skill，遵循 WRITING_STYLE、CONTENT_STANDARD 和 taxonomy。正文按请求发布、完成消费、资源回收组织，没有添加公司题目来源或性能宣传。仅修改本章三个文件，没有改共享 manifest、roadmap、skill、测试或已验收章节，没有提交 commit。

## 来源核查

正文 15 条 references 均已实际打开。UAPI 采用 Linux v6.8 固定版本，liburing 资料采用仓库 liburing-2.8 标签，避免把新功能混入示例。GCC 与 Linux man-pages、内核 sysctl 文档使用访问当日页面；本机编译器、头文件包和运行内核另行记录。

| 核查内容                                     | 资料                                                                                                                                                                                                               |
| -------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| SQE/CQE、offset、feature、opcode 定义        | [Linux v6.8 UAPI](https://raw.githubusercontent.com/torvalds/linux/v6.8/include/uapi/linux/io_uring.h)                                                                                                             |
| SQ/CQ 所有权、发布消费及完成关联             | [io_uring overview](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring.7)                                                                                                                  |
| SINGLE_MMAP、NODROP、SUBMIT_STABLE 的边界    | [io_uring_setup](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_setup.2)                                                                                                               |
| enter 参数、提交结果与 CQE 错误              | [io_uring_enter](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_enter.2)                                                                                                               |
| 实现采用的 acquire/release 原语              | [GCC atomic builtins](https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html)、[liburing barrier helpers](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/src/include/liburing/barrier.h) |
| 提交与实际完成不同阶段                       | [io_uring_submit](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_submit.3)                                                                                                             |
| 数据 buffer 寿命及读取结果                   | [prep_read](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_prep_read.3)                                                                                                                |
| 取消与原请求分别完成，EALREADY 含义          | [prep_cancel](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_prep_cancel.3)                                                                                                            |
| 读取完成后归还 CQ 槽位                       | [cqe_seen](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_cqe_seen.3)                                                                                                                  |
| opcode 能力探测与 NOP 范围                   | [get_probe](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_get_probe.3)、[prep_nop](https://raw.githubusercontent.com/axboe/liburing/liburing-2.8/man/io_uring_prep_nop.3)             |
| libc syscall 包装的错误约定                  | [syscall(2)](https://man7.org/linux/man-pages/man2/syscall.2.html)                                                                                                                                                 |
| fd 编号、底层文件引用及 Linux close 错误处理 | [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)                                                                                                                                                     |
| 部署禁用及权限条件                           | [io_uring_disabled](https://docs.kernel.org/admin-guide/sysctl/kernel.html#io-uring-disabled)                                                                                                                      |

## 技术自审

1. 开篇列出 C++20、Linux v6.8 UAPI 与 liburing 2.8 的解释范围。没有声称 GCC builtin 对内核共享映射的同步是纯 ISO C++ 保证，也没有泛化为 Windows 接口。
2. SQE 数组、SQ 索引数组与 CQE 数组分别解释。示例只发布一项 SQE，release 更新 SQ tail 前完成 SQE 和索引写入；消费端 acquire 读取 CQ tail，先复制 CQE，再 release 归还 CQ head。
3. ring 容量和 mask 在运行时检查。mmap 长度由内核返回的 offset 和容量推导，乘法前检查有界长度；各字段范围和对齐检查通过后访问，没有硬编码内核布局地址。
4. SINGLE_MMAP 使用 SQ/CQ 所需长度的最大值，并只保存一项映射供清理；SQE 映射独立。非 SINGLE_MMAP 分支分别保存 SQ/CQ 映射，不重复 munmap。
5. SQ 发布仅发生一次。重试 enter 时根据内核 SQ head 计算剩余提交数，不重写 SQE、不重复推进 tail。示例为单线程访问一个 ring，没有声称原子 tail 自动支持多个提交者。
6. 三类结果分开：本例 libc syscall 返回 -1 并设置 errno；库接口按各自约定返回；异步请求错误在 CQE.res 中以负 errno 表达。NOP 负 res 明确失败，没有归入环境 SKIP。
7. user_data 为独立常量标识，完成数量、标识、flags 和 NOP 结果均检查。示例不靠“第一个提交对应第一个完成”的一般假设识别请求。
8. setup 的 ENOSYS、EPERM、EACCES、EOPNOTSUPP 输出带错误码的 SKIP；其他错误报告失败。内核版本字符串不作为功能开关，实际 setup 返回 feature，并执行本例真正使用的 NOP。没有声称探测了所有 opcode 或所有设备路径。
9. 最多 100000 次轮询并检查两秒 deadline。enter 使用 min_complete=0、flags=0，不请求 GETEVENTS 等待；该用户态 watchdog 不限制系统调用、调度或 close 的实际耗时，也不能证明任意异步 I/O 已停止访问资源。
10. 资源对象析构不抛出，记录清理错误并继续其他清理。失败路径可以安全回收本例 ring，因为唯一的操作是没有外部 buffer 或应用 fd 的 NOP；正文明确禁止将此推导为任意 I/O 超时后可释放 buffer。
11. SQE 消费与数据 buffer 使用结束分开。SUBMIT_STABLE 只解释提交辅助数据的相应规则，没有用它缩短实际读写 buffer 的寿命。fd 保留到原操作完成作为简单保守协议，未声称内核取得文件引用后任何 close 都必然破坏已进行的 I/O。
12. 原请求与取消请求分别维护 user_data、CQE 和资源状态。根据主 agent 意见重新打开 prep_cancel 文档，已将 EALREADY 从“无法立即取消”改为“执行已推进到无法取消的阶段”，并说明仍须等原操作最终 CQE；它可能正常或因取消中断完成。成功取消时按该版本文档原完成已发布，应用仍需消费并结清原请求。
13. NODROP 没有写成绝不丢完成。正文保留严重内存不足等失败边界，区分 CQ 槽位、内部 overflow 暂存与 cq_overflow 计数，要求背压与及时消费。本例只读单项无积压路径，不能验证溢出机制。
14. epoll 就绪与普通 I/O 完成作范围明确的比较，保留 io_uring poll 类操作的区别。短读写、业务分帧、对端业务确认和本地 I/O 完成分别处理。
15. 15 题知识点区分明确，各层 5 题，均为 derived 且 companies 为空。L3 覆盖完成积压、关闭、并发使用 ring、轮询成本及迁移接口的工程取舍。没有用 NOP 给出吞吐、延迟或零拷贝结论。

## 实际验证

环境：

```text
Linux 6.18.33.2-microsoft-standard-WSL2 x86_64
g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
linux-libc-dev:amd64 6.8.0-139.139
/proc/sys/kernel/io_uring_disabled = 0
```

作者常规编译参数为 `-std=c++20 -O2 -pthread -Wall -Wextra -Wpedantic -Werror`。ASan/UBSan 使用 `-std=c++20 -O1 -g -pthread -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie`。两种构建均运行成功；主 agent 对补入显式 `<string>` 后的版本独立严格编译并复跑 ASan/UBSan，结果一致。没有 sanitizer 报告。

实际输出：

```text
sq_entries=2 cq_entries=4 single_mmap=1 nodrop=1
NOP completed: user_data matched, res=0, cq_overflow=0
ring resource cleanup passed
```

这些结果只证明本机本次完成了单项 NOP 控制路径及清理。没有做真实网络或存储 I/O、取消、CQ 压力、multishot、注册资源或跨机器性能实验。没有故障注入以触发 setup 受限、非 SINGLE_MMAP、mmap 失败、超时和清理失败；这些分支经过源码自审，不能标为实际覆盖。ASan/UBSan 也不能证明所有内核共享内存顺序正确。

正文与本报告经 Prettier 和 Markdownlint 检查，内容 schema、11 节结构、题目分层、代码 include 与 related 链接单独核对；GFM 表格逐行列数检查避免管道字符意外拆列。最终检查结果与 hash 由作者交付消息记录，发布验收由主 agent 完成。
