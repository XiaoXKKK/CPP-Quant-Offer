# 模板与 concepts 专题自审交接

- 日期：2026-09-19（UTC 日期）。
- 类型：作者自审；主 agent 另行完成独立验收。
- 范围：`content/topics/cpp/templates-concepts.md`、`examples/templates-concepts.cpp`。
- 自审结论：正文、题目与独立 demo 通过；全站构建和页面验收由主 agent 汇总。

## 范围与写作

应用了项目 `cpp-quant-writing`、写作约定、质量标准与贡献指南。专题聚焦 C++20 实例化、requires、SFINAE 直接上下文、约束短路和 subsumption，没有扩展为模板技巧汇编。

正文包含规定的 11 节，18 道题按 L1/L2/L3 各 6 道分布，全部 `derived`，`companies` 均为空。L3 覆盖编译成本、类型擦除、最小接口要求、兼容性、负例测试和行情数据语义。题目逐项核对了正文依据与评分点，未新增岗位/公司界面或重复来源声明。

自然语言复核删除模板式开场和笼统收益承诺。保留的对照用于区分 requires 两种语法、表达式合法与结果为真、接口约束与运行时值，以及声明可调用与函数体可实例化。

## 资料核查

2026-09-19 实际打开并阅读了 frontmatter 中的全部资料：

| 资料                                                                                     | 核对内容                                                            |
| ---------------------------------------------------------------------------------------- | ------------------------------------------------------------------- |
| [N4861 expr.prim.req](https://timsong-cpp.github.io/cppwp/n4861/expr.prim.req)           | 四类需求；词法顺序；局部参数；模板外硬错误和 IFNDR；结果的 decltype |
| [N4861 temp.inst](https://timsong-cpp.github.io/cppwp/n4861/temp.inst)                   | 声明与定义的实例化边界，类成员并非一并实例化                        |
| [N4861 temp.deduct](https://timsong-cpp.github.io/cppwp/n4861/temp.deduct)               | 直接上下文与实例化副作用，SFINAE 不覆盖任意函数体错误               |
| [N4861 temp.res](https://timsong-cpp.github.io/cppwp/n4861/temp.res)                     | 依赖与非依赖名称，未实例化模板也有合法性要求                        |
| [N4861 temp.constr.op](https://timsong-cpp.github.io/cppwp/n4861/temp.constr.op)         | 约束合取与析取的检查顺序                                            |
| [N4861 temp.constr.atomic](https://timsong-cpp.github.io/cppwp/n4861/temp.constr.atomic) | 原子约束的表达式来源和参数映射                                      |
| [N4861 temp.constr.normal](https://timsong-cpp.github.io/cppwp/n4861/temp.constr.normal) | concept 展开和非法参数映射，短路不能补救规范化错误                  |
| [N4861 temp.constr.order](https://timsong-cpp.github.io/cppwp/n4861/temp.constr.order)   | subsumption 按相同原子及逻辑结构排序                                |
| [N4861 concepts.lang](https://timsong-cpp.github.io/cppwp/n4861/concepts.lang)           | same_as、convertible_to 与整数概念                                  |
| [N4861 concepts.equality](https://timsong-cpp.github.io/cppwp/n4861/concepts.equality)   | 编译期 satisfies 与履行语义 models 的区别                           |
| [N4861 stmt.if](https://timsong-cpp.github.io/cppwp/n4861/stmt.if)                       | constexpr if 与丢弃分支规则                                         |
| [GCC Developer Options](https://gcc.gnu.org/onlinedocs/gcc/Developer-Options.html)       | -ftime-report 的用途；未编造性能测量                                |

补充打开了 [N4861 temp.class.spec.match](https://timsong-cpp.github.io/cppwp/n4861/temp.class.spec.match) 核对检测 trait 的偏特化匹配，以及 [temp.concept](https://timsong-cpp.github.io/cppwp/n4861/temp.concept) 核对 concept 定义。没有把访问失败的 `concepts.requirements` 或 GCC 13.3 文档路径列作已核查来源；前者未用于结论，GCC 工具参数改由可访问的官方文档核查。

## 技术自审

- `QuoteLike` 描述嵌套类型、const 调用和精确结果类型。它不证明价格范围、单位、读操作纯净性或快照一致性。
- `NoThrowQuote` 在共享的 `QuoteLike` 原子基础上增加 noexcept 需求，重载选择正反结果均用 static_assert 验证。
- 简单需求里写布尔 trait 只检查表达式可形成；嵌套需求才检查条件。`IntegralSpelling<double>` 和 `ReallyIntegral<double>` 给出对照。
- 未把 requires 说成所有上下文下的错误屏蔽机制；正文区分模板外错误、函数体实例化错误、约束不满足及 IFNDR。
- `body_checked_later` 显式返回 int，保证正常模式只检查声明；失败模式实际实例化函数体。正文同时解释 auto 返回推导可能改变检查时机。
- `checked_spread` 没有 noexcept 声明，潜在抛出 getter 的异常按普通调用传播。无异常约束不推导出不分配、不阻塞或固定时延。
- 本例价格范围为教学限制，先检查范围和顺序再做减法；三条固定样本的累加有界。没有给一般金融产品附加非负价格假设。

## 实际验证

环境：Windows 宿主上的 WSL，GCC `13.3.0`。测试二进制使用独立目录 `/tmp/cpp-quant-concepts-review`，未调用全量 `scripts/test_cpp.py`。

正常构建与运行成功：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/templates-concepts.cpp -o /tmp/cpp-quant-concepts-review/demo
/tmp/cpp-quant-concepts-review/demo
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fno-omit-frame-pointer -fsanitize=address,undefined -fno-pie -no-pie examples/templates-concepts.cpp -o /tmp/cpp-quant-concepts-review/demo-san
/tmp/cpp-quant-concepts-review/demo-san
```

实际编译时源文件使用 `/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/` 下的绝对路径。两种成功运行的输出均为 `concept checks passed; total spread=3`。ASan/UBSan 无报告。一次 sanitizer 链接因临时输出目录不存在而失败，重新创建该独立目录后编译与运行通过，未将链接失败算作测试通过。

四个宏分别使用 `-std=c++20 -Wall -Wextra -Wpedantic -Werror -fsyntax-only -D宏名` 独立编译，均按预期失败，且逐条阅读了诊断：

| 宏                           | 实际诊断中的失败位置或原因                                                                |
| ---------------------------- | ----------------------------------------------------------------------------------------- |
| `CONCEPTS_FAIL_CONSTRAINT`   | `no matching function for call to checked_spread(MissingAsk)`，候选有 `QuoteLike<T>` 约束 |
| `CONCEPTS_FAIL_SUBSUMPTION`  | `call of overloaded ambiguous(int) is ambiguous`，列出两个候选                            |
| `CONCEPTS_FAIL_NON_TEMPLATE` | 对 `int` 请求成员 `bid_ticks`                                                             |
| `CONCEPTS_FAIL_BODY`         | 实例化 `body_checked_later<int>` 时，对 `int` 请求成员 `missing`                          |

预期失败没有用“任意非零退出”代替原因核对；四种错误均要求诊断，未使用 IFNDR 构造可移植拒绝测试。正常运行的 static_assert 不执行这些失败分支。代码断言还覆盖缺 getter、错误价格类型、仅可写成员、noexcept 差异、范围外和倒挂输入。

## 结构与交接

- `readTopics()` 与 `validateTopics()` 检查本专题通过：11 节、18 题、每层 6 题、schema、demo、内部链接和 related。
- Prettier 已格式化本专题正文。
- `npx markdownlint-cli2 content/topics/cpp/templates-concepts.md` 通过；按仓库配置同时检查到 28 个 Markdown 文件，0 issues。
- 没有修改前一章、E2E、共享 review manifest、roadmap 或其他 agent 的文件，未创建提交或 PR。
- 全站构建、浏览器和最终发布 hash 由主 agent 验收后统一处理，本记录只声称上述实际执行的检查。

格式化后版本：

```text
3cc0a36d9039a64b65057fff5cf38d0b97fd3df53812e2022aafba3f09898004  content/topics/cpp/templates-concepts.md
e803b5e9d843adbf0fd6aa3615f541db6eae1e44b1f0c3882f9bddf9a4306df9  examples/templates-concepts.cpp
```
