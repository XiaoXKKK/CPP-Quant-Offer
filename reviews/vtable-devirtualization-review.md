# 虚函数与去虚拟化专题自审交接

- 日期：2026-09-19。
- 类型：作者自审，主 agent 独立验收。
- 范围：`content/topics/cpp/vtable-devirtualization.md`、`examples/vtable-devirtualization.cpp`。
- 结论：语言规则、ABI 限定、运行断言和汇编记录核对完成；不声称测得性能收益。

## 范围与写作

已应用项目 `cpp-quant-writing`、写作约定、质量标准与贡献指南。正文按规定排列 11 节，18 道题中 L1/L2/L3 各 6 道，全部为 `derived`，`companies` 为空。题目检查最终覆盖函数、生命周期、调整入口、优化边界与工程测量；没有增加岗位或公司界面筛选。

语言复核保留必要的技术区别，删去无条件性能承诺。C++20 语义与 Linux GCC 的 ABI 产物分别说明，不把本次偏移、目标检查或内联结果写成跨平台保证。正文明确本实验未启用 LTO 或 PGO。

## 资料核查

2026-09-19 实际打开并核对了全部 frontmatter 来源：

| 来源                                                                                                | 核对内容                                                 |
| --------------------------------------------------------------------------------------------------- | -------------------------------------------------------- |
| [N4861 class.virtual](https://timsong-cpp.github.io/cppwp/n4861/class.virtual)                      | 覆盖、最终覆盖函数、override、final、显式限定调用        |
| [N4861 class.cdtor](https://timsong-cpp.github.io/cppwp/n4861/class.cdtor)                          | 构造与析构阶段的虚调用、间接调用边界                     |
| [N4861 class.pre](https://timsong-cpp.github.io/cppwp/n4861/class.pre)                              | final 类不能成为基类                                     |
| [N4861 class.abstract](https://timsong-cpp.github.io/cppwp/n4861/class.abstract)                    | 纯虚函数定义与构造析构期纯虚调用的未定义行为             |
| [N4861 expr.delete](https://timsong-cpp.github.io/cppwp/n4861/expr.delete)                          | 普通多态删除与虚析构，destroying delete 的独立条件       |
| [N4861 expr.dynamic.cast](https://timsong-cpp.github.io/cppwp/n4861/expr.dynamic.cast)              | 合法对象转换、失败指针转换和完整对象地址                 |
| [Itanium C++ ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html)                               | address point、offset-to-top、析构入口、次虚表和调整入口 |
| [ABI implementation examples](https://itanium-cxx-abi.github.io/cxx-abi/abi-examples.html)          | 多继承中的虚调用与 thunk                                 |
| [GCC 13.3 Optimize Options](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Optimize-Options.html)    | 去虚拟化、推测性保护路径及 LTO 选项边界                  |
| [GCC 13.3 C++ ABI overview](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/G_002b_002b-and-GCC.html) | GCC 的 ABI 背景                                          |

未把 Itanium 处理器上的函数描述符表示套用到本次 x86-64 目标。构造析构期纯虚调用与有定义的纯虚函数被显式限定调用分开解释。推测性去虚拟化的候选来自编译时可见信息，本例没有运行时采样训练。

## 代码与运行验证

环境为 Windows 宿主上的 WSL，GCC `13.3.0`，目标 `x86_64-linux-gnu`。独立执行本例，未运行共享全量 C++ 测试脚本。

正常构建分别使用 `-std=c++20 -O0` 和 `-std=c++20 -O2`，均加 `-Wall -Wextra -Wpedantic -Werror`，编译与运行通过。Sanitizer 构建使用 `-std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fno-omit-frame-pointer -fsanitize=address,undefined -fno-pie -no-pie`，运行通过且无报告。三种运行的输出均为：

```text
dispatch checks passed; lifecycle=101,202,302,401
```

断言覆盖正负值、两个不同的 Value 实现、局部已知类型、次基类分派、成功与失败的 dynamic_cast、显式限定调用、基类拥有者销毁派生对象，以及完整构造析构轨迹。Trace 的容量足够容纳四次事件，拥有者先销毁，记录容器随后销毁。没有解引用失效对象、读取手工转换的 vptr 或按猜测槽位调用函数。

## 汇编与符号证据

实际执行 `-O0/-O2 -S -masm=intel` 并阅读了相应函数；也执行了 `nm -C` 和 `objdump -d -C -Mintel`。可复现命令在正文中，源文件实际以 WSL `/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/` 绝对路径传入。

| 路径                    | 本次实际结果                                               |
| ----------------------- | ---------------------------------------------------------- |
| O0 unknown              | 从虚表项加载函数地址后 `call rdx`                          |
| O2 unknown              | `mov rax, QWORD PTR [rdi]` 后经 16 字节槽位间接尾跳转      |
| O0 final                | 直接调用 Fixed::value                                      |
| O2 final                | `mov eax, DWORD PTR 8[rdi]` 后返回                         |
| O0 known_local          | 通过基类引用间接调用，并执行局部对象析构                   |
| O2 known_local          | `mov eax, 7` 后返回                                        |
| O2 label                | 比较候选 thunk 地址，命中读取字段，不匹配经 `jmp rax` 回退 |
| O0 Combined label thunk | `sub rdi, 8` 后跳转至实现                                  |
| O2 Combined label thunk | 调整融入成员偏移，直接从子对象地址读取字段                 |

O0 虚表组显示 Combined 的次表 offset-to-top 为 -8，以及 Fixed 的完整对象和 deleting destructor 入口。`nm -C` 确认 `vtable for vtable_demo::Combined` 与 `non-virtual thunk to vtable_demo::Combined::label() const`。这些值只描述当前编译产物。

复核产物保存在忽略目录 `.artifacts/vtable-review/`，包括 `demo-O0.s`、`demo-O2.s`、`symbols.txt`、`disassembly-O0.txt`、`demo-O0` 和 `demo-san`。此前普通 O0/O2 运行使用独立临时目录；持久目录中的 O0 二进制又编译运行一次，以保存 objdump 证据。没有将汇编指令数量换算成延迟或吞吐。

## 验收边界

- 正文结构、题量、schema、demo 路径、内部链接及 related 已通过项目内容校验。
- 正文与交接经 Prettier 格式化；markdownlint 按仓库配置检查 36 个 Markdown 文件，0 issues。一次辅助统计误将字符串传给 `headings`，修正为传入解析后的 `tree` 后，结构统计与校验通过。
- 本交接只涵盖该章三文件；共享 manifest、roadmap、配置、旧章及 E2E 文件均未修改。
- 全站构建、页面和最终 manifest hash 由主 agent 汇总验收。
- 本例未验证跨翻译单元 LTO、动态库、其他编译器或其他体系结构；相关讨论给出条件，不将其列作已完成实验。
