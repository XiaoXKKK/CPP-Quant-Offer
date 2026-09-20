# Ring Buffer 自审交接

日期：2026-09-19。本文记录作者自审和实际执行证据；主代理另行负责独立技术、全量构建与页面验收。本章修改范围为 `content/topics/design/ring-buffer.md`、`examples/ring-buffer.cpp` 和本报告。

## 内容与来源

核对 roadmap H 与 taxonomy，选择 design / Algorithm Coding、System Design。应用 cpp-quant-writing，重读工作流程、WRITING_STYLE、CONTENT_STANDARD，保留单 owner、不重入、别名参数和异常范围等条件。正文 11 节、15 道 derived 题，L1/L2/L3 各 5 道，所有题目有答案、评分点，companies 为空。

实际打开 N4861 的 optional.optional、optional.assign、optional.mod、basic.life 和 intro.races。重点核对 emplace 构造抛出时槽位不包含值、reset 结束包含值生命周期、optional 内嵌包含值，以及对象生命周期和跨线程访问的边界。正文没有把一般 optional 赋值的异常保证代替 emplace 的保证。曾尝试打开 Boost circular_buffer implementation 页面但未成功，未将其纳入来源或据此提出结论。

## 状态与对象审查

RingBuffer 采用 N 个 optional 槽与 head、tail、size，编译期要求 N 大于零且 T 无异常析构。所有操作单 owner、不可重入；T 的构造与析构不能调用当前队列。类注释与正文均写明这一契约。

空时不读 head 中的 T；满时不调用 emplace。成功构造后才推进 tail 和 size，且下一槽由不变量保证为空。失败构造时 optional 保持空，游标和占用不变。此保证不回滚通过别名修改的已有元素，也不回滚外部副作用；正文、题目和练习明确区分结构与值的保证。

pop 先 reset 再前移 head，T 析构不抛出；clear 顺序释放剩余元素。类析构调用 clear 后，数组还会销毁全部 N 个已空 optional 槽，因此正文按 O(N) 槽位处理加存活 T 析构解释整体析构，没有只引用 clear 的 O(k)。

advance 在 N−1 时归零，其余路径加一；physical_index 用 N−head 分段映射，不先计算可能回绕的 head+offset。offset 仅在小于 size 后传入，因此所有物理下标均小于 N。容量 1 没有特殊非法状态。

复制和移动均删除，front/at 返回有限生命周期借用；不读取已经 pop 的指针，不把槽位复用当作原业务对象继续存活。optional 数组不被重新解释为 T 数组，没有手写 placement new、裸存储对齐或虚构 span。

Item 不可复制、不可移动，通过 make_unique 管理 Payload。固定容量只保证 RingBuffer 自身不扩容，并不使 Item 构造免于分配。本例的构造失败是指定参数触发的故障注入，成员 Payload 已构造后由 unique_ptr 正常清理；没有制造真实内存耗尽。

## 有限测试

- 容量 3：空读、空 pop、三个元素填满、满时拒绝且不构造新 T、弹出后失败构造、随后正常回绕入队。
- 借用在原元素仍存活时地址稳定；const 访问与越界 at 返回 nullptr。
- clear、重复 clear、带剩余元素离开作用域，成功构造数与析构数一致，Item 和 Payload 存活数最终都为零。
- 容量 1：20 次入队、满时拒绝、出队循环。
- 容量 1 和 3 各枚举 3⁸ = 6561 条长度 8 的轨迹，操作为 push/pop/clear。独立 std::deque oracle 只维护逻辑 FIFO，逐步核对返回值、size、满空和每个元素，不复用环形映射。
- 另做 10000 步反复回绕，每一步都核对 deque。

这些测试不覆盖一般用户类型的全部副作用，也不证明线程安全。运行中没有使用失效指针；未执行未同步并发操作，没有性能数字。

## 实际执行

平台为 x86-64 WSL2，GCC 13.3.0。本人使用独有临时目录执行普通和 sanitizer 构建，均通过，无 warning 或 sanitizer 报告：

```bash
mkdir -p /tmp/cpp-quant-ring-review
src=/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/ring-buffer.cpp
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  "$src" -o /tmp/cpp-quant-ring-review/demo
/tmp/cpp-quant-ring-review/demo
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  "$src" -o /tmp/cpp-quant-ring-review/demo-sanitize
/tmp/cpp-quant-ring-review/demo-sanitize
```

输出：

```text
Nonmovable lifetime, constructor rollback, capacity 1/3: OK
2 x 6561 exhaustive traces and 10000 wrap steps agree with deque.
```

主代理另行报告已完成全文代码审读及严格普通、ASan/UBSan 独立复跑。之后代码只补充单 owner、不重入及别名副作用注释，没有逻辑修改，所以未为注释重复运行 sanitizer。

另在 `/tmp/cpp-quant-ring-boundaries` 创建两个临时翻译单元，包含示例并将 main 重命名，分别实例化 `RingBuffer<int, 0>` 和析构为 `noexcept(false)` 的 T。两次 `g++ -std=c++20 -c` 都按预期失败，核对到本类 static_assert 的具体诊断：`RingBuffer capacity must be positive` 和 `RingBuffer requires nonthrowing destruction`。包装器重命名 main 另产生末尾无 return 的 warning，负向测试未开启 Werror；该 warning 不属于正常示例主函数，也不是判断失败的依据。没有运行错误产物。

## 格式与交接

targeted validateTopics 已通过，确认分类、关联、15 道题和 demo 路径合法。正文与报告的 Prettier 检查通过；Markdown 工具按仓库配置实际扫描 82 个文件，报告 0 问题。主代理已通读正文及 15 题，指出析构成本应包括全部 optional 槽，该限定已补入性能段。最终 hash 交由主代理记录。

本章没有修改共享 manifest、全量 harness 或旧章；后续 roadmap 文档维护由主代理另行授权，独立验收。网页与全量测试不冒充作者自审已执行的结果。
