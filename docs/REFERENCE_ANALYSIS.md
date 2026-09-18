# 参考项目分析与采用决策

研究日期：2026-09-19（Asia/Shanghai）。阅读了 README、贡献指南、目录树、workflow/skill、渲染器、模板、manifest 和 review verifier，而非只看首页截图。以下判断针对固定提交，后续变更可能不同。

| 项目               | 研究快照                                                                                                | 定位                                      |
| ------------------ | ------------------------------------------------------------------------------------------------------- | ----------------------------------------- |
| ARIS-in-AI-Offer   | [b2312e5](https://github.com/wanshuiyin/ARIS-in-AI-Offer/tree/b2312e5bb108911ccd6f7e5b8b292bc196dacff0) | AI 教程与面试 cheat sheet，带渲染和审查链 |
| HFT-Interview-Prep | [8af5085](https://github.com/Unays7/HFT-Interview-Prep/tree/8af508584934f85e45c758f02d3fc662d2798f6f)   | 按能力领域组织的资源集合                  |
| quant_dev_notes    | [4628636](https://github.com/XiaoXKKK/quant_dev_notes/tree/4628636419d695eba0f70f5ae499554a8cca8115)    | 从 C++ 性能到交易架构的章节式知识体系     |

## ARIS：目录不是简单的文章文件夹

`docs/tutorials/` 将 Markdown、HTML 和 review JSON 放在相邻位置，代码位于 `docs/tutorials/code/`；`tools/` 收纳渲染、索引、代码页面生成与审查验证，`tools/templates/` 提供展示外壳，`skills/` 固化教程与渲染流程。另有博客和主页生成内容，不应把全部 HTML 都视为同一流水线的产品。[目录快照](https://github.com/wanshuiyin/ARIS-in-AI-Offer/tree/b2312e5bb108911ccd6f7e5b8b292bc196dacff0)

关键启示是区分 authoring、view、execution、review 四类产物。本项目分别放在 content、构建产物、examples、reviews，HTML 不提交，避免每次统一 UI 修改都产生整站渲染 diff。

## 内容组织与 cheat sheet 的教育目标

其教程流程把主题收窄，先规划结构、公式、实现和分层题，再围绕短答、直觉、推导、变体、复杂度与易错点展开。问题不是最后附加的装饰，而是检查正文是否支撑面试回答的验收工具；折叠答案允许主动回忆。[interview-cheatsheet workflow](https://github.com/wanshuiyin/ARIS-in-AI-Offer/blob/b2312e5bb108911ccd6f7e5b8b292bc196dacff0/skills/interview-cheatsheet/SKILL.md)

本项目吸收“短答可复述、推导可追问、代码可运行、问题分层”的设计。改为用户指定的 11 段结构、每层至少 5 题，并为每题增加独立来源和 rubric。代码重心从公式复现转为生命周期、并发正确性、系统边界和性能测量。采用内容完整性标准，不继承硬性行数目标。

## HTML 展示如何服务长文

academic 模板提供长文排版、侧栏目录、代码与表格、可折叠问答。模板还处理目录跟随、长代码折叠、图片放大与打印退化等阅读细节；dashboard 模板具有不同用途。渲染器强调 Markdown 为源、HTML 为生成视图，并记录源 hash。[模板](https://github.com/wanshuiyin/ARIS-in-AI-Offer/blob/b2312e5bb108911ccd6f7e5b8b292bc196dacff0/tools/templates/academic.html) · [渲染契约](https://github.com/wanshuiyin/ARIS-in-AI-Offer/blob/b2312e5bb108911ccd6f7e5b8b292bc196dacff0/skills/render-html/SKILL.md)

本项目采用高密度文档索引、固定知识侧栏和窄正文目录，减少介绍页与卡片层级。使用系统字体、绿色阅读强调和细分隔线，避免复制报纸式视觉和 dashboard。手机收起侧栏，保留完整正文；长代码在代码框内滚动。全站样式集中维护，搜索、标签和学习记录作为轻量增强，不阻碍静态阅读。

## Tutorial workflow 与审查 gate

参考流程将内容审查与渲染保真审查分开：前者检查论据、实现和答案，后者检查内容是否丢失、结构是否破坏及展示安全。`review.json` 保存审查结论与轨迹；render manifest 固定重建参数。贡献指南要求贡献者检查这些配套文件。[贡献指南](https://github.com/wanshuiyin/ARIS-in-AI-Offer/blob/b2312e5bb108911ccd6f7e5b8b292bc196dacff0/CONTRIBUTING_CN.md) · [manifest](https://github.com/wanshuiyin/ARIS-in-AI-Offer/blob/b2312e5bb108911ccd6f7e5b8b292bc196dacff0/tools/tutorials_render_manifest.json)

verifier 不只确认 JSON 存在，还区分陈旧源 hash、非可发布结论、未管理 HTML 等状态；严格模式可重渲染检查产物是否漂移。源码明确承认审查 sidecar 自我声明的局限，最终仍需要真实审查过程。[verify_reviews.py](https://github.com/wanshuiyin/ARIS-in-AI-Offer/blob/b2312e5bb108911ccd6f7e5b8b292bc196dacff0/tools/verify_reviews.py)

本项目首期诚实记录同一执行者的严格自审，不伪造“独立专家审查”或模型调用 ID。增加适用于 C++ 的编译、断言、sanitizer 和内存模型检查；CI 只对可机器验证部分背书。发布前同时检查源内容和桌面/移动呈现。

## 另外两份参考的价值

HFT-Interview-Prep 将 OS/底层、网络、DSA、语言与系统设计分组，兼有阅读、观看和动手资源，强调自己构建系统。这适合作为“能力覆盖检查”，不适合作为无来源的面试题原料。本项目把其资源型入口转化为依赖有序的 topic 路线和代码实验。[README](https://github.com/Unays7/HFT-Interview-Prep/blob/8af508584934f85e45c758f02d3fc662d2798f6f/README.md)

quant_dev_notes 的 SUMMARY 以 Part / chapter / subtopic 组织，从 C++ 性能、并发低延迟、性能度量到交易架构和量化方法。优点是覆盖连续、有目录深度；面试读者则还需要跨章节检索、岗位标签和可独立抽取的题目。因此保留它对工程主题的广度启发，用 category + area + topic 的扁平稳定 ID 减少深层导航负担。[SUMMARY](https://github.com/XiaoXKKK/quant_dev_notes/blob/4628636419d695eba0f70f5ae499554a8cca8115/SUMMARY.md)

## 转化矩阵

| 观察                 | 本项目采用                            | 不照搬的原因                               |
| -------------------- | ------------------------------------- | ------------------------------------------ |
| 源文档与阅读视图分离 | Markdown/MDX + 静态生成               | 大规模手工维护 HTML 容易漂移               |
| 分层问答             | 结构化 question + 折叠答案 + 作答区   | 需要复用到随机面试、搜索与 provenance 校验 |
| 可执行实现           | `.cpp` 直接嵌入、编译运行与断言       | C++ 还涉及 UB、平台系统调用和 ABI          |
| review trace         | 明确 self-review、hash 与问题修复记录 | 不伪造独立审查结论                         |
| 长文目录             | 桌面 TOC + 移动正文优先               | 工程知识库不需要演示式大图                 |
| 大量资源 / 章节      | 全领域地图、分批发布                  | 覆盖计划不等于已完成内容                   |

分析参考项目不等于运行它们的 skill。没有把参考仓库中的执行指令作为本项目的权限或发布规则。所有示范正文与代码新编写，参考链接保留用于溯源。
