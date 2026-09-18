# 后续 Topic 批量建设计划

## 原则与节奏

每批 3–5 篇，每篇独立审查和 PR。先维护术语表、前置依赖和来源清单，再写 demo；正文与题目服务验证目标，不按固定速度灌入内容。当前 5 篇示范不会被扩写成覆盖所有子领域的巨型文章。

优先级依据：是其他主题的前置知识、是否高风险易误解、能否提供可运行反例、是否与目标岗位直接相关。这里“高频”不代表公司面试统计。

## 批次计划

| 批次         | Topics                                                                     | Demo / 验证                                             | 依赖                   |
| ------------ | -------------------------------------------------------------------------- | ------------------------------------------------------- | ---------------------- |
| A · C++ 基线 | RAII/异常安全；move/值类别；对象生命周期与布局；vector 失效；模板/concepts | 资源计数、移动轨迹、失效边界、编译期约束                | 无；先统一术语         |
| B · 工具链   | ODR/链接；ABI/name mangling；vtable 与 devirtualization                    | 多翻译单元构建、符号检查、汇编对比                      | A                      |
| C · 并发     | mutex/condition_variable；SPSC 独立专题；ABA/回收；NUMA first-touch        | 丢唤醒反例说明、回绕压力、生命周期验证、绑核实验        | memory model exemplar  |
| D · 网络     | TCP framing；UDP multicast 与 sequence；io_uring；连接生命周期             | socketpair 分帧、重排/丢包注入、Linux feature detection | epoll exemplar         |
| E · 观测     | perf/flamegraph；benchmark methodology；尾延迟/背压；branch prediction     | 固定事件回放、原始分位数、可归因 A/B                    | A、C、D                |
| F · 交易链路 | Feed Handler；snapshot recovery；Matching Engine；OMS/pre-trade risk       | 事件参考模型、gap/replay、规则可配置撮合、状态机测试    | Order Book exemplar、D |
| G · 系统设计 | market-data fanout；交易网关；确定性回放与容灾                             | 容量模型、序列与恢复设计、故障场景表                    | C–F                    |
| H · 编码     | LRUCache；内存池；ring buffer；price ladder；timer wheel                   | 边界断言、复杂度、与简单 oracle 比较                    | A、B；并行于后续领域   |

## 每篇生产流程

1. **Topic brief**：用一句话描述面试问题，列学习目标、前置知识、范围外内容。
2. **Evidence map**：把关键断言对应到标准/手册/协议段落；固定适用版本。公开面经另存来源。
3. **Executable baseline**：先写最简单正确 demo，列正常、边界、失败场景与不变量。
4. **Tutorial**：按 11 节展开；性能章节先定义实验与指标，没有数据就不报数字。
5. **Question matrix**：L1 术语、L2 机制、L3 岗位权衡；15–30 题逐条检查答案与 rubric。
6. **Review**：审查同步、所有权、异常、溢出、平台与业务假设，再检查桌面/手机阅读。
7. **Release**：检查、构建、demo、sanitizer、浏览器测试通过，更新真实审查记录并合并。

允许先写 draft，但主分支不保留不可校验草稿；选择不足 15 个有价值问题的主题时应调整范围，而不是制造同义题凑数。

## 维护与复查

- 每次编译器、C++ 标准或关键库升级，重跑相关 demo，并复查受影响文字。
- 定期外链检查只验证可达性；人工复查语义变化与公开面经归属。
- 每季抽查已经发布的高风险并发和交易主题，不以新文章代替纠错。
- 优先处理 correctness issue，其次测量误导，再处理阅读体验。
- 发布约 100 篇时评估全文索引体积与移动搜索延迟，再决定是否采用分片/worker。

## 首期后的直接下一步

先做 A 批中的 RAII、move、vector 失效三篇，及 H 批的 LRUCache 作为 coding 模板。它们分别补齐资源管理、值语义、容器边界和数据结构组合，为后续内存池、回收与低延迟系统设计打基础。任何公司专项整理都需先收集可核实公开来源，再开启公司筛选项。
