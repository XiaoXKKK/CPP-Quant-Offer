# ABI 与 name mangling 专题自审

日期：2026-09-19。类型：作者自审。主 agent 已阅读正文、题目和示例；本记录不代替独立验收，也不写发布 hash。

## 交付范围

- `content/topics/cpp/abi-name-mangling.md`：正文 11 节，15 题，L1/L2/L3 各 5 题。
- `examples/abi-name-mangling.cpp`：单文件 C++20 程序，同时用于运行断言和目标文件符号检查。
- 本文件：来源核查、工程边界与实际验证结果。

已应用项目 `cpp-quant-writing` skill，检查重复收尾、机械对照、夸大性能结论和聊天残留。没有改动已验收的章节、共享 manifest、roadmap 或 skill，没有提交 commit。未新增辅助源码；编译产物在 WSL 临时目录。

## 来源与技术边界

正文 references 的 11 条来源均已实际打开。主要结论对应如下：

| 核查内容                                                            | 来源                                                                                                                                                                                                              |
| ------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 语言链接语义与实现相关表示                                          | [C++20 N4861 dcl.link](https://timsong-cpp.github.io/cppwp/n4861/dcl.link)                                                                                                                                        |
| C++ ABI 范围、重载编码及普通非模板函数返回类型不进入名称编码        | [Itanium C++ ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html)                                                                                                                                             |
| 参数分类、寄存器及 MEMORY 类返回值存储地址                          | [AMD64 ABI Draft 0.99.6 第 3.2 节](https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf)                                                                                                                  |
| ELF 绑定、可见性及 hidden 函数间接调用的边界                        | [ELF gABI 符号表](https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.symtab.html)、[GCC 13.3 visibility 属性](https://gcc.gnu.org/onlinedocs/gcc-13.3.0/gcc/Common-Function-Attributes.html)                     |
| libstdc++ dual ABI 宏与语言模式独立、用户类名不足以表达成员布局差异 | [libstdc++ dual ABI](https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_dual_abi.html)                                                                                                                         |
| 原始符号、反修饰与 ELF 字段检查                                     | [nm](https://sourceware.org/binutils/docs/binutils/nm.html)、[c++filt](https://sourceware.org/binutils/docs/binutils/c_002b_002bfilt.html)、[readelf](https://sourceware.org/binutils/docs/binutils/readelf.html) |
| 匹配的 C++ 异常 ABI 与 noexcept 逃逸语义                            | [Itanium exception handling](https://itanium-cxx-abi.github.io/cxx-abi/abi-eh.html)、[C++20 N4861 except.spec](https://timsong-cpp.github.io/cppwp/n4861/except.spec)                                             |

AMD64 引用明确固定在可访问的旧版草案及有关调用序列的章节，没有称其为最新规范。实验明确限定 Linux x86-64、GCC、ELF，没有把 Itanium 名称编码推广到 Windows/MSVC。

正文区分语言链接、符号绑定和可见性。GLOBAL HIDDEN 在可重定位目标文件中仍可出现，不代表动态导出；LOCAL DEFAULT 也不等同于全局隐藏符号。没有把 hidden 写成访问控制或把 nm 的 T 当作动态导出的充分证据。

demo 的非空参数必须指向存活、完整且可访问的声明类型对象。struct_size 只用于版本约定，不验证任意指针，也不授权读取短缓冲区。V1 接口只接受精确大小，没有用该检查声称未来任意布局都兼容。该源码不是 C 公共头，也未以 extern C 声称任何语言都能互操作。

求和在 int64_t 中完成，再检查 int32_t 范围。所有拒绝路径不写输出，接口不转移所有权。问答说明跨组件释放须匹配分配器和运行时，未将其一律判为错误；兼容 C++ 组件可以依赖匹配异常 ABI，通用 FFI 则需自己的失败约定。

15 题分别覆盖基础区分、符号和调用机制、接口演进与排障。L3 答案保留版本、所有权、异常和低延迟接口粒度的工程取舍，没有增加同义题凑数。

## 实际验证

环境为 Windows 宿主 WSL，目标 `x86_64-linux-gnu`，GCC 13.3.0，GNU Binutils 2.42。readelf 文件头显示 ELF64、小端、REL 和 AMD x86-64。

常规编译采用 `-std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`。ASan/UBSan 采用 `-std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie`。两组均退出 0，断言通过，sanitizer 无报告。

两组运行输出：

```text
request layout: size=16 align=4 left-offset=8
overloads, linkage and versioned boundary checks passed
```

第一行只记录本机布局，程序没有对这些数值写固定断言。

目标文件用 `-std=c++20 -O0 -fno-inline -Wall -Wextra -Wpedantic -Werror -c` 生成。实际运行 nm、c++filt、readelf 后得到：

| 原始名称                            | 反修饰名称                                          | ELF 绑定与可见性 |
| ----------------------------------- | --------------------------------------------------- | ---------------- |
| `_ZN8abi_demo5scaleEj`              | `abi_demo::scale(unsigned int)`                     | `GLOBAL DEFAULT` |
| `_ZN8abi_demo5scaleEd`              | `abi_demo::scale(double)`                           | `GLOBAL DEFAULT` |
| `abi_sum_v1`                        | `abi_sum_v1`                                        | `GLOBAL DEFAULT` |
| `abi_hidden_probe`                  | `abi_hidden_probe`                                  | `GLOBAL HIDDEN`  |
| `_ZN12_GLOBAL__N_112local_adjustEj` | `(anonymous namespace)::local_adjust(unsigned int)` | `LOCAL DEFAULT`  |

一次后续 WSL 调用发现临时目标文件已不存在，未运行到 sanitizer 编译；重新在同一次 WSL 调用中编译、检查并运行后全部通过。这是临时构建产物丢失，不是源码诊断或运行断言失败。

终稿通过本章 schema、章节与题目分层、demo include、related 和内部链接检查。Prettier、Markdownlint 检查通过。核查脚本一度用错 readTopics 结果的字段层级，在打印信息时抛出 TypeError；改为读取 data.id 后重跑，确认 15 题、每层 5 题且本章错误列表为空。

未创建或装载共享库，未执行动态卸载、跨语言、跨工具链或 Windows/MSVC 兼容性实验，也没有性能测量。符号观察与 sanitizer 通过不证明任意版本的二进制兼容。主 agent 将独立验收并记录终稿 hash。
