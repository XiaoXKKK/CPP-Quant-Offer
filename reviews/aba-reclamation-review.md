# ABA 与安全回收自审交接

日期：2026-09-19。本报告为作者自审，主代理负责独立技术与页面验收。范围仅为 `content/topics/concurrency/aba-reclamation.md`、`examples/aba-reclamation.cpp` 和本文件，没有修改旧章、全局 manifest、roadmap 或公共测试 harness。

## 内容契约和来源

已继续应用项目 cpp-quant-writing skill、写作约定、内容标准。正文 11 节，18 道 derived 题，L1/L2/L3 各 6 道，每题有答案和评分点，companies 为空。分类为 concurrency 的 C++ Memory Model / Multithreading / Atomic / Lock-Free；分类不表示 demo 满足 lock-free。

正文固定 C++20，实际代码依赖 atomic shared_ptr 和 jthread。hazard pointer、epoch 只解释协议与工程边界，没有声称 C++20 提供标准 hazard API，也没有实现并发布未经证明的回收器。

所有 references 于 2026-09-19 通过浏览工具实际打开。N4861 覆盖 CAS 更新 expected、失败序、acquire/release、原子智能指针的计数与销毁顺序、指针加所有权的 CAS 等价，以及 jthread 析构 join。

Michael 的 2004 年原始 hazard pointer 论文通过多伦多大学课程保存的完整 PDF 打开，另核对 IBM Research 原出版记录。正文采用其中的连续保护和退休扫描机制，没有引用历史机器的性能数字或把论文当时的硬件限制写成当前事实。后续再次按行打开同一 PDF 曾超时，先前已成功获得原文并阅读保护与退休部分。

P2530R3 从 WG21 官方站点实际打开，阅读保护概述与 try_protect 的发布后重读步骤。正文明确它是 2023 年面向 C++26 的提案，只作机制参考；reset_protection 的协议义务没有被简化成一次 release-store。

Brown 的论文从作者托管的 fullpaper.pdf 实际打开，核对经典 EBR 的停滞积压、静止边界以及从退休节点继续遍历的适用性问题。作者目录首页最初打不开，改用搜索返回的原始 PDF，随后也打开 arXiv 版本；正文引用成功读取的作者 PDF。

## 算法与生命周期自审

索引模型在单线程中安排 T1/T2 的逻辑时序。数组槽位从未销毁，原子索引始终在合法值范围；旧 CAS 成功后断言逻辑栈头指向已经移出的 B，只展示状态错误，不执行悬空指针访问。tag 模型用两位代数分别演示未回绕拒绝与四次变化后身份重合，不依赖实际地址复用。

别名比较测试的两个控制块各自拥有一个堆整数，存储指针都别名到同一个局部 payload。payload 声明早于所有别名，活得更久，且别名不删除 payload。它只验证 atomic shared_ptr 的 CAS 同时考虑指针和所有权，不能作为别名指针总能自动保活任意对象的例子。

OwnedStack 节点 value 与 next 都不可变。push 从 acquire 取得的 old 构造候选，成功 acq_rel CAS 发布；失败 acquire CAS 更新 old，丢弃未发布候选后重建。每次压入使用新节点，不暴露把旧节点重新压入的接口。

pop 在读取 old->next/value 前已持有 old 所有权。成功 CAS 是弹出的提交点，返回独立整数；失败 CAS 得到拥有型的新候选后重试。next 是 shared_ptr，故 old 保活时其后继拥有链也存活。空返回以观察到空 head 为边界。

主代理预审提醒 head 仍可 A→新 B→A。正文与 q12 已明确：这里可以无害，因为 A 没有结束生命周期、next 没变且不允许已弹出 A 改链重插。没有宣称 shared_ptr 消除任意可变算法的逻辑 ABA，也没有将控制块比较当作全部正确性证明。

快照可保留整段后继拥有链。最后引用释放可能递归销毁；所有测试链长上界 128，未声称类的接口自动限制任意用户调用的链长。Node 计数使用原子，创建与销毁总量随 CAS 冲突变化，断言比较的是最终相等与 live 归零，没有要求某个固定分配次数。

TestAllocator 在下一次 allocate 调用中注入 bad_alloc；该次节点尚未构造和发布，没有制造真实内存耗尽。失败检查在单线程阶段执行，确认 head 和 live 不变。并发情况下只保证失败的本次 push 未发布新头，不能保证其他线程没有修改栈。

生产线程分别写自己的 exception_ptr 数组元素，没有共享同一异常槽。jthread 容器先于 stack 析构，部分线程创建失败时也会 join 已启动线程，主线程读取异常槽在 join 后。消费回调不分配结果容器，只更新固定原子计数。关闭仍要求没有线程在栈对象销毁后访问其成员；shared_ptr 只保活节点。

## 已运行的测试

WSL Ubuntu，GCC 13.3.0，libstdc++。只编译本章 demo 到独立 `/tmp/cpp-quant-aba-review/`，没有运行全仓 C++ 脚本。

严格普通构建和运行通过，退出码 0：

```bash
mkdir -p /tmp/cpp-quant-aba-review
src=/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/aba-reclamation.cpp
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -pthread "$src" -o /tmp/cpp-quant-aba-review/demo
/tmp/cpp-quant-aba-review/demo
```

ASan/UBSan 构建运行通过，退出码 0、无报告：

```bash
mkdir -p /tmp/cpp-quant-aba-review
src=/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/aba-reclamation.cpp
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -pthread -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie "$src" -o /tmp/cpp-quant-aba-review/demo-sanitize
/tmp/cpp-quant-aba-review/demo-sanitize
```

两种构建输出均为：

```text
atomic shared_ptr head lock-free: false
ABA models, ownership, delayed destruction and 128 items: OK
```

非 PIE 参数用于避开本机 sanitizer 映射冲突，与代码的语言要求无关。没有运行故意悬空访问，也未把 sanitizer 当成回收算法证明。

覆盖内容：安全索引 ABA、有限 tag 检测与回绕、不同所有权的相同存储指针比较、空栈、LIFO、分配器故障注入、两节点快照延迟销毁、释放快照后的最终回收、旧节点快照不能提交对新发布同值节点的弹出。

并发测试分两阶段：四个生产者各压入 32 个唯一整数，全部 join；再四个消费者弹空，逐值检查计数为 1。总共 128 个业务值，重试候选的 Node 构造数可更多。最终所有 Node 都销毁。没有测试 push/pop 混合的全部交错，逐值无丢失重复不等于验证任意历史线性化。未运行 TSan，不作 TSan 已通过的声明。

## 自审问题与边界

L1 逐题区分当前比较值、生命周期与退休状态；L2 覆盖重读窗口、保护范围、epoch 公告、原子 shared_ptr 比较和无害 A→B→A；L3 涉及阻塞回收、快照积压、异常与关闭、测试覆盖、进展及配置版本。未把安全回收等同业务新鲜度。

不承诺无分配、lock-free 或 wait-free。push 每次重试新分配，pop 释放引用可能触发析构；atomic shared_ptr 本机报告 false。生命周期计数与故障开关增加原子开销，不能用 demo 时间作性能比较。

作者已运行本专题 readTopics/validateTopics 检查，schema、11 节、题目分层、唯一 demo include 与内部链接通过。正文与报告 Prettier 检查通过；Markdownlint 按项目配置扫描当时的 43 个文件，0 issues。未运行全站构建或浏览器渲染，留给主代理统一验收。没有公共内容审查 hash 更新。
