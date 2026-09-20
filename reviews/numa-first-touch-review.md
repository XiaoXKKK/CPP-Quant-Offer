# NUMA 与 first-touch 专题自审

日期：2026-09-19。类型：作者自审。主 agent 已通读正文、15 题和示例，并独立复跑 ASan/UBSan。本文件记录作者核查，不替代主 agent 的发布 hash。

## 交付与分类

- `content/topics/performance/numa-first-touch.md`：正文 11 节，15 题，L1/L2/L3 各 5 题。
- `examples/numa-first-touch.cpp`：Linux 单文件示例，动态 affinity mask、首次写匿名私有映射、只读页节点查询、恢复和清理。
- 本文件：资料、边界与实际验证记录。

taxonomy 将 CPU Cache / NUMA 列在 performance 下，因此正文使用该分类，已向主 agent 告知目录调整。demo 标为 linux，没有伪装成纯标准 C++ 或浏览器通用程序。

已应用项目 `cpp-quant-writing` skill 和写作、质量规范，复核是否遗漏平台条件、是否从一次观察夸大性能、是否机械重复结论。没有修改前面已验收章节、共享 manifest、roadmap、skill 或测试脚本，没有提交 commit。

## 来源核查

正文 15 条 references 均已实际打开。Linux man-pages 页面显示版本 6.19；内核文档使用访问当日内容。运行内核单独记录，没有把文档版本当成本机版本。

| 结论                                             | 核查资料                                                                                                                                                             |
| ------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| NUMA、节点统计及 numa_maps 的地址关联            | [numa(7)](https://man7.org/linux/man-pages/man7/numa.7.html)                                                                                                         |
| CPU 拓扑导出                                     | [kernel CPU topology](https://docs.kernel.org/admin-guide/cputopology.html)                                                                                          |
| 每线程 affinity、允许集合、返回值与动态 CPU mask | [sched_setaffinity(2)](https://man7.org/linux/man-pages/man2/sched_setaffinity.2.html)、[CPU_SET(3)](https://man7.org/linux/man-pages/man3/CPU_SET.3.html)           |
| 系统、线程与范围策略、许可和回退                 | [NUMA memory policy](https://docs.kernel.org/admin-guide/mm/numa_memory_policy.html)、[set_mempolicy(2)](https://man7.org/linux/man-pages/man2/set_mempolicy.2.html) |
| 匿名映射、零页、写入和 COW、已有页默认不迁移     | [mmap(2)](https://man7.org/linux/man-pages/man2/mmap.2.html)、[mbind(2)](https://man7.org/linux/man-pages/man2/mbind.2.html)                                         |
| cpuset 请求资源与 effective 许可                 | [cgroup v2](https://docs.kernel.org/admin-guide/cgroup-v2.html)                                                                                                      |
| 自动 NUMA balancing 的采样与迁移                 | [kernel numa_balancing](https://docs.kernel.org/admin-guide/sysctl/kernel.html#numa-balancing)                                                                       |
| THP 粒度、零页与局部 NOHUGEPAGE 建议             | [THP](https://docs.kernel.org/admin-guide/mm/transhuge.html)、[madvise(2)](https://man7.org/linux/man-pages/man2/madvise.2.html)                                     |
| nodes 为空时查询、不迁移、整体与逐页错误         | [move_pages(2)](https://man7.org/linux/man-pages/man2/move_pages.2.html)                                                                                             |
| 外部 CPU 和内存策略复现步骤                      | [numactl(8)](https://man7.org/linux/man-pages/man8/numactl.8.html)                                                                                                   |
| new-expression 与操作系统 NUMA 放置不同层次      | [C++20 N4861 expr.new](https://timsong-cpp.github.io/cppwp/n4861/expr.new)                                                                                           |

## 技术自审

1. 正文把 CPU 运行位置、内存策略、页位置和观察时点分开，没有把绑核写成迁移，也没有把 first-touch 解释成任何 new 必得本地页。
2. first-touch 结论限定全新匿名私有页及适用策略。读零页、已有后备页、分配器复用、文件页缓存与需要新后备的 COW 分开描述，没有声称每次写都会复制或分配。
3. 说明线程策略、范围策略与 default 回退，保留 cpuset 约束及内存不足等放置条件。示例不设置 NUMA 内存策略，继承的策略可以影响结果。
4. CPU mask 从 worker 的实际允许集合读取。按 128 起步倍增，使用 CPU_ALLOC/CPU_FREE 及带大小的宏；计数均为存储单元的整倍数，没有把在线 CPU 数当最大 CPU ID，也不硬编码 CPU0。
5. 原始 mask 比亲和性保护对象活得更久。设置目标亲和性失败时不称绑定完成，成功后查询实际集合和运行 CPU；析构恢复原 mask 并读回核对。恢复错误通过整数保存，析构不抛异常。
6. mmap 长度在乘法前检查上界，最多 32 MiB，实际只取 32 个基础页跨度。MAP_FAILED、线程异常、查询异常与 munmap 错误都报告。所有 worker 访问结束并 join 后才解除映射。
7. 映射没有在主线程读写，也没有 MAP_POPULATE。MADV_NOHUGEPAGE 在首次写入前仅用于本映射，结果明确输出；它不改变全局 THP 设置，也不是 NUMA 节点绑定。
8. volatile 写只用于保留采样位置的实际 store，正文没有将其当作线程同步。主线程在 join 后读取字节及 worker 的结果状态。
9. move_pages 使用 pid 0、nodes=nullptr、flags=0，只查询不迁移。整体失败后不解释 status；EPERM、EACCES、ENOSYS、EOPNOTSUPP 打印 SKIP，其他错误抛出并清理。非负 status 为节点号，逐页负 status 明确打印 error，正文不将其计入成功放置。
10. 没有断言样本必须在“本地”节点。THP、自动 NUMA balancing、迁移、许可变化及回收等动态行为作为观察条件处理，没有把一次查询写成永久绑定。
11. 多节点复现步骤要求先选真实获准 CPU 和内存节点，由 numactl 设置外部策略，每次使用新进程、新映射，并核对实际页位置。步骤只比较放置，不伪装为延迟基准。
12. 15 题各层 5 题，L3 包含分片所有权、测量归因、环境限制、动态位置变化及共享只读表策略取舍。题目均为 derived，companies 为空。

## 实际环境与结果

本机环境：

```text
Linux 6.18.33.2-microsoft-standard-WSL2 x86_64 GNU/Linux
g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
base page size: 4096
online nodes: 0
online CPUs: 0-15
Cpus_allowed_list: 0-15
Mems_allowed_list: 0
node0/cpulist: 0-15
node0/distance: 10
THP top-level: always [madvise] never
```

`/proc/sys/kernel/numa_balancing` 不存在，因此没有取得该配置值，未声称已验证该功能关闭。`/proc/self/numa_maps` 可读，但示例本身使用 move_pages 逐地址查询；没有把另一个进程的 numa_maps 输出当本示例放置证据。

普通编译参数：`-std=c++20 -O2 -pthread -Wall -Wextra -Wpedantic -Werror`。ASan/UBSan 参数：`-std=c++20 -O1 -g -pthread -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie`。两种构建运行成功，断言通过，sanitizer 无报告。主 agent 也独立复跑了 ASan/UBSan。

默认运行的实际输出：

```text
MADV_NOHUGEPAGE applied to demo mapping
page_size=4096 bytes=131072 samples=32
allowed_cpu_count=16 selected_cpu=0
page-node samples: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
write/read checks passed; worker affinity restored
mapping cleanup passed; no placement or speed guarantee asserted
```

另选本次已确认获准的 CPU 15，用 taskset 将整个程序的初始允许集合限制为该 CPU。最终源码的 O2 构建输出：

```text
MADV_NOHUGEPAGE applied to demo mapping
page_size=4096 bytes=131072 samples=32
allowed_cpu_count=1 selected_cpu=15
page-node samples: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
write/read checks passed; worker affinity restored
mapping cleanup passed; no placement or speed guarantee asserted
```

两次都在同一暴露节点内，只能验证选择获准 CPU 和放置观察，不能形成远端内存对照。本机未遇到 move_pages 权限拒绝，SKIP 路径经代码审查但未注入运行；未安装 strace，也未新增辅助故障注入源码。没有测量性能、执行页迁移或变更系统 NUMA/THP 全局配置。

## 内容检查与限制

readTopics/validateTopics 检查 schema、11 节顺序、15 题分层、canonical include、related 和内部链接，本章错误为空。Prettier 和 Markdownlint 检查通过；remark-gfm 检查正文三张表的行列及单元格文本，未发现分列错误。

未在多节点物理服务器或其他架构执行，未验证全部 CPU 热插拔与动态 cpuset 场景。ASan/UBSan 只用于有限执行诊断，插桩构建的内存和调度行为不能作为性能基线。最终源文档与示例由主 agent 独立绑定 hash。
