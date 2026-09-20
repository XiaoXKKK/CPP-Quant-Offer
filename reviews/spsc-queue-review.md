# SPSC 队列专题自审交接

- 日期：2026-09-19。
- 类型：作者自审，另由主 agent 独立验收。
- 范围：`content/topics/concurrency/spsc-queue.md`、`examples/spsc-queue.cpp`。
- 结论：边界、回绕、非平凡负载寿命、有限条数传输和两条同步链已复核；TSan 启动失败，未取得竞争检查结果。

## 内容范围与写作

应用了项目 `cpp-quant-writing`、写作约定、内容质量标准与贡献指南。11 节顺序符合契约，18 道题按 L1/L2/L3 各 6 道分布，全部 `derived`，`companies` 为空。未新增岗位、公司界面或无依据的面试归属。

本章在 memory-model 基础篇之外增加非默认构造、不可复制负载的 RAII 转移、optional 槽位寿命、满时保留消息、确定性回绕、静止未排空清理及停止协议。示例没有在线 close API；正文另外给出生产者最后发布后关闭、消费者确认关闭后再次检查并排空的条件。保持语言自然，去掉未测得的低延迟收益和无条件进展承诺。

## 来源核查

2026-09-19 实际打开并阅读以下来源，均已列入 frontmatter：

| 来源                                                                                                           | 对应审查点                                      |
| -------------------------------------------------------------------------------------------------------------- | ----------------------------------------------- |
| [N4861 atomics.order](https://timsong-cpp.github.io/cppwp/n4861/atomics.order)                                 | 发布和复用方向的 release/acquire                |
| [N4861 intro.races](https://timsong-cpp.github.io/cppwp/n4861/intro.races)                                     | happens-before、原子修改顺序、读读与写读一致性  |
| [N4861 basic.life](https://timsong-cpp.github.io/cppwp/n4861/basic.life)                                       | 槽内对象构造、销毁和跨线程存储复用              |
| [N4861 optional.ctor](https://timsong-cpp.github.io/cppwp/n4861/optional.ctor)                                 | 返回 optional 的移动构造与约束                  |
| [N4861 optional.assign](https://timsong-cpp.github.io/cppwp/n4861/optional.assign)                             | emplace 构造与异常语义                          |
| [N4861 optional.mod](https://timsong-cpp.github.io/cppwp/n4861/optional.mod)                                   | reset 销毁包含值，返回无值状态                  |
| [N4861 thread.thread.member](https://timsong-cpp.github.io/cppwp/n4861/thread.thread.member)                   | 成功 join 等待并与线程结束同步                  |
| [N4861 atomics.lockfree](https://timsong-cpp.github.io/cppwp/n4861/atomics.lockfree)                           | lock-free 为实现性质，非 lock-free 原子可能阻塞 |
| [GCC 13.3 instrumentation options](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Instrumentation-Options.html) | sanitizer 用途与分开运行 TSan 的要求            |

没有借资料声称该教学队列经过形式化验证，或把本机 `is_always_lock_free == 1` 写成所有平台的保证。

## 并发与生命周期自审

- `write_` 仅生产者修改，`read_` 仅消费者修改。本端索引 relaxed，远端 acquire，推进 release；没有同侧并发调用或共享 reset。
- 生产者 `emplace` 及 optional 有值状态建立先于 write 的 release；消费者获取发布之后才检查有值并移动。
- 消费者移动结果并 reset，销毁槽内移后对象，再 release 推进 read；生产者获取复用许可后才检查无值和重新构造。
- 返回值独立拥有 Packet。推进 read 后仅可能移动或销毁本地 optional，不再访问原槽。Packet 的 unique_ptr 不借用原槽内对象。
- `T` 的移动构造与析构均有不抛异常静态约束；正文明确这一约束不等于不分配、不加锁或固定执行时间。
- 索引始终处于 `[0, Slots)`，显式归零。物理槽数至少 2，保留一个槽，可用容量 `Slots - 1`；没有无限递增计数溢出或依赖二次幂槽数。
- Resource 的计数为原子，统计对象先构造、最后销毁。压力测试的资源预先分配，生产者只操作自己的输入向量元素；消费者经转移所有权后访问资源。
- 调用线程是唯一消费者，另一个 jthread 是唯一生产者。按有限 N 消费后显式 join，队列随后销毁；没有停机时的并发析构。
- 静止未排空测试只检查资源被释放，未将其算成业务消息已处理。在线 done 协议仅在正文解释，未声称运行例实现了在线关闭、超时或取消。

## 实际验证

环境：Windows 宿主上的 WSL，GCC `13.3.0`，目标 `x86_64-linux-gnu`。二进制保存在忽略目录 `.artifacts/spsc-review/`，本 agent 没有执行全量 C++ 测试脚本。

正常构建：

```bash
g++ -std=c++20 -O2 -pthread -Wall -Wextra -Wpedantic -Werror examples/spsc-queue.cpp -o .artifacts/spsc-review/demo
.artifacts/spsc-review/demo
```

ASan/UBSan 构建：

```bash
g++ -std=c++20 -O1 -g -pthread -Wall -Wextra -Wpedantic -Werror -fno-omit-frame-pointer -fsanitize=address,undefined -fno-pie -no-pie examples/spsc-queue.cpp -o .artifacts/spsc-review/demo-san
.artifacts/spsc-review/demo-san
```

实际命令使用 `/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/` 绝对路径。以上两种构建运行成功，输出均为：

```text
SPSC checks passed; resources=55282; always_lock_free=1
```

动态工具运行没有报告。验证内容如下：

| 测试                                    | 确定覆盖的行为                                                          |
| --------------------------------------- | ----------------------------------------------------------------------- |
| 物理槽数 2、3、5，各 128 轮             | 初始空、容量 1/2/4 的满、失败不移动、释放单槽后重试、FIFO 和重复回绕    |
| 静止且未排空的 3 槽队列                 | 两个剩余拥有者由队列析构释放，资源 live 回到 0                          |
| 物理槽数 2、3、17，各三轮，每轮 6000 条 | 一生产者与一消费者并发，逐条序号和校验值匹配，join 后为空且资源计数相等 |

资源数为 `128 × (2 + 3 + 5) + 2 + 3 × 3 × 6000 = 55282`。边界断言先在顺序状态测试中保证，压力测试不依赖某种调度必定出现满或空。九轮并发测试覆盖多圈传输，不据此声称枚举了所有执行。

TSan 单独使用 `-fsanitize=thread -fno-pie -no-pie` 加同样的 C++20、pthread、告警及调试参数编译成功，运行以非零状态退出，实际诊断为：

```text
FATAL: ThreadSanitizer: unexpected memory mapping 0x706158472000-0x706158900000
```

这是运行环境启动失败，未声称 TSan 通过，也没有把缺少报告当作无竞争证据。

## 结构与交接

- `readTopics()`、`validateTopics()` 与 H2 统计通过：11 节、18 题，各层 6 题；schema、demo、内部链接与 related 无本章错误。
- 正文及交接已由 Prettier 格式化。最终 markdownlint 按项目配置检查到 41 个 Markdown 文件，0 issues。
- 未修改既有章节、共享 manifest、roadmap、配置或测试，未提交或创建 PR。
- 代码 SHA256：`308dbddf9e9a459c6e216f4dc93cf4cfe3b9dcb01c50b27318393a0285b1c371`。
- 主 agent 汇总全站构建和页面验收；本章不包含性能测量、在线关闭实现或多生产者支持。
