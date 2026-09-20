# RAII 与异常安全自审交接

日期：2026-09-19（UTC）。本报告记录作者自审，最终独立验收记录由主代理维护。范围仅为 `content/topics/cpp/raii-exception-safety.md`、`examples/raii-exception-safety.cpp` 与本交接文档；没有更新全局审查 manifest。

## 内容范围

专题按 C++20 编写，包含 11 节、18 道推导题，L1/L2/L3 各 6 道。已读取并应用项目 `cpp-quant-writing` skill、写作约定、内容质量标准、贡献指南、schema 与 taxonomy。题目无公司归属，页面正文没有重复来源声明。

主线为资源拥有关系、成员构造失败、基本保证与强保证组合、`noexcept` 和提交条件。并发发布、真实文件关闭和订单恢复只说明需要额外证明的边界，没有把教学程序描述为这些系统的完整实现。

## 论据核对

以下页面于 2026-09-19 通过浏览工具真实打开并阅读；正文 references 保存同日访问日期。

| 结论                                              | 证据                                                                                                                                                                                                                                               |
| ------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 已完成成员清理、逆序析构、委托构造例外            | [N4861 except.ctor](https://timsong-cpp.github.io/cppwp/n4861/except.ctor)                                                                                                                                                                         |
| 成员声明顺序决定初始化顺序                        | [N4861 class.base.init](https://timsong-cpp.github.io/cppwp/n4861/class.base.init)                                                                                                                                                                 |
| 不抛出边界、析构默认异常说明                      | [N4861 except.spec](https://timsong-cpp.github.io/cppwp/n4861/except.spec)                                                                                                                                                                         |
| 展开中的析构异常及终止前展开范围                  | [N4861 except.terminate](https://timsong-cpp.github.io/cppwp/n4861/except.terminate)                                                                                                                                                               |
| unique_ptr 移动后源为空、reset 空指针、删除器条件 | [构造](https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.ctor)、[修改操作](https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.modifiers)、[析构](https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.dtor)              |
| vector 提交、分配器前置条件、移动失败例外         | [vector.capacity](https://timsong-cpp.github.io/cppwp/n4861/vector.capacity)、[容器要求](https://timsong-cpp.github.io/cppwp/n4861/container.requirements.general)、[vector.modifiers](https://timsong-cpp.github.io/cppwp/n4861/vector.modifiers) |
| 保证等级与分步故障注入方法                        | [Abrahams 原文](https://www.boost.org/doc/libs/1_31_0/more/generic_exception_safety.html)                                                                                                                                                          |
| 及时建立拥有者防止泄漏                            | [Core Guidelines E.6](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Re-raii)                                                                                                                                                        |

曾尝试 Boost 的 community 路径，浏览工具超时，因此换成实际可打开的版本化原文路径。没有把超时页面记为已核对。Abrahams 原文用于术语和推理方法，其中早期库行为不替代 C++20 规范。

## 自审发现与处理

1. 初稿把“复制构造失败”和“完整临时 vector 的析构”写进同一句，容易误导为未完成构造的 vector 会调用自身析构。已拆开：复制失败由容器构造清理路径处理，构造完成后的验证/追加失败才析构临时对象。
2. 补入 unique_ptr 移动构造和 reset 的直接规范来源，核对源指针为空与重复空 reset 的断言。
3. 将示例介绍中的“两个有界资源计数器”改为“资源计数与释放轨迹”，与实际固定数组和计数器一致。
4. 练习说明改为基线尚未覆盖的基本保证负数输入、空批量注入失败，避免要求读者重复已有强保证负数断言。
5. 按主代理验收反馈补入同批 move/value category 与 vector 失效的内部链接及 related，分别承接所有权转移与成功更新后的借用边界。

逐题核对：q01–q06 对应拥有关系和保证等级；q07–q12 对应构造、操作组合、分配器、析构、迁移和引用；q13–q18 对应并发发布、外部副作用、故障注入、成本、错误报告和回滚。每题至少两个评分点，L3 包含工程前提与恢复或测量约束。

## 实际执行结果

运行环境：WSL Ubuntu，`g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`。测试产物仅写入 `/tmp/cpp-quant-raii-review/`，没有运行共享的全量 `test_cpp.py`。

普通编译与运行通过：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror /mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/raii-exception-safety.cpp -o /tmp/cpp-quant-raii-review/demo
/tmp/cpp-quant-raii-review/demo
```

ASan/UBSan 编译与运行通过，退出码为 0，无 sanitizer 报告：

```bash
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie /mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/raii-exception-safety.cpp -o /tmp/cpp-quant-raii-review/demo-sanitize
/tmp/cpp-quant-raii-review/demo-sanitize
```

两次运行均输出：

```text
RAII cleanup, ownership transfer, basic/strong guarantees: OK
```

覆盖三个构造失败位置、正常释放顺序、唯一拥有权转移、空拥有者重复 reset、批量四个失败位置、基本保证保留前缀且仍可继续使用、强保证旧值不变、空输入与负数拒绝。析构观察只写测试计数器与固定数组，不分配、不输出流。

已调用项目 `readTopics()` 与 `validateTopics()`，筛选本专题错误，结果为 schema、11 节顺序、内部链接、demo include 均通过，题数输出为 18、各层 6。对正文执行 Prettier 格式化与检查通过；Markdownlint 调用因项目配置同时扫描仓库 Markdown，当次输出 0 issues。

## 验收边界

作者未进行浏览器桌面/手机渲染验收或全量构建。主代理已反馈完整审读正文、18 题与 demo，未发现阻断技术问题；最终验收及其运行证据由主代理记录。正文设为 `published` 以便接入发布；全局 manifest 尚缺本专题 hash，需最终验收完成后记录。

没有真实内存分配失败注入、异常成本基准或跨编译器测试。ASan/UBSan 的有限执行无报告不构成完整正确性证明。资源计数仅支持测试中的单线程有界负载；本程序不演示真实 fd、网络发送、事务持久化或并发状态发布。
