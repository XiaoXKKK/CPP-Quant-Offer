# 验证记录与边界

首期验证对象：5 篇教程、75 道问答、5 个 C++20 示例，以及静态网站和维护工具。

## 可重复运行

```bash
npm run verify
python3 scripts/test_cpp.py
python3 scripts/test_cpp.py --sanitize address
npm run test:e2e
```

本地 Windows 使用 WSL Ubuntu 运行 Linux C++ 测试。CI 在 Ubuntu 上分别使用 GCC 与 Clang，并执行 ASan/UBSan；浏览器测试包含桌面 Chromium 与移动视口 Chromium，移动测试不等于 Safari 真机认证。

## 检查覆盖

schema 与负例测试检查字段、来源、公司归属、层级数量、ID、分类及 demo 路径；AST 校验章节、内部链接与片段。构建后检查实际 HTML 中的站内 URL、资源与 fragment，包含 Pages 子路径。

Playwright 检查全文搜索、组合筛选、空状态、URL 保留、已读与收藏持久化、答案隐藏、草稿保存、不重复随机题轮、代码练习成功/失败路径、移动布局与存储错误。Compiler Explorer 的自动回归采用响应 mock，避免外站抖动阻断 CI。

C++ 测试检查队列空满与回绕、跨线程顺序、快照一致性、epoll 受控 LT/ET 行为和 EOF、订单簿删除/溢出等边界。false sharing 示例只验证计数，不断言固定性能差异。

## 不代表的保证

- 测试通过不是并发算法的全执行证明，内存序仍需同步链论证。
- ASan/UBSan 不覆盖所有线程竞态；TSan 运行受环境与地址布局影响，另行报告，不伪称已通过。
- 共享机器的计时不是生产 benchmark，没有发布纳秒级延迟或固定加速数字。
- 外链可达不能证明其内容真实；公开面经必须人工核对归属。
- 审查记录明确为同一执行者 self-review，不冒充独立审查。

最终执行与问题修复记录见 [首期自审报告](REVIEW.md)。
