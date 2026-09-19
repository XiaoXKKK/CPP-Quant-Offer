# 信息架构与技术决策

## 产品边界

面向秋招准备，但不按招聘年份拆出重复文章。主题 ID 长期稳定，更新日期、C++ 标准、平台、论据和来源显式记录。第一阶段是可持续维护的知识库，不是经验帖聚合器，不展示虚构学习统计。

读者主路径：索引检索 → 30 秒回答 → 原理与实现 → 跑代码 → 独立作答 → 收藏复习。另一条路径是随机面试 → 找到薄弱问题 → 回到对应专题。已读是读者手动确认，不通过页面打开时间猜测。

## 信息层次

| 层次      | 含义                                        | 维护位置                            |
| --------- | ------------------------------------------- | ----------------------------------- |
| Category  | 稳定的一级导航，共 7 组                     | `src/data/taxonomy.mjs`             |
| Area      | 领域内的知识方向，共 21 个                  | 同上                                |
| Topic     | 可独立学习、测试、审查的窄专题              | `content/topics/<category>/<id>.md` |
| Question  | 具有独立 ID、层级、答案、评分点和来源的题目 | topic frontmatter                   |
| Demo      | 经过编译运行的可执行实验                    | `examples/<id>.cpp`                 |
| Reference | 论据来源，和面试来源分开                    | topic.references                    |

一级分类：C++ 语言与工具链；内存模型与并发；Linux 与操作系统；网络与 I/O；性能与低延迟；交易基础设施；系统设计与算法。taxonomy 包含用户要求的全部 21 个方向。没有示范文章的领域显示“待建设”，不发布空文章或占位题。

页面按知识分类、标签、难度和学习状态筛选，随机面试按专题与层级筛选。旧版岗位与来源字段暂留在内容 schema 中，以兼容已有文章与审查记录；页面不再展示岗位、公司分类。

## 目录设计

```text
content/topics/          Markdown / MDX：正文与结构化 metadata
examples/                C++20 demo：唯一可执行代码源
src/data/                taxonomy、Zod schema
src/content.config.ts    Astro content loader
src/layouts/             公共导航与阅读外壳
src/components/          问答与代码实验
src/pages/               索引、专题、地图、面试、指南、404
src/scripts/             小型渐进增强交互
src/lib/                 搜索、抽样、进度解析、内容查询
scripts/                 schema 导出、内容/链接检查、C++ 编译运行
schemas/                 生成的编辑器 JSON Schema
tests/                   逻辑测试与 Playwright 浏览器测试
reviews/                 真实自审记录与内容/示例 hash
docs/                    架构、质量、贡献、计划与验证文档
.github/workflows/       CI、部署、定期外链检查
```

内容跟分类存储，URL 只依赖稳定 topic ID：`/topics/cpp-memory-model/`。改分类不改 URL；改 ID 必须做迁移规划，因为进度、关联和问题链接使用 ID。问题 ID 也不因重新排序而重编号。

## 为什么采用 Astro 静态站点

专题是长文，内容应在构建阶段渲染为 HTML，首屏不等待客户端框架。Astro 的内容集合负责加载 Markdown/MDX，Zod 提供统一契约，Shiki 在构建时高亮代码。交互仅用小型 TypeScript 模块，不引入整个 SPA 与客户端路由。

使用官方 [content collections](https://docs.astro.build/en/guides/content-collections/) 和 [GitHub Pages](https://docs.astro.build/en/guides/deploy/github/) 部署机制。Pages 的子目录 base 贯穿所有页面、资源、内部专题链接；构建后再次检查最终 HTML 的文件与 fragment。页面刷新不依赖服务器 fallback。

相比每篇都提交手写 HTML，这使 UI 变更统一，源文件与展示不会形成双份编辑。锁文件固定依赖，HTML 只作为 Actions artifact 部署。

## 内容编译链

```text
Markdown / MDX + schema + taxonomy
             ↓ metadata / section / provenance / internal-link validation
tested .cpp → remark include → syntax highlight → static HTML
             ↓ browser tests + output-link validation
                     Pages artifact
```

代码块 `cpp include=examples/<id>.cpp` 必须为空，构建时读取唯一代码源。额外 C++ 片段需声明 `runnable` 或 `compile-only`，C++ 测试器自动提取。伪代码使用 `text`，不把不可编译片段冒充 runnable C++。

每个专题 11 个 H2 严格有序；题目数据只保存一份，文章和随机模式共享 Questions 数据。MDX 允许复用 Astro 组件，但属于可执行代码，贡献者变更必须按代码审查，绝不接受浏览器上传的 MDX。

## 搜索、筛选与规模

初期把发布专题的标题、正文、metadata 和题库嵌入索引页，经过 JSON `<` 转义；浏览器使用 Unicode NFKC、大小写归一化、空格分词和子串 AND 匹配。这对中英混合技术词可靠，且不依赖远程搜索服务。结果显示相关正文/答案片段，筛选状态写入 URL。

这是 O(总文本量) 的小规模基线，不宣称百兆文档也即时。超过约 100 篇或索引压缩前 2 MB 时，先测低端手机输入延迟，再迁移到分片索引、worker 或支持中文 token 的搜索引擎。搜索 API 边界在 `matchesTopic`，索引内容可由构建步骤迁移而不改变 authoring schema。

## 学习数据

版本化 localStorage 键保存 read/saved/drafts/code。只有点击“运行”才把代码发送到 godbolt.org；本站没有服务端账号和远端学习数据。存储失败退化为内存，并明确提示导出备份。导入验证版本、类型、ID 和体积；合并集合，草稿冲突保留当前设备值。

跨页使用同一存储；跨 tab 收到 storage 事件刷新状态按钮，已打开的编辑框不会被外部事件覆盖。完整跨设备自动同步不属于首期，导入/导出提供可用的迁移路径。

随机面试用 Fisher–Yates 洗牌后的题目 deck，同轮无重复；筛选改变要求重新开始。最后一题可查看答案，再结束本轮。自测不提供虚假的自动评分，只给人工核对的 rubric。

## 在线代码练习边界

CodeLAB 使用原生 dialog 打开占满视口的工作区。桌面左侧显示练习说明与高亮原始示例，右侧显示 Monaco 编辑器和运行结果；中间分隔条支持拖动与方向键。移动端改为上下布局。关闭后返回文章，并保留草稿和编辑状态。

Monaco 与 editor worker 由 Vite 本地打包，打开 CodeLAB 时才加载，不依赖 CDN。支持 C++ 语法高亮、缩进、括号匹配、查找替换、注释与折叠；当前没有接入 clangd，因此不提供完整 C++ 语义诊断或跨文件补全。加载失败时保留 textarea 供编辑、保存和运行。

参考 [Compiler Explorer API](https://github.com/compiler-explorer/compiler-explorer/blob/main/docs/API.md)，选择公开 GCC 13.2 编译器、C++20 和 pthread，按明确按钮请求远程编译执行；30 秒超时后可重试。网络/限流/系统调用限制有错误提示，原文保留，并提供复制到 CE 或本地运行的路径。

不是本地沙箱，也不承诺远程持续可用。E2E 对成功和失败路径使用协议级 mock，真实外部连通性另行记录；不能把 mock 通过描述为远程服务实测。

## CI 与维护

PR 和 main 都运行内容、Markdown、类型、JS lint、格式、逻辑、构建、产物链接和桌面/移动浏览器测试。Linux GCC 与 Clang 编译并运行示例，另运行 ASan/UBSan。所有 jobs 通过才允许部署，PR 没有 Pages 写权限。

内容自审记录绑定源文档和 demo 的 SHA-256：改变任一内容后须重新审查并更新记录，不能自动盖章。hash 能发现漂移，不能证明审查诚实或论据正确。外部链接检查独立定期执行，外站临时 429/403 不应阻断纯本地构建。

将来新增专题不必改页面组件；新增问题来源类型需要 schema、UI、测试和贡献指南一起升级。schemaVersion 升级需附迁移脚本；进度 version 单独管理。
