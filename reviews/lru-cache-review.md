# LRUCache 自审交接

日期：2026-09-19（UTC）。本报告为作者自审，最终技术与页面验收由主代理完成。仅编辑 `content/topics/design/lru-cache.md`、`examples/lru-cache.cpp` 和本文件；没有修改已验收 RAII 章、roadmap 或全局审查 manifest。

## 范围与内容契约

继续应用项目 `cpp-quant-writing` skill、写作约定和内容质量标准。专题归入 design 的 Algorithm Coding / System Design，按 11 节契约编写；18 道 derived 题，L1/L2/L3 各 6 题，每题至少两个评分点，companies 均为空。

实现固定为 `int` 键和值，单线程，容量固定且以条目数计。提供 get、put、erase 和值快照，复制及移动操作全部删除。明确未实现任意类型、TTL、加载器、持久化、并发控制、节点池或硬内存预算。正文没有给出吞吐或延迟实测数值。

## 真实来源核对

所有 references 均于 2026-09-19 通过浏览工具实际打开。主要论据如下：

| 断言                                                   | 资料                                                                                             |
| ------------------------------------------------------ | ------------------------------------------------------------------------------------------------ |
| 同一 list 单节点 splice 保留迭代器、不抛出、常数复杂度 | [N4861 list.ops](https://timsong-cpp.github.io/cppwp/n4861/list.ops)                             |
| list 插入异常无效果，删除只失效对应节点引用/迭代器     | [N4861 list.modifiers](https://timsong-cpp.github.io/cppwp/n4861/list.modifiers)                 |
| 哈希操作平均/最坏复杂度，rehash 失效范围               | [N4861 unord.req](https://timsong-cpp.github.io/cppwp/n4861/unord.req)                           |
| 哈希表单元素插入异常保证及 erase 的异常条件            | [N4861 unord.req.except](https://timsong-cpp.github.io/cppwp/n4861/unord.req.except)             |
| 标准整数哈希不抛出                                     | [N4861 unord.hash](https://timsong-cpp.github.io/cppwp/n4861/unord.hash)                         |
| 测试分配器的 allocate/deallocate/rebind 配合           | [N4861 allocator.requirements](https://timsong-cpp.github.io/cppwp/n4861/allocator.requirements) |
| 工程系统可采用采样式近似 LRU，TTL 与 LRU 各有作用      | [Redis key eviction](https://redis.io/docs/latest/develop/reference/eviction/)                   |

Redis 文档只作为现实策略差异的例子，没有将它的近似算法、容量零含义或配置参数套用到本例。

## 技术自审

新增键的执行顺序为 list 新头、map 插入、必要时删除旧尾索引与节点。map 插入失败时删掉新头并重抛，不提前淘汰旧尾。强保证限于固定类型下的键值、大小和最近访问顺序，准备阶段允许暂时多一个条目。

重点核对了两种迭代器：map 的 rehash 使 map 迭代器失效，不使 map 元素引用或指针失效；map 中存储的 list 迭代器则指向另一个容器的节点。实现没有把旧 map 迭代器保留到可能 rehash 的 emplace 之后。主代理初审也要求正文明确此区别，正文表格与 q09 已覆盖。

命中更新只操作 int 赋值和同表 splice，固定 hash/equality 不抛出。淘汰的 erase(key) 因此不抛出，释放计数也不分配、不输出流。双容器成员依赖使默认复制不正确，文中解释了重建新索引的需要；没有声称所有移动实现都不可能正确。

检查器逐个验证 list 键能找到索引，且映射迭代器等于当前节点，再核对两容器大小与容量。参考模型使用 vector 线性查找和重排，逐步比较完整顺序，避免只检查值而漏掉顺序错误。

逐题核对：q01–q06 覆盖策略、组合表示、容量、更新、缺失表示与有效性；q07–q12 覆盖不变量、splice、rehash、回滚、复制及泛型化；q13–q18 覆盖共享写入、分片、模型测试、预分配、测量及交易状态恢复边界。未发现需要放宽异常保证或删除题目的技术问题。

## 已运行验证

环境为 WSL Ubuntu，GCC 13.3.0。编译输出写入独立目录 `/tmp/cpp-quant-lru-review/`，没有运行共享全量 C++ 测试脚本。

普通构建与运行通过：

```bash
mkdir -p /tmp/cpp-quant-lru-review
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror /mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/lru-cache.cpp -o /tmp/cpp-quant-lru-review/demo
/tmp/cpp-quant-lru-review/demo
```

ASan/UBSan 构建与运行通过，退出码 0，无 sanitizer 报告：

```bash
mkdir -p /tmp/cpp-quant-lru-review
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie /mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/lru-cache.cpp -o /tmp/cpp-quant-lru-review/demo-sanitize
/tmp/cpp-quant-lru-review/demo-sanitize
```

libstdc++ debug iterators 构建与运行通过：

```bash
mkdir -p /tmp/cpp-quant-lru-review
g++ -std=c++20 -O0 -g -Wall -Wextra -Wpedantic -Werror -D_GLIBCXX_DEBUG /mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/lru-cache.cpp -o /tmp/cpp-quant-lru-review/demo-debug
/tmp/cpp-quant-lru-review/demo-debug
```

三种运行均输出：

```text
LRU boundaries, 10000 oracle steps, rehash and allocation rollback: OK
```

Sanitizer 和 debug 构建的首次链接因临时输出目录不存在失败，未执行测试；在同一命令中补建该目录后两者通过，未改 C++ 代码。未把首次链接失败记成代码测试通过。

程序覆盖容量 0/1、更新命中、正常淘汰、删除、重复删除和未命中；容量 0–4 的固定种子轨迹共 10000 步，每步核对返回值、完整顺序和 map/list 一致性，并周期请求 rehash。空缓存与满缓存新增的实际分配调用逐个注入 `bad_alloc`，失败后比较旧快照与活跃分配块数，全部对象结束后检查分配块归零。

项目 `readTopics()` 与 `validateTopics()` 对本专题输出通过，18 题分层为 6/6/6，11 节、内部链接、schema 和唯一 demo include 均通过。正文 Prettier 格式化完成。Markdownlint 受项目全局配置影响同时检查当前仓库 Markdown，当次为 0 issues。

## 留给独立验收的边界

没有运行全站构建或浏览器桌面/移动端检查，最终 hash 由主代理验收后写入全局 manifest。有限模型轨迹不构成穷举证明；分配故障只覆盖所测初态在当前标准库中的分配路径，未覆盖任意用户类型的异常或线程调度。

实现仍会分配 list 节点、map 节点及桶数组。FaultAllocator 是测试探针，不能作为生产内存池；其全局计数没有线程同步。测试与断言不应作为性能基准，也不能在定义 NDEBUG 后声称运行了相同验证。
