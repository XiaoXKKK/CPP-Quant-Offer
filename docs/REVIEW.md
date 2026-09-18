# 首期严格自审

审查类型：实现者自审（self-review），不是独立第三方审查。

日期：2026-09-18 UTC（北京时间 2026-09-19）。范围包括 5 篇完整正文、75 个问题和答案、5 个 C++ 实验、schema/校验器、浏览器状态、Pages 路由、构建和 CI。先运行测试与构建，再检查契约与实现，修复后回归。源文件与 demo hash 见 [内容审查记录](../reviews/content-review.json)。

## 发现并修复

| 严重度 | 问题                                                                                                    | 修复与验证                                                                                                |
| ------ | ------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| P1     | CE executor 请求的真实响应把运行结果放在顶层，初版只识别嵌套 execResult，可能把运行退出码当成编译退出码 | 分别解析 buildResult 与 execution，增加两种返回格式及非零退出码回归；真实浏览器远程执行 shared_mutex 成功 |
| P1     | 手机导航沿用桌面 align-self，菜单实际从视口顶端开始，首项被顶栏挡住                                     | 改为显式动态视口高度与独立定位；手机测试点击首项验证                                                      |
| P2     | 全文索引只覆盖 Markdown，构建嵌入的 C++ 原文没有进入搜索                                                | 把同一 canonical `.cpp` 加入索引；搜索 epoll_create1 只命中 epoll                                         |
| P2     | 导入成功提示覆盖移动菜单底部的导出按钮                                                                  | 提示不拦截指针且自动消退；完整导入合并/导出 round-trip 测试                                               |
| P2     | reviewed/updated 关系校验方向错误，允许更新后仍显示过期审查                                             | 发布内容必须在更新当日或之后审查，并禁止未来日期；hash 另校验精确内容                                     |
| P2     | C++ 围栏别名或非标准围栏可能绕过 Python 代码块提取                                                      | 限定可编译 C++ 围栏规范，不符合则阻断；统一源 include 严格检查                                            |
| P2     | 引用式 Markdown link 没进入源链接检查与重写                                                             | 同时处理 linkReference/definition，构建产物再次检查真实目标                                               |
| P2     | 可复用页面中的难度文字和题目数量被写死                                                                  | 由 schema 字段和 collection 实时生成                                                                      |
| P2     | 宽泛 TS include 扫入 Playwright 的生成报告                                                              | 显式排除产物和本地实验目录，恢复干净类型检查                                                              |
| P2     | 新依赖组合含有已知 smol-toml DoS 问题，并存在 Node engine 不匹配                                        | 升级到已修复版本，声明匹配的 Node 基线，锁定实际依赖；npm audit 无漏洞                                    |
| P3     | Order Book 依赖传递包含取得 std::prev                                                                   | 显式包含 iterator；正常与 sanitizer 编译运行                                                              |
| P3     | 首版 30 秒短答偏长                                                                                      | 精简为机制、边界和处理策略，详细论证移交后文                                                              |

## 内容逐项复核

- **epoll**：受控 LT/ET 实验固定无并发消费者和无新写入；没有宣称 ET 永远只通知一次。EOF/EAGAIN、ONESHOT、fd 复用、预算和用户态 ready queue 均区分清楚。示例用 UNIX stream socket，不伪称 TCP 完整 reactor。
- **memory model**：逐槽位检查 producer 发布 head 与 consumer 回收 tail 两条同步链。保留单生产者/单消费者前提；原子是否 lock-free 和外层循环进展分开；没有声称压力测试能证明所有执行。
- **shared_mutex**：不承诺公平性，不把释放后加写锁叫作原子升级。快照锁内复制，条件更新在独占区重验。短临界区成本与引用计数回收有明确限制。
- **Order Book**：add/reduce/cancel 同时维护订单、索引、档位总量与空档删除；数字使用整数与溢出检查；默认复制禁止。修正“有哈希索引就全路径 O(1)”的误解，明确未实现撮合与 gap recovery。
- **false sharing**：原子确保计数合法，64 是实验参数；没有速度断言。区分真实通信、布局争用与 NUMA，说明计时包含线程收尾，不能把共享沙箱结果视为生产基准。
- **75 题**：每层 5 题/专题，答案有评分点，全部 derived，companies 全为空；没有混入无法核实的公司真题。

## 已执行的本地验证

- schema、章节、链接、代码围栏、来源与审查 hash 检查。
- Astro / TypeScript：0 errors、0 warnings、0 hints；ESLint、Markdownlint、Prettier。
- 8 项逻辑/负例测试；18 项桌面及移动视口浏览器测试。
- 5 个 C++20 示例：WSL Ubuntu，GCC 13.3，正常运行与 ASan/UBSan。
- 10 个静态 HTML 页面及 JSON Schema endpoint；默认 Pages 子路径和根路径构建/内部链接检查。
- 临时 MDX fixture：表达式、C++ include、内部链接均完成真实构建后移除。
- Compiler Explorer：真实 REST 与浏览器调用均测试；编译 0、运行 0，打印 consistent snapshots。
- 桌面 1440px 与手机 390px 截图检查，所有主题页面无横向溢出（代码/表格容器可独立滚动）。

CI 另配置 GCC/Clang 两种编译器及 sanitizer，最终远程执行以 GitHub Actions 结果为准。没有把本地只运行 GCC 的事实写成已运行 Clang。

## 明确保留的边界

没有独立第三方内容认证或真实 Safari 真机测试；没有完成 TSan、弱内存硬件或性能基准实验。测试覆盖不是并发正确性证明。SPSC 是 int 单生产者/单消费者教学实现，簿是内存内 MBO 子集；外部编译器和外链可用性不由本站控制。

本地 Node 外链检查中，14 个资料正常返回，GCC 原始源码因本机 Node 网络路径失败；同一地址通过 PowerShell 实测 HTTP 200。该例外明确记录，不能声称外链检查全绿。外链任务与核心构建分开，在 GitHub 网络环境定期复查。

后续优先补充独立技术审查、ARM64/TSan 验证和可回放的订单簿参考模型，再据真实负载开展性能实验；计划详见 [ROADMAP](ROADMAP.md)。
