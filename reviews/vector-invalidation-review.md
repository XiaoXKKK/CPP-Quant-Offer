# vector 失效专题自审交接

- 日期：2026-09-19（UTC 日期）。
- 类型：作者自审；独立验收由主 agent 完成，本记录不代替独立验收。
- 范围：`content/topics/cpp/vector-invalidation.md`、`examples/vector-invalidation.cpp`；另按主 agent 授权维护 `tests/e2e/site.spec.ts` 的内容扩展断言。
- 结论：正文、题目和独立 demo 自审通过；页面视觉验收与全站构建由主 agent 汇总。

## 内容与证据

已读取项目写作技能、写作约定、质量标准、贡献指南、schema、taxonomy 和 shared_mutex 示范。正文有 11 个规定二级标题，18 道题分为 L1/L2/L3 各 6 题，全部 `derived`，`companies` 为空。题目按具体知识点编写，未新增岗位或公司筛选，也未在正文重复题目来源声明。

2026-09-19 实际打开并阅读以下资料；frontmatter 包含对应链接和访问日期：

| 资料                                                                                                                                                   | 核对内容                                                                  |
| ------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------- |
| [N4861 vector.capacity](https://timsong-cpp.github.io/cppwp/n4861/vector.capacity)                                                                     | reserve 的重新分配条件；resize 的增删效果；shrink_to_fit 非强制；异常例外 |
| [N4861 vector.modifiers](https://timsong-cpp.github.io/cppwp/n4861/vector.modifiers)                                                                   | 插入与删除的失效范围；旧 end；尾部单元素插入的异常保证；复杂度            |
| [N4861 vector.overview](https://timsong-cpp.github.io/cppwp/n4861/vector.overview)                                                                     | 连续容器条件；尾部追加的摊还复杂度                                        |
| [N4861 stmt.ranged](https://timsong-cpp.github.io/cppwp/n4861/stmt.ranged)                                                                             | range-for 在循环前保存 end                                                |
| [N4861 views.span](https://timsong-cpp.github.io/cppwp/n4861/views.span)                                                                               | 非拥有视图的指针与长度                                                    |
| [N4861 vector.bool](https://timsong-cpp.github.io/cppwp/n4861/vector.bool)                                                                             | bool 特化的代理引用与存储差异                                             |
| [N4861 sequence.reqmts](https://timsong-cpp.github.io/cppwp/n4861/sequence.reqmts)                                                                     | erase 合法范围、返回值及 clear 的元素生命周期                             |
| [N4861 vector.erasure](https://timsong-cpp.github.io/cppwp/n4861/vector.erasure) 与 [alg.remove](https://timsong-cpp.github.io/cppwp/n4861/alg.remove) | erase_if 的 remove_if 加尾部 erase 路径及保序性                           |
| [N4861 unique.ptr.single.ctor](https://timsong-cpp.github.io/cppwp/n4861/unique.ptr.single.ctor)                                                       | 拥有者移动后保存同一对象指针                                              |
| [GCC 13.3 stl_vector.h](https://raw.githubusercontent.com/gcc-mirror/gcc/releases/gcc-13.3.0/libstdc++-v3/include/bits/stl_vector.h)                   | 三个边界成员和 _M_check_len；只作为具体实现说明                           |

自审重点：

- 表格区分 `reserve <= capacity`、等长 resize、缩小 resize、未扩容追加和中间修改；未把“未扩容”写成所有迭代器有效。
- 异常段区分 reserve/resize 的抛异常移动例外、末尾单个插入的条件保证、中间插入与 erase 的赋值异常。未把未指定效果等同 UB。
- 未宣称固定增长倍数，未把 span 长度或索引视为业务身份，未把预留容量当成线程安全与批次上限。
- 18 个答案逐一对应正文与资料；L3 覆盖借用期限、性能实验、对象地址、容量溢出、工具验证和批量删除。
- 写作复核移除模板式开头、夸大收尾和机械反转；保留纠正真实 API 混淆所需的对照。没有个人经历、公司归属或性能数字。

## 代码验证

环境为 Windows 宿主上的 WSL，GCC `g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`。二进制独立写入 `/tmp/cpp-quant-vector-review`，没有调用全量 `scripts/test_cpp.py`，没有与其他 agent 共享测试输出目录。

以下三种构建均实际编译并运行成功，每次输出 `vector boundary checks passed`：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/vector-invalidation.cpp -o /tmp/cpp-quant-vector-review/vector-demo
/tmp/cpp-quant-vector-review/vector-demo
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fno-omit-frame-pointer -fsanitize=address,undefined -fno-pie -no-pie examples/vector-invalidation.cpp -o /tmp/cpp-quant-vector-review/vector-san
/tmp/cpp-quant-vector-review/vector-san
g++ -std=c++20 -O0 -g -Wall -Wextra -Wpedantic -Werror -D_GLIBCXX_DEBUG examples/vector-invalidation.cpp -o /tmp/cpp-quant-vector-review/vector-debug
/tmp/cpp-quant-vector-review/vector-debug
```

执行时源文件使用对应 `/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/` 绝对路径。断言覆盖：容量不变操作后的 end、未扩容追加保留的前缀和 span、强制 reserve 扩容后重新获取句柄、resize 缩小/相等/增长/清空、插入/删除返回值、空范围删除、空输入/全删除/全保留/连续删除、追加时只遍历原始元素、构造失败无影响、超 max_size 的 length_error、允许不收缩的 shrink_to_fit。

失效句柄从未被解引用、比较、递增或用于打印。正常运行和 sanitizer 只构成有限测试证据，不能证明任意类型、分配器和并发使用正确。

## 内容与测试文件检查

- `readTopics()` 配合 `validateTopics()` 对本专题检查通过：11 节、18 题、schema、demo 路径、内部链接和 related。
- `npx prettier --write content/topics/cpp/vector-invalidation.md tests/e2e/site.spec.ts` 完成格式化。
- `npx markdownlint-cli2 content/topics/cpp/vector-invalidation.md` 通过；仓库配置同时纳入其他 Markdown，实际报告 0 issues。
- `npx eslint tests/e2e/site.spec.ts` 通过。
- `npx astro check` 通过：37 files，0 errors，0 warnings，0 hints。

`site.spec.ts` 保留全文检索、demo 检索、组合筛选、URL 恢复和空态检查。专题总数来自 `readTopics()` 中的 published 集合；搜索预期来自实际页面索引的可搜索字段，比较完整匹配 ID 集合，并要求 epoll 仍命中。L1 根据真实元数据检查结果，固定不存在的查询负责独立验证空态。为 TypeScript 提供了明确的数据结构类型，没有用 `any` 或关闭检查。

交接时尚未运行修改后的浏览器用例；旧 dist 与新专题数量不同，等待主 agent 刷新全站构建后进行验收。未修改共享 `reviews/content-review.json`、roadmap、技能文件或其他 agent 的专题；未创建提交或 PR。

## 文件版本

正文与 demo 在格式化和自审后的 SHA-256：

```text
00a545efcbc66ade3315fc39d9ff46c6d2896e13280a6d66b91475eb824cc7d2  content/topics/cpp/vector-invalidation.md
5e906c061e6da8bb4c7259e5ef10ed0b3f33be8ecc0b775558d2dcdc1a68cdfa  examples/vector-invalidation.cpp
```

主 agent 应独立验收后更新共享 hash；后续正文或 demo 若变化，应重新核对本记录。
