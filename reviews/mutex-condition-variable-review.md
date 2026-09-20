# mutex 与 condition_variable 专题自审

日期：2026-09-19。类型：作者自审。主 agent 已独立阅读并复跑示例；正文和题目仍由主 agent 作最终验收。本文件不代写共享审查 hash。

## 交付范围

- `content/topics/concurrency/mutex-condition-variable.md`：正文 11 节，18 题，L1/L2/L3 各 6 题。
- `examples/mutex-condition-variable.cpp`：固定容量 int 队列、确定状态边界、关闭竞争及生产者失败后排空。
- 本文件：作者自审与真实验证结果。

已读取并应用 `cpp-quant-writing` skill、WRITING_STYLE、CONTENT_STANDARD、CONTRIBUTING、taxonomy 和 schema。语言复核保留了同锁协议、超时含义、异常及生命周期前提，清理套话和重复收尾。未改动前面已验收的章节、共享 manifest、roadmap、skill 或测试脚本，没有提交 commit。

## 资料核查

正文 references 的 11 条 C++20 草案 N4861 来源均已实际打开。

| 内容                                                            | 核查依据                                                                                          |
| --------------------------------------------------------------- | ------------------------------------------------------------------------------------------------- |
| 同一 mutex 的解锁与后续成功加锁同步、互斥访问及无 FIFO 承诺     | [mutex requirements](https://timsong-cpp.github.io/cppwp/n4861/thread.mutex.requirements.mutex)   |
| unique_lock 的锁所有权、显式解锁与重新加锁                      | [unique_lock](https://timsong-cpp.github.io/cppwp/n4861/thread.lock.unique)                       |
| wait 的三个原子部分及条件变量上的操作全序                       | [condition variables](https://timsong-cpp.github.io/cppwp/n4861/thread.condition)                 |
| 同 mutex 前提、谓词重试、虚假唤醒、通知范围、超时返回与析构条件 | [condition_variable](https://timsong-cpp.github.io/cppwp/n4861/thread.condition.condvar)          |
| 调度与重获锁使返回晚于截止点、计时边界                          | [timing specifications](https://timsong-cpp.github.io/cppwp/n4861/thread.req.timing)              |
| 单调计时基准                                                    | [steady_clock](https://timsong-cpp.github.io/cppwp/n4861/time.clock.steady)                       |
| 线程内顺序、同步与数据竞争                                      | [intro.races](https://timsong-cpp.github.io/cppwp/n4861/intro.races)                              |
| jthread 构造、线程入口异常与析构 stop/join                      | [jthread construction/destruction](https://timsong-cpp.github.io/cppwp/n4861/thread.jthread.cons) |
| 成功 join 与工作线程完成之间的同步                              | [jthread members](https://timsong-cpp.github.io/cppwp/n4861/thread.jthread.mem)                   |
| condition_variable_any 的 stop_token 等待与谓词返回值           | [interruptible waits](https://timsong-cpp.github.io/cppwp/n4861/thread.condvarany.intwait)        |
| 普通 thread 仍 joinable 时析构会 terminate                      | [thread destructor](https://timsong-cpp.github.io/cppwp/n4861/thread.thread.destr)                |

## 逐条技术自审

1. 队列的数组、head、tail、size 和 closed 都由同一 mutex 保护；两个条件变量的等待也使用这把 mutex。正文明确释放锁并进入等待的原子性，以及唤醒后需要重新获得锁。
2. 谓词包括关闭状态。push 在锁内发现 closed 后拒绝提交，pop 在有元素时继续出队，只有关闭且为空才返回 closed。close 单向置位并通知两个等待组，重复调用仍正确。
3. 正文画出生产者写入、unlock、消费者 lock、消费者读取的 happens-before 链。没有把 notify 当作共享数据访问的独立替代同步，也没有把条件变量全序误写为整个程序的全序。
4. 说明虚假唤醒和真实通知后的竞争都要求重检。错误原子 ready 方案只用时序表解释，没有运行可能丢唤醒挂死的错误程序。即便原子访问无数据竞争，仍可缺少进展协议。
5. 解锁后通知与持锁通知都可正确，性能只写可能的机制。解锁后继续访问条件变量的寿命由队列活到所有 worker join 之后保障，没有称 notify_all 后就可以立即销毁整个队列。
6. 两个等待组避免 notify_one 唤醒不合适谓词的线程；没有为 mutex 或条件变量承诺 FIFO、公平调度或无饥饿。
7. timed 操作使用 steady_clock 的绝对 deadline。正文区分手写循环重置 wait_for 预算与一次谓词 wait_for 的正常语义，也明确过期后谓词为真仍可成功，返回 true 不证明在业务截止前提交。
8. 数组存储有界，容量为正，索引回绕和空满计数有断言。测试最多接收 128 个有界整数，不存在累加溢出或按未知值索引。int 复制不抛异常，正文没有把相同提交保证推广到任意 T。
9. 接收数组和 received_count 在 join 前仅由 consumer 访问，accepted_count 仅由主线程写。consumer_error 由 consumer 写、主线程在 join 后读，没有为这些结果对象添加不必要的原子访问。
10. 生产者注入异常发生在已接受 37 项后，主线程保存异常、close、join，逐项核对全部已接受值已被健康消费者取走。消费者自身异常会 close 并报告失败，该路径没有保证剩余数据已经处理。同步原语失效或关闭过程再次抛出不在可恢复范围内。
11. 普通 condition_variable 不会自动接收 jthread 停止请求。正文说明 condition_variable_any 的可中断重载，但没有在 stop 时直接丢弃队列。jthread 自动 join 不替代用户的退出谓词。
12. 区分出队、业务处理和外部提交。队列排空与 join 成功不作为业务成功的充分条件。题目中也要求说明消费者失败后的剩余数据及进度处置。
13. 18 道题各自覆盖不同机制或边界。L3 涉及混合等待组、泛型元素异常、生产者与消费者不同故障、测试证据、背压公平性和整体对象寿命，未用同义定义凑题。所有题目均为 derived，companies 为空。

## 运行记录

环境：Windows 宿主 WSL，`g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`。临时编译目录为 `/tmp/cpp-quant-condvar-check`。

常规编译采用 `-std=c++20 -O2 -pthread -Wall -Wextra -Wpedantic -Werror`。ASan/UBSan 采用 `-std=c++20 -O1 -g -pthread -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie`。两组均成功编译并运行，断言通过，sanitizer 无报告。主 agent 也已独立复跑严格 warning 及 ASan/UBSan。

实际成功输出：

```text
accepted=128 drained=128 injected_failure=0
accepted=37 drained=37 injected_failure=1
empty, full, close, drain and failure checks passed
```

单线程测试包括空队列超时、满队列超时、回绕、重复 close、关闭后 push 拒绝、关闭后排空、结束读取保持输出不变。使用已到达的 deadline，不根据耗时判断正确性。

关闭竞争测试分别从空、满状态启动一个 worker，然后立即 close。无论 worker 在关闭前还是关闭后进入队列方法，结果都应为 closed。它没有证明某次运行中 worker 必定实际阻塞过，也没有用 sleep 推断线程到达位置。

并发测试使用同一个 30 秒 deadline 作 watchdog，正常结果以状态与逐项数据为准。deadline 不是 API 的严格完成上界；锁获取、重锁和调度仍可能延迟。示例未强制覆盖所有调度，也未注入内核级虚假唤醒。

另以 `-fsanitize=thread -fno-omit-frame-pointer -fno-pie -no-pie` 成功编译 TSan 版本。启动退出 1，报告如下，未得到 TSan 运行通过结果：

```text
FATAL: ThreadSanitizer: unexpected memory mapping 0x7324c8e72000-0x7324c9300000
```

## 内容校验与限制

readTopics/validateTopics 检查本章 schema、11 节顺序、18 题分层、题目 ID、canonical include、related 与内部链接，没有本章错误。初次 Markdownlint 发现核心概念表中的逻辑或运算符被当成分列符，主 agent 同时指出此渲染问题；已改成保留行内代码的自然语言谓词。使用 remark-gfm 检查三张表的行列及核心表单元格文本，确认谓词完整留在第三列。随后 Prettier 与 Markdownlint 均通过。q16 也补上未触发 watchdog 的前提，避免把极端调度停顿写成标准保证。

没有执行性能实验、浏览器验收或多生产者多消费者压力测试。ASan/UBSan 无报告及有限状态测试不证明并发协议对所有执行正确，也不证明生产系统能在消费者故障时恢复消息。主 agent 完成独立技术与阅读验收后再记录源文档和示例的 hash。
