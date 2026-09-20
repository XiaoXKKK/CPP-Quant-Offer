# shared_mutex 容器引用说明修正

2026-09-19（UTC），作者自审交接。按主代理指定范围修正 `content/topics/concurrency/shared-mutex.md` 的返回元素引用段落，未修改示例程序、题目或全局审查 hash。

原文混淆了 `std::map` 与 `std::unordered_map`，并把 rehash 对迭代器的影响写成对元素引用的影响。修正后明确：map 没有 rehash；unordered_map 的 rehash 使迭代器失效，但不使元素引用或指针失效；删除对应元素会使其引用失效。补充了引用有效与访问同步是两项条件，解锁后的未同步冲突读写仍可能构成 data race。

继续应用项目 cpp-quant-writing skill，保留原段关于锁内复制独立快照及引用计数、回收成本的说明。updated/reviewed 更新为 2026-09-19，新增三条真实访问的 C++20 N4861 来源：

- [associative.reqmts](https://timsong-cpp.github.io/cppwp/n4861/associative.reqmts)：关联容器插入与删除的失效规则。
- [unord.req](https://timsong-cpp.github.io/cppwp/n4861/unord.req)：rehash 不使元素指针与引用失效，erase 只影响被删除元素。
- [intro.races](https://timsong-cpp.github.io/cppwp/n4861/intro.races)：冲突访问与同步要求。

2026-09-19 已通过浏览工具打开上述页面；同时查看 map.overview 核对 map 接口。此次没有修改 C++，无需把重复运行原 demo 当作对标准失效规则的证明。主代理需在最终验收后更新正文 hash，原 demo hash 应保持不变。

已通过项目 schema、11 节顺序与内部链接检查，正文和本报告已用 Prettier 格式化，Markdownlint 当次输出 0 issues。`git diff --stat` 确认示例程序没有差异。未执行全站构建或浏览器检查。
