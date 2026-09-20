# 对象生命周期与布局专题自审

日期：2026-09-19。类型：作者自审。主 agent 已另行审读正文、题目和关键标准条款；本文件只记录作者完成的核查与验证，不替主 agent 写发布 hash。

## 交付范围

- `content/topics/cpp/object-lifetime-layout.md`：C++20 正文 11 节，18 题，L1/L2/L3 各 6 题。
- `examples/object-lifetime-layout.cpp`：布局关系、对象表示复制、简单类型隐式创建和单槽显式生命周期。
- 本文件：作者自审和真实运行结果。

已应用项目 `cpp-quant-writing` skill，复核机械对照、重复收尾、标题套话与聊天残留。没有修改前一章、共享 review manifest、roadmap 或 skill。没有提交 commit，也没有运行全量 C++ 测试脚本。

## 技术核查

正文 references 中的 16 条来源均已真实打开，固定在 C++20 草案 N4861。核查的主要对应关系如下：

| 主张                                   | 依据                                                                                                                                                                                                                                                 |
| -------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 存储、初始化完成、销毁与复用           | [basic.life](https://timsong-cpp.github.io/cppwp/n4861/basic.life)                                                                                                                                                                                   |
| 字节数组提供存储及隐式对象创建边界     | [intro.object](https://timsong-cpp.github.io/cppwp/n4861/intro.object)                                                                                                                                                                               |
| 标准布局和平凡可复制的独立条件         | [class.prop](https://timsong-cpp.github.io/cppwp/n4861/class.prop)                                                                                                                                                                                   |
| 对象表示、填充与字节复制保证           | [basic.types](https://timsong-cpp.github.io/cppwp/n4861/basic.types)                                                                                                                                                                                 |
| 对齐与数组元素空间                     | [basic.align](https://timsong-cpp.github.io/cppwp/n4861/basic.align)、[expr.sizeof](https://timsong-cpp.github.io/cppwp/n4861/expr.sizeof)                                                                                                           |
| C++20 成员顺序及 offsetof 边界         | [class.mem](https://timsong-cpp.github.io/cppwp/n4861/class.mem)、[support.types.layout](https://timsong-cpp.github.io/cppwp/n4861/support.types.layout)                                                                                             |
| 显式构造、销毁与构造失败               | [specialized.construct](https://timsong-cpp.github.io/cppwp/n4861/specialized.construct)、[specialized.destroy](https://timsong-cpp.github.io/cppwp/n4861/specialized.destroy)、[except.ctor](https://timsong-cpp.github.io/cppwp/n4861/except.ctor) |
| malloc 与 memcpy 的指定创建语义        | [c.malloc](https://timsong-cpp.github.io/cppwp/n4861/c.malloc)、[cstring.syn](https://timsong-cpp.github.io/cppwp/n4861/cstring.syn)                                                                                                                 |
| 转换和 launder 不创建任意对象          | [expr.reinterpret.cast](https://timsong-cpp.github.io/cppwp/n4861/expr.reinterpret.cast)、[ptr.launder](https://timsong-cpp.github.io/cppwp/n4861/ptr.launder)                                                                                       |
| optional 管理的生命周期与 emplace 语义 | [optional](https://timsong-cpp.github.io/cppwp/n4861/optional)                                                                                                                                                                                       |

生命周期正文明确：类对象在析构调用开始时结束生命周期，构造和析构期间另有访问规则。没有把“已销毁”扩大为任何指针操作都必然 UB；禁止的是不满足条件的活对象访问。

demo 中的旧指针仅在同类型、非 const 完整对象精确覆盖原存储、第二次构造成功之后读取。两次构造之间没有访问该对象。正文排除了基类与 `no_unique_address` 成员等潜在重叠子对象的直接推广，并区分透明替换与业务订单身份。

隐式创建示例只对含两个 int 的 Pair 使用 malloc，检查空指针并在读取前给两个成员赋值。正文说明非 implicit-lifetime 子对象不会因此自动开始生命周期；Tracked 始终通过 construct_at 显式构造。

布局断言只检查标准允许的关系，没有硬编码 sizeof、alignof 或偏移的具体数值。字节复制从已初始化的 Packet 取表示，再恢复同类型对象，按成员比较结果，不读取或比较填充来判断值相等。

OneSlot 构造成功后才保存对象指针；占用时拒绝再次构造；空槽 reset 不重复析构。构造失败不析构完整对象。类禁止复制且不生成移动，避免指向自身存储的成员指针被错误迁移。它与 optional::emplace 的占用处理不同，正文已说明。

逐题核对了答案和评分点。L3 覆盖解析边界、代际句柄、抽象选择、布局测量、并发借用及测试证据，未以定义改写凑题。

## 验证记录

环境：Windows 宿主的 WSL，`g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`。输出目录：`/tmp/cpp-quant-lifetime-check`。

常规编译采用 `-std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`；ASan/UBSan 编译采用 `-std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie`。两组均退出 0，断言通过，sanitizer 无报告。

本机两组输出一致：

```text
Packet: size=24 align=8 offsets=0,8,16
ReorderedPacket: size=16 align=8
attempts=4 constructed=3 destroyed=3 live=0
layout, representation and lifetime checks passed
```

前两行仅是本机实现观察；正文预期输出只固定状态计数和成功消息，不承诺重排后的大小总会减少。

使用 readTopics 和 validateTopics 完成本章 schema、11 节顺序、18 题分层、题目 ID、demo include、内部链接与 related 核查，没有本章错误。Prettier 和 Markdownlint 检查通过。

未执行 Clang、浏览器验收或性能实验。sanitizer 的无报告不替代生命周期规则审查，也不证明并发对象池正确。终稿源文档与 demo 的 SHA-256 由主 agent 独立验收后写入共享记录。
