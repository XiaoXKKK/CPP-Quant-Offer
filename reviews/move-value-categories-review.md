# move 与值类别专题自审

日期：2026-09-19。类型：作者自审；主 agent 的独立验收与发布 hash 另行记录。本记录不冒充浏览器验收或跨编译器测试。

## 范围与结论

本次新增 `content/topics/cpp/move-value-categories.md` 和 `examples/move-value-categories.cpp`。正文采用 C++20，包含约定的 11 节和 18 道题，L1、L2、L3 各 6 题。已应用项目 `cpp-quant-writing` skill，复核机械对照、重复结尾、装饰性排版与聊天残留；题目按知识点组织，没有公司归属声明或岗位分类界面改动。

自审结论：可交给主 agent 验收。未写入共享 `reviews/content-review.json`，未提交 Git commit，也未运行全量 C++ 测试脚本。

## 依据核查

所有 references 均于本次工作中真实打开，使用固定 C++20 草案 N4861，避免混入更新标准中的隐式移动规则。没有复制来源的大段文字。

| 结论                                     | 核查条款                                                                                                                                           |
| ---------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------- |
| 表达式类别与命名右值引用                 | [basic.lval](https://timsong-cpp.github.io/cppwp/n4861/basic.lval)                                                                                 |
| decltype 的名字例外与表达式规则          | [dcl.type.decltype](https://timsong-cpp.github.io/cppwp/n4861/dcl.type.decltype)                                                                   |
| move、forward、move_if_noexcept 返回类型 | [forward](https://timsong-cpp.github.io/cppwp/n4861/forward)                                                                                       |
| 转发引用与折叠                           | [temp.deduct.call](https://timsong-cpp.github.io/cppwp/n4861/temp.deduct.call)、[dcl.ref](https://timsong-cpp.github.io/cppwp/n4861/dcl.ref)       |
| 特殊成员生成、删除候选与成员移动         | [class.copy.ctor](https://timsong-cpp.github.io/cppwp/n4861/class.copy.ctor)                                                                       |
| 构造 trait 不证明存在移动构造            | [meta.unary.prop](https://timsong-cpp.github.io/cppwp/n4861/meta.unary.prop)                                                                       |
| 标准库移动后有效但未指定                 | [lib.types.movedfrom](https://timsong-cpp.github.io/cppwp/n4861/lib.types.movedfrom)                                                               |
| unique_ptr 源指针为空的特定后置条件      | [unique.ptr.single.ctor](https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.ctor)                                                         |
| prvalue 初始化与可选 NRVO                | [dcl.init](https://timsong-cpp.github.io/cppwp/n4861/dcl.init)、[class.copy.elision](https://timsong-cpp.github.io/cppwp/n4861/class.copy.elision) |
| noexcept 传播边界与 reserve 的异常保证   | [except.spec](https://timsong-cpp.github.io/cppwp/n4861/except.spec)、[vector.capacity](https://timsong-cpp.github.io/cppwp/n4861/vector.capacity) |
| 引用形参不能传递临时对象寿命延长         | [class.temporary](https://timsong-cpp.github.io/cppwp/n4861/class.temporary)                                                                       |

## 自审中的修正

- 扩容实验使用 `capacity() + 1`，并先断言 `capacity() < max_size()`；没有依赖 `reserve(2)` 返回恰好为 2 的容量。
- 标准库的有效但未指定状态不推广到所有用户类。正文明确 `Trace` 移动构造把源整数设为 `-1`，默认化移动赋值没有这一行为。
- 保留 `unique_ptr` 移动后为空的断言，移除对 `string` 必为空的任何假设；调用 `clear()` 和赋值验证源对象可复用。
- 将转发的“保留值类别”细化为保留左右值属性，明确 prvalue 经引用参数转发后是 xvalue。
- 使用删除复制和移动构造的 `Immovable` 验证同类型 prvalue 初始化；对具名返回与 `vector` 搬移计数只输出观察结果，不作标准必然次数断言。
- 对 `reserve` 使用规范中的 CopyInsertable 条件，说明不可复制元素的移动构造抛异常不在无效果保证内。`Risky` 示例只有可能抛异常的签名，没有异常注入，正文已说明验证范围。
- 核对所有题目答案与正文依据。L3 分别讨论扩容失败、接口参数所有权、测量方案、自引用布局、队列重试和资源回收延迟，没有用换一种措辞重复基础题。

## 真实验证

环境为 Windows 宿主的 WSL，GCC `g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`。独立输出目录为 `/tmp/cpp-quant-move-check`。

三组编译均开启 `-std=c++20 -Wall -Wextra -Wpedantic -Werror`，并运行所得程序：

| 组别         | 额外参数                                                                       | 结果                                     |
| ------------ | ------------------------------------------------------------------------------ | ---------------------------------------- |
| 常规         | `-O2`                                                                          | 退出 0，全部断言通过                     |
| 关闭可选消除 | `-O2 -fno-elide-constructors`                                                  | 退出 0，全部断言通过，Immovable 编译成功 |
| Sanitizer    | `-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-pie -no-pie` | 退出 0，无 ASan/UBSan 报告               |

常规及 sanitizer 的本机观察：

```text
explicit construction: copies=1 moves=1
named return: copies=0 moves=0
nothrow reserve: copies=0 moves=2
potentially throwing reserve: copies=2 moves=0
value categories, forwarding, ownership and return checks passed
```

关闭可选消除后，`named return` 变为 `copies=0 moves=1`，其余输出相同。这些是 GCC 13.3 与当前标准库的运行结果，不构成其他实现选择相同搬移策略的证明。

内容检查使用 `readTopics()` 和 `validateTopics()`，检查本章 schema、H2 顺序、demo include、内部链接、related 和题目 ID，没有发现本章错误。Markdownlint 与 Prettier 检查通过。两条新关联已指向实际存在的 RAII 与 vector 章节。

没有运行 Clang、浏览器或性能 benchmark；构造计数用于解释机制，sanitizer 无报告仅覆盖本次有限执行。最终内容与 demo 的 SHA-256 由主 agent 在独立验收后写入共享记录。
