# CPP-Quant-Offer

面向 C++ Developer、Quant Developer、Low-Latency C++ Developer 与 Trading Infrastructure Engineer 的长期知识库。

[在线阅读](https://xiaoxkkk.github.io/CPP-Quant-Offer/) · [信息架构](docs/ARCHITECTURE.md) · [参考项目分析](docs/REFERENCE_ANALYSIS.md) · [贡献指南](CONTRIBUTING.md) · [质量标准](docs/CONTENT_STANDARD.md) · [后续计划](docs/ROADMAP.md)

首期发布 5 篇示范教程、75 道分层问答和 5 个可执行 C++20 示例。题目均为根据岗位知识推导的练习题；没有虚构的公司真题。

## 内容入口

| 专题                                                                               | 核心问题                       | 示例平台 |
| ---------------------------------------------------------------------------------- | ------------------------------ | -------- |
| [epoll LT vs ET](content/topics/network/epoll-lt-et.md)                            | 就绪、EAGAIN、预算与公平性     | Linux    |
| [C++ memory model](content/topics/concurrency/cpp-memory-model.md)                 | 发布与复用的双向同步           | C++20    |
| [shared_mutex](content/topics/concurrency/shared-mutex.md)                         | 读写所有权、升级、快照生命周期 | C++20    |
| [Order Book](content/topics/trading/order-book.md)                                 | MBO、不变量、ID 索引与恢复边界 | C++20    |
| [CPU cache / false sharing](content/topics/performance/cpu-cache-false-sharing.md) | 一致性争用、布局与实验控制     | C++20    |

每篇包含短答、概念、原理、内部实现、runnable demo、追问、误区、性能、交易场景、关联专题与 L1/L2/L3 问答。Markdown 中的 `cpp include=...` 在构建时嵌入对应 `.cpp`，GitHub 阅读时可直接打开 frontmatter 中的 demo 文件；浏览器中显示完整高亮代码。

## 本地开发

需要 Node.js 24.16+（或 22.22.3+ 的 Node 22；CI 使用 Node 24）、npm、Python 3；C++ 测试需要 Linux GCC 或 Clang。Windows 使用 WSL 运行 Linux 示例。

```bash
npm ci
npm run dev
```

打开终端显示的地址，默认路径是 `/CPP-Quant-Offer/`。

```bash
npm run verify
npm run test:cpp
npx playwright install chromium
npm run test:e2e
```

在 Windows 的仓库根目录执行 C++ 测试：

```powershell
wsl -d Ubuntu -- bash -lc 'cd /mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer && python3 scripts/test_cpp.py'
```

请按你的实际目录替换示例路径。不要在测试中设置 NDEBUG；示例使用断言核对不变量。更完整的测试说明见 [验证与限制](docs/VALIDATION.md)。

## 网站功能

- 静态生成的专题页，无 JavaScript 也能读正文、展开答案。
- 中文/英文全文搜索，覆盖正文、问题和参考答案；多词 AND 匹配。
- 分类、tag、难度、岗位、公司类型和有来源的面经公司筛选，URL 可分享。
- 已读、收藏、作答与代码草稿本地保存，JSON 导入/导出迁移。
- 随机面试采用本轮不重复的抽样，切换题目后答案重新隐藏。
- 代码实验区可编辑并调用 Compiler Explorer；不可用时提供复制与本地运行路径。
- 桌面阅读目录、移动端导航、键盘搜索快捷键 `/`。

具体公司标签只接受可核实的公开面经。公司类型是适用岗位信息，不能推断该公司曾考过题目。本地记录不跨设备自动同步；清除浏览器数据会清除记录。

## GitHub Pages

仓库 Settings → Pages → Source 选择 GitHub Actions。主分支通过所有检查后，工作流上传 `dist/` 并部署。首次启用可使用：

```bash
gh api --method POST repos/OWNER/CPP-Quant-Offer/pages -f build_type=workflow
```

默认部署配置为 `https://xiaoxkkk.github.io/CPP-Quant-Offer/`。Fork 改名时，在仓库 Actions Variables 设置 `SITE_URL`（例如 `https://alice.github.io`）和 `BASE_PATH`（例如 `/my-repo`）。自定义域名根路径使用 `/`。所有资源、链接和搜索筛选都不依赖 SPA fallback。

## 维护约定

Markdown/MDX 和 C++ 是源文件；生成 HTML 不提交。schema 是 [唯一内容契约](src/data/schema.mjs)，[JSON Schema](schemas/topic.schema.json) 从它导出，跨字段检查仍由运行时校验完成。技术自审结论与文件 hash 记录在 [reviews](reviews/content-review.json)，其真实性依赖审查过程，不是独立认证。

灵感来自 [ARIS-in-AI-Offer](https://github.com/wanshuiyin/ARIS-in-AI-Offer)、[HFT-Interview-Prep](https://github.com/Unays7/HFT-Interview-Prep) 和 [quant_dev_notes](https://github.com/XiaoXKKK/quant_dev_notes)。本文与示例为新编写，未复制其 UI 或教程正文。代码与原创内容采用 [MIT License](LICENSE)。
