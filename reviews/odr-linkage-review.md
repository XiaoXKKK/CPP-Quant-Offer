# ODR / 链接自审交接

日期：2026-09-19。本报告为作者自审，独立验收由主代理完成。本次只新增 `content/topics/cpp/odr-linkage.md`、`examples/odr-linkage.cpp` 和本文件；未改已验收章节、公共 C++ harness、roadmap 或全局 manifest。

## 内容与范围

继续应用项目 cpp-quant-writing skill、写作约定和内容质量标准。专题使用 cpp / Modern C++ / Compiler / Linker / ABI 分类，正文 11 节，18 道 derived 题，L1/L2/L3 各 6 道，companies 为空。

版本固定为 C++20，讨论普通源文件与文本头文件，不将结论直接推广到命名模块、动态库装载或跨 DSO 可见性。默认 demo 是单文件可移植标准 C++20；多翻译单元、静态归档与符号实验明确使用 Linux GCC/GNU Binutils。没有性能数值或通用 ABI 承诺。

## 来源核对

正文 11 条 reference 均于 2026-09-19 用浏览工具实际打开。C++20 N4861 的 `basic.def`、`basic.def.odr`、`basic.link`、`dcl.inline`、`lex.separate` 分别支持声明/定义、ODR-use 与多定义、名字链接、inline 身份和翻译单元。另打开 `intro.compliance`，确认需要诊断与必须拒绝编译并非同一要求。

GCC 文档固定到 13.3.0 的 Overall Options 与 Link Options。GNU Binutils 官方 nm、ar 及 ld Options 文档用于说明符号标记、归档和库搜索顺序；标准与本次 ELF/GNU 实现观察分别表述。没有把 GNU unique 或弱符号编码说成 ISO C++ 规定。

## 技术自审

公共定义区在三次编译中保持相同。外部 inline `touch_shared` 只引用外部 inline `shared_visits` 与 `function_counter`；后者的函数局部 static 在符合条件的外部 inline 函数中是同一对象。per-TU 的可变 `internal_value` 仅由各自唯一的外部函数定义访问，不被放进外部 inline 的名字查找中。

多 TU 正常构建检查三个共享地址相同、内部地址不同，以及共享值总计 2、内部各计 1。单 TU 模式中两个访问函数共用本 TU 的内部对象，值为 2。整数操作有界、单线程，不存在溢出或未经同步的并发共享；返回指针指向程序运行期间仍存活的静态存储期对象。

`declared_only` 仅被 sizeof 使用，没有定义。其已知 int 类型足以计算大小，不产生该对象的 ODR-use。文中明确 ODR-use 不能简化为“出现变量名”或“只有取地址”，30 秒回答保留弃置语句之外这一条件。

错误变体单独编译、单独链接，未运行其产物。同 TU 变量重定义用于观察需要诊断的规则；跨 TU 非 inline 重定义和缺定义只称为本 GNU 工具链预期报错，没有宣称标准要求链接诊断。未制造和运行不同 inline 定义的 IFNDR 程序。

归档正确顺序为引用对象 MAIN 在库之前；反序失败观察的是成员提取边界。nm 对象节内偏移未被解释为最终地址，地址身份由正常程序断言检查。W/u 仅为本次实现观察，不能证明 token、名字查找或类型定义一致。

逐题核对：q01–q06 覆盖声明、TU、内部链接、guard、inline 和 sizeof；q07–q12 覆盖名字查找、局部 static、const 链接例外、诊断、符号和归档顺序；q13–q18 涉及构建宏、共享计数、增量构建、静态注册、定位缺定义和性能测量。所有答案均有正文支撑，没有将链接成功视为合法性证明。

## 已运行的 C++ 验证

环境为 WSL Ubuntu，GCC 13.3.0，GNU ld、nm、ar 2.42。对象及可执行文件使用独立目录 `/tmp/cpp-quant-odr-review/`。未运行会与其他代理争用输出目录的全量 `test_cpp.py`。

源文件路径为 `/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/odr-linkage.cpp`。正常构建采用 `-std=c++20 -O0 -g -Wall -Wextra -Wpedantic -Werror`；默认模式另以 `-O2` 构建运行，均通过。

三 TU 的具体编译方式与正文一致：

```bash
mkdir -p /tmp/cpp-quant-odr-review
src=/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/odr-linkage.cpp
work=/tmp/cpp-quant-odr-review
flags=(-std=c++20 -O0 -g -Wall -Wextra -Wpedantic -Werror)
for unit in A B MAIN; do
  g++ "${flags[@]}" -DODR_MULTI_TU -DODR_TU_$unit -c "$src" -o "$work/$unit.o"
done
g++ "$work/MAIN.o" "$work/A.o" "$work/B.o" -o "$work/multi"
"$work/multi"
ar rcs "$work/libodr-parts.a" "$work/A.o" "$work/B.o"
g++ "$work/MAIN.o" "$work/libodr-parts.a" -o "$work/archive"
"$work/archive"
nm -C "$work/A.o" "$work/B.o"
```

两个多 TU 正常程序退出码均为 0，输出 `multi-TU: shared entities and separate internal state OK`。默认单 TU 输出 `single-TU: entity identity and bounded updates OK`。

实际 nm 检查得到：A 的 `external_value` 为 D，B 对它为 U；`view_a/touch_a` 或 `view_b/touch_b` 为 T；每个对象的 `internal_value` 为 b；`function_counter/touch_shared` 为 W；`shared_visits` 与 `function_counter()::count` 为 u。`declared_only` 未出现在输出中。

首次综合脚本在正常构建运行之后，因本环境 WSL 缺少 rg，改用的 grep 未开启扩展正则，导致带 `|` 的输出筛选没有匹配而退出。随后用简单字符串筛选重新执行符号与预期失败检查，通过；未修改 C++ 源码，也未将该脚本中止记为诊断验证完成。

以下预期失败均核对了非零退出码及对应日志，没有只凭命令失败认定覆盖成功：

| 构建                                           | 实际诊断                                                        |
| ---------------------------------------------- | --------------------------------------------------------------- |
| 默认单 TU 加 `-DODR_BAD_SAME_TU` 编译          | `external_value` redefinition，第二个定义位于原始源文件第 50 行 |
| B 加 `-DODR_BAD_DUPLICATE`，与正常 A/MAIN 链接 | `external_value` multiple definition，指出 A 与 B 的定义        |
| A 加 `-DODR_BAD_MISSING`，与正常 B/MAIN 链接   | `external_value` undefined reference，来自 view/touch 路径      |
| 将正常库放在 MAIN.o 前链接                     | `view_a/view_b/touch_a/touch_b` undefined reference             |

验证脚本最终输出 `ODR expected diagnostics and symbol checks: PASS`。正文保留可复制的正常构建和错误变体命令，并要求逐项核对诊断原因。

ASan/UBSan 对默认程序和正常多 TU 程序均完成编译运行，退出码 0、无报告。编译所有 TU 和最终链接均使用：

```text
-std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror
-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie
```

禁用 PIE 用于避开本机 sanitizer shadow mapping 环境冲突，不作为示例语言要求。Sanitizer 只检查有限正常执行，未声称能够证明 ODR 正确性。主代理另告知已独立复跑默认及三 TU sanitizer、核对 nm；该结果不充作作者自己的检查记录。

## 内容检查与验收边界

本专题 `readTopics()/validateTopics()` 校验通过，涵盖 schema、18 题分层、11 节、内部链接及唯一 demo include。正文与本报告完成 Prettier 格式检查；Markdownlint 结果另在交付消息中说明。文件只保留成稿，删除了写作过程口吻，没有重复展示推导来源声明。

未运行浏览器桌面/移动端渲染或全站构建，留给主代理统一验收。多 TU 模式必须执行额外命令；默认单文件 harness 本身不能覆盖跨翻译单元身份或归档顺序。错误构建实验针对当前 GNU 工具链，不能承诺所有平台给出相同诊断文字或符号表示。
