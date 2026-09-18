# Contributor guide

欢迎修正结论、完善边界测试和新增窄范围专题。请先阅读 [内容质量标准](docs/CONTENT_STANDARD.md)、[架构](docs/ARCHITECTURE.md) 和 [批次计划](docs/ROADMAP.md)。

## 新专题流程

1. 确定一个可以在 20–60 分钟完成的主题，列前置知识、目标岗位和权威资料。先查现有标签与 related，避免重复。
2. 在对应分类目录新建 Markdown，使用稳定 kebab-case 文件名与 ID。参考已有 exemplar，不复制不相关段落凑长度。
3. 在 `examples/` 写完整 C++20 demo，含 main、正确性断言、边界输入和可观察输出。Linux 专用内容标记 platform=linux。
4. 按统一 11 节填写正文。第五节使用空 `cpp include=examples/<id>.cpp` 代码块，构建会注入真实代码；其他 C++ 块标注 runnable 或 compile-only。
5. frontmatter 添加 15–30 个 question，每层至少 5 题，稳定 question ID、参考答案和至少两个评分点。优先给反例与失败条件，避免所有答案只重复定义。
6. 核对资料、运行示例、进行内容与页面审查。记录真实发现和修复；更新 reviews 中源文件与 demo 的 SHA-256。禁止未审查而机械刷新 hash。
7. 运行所有检查，提交聚焦一个主题的 PR。PR 描述包含适用标准/平台、测试结果、已知限制、论据和审查类型。

## 内容契约

[schema.mjs](src/data/schema.mjs) 为权威定义，[topic.schema.json](schemas/topic.schema.json) 是生成的编辑器辅助。frontmatter 使用 YAML；现有文章以 JSON 对象形式书写，它是合法 YAML，可减少日期被隐式转换的歧义。手写 YAML 时日期用引号。

| 字段                        | 含义                                     |
| --------------------------- | ---------------------------------------- |
| schemaVersion / id          | schema 版本与稳定专题标识                |
| title / description         | 标题与真实范围摘要                       |
| category / areas / tags     | 一级分类、合法领域和自由 kebab-case 标签 |
| difficulty                  | 专题阅读门槛；独立于题目层级             |
| roles / companyTypes        | 适用岗位与公司类型，不是公司考题归属     |
| status                      | draft 或 published；draft 不进入网站     |
| updated / reviewed          | ISO 日期，按 UTC 记录，不写未来审查时间  |
| standard / estimatedMinutes | 标准版本和人工估计阅读时长               |
| prerequisites / related     | 人类可读前置知识与稳定关联 ID            |
| demo                        | 受控路径、平台和有实质目标的练习说明     |
| references                  | 论据资料的 title/url/kind/accessed       |
| questions                   | 独立结构化题库，来源见下                 |

draft 同样需要满足 schema 和正文契约，避免长期不可校验的半篇文档进入仓库。早期计划只写在选题 issue 或 roadmap。

## 题目来源

推导题：`source.kind=derived`，写清 rationale，companies 必须为空。可以标角色和公司类型，但不得声称某公司考过。

公开面经：`source.kind=public-interview`，必填 URL、title、accessed、note，published 和 interviewDate 字段必填但未知可为 null。note 说明原文是亲历记录、二手转载还是概括，区分实际原题与编辑者补充追问。公开面经也是未经公司确认的公开叙述，不应写成官方题库。

不复制受保护的长篇原文；简要转述题意并链接出处。不要补造具体月份、公司、轮次或岗位。仅网址存在不能证明来源真实，审查者要打开来源核对上下文。

## Markdown / MDX 与代码

优先 Markdown。需要教学组件才用 MDX；它会在构建时执行，必须作为代码审查。不要嵌入跟踪脚本、任意 iframe 或私有数据。所有 Markdown 代码围栏注明语言，代码中不隐藏平台依赖。

内部链接使用相对 `.md` / `.mdx` 路径；校验器检查源文件及锚点，构建转换为稳定 topic URL。新增 alias/改名需迁移已有链接和进度 ID，不能随意用标题重生成 ID。

验证额外 C++ 代码块的方法：

```text
围栏语言与元数据：cpp runnable
内容：完整 main，含验证断言

围栏语言与元数据：cpp compile-only
内容：可单独编译为 object 的完整翻译单元

伪代码：text，不声明为可运行 C++
```

## 本地检查与审查记录

```bash
npm ci
npm run schema
npm run format
npm run check
npm run lint
npm test
python3 scripts/test_cpp.py
python3 scripts/test_cpp.py --sanitize address
npm run build
npx playwright install chromium
npm run test:e2e
```

修改 schema 后提交生成 JSON Schema。新增或修改内容后，在更新 review hash 之前先完成技术和阅读审查。SHA-256 示例：

```bash
sha256sum content/topics/concurrency/cpp-memory-model.md examples/cpp-memory-model.cpp
```

记录 `kind=self-review` 或实际完成的其他审查类型，不虚构审查人、结论、线程 ID。hash 只是文件版本绑定，不替代人工判断。文件换行统一 LF；hash 计算前先 format，避免格式变化使审查记录失效。
