---
{
  'schemaVersion': 1,
  'id': 'numa-first-touch',
  'title': 'NUMA 与 first-touch：线程在哪，页在哪',
  'description': '区分 CPU 亲和性与内存放置，观察 Linux 匿名私有映射首次写入后的页节点，并核对 cpuset、THP、迁移及单节点环境的证据限制。',
  'category': 'performance',
  'areas': ['CPU Cache / NUMA', 'Performance Engineering', 'Low-Latency Programming'],
  'tags': ['numa', 'first-touch', 'cpu-affinity', 'memory-policy', 'page-placement'],
  'difficulty': 'L2',
  'roles':
    [
      'C++ Developer',
      'Quant Developer',
      'Low-Latency C++ Developer',
      'Trading Infrastructure Engineer',
    ],
  'companyTypes': ['高频交易', '量化私募', 'Trading Firm'],
  'status': 'published',
  'updated': '2026-09-19',
  'reviewed': '2026-09-19',
  'standard': 'C++20',
  'estimatedMinutes': 50,
  'prerequisites':
    ['虚拟地址与物理页', 'CPU cache 与内存访问', '线程生命周期和 Linux 系统调用错误处理'],
  'related':
    [
      'cpu-cache-false-sharing',
      'cpp-memory-model',
      'object-lifetime-layout',
      'raii-exception-safety',
    ],
  'demo':
    {
      'file': 'examples/numa-first-touch.cpp',
      'platform': 'linux',
      'exercise': '在实际拥有两个获准内存节点的机器上，固定同一个允许 CPU，分别用外部内存策略启动全新进程，保存页节点查询结果与策略；查询失败时记录原因，不根据 CPU 位置推测页位置。',
    },
  'references':
    [
      {
        'title': 'Linux man-pages 6.19: numa(7)',
        'url': 'https://man7.org/linux/man-pages/man7/numa.7.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux kernel: CPU topology via sysfs',
        'url': 'https://docs.kernel.org/admin-guide/cputopology.html',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux man-pages 6.19: sched_setaffinity(2)',
        'url': 'https://man7.org/linux/man-pages/man2/sched_setaffinity.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux man-pages: CPU_SET(3) dynamic masks',
        'url': 'https://man7.org/linux/man-pages/man3/CPU_SET.3.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux kernel: NUMA memory policy',
        'url': 'https://docs.kernel.org/admin-guide/mm/numa_memory_policy.html',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux man-pages: mmap(2)',
        'url': 'https://man7.org/linux/man-pages/man2/mmap.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux man-pages: mbind(2)',
        'url': 'https://man7.org/linux/man-pages/man2/mbind.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux man-pages: set_mempolicy(2)',
        'url': 'https://man7.org/linux/man-pages/man2/set_mempolicy.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux kernel: cgroup v2 cpuset controller',
        'url': 'https://docs.kernel.org/admin-guide/cgroup-v2.html',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux kernel: automatic NUMA balancing',
        'url': 'https://docs.kernel.org/admin-guide/sysctl/kernel.html#numa-balancing',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux kernel: transparent huge pages',
        'url': 'https://docs.kernel.org/admin-guide/mm/transhuge.html',
        'kind': 'implementation',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux man-pages: madvise(2)',
        'url': 'https://man7.org/linux/man-pages/man2/madvise.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Linux man-pages: move_pages(2) query mode',
        'url': 'https://man7.org/linux/man-pages/man2/move_pages.2.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'numactl(8): CPU and memory policy controls',
        'url': 'https://man7.org/linux/man-pages/man8/numactl.8.html',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: new-expression',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/expr.new',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'numa-first-touch-q01',
        'level': 'L1',
        'prompt': 'NUMA node 与 CPU 核、socket 是什么关系？',
        'answer': 'NUMA node 是操作系统暴露的内存拓扑单位，描述哪些 CPU 和内存具有相应的访问关系。它不必与一个物理 socket 一一对应；平台可以在一个 socket 内划分多个节点，也可能有不带 CPU 的内存节点。应查询实际拓扑和可用资源，不能从 CPU 编号连续或 socket 数量直接推导节点。',
        'rubric': ['区分内存拓扑和处理器层级', '节点与 socket 非必然一一对应', '查询实际拓扑'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 NUMA 拓扑与处理器编号的不同含义设计基础题。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q02',
        'level': 'L1',
        'prompt': '线程绑到某个 CPU，是否等于把它使用的内存绑到该节点？',
        'answer': '不等于。CPU 亲和性约束线程可以在哪里运行，内存策略决定后续相关页分配如何选择节点。已有物理页不会因一次 sched_setaffinity 调用自动搬迁。即使绑核后才写新匿名页，实际放置仍受有效内存策略、允许节点、可用内存和系统行为影响。',
        'rubric': ['CPU 与内存是独立控制', '绑核不迁移已有页', '新分配仍有策略与许可条件'],
        'source':
          { 'kind': 'derived', 'rationale': '根据调度位置和页放置接口的独立职责设计概念题。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q03',
        'level': 'L1',
        'prompt': '新匿名私有映射只读一遍，能否作为首次写入本地页的证据？',
        'answer': '不能。对尚未分配私有后备页的匿名映射，初次读访问可以使用共享零页；首次写入才需要建立可写私有后备页，并按适用内存策略选择放置。实验应区分读触发的映射、写入分配和已经存在的物理页，不能把读到零当成本地物理页已经分配。',
        'rubric': ['读访问可能使用共享零页', '写入触发私有后备分配', '查询放置而非由值推断'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据匿名映射读缺页与写缺页的差别设计 first-touch 题。',
          },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q04',
        'level': 'L1',
        'prompt': 'C++ 的 new 返回了内存，能保证这些字节位于调用线程的本地节点吗？',
        'answer': 'C++ 标准没有 NUMA 本地性保证。分配器可能复用已有页，初始化可能已由其他线程完成，内存策略也可能要求 interleave 或绑定其他节点。需要分别检查存储来源、首次实际写入者、策略和页节点；不能把一次 new 调用视为一次新的本地物理页分配。',
        'rubric': ['无语言层 NUMA 保证', '分配器复用和提前初始化', '策略与页位置需要独立证据'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据语言分配接口和操作系统页分配的层次差别设计题目。',
          },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q05',
        'level': 'L1',
        'prompt': '示例调用 move_pages，为什么不会把页面迁移到别的节点？',
        'answer': '它把 nodes 参数设为 nullptr，使用的是查询当前页节点的模式，并把 flags 设为零。调用成功后还要检查每个 status：非负值是节点号，负值是逐页错误。系统调用整体失败时 status 不能作为放置证据；查询本身既不设置分配策略，也不执行迁移。',
        'rubric': ['nodes 为空表示查询', '整体返回与逐页状态分别检查', '查询不设置策略或迁移'],
        'source': { 'kind': 'derived', 'rationale': '根据 move_pages 的双重用途设计接口审查题。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q06',
        'level': 'L2',
        'prompt': '线程内存策略与 VMA 策略如何影响同一地址范围的后续分配？',
        'answer': '适用的地址范围策略优先于线程策略，线程未提供非默认策略时再回退到系统默认。set_mempolicy 作用于调用线程，并会被之后由它创建的线程继承，不会自动改写已经存在的兄弟线程策略。mbind 面向地址范围，映射类型和 COW 规则会影响其作用，不能把 default 简化为永远绑定某个固定节点。',
        'rubric':
          ['范围策略与线程策略的优先关系', '线程策略继承与已有线程区别', '映射类型及 default 回退'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 Linux 内存策略的作用域设计分配路径追问。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q07',
        'level': 'L2',
        'prompt': '主线程已经清零一块缓冲区，再让绑核 worker 重写，是否完成了重新 first-touch？',
        'answer': '通常只是写已经存在的后备页，不会因为换了写线程就重新选择节点。策略变更默认也主要影响新分配，已有页迁移需要另外的机制或重新创建存储。若要观察初始放置，应使用全新映射，让指定 worker 在策略建立后首次实际写入，再查询页位置。',
        'rubric': ['重复写已有页不等于新分配', '策略变更与迁移分开', '全新存储和初始化顺序'],
        'source':
          { 'kind': 'derived', 'rationale': '根据提前清零破坏放置实验的因果关系设计追问题。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q08',
        'level': 'L2',
        'prompt': 'MAP_PRIVATE 文件映射与匿名私有映射，为什么不能套用完全相同的首次读解释？',
        'answer': '私有文件映射的读可以使用文件页缓存，相关页可能已经由其他访问者建立，不能当成新匿名零页。发生需要复制的写时，COW 产生匿名私有后备页，此时适用的分配规则与原文件页不同。fork 后的共享匿名页也要区分已有共享页和需要新分配的 COW 页；不是每次写都必然复制。',
        'rubric': ['文件页缓存与匿名零页不同', 'COW 新页与旧共享页区分', '写入不必每次触发复制'],
        'source':
          { 'kind': 'derived', 'rationale': '根据文件后备、共享和 COW 分配边界设计机制题。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q09',
        'level': 'L2',
        'prompt': '机器有两个节点，容器内程序为什么仍可能无法绑定其中一个节点？',
        'answer': '系统在线资源不等于当前任务获准使用的资源。cpuset 限制允许 CPU 和内存节点，内存策略需要在这些许可范围内工作；亲和性请求也会受到在线 CPU 和 cpuset 的限制。应读取当前允许集合，在 cgroup v2 下核对 effective 文件，并检查系统调用结果，不根据宿主拓扑硬编码节点或 CPU0。',
        'rubric': ['在线与获准资源区分', 'cpuset 优先约束', 'effective 集合及调用返回值'],
        'source':
          { 'kind': 'derived', 'rationale': '根据容器资源约束设计拓扑可见但不可用的排障题。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q10',
        'level': 'L2',
        'prompt': '为什么 first-touch 实验必须记录 THP 设置，而不能默认每 4 KiB 独立放置？',
        'answer': '系统基础页大小未必是 4 KiB，THP 还可能使用更大的分配或映射粒度，并在运行中合并或拆分页面。一个首次缺页可能影响更大的范围，多个采样地址也可能属于同一大页。应读取实际页大小，记录 THP 配置与映射观察；示例尝试对自己的映射设置 MADV_NOHUGEPAGE，并检查是否成功。',
        'rubric':
          ['基础页大小与 THP 粒度', '多个地址不等于独立分配页', '记录配置与检查局部建议结果'],
        'source':
          { 'kind': 'derived', 'rationale': '根据大页对分配粒度和观察口径的影响设计实验题。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q11',
        'level': 'L3',
        'prompt': '按节点分片的订单数据，初始化与后续所有权应怎样配合？',
        'answer': '让长期拥有该分片的 worker 在目标 CPU 与适用策略建立后初始化全新后备页，随后尽量由同一分片处理。还要避免主线程预先清零、分片间共享写热点，以及两个分片落在同一放置粒度内。工作迁移或负载重分配时重新评估数据位置与迁移成本，不能只在启动阶段记录一次 CPU 绑定。',
        'rubric':
          ['初始化者与长期访问者一致', '提前触页和跨片共享边界', '重分配时的放置与迁移成本'],
        'source':
          { 'kind': 'derived', 'rationale': '根据分片所有权变化与内存局部性设计工程取舍题。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q12',
        'level': 'L3',
        'prompt': '怎样区分远端内存访问代价与首次缺页、缓存命中或带宽瓶颈？',
        'answer': '先确认访问 CPU、策略和实际页节点，再将首次分配与稳态访问分开。固定数据规模、访问顺序、并发度和 CPU 放置，记录缓存状态与 THP、NUMA balancing。延迟实验可使用依赖访问减少并行重叠，带宽实验则控制并行度；保存完整条件与分布，不能把一次写页总耗时全部归因于远端内存。',
        'rubric': ['先验证真实放置', '缺页和稳态成本分离', '延迟与带宽实验及条件记录'],
        'source':
          { 'kind': 'derived', 'rationale': '根据多种成本混合的测量风险设计 NUMA 实验方法题。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q13',
        'level': 'L3',
        'prompt': '环境只有一个节点，或者页节点查询被拒绝，实验还能给出哪些结论？',
        'answer': '单节点可以验证允许 CPU 内绑定、写读正确性、亲和性恢复和清理，也可以记录当前页节点，但无法比较本地与远端。查询被拒绝时应记录 SKIP 和错误原因，不能凭 CPU 编号宣称页在本地。可在受支持的环境读取并正确关联 numa_maps，或把放置与跨节点性能验证留给具备条件的机器。',
        'rubric': ['单节点的有限验证范围', '查询失败不推断位置', '替代证据与待验证项明确'],
        'source':
          { 'kind': 'derived', 'rationale': '根据权限和实验环境限制设计证据诚实性验收题。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q14',
        'level': 'L3',
        'prompt': '初始化时查询页面在节点 A，运行一段时间后为什么还要重新查询？',
        'answer': '页位置不是永久不变的属性。自动 NUMA 平衡可根据访问模式迁移页面，显式迁移、资源策略调整、回收后重新建立后备等也会改变观察。应记录相关配置和访问阶段，在可比时点采样；如果节点变化，要区分线程迁移、页面迁移及策略变化，不能只靠初始 first-touch 解释整个运行期。',
        'rubric': ['页面位置是时点观察', '自动平衡及其他变化来源', '区分线程与页面迁移并复核'],
        'source':
          { 'kind': 'derived', 'rationale': '根据运行时动态放置变化设计持续观察和归因题。' },
        'companies': [],
      },
      {
        'id': 'numa-first-touch-q15',
        'level': 'L3',
        'prompt': '大只读表被多个节点频繁访问时，local、interleave 与复制如何取舍？',
        'answer': '单节点放置可能让其他节点承担远端流量，也可能集中内存带宽；interleave 分散页及带宽，但每个访问者仍会访问部分远端数据。按节点复制可提高局部性，代价是额外内存、初始化和版本发布成本。选择应依据访问分布与工作集大小测量，读取在 cache 命中时也不能简单按物理页所在节点估计每次延迟。',
        'rubric': ['局部性与聚合带宽权衡', '复制的内存与更新成本', '缓存和访问分布影响结论'],
        'source':
          { 'kind': 'derived', 'rationale': '根据跨节点共享只读数据的不同放置方案设计权衡题。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

NUMA 系统把内存划分为节点，CPU 访问不同节点的内存会有不同的路径与成本。CPU 亲和性控制线程在哪里运行，内存策略控制相关新页分配如何选节点。对全新的 Linux 匿名私有映射，首次实际写入通常触发私有后备页分配；采用本地分配策略时，写入线程的位置会影响选择，但还受允许节点与可用内存等条件约束。绑核不会自动迁移旧页，first-touch 也不是 C++ new 的语言保证。

## 核心概念

本文使用 C++20 和 Linux 接口。资料依据包含 Linux man-pages 6.19 与访问当日的内核文档；实际验证环境是 WSL2 的 Linux 6.18.33.2、GCC 13.3。文档版本与运行内核不同，接口和配置以本机检查结果为准。

| 概念           | 回答的问题                      | 不能直接推导的结论                      |
| -------------- | ------------------------------- | --------------------------------------- |
| CPU topology   | 逻辑 CPU、core、socket 如何组织 | socket 数量就是 NUMA 节点数             |
| NUMA node      | CPU 与内存资源之间的节点关系    | 每个节点都有 CPU 或等量内存             |
| CPU affinity   | 当前线程被允许在哪些 CPU 上运行 | 它使用的页已经位于这些 CPU 本地         |
| memory policy  | 相关分配按哪些规则选择节点      | 已有页已自动迁移到目标节点              |
| page placement | 采样时后备页位于哪个节点        | 整个运行期位置不变，或每次访问都到 DRAM |

节点编号、CPU 编号和 socket 编号属于不同层次。实际机器可能在一个 socket 中暴露多个节点，也可能包含没有 CPU 的内存节点；虚拟机还可能只暴露宿主拓扑的一部分。先读取系统提供的拓扑，再检查任务的资源许可。[NUMA 概述](https://man7.org/linux/man-pages/man7/numa.7.html)、[CPU topology](https://docs.kernel.org/admin-guide/cputopology.html)

缓存命中可以使一次访问不必到达内存控制器，因此“页在远端节点”并不等于每次读取都支付一次远端 DRAM 延迟。页放置是分析输入，还需要访问模式与缓存状态。

## 原理深入

### 从虚拟映射到首次可写后备页

不带预填充的匿名 mmap 建立虚拟地址范围，并不意味着每个地址立即拥有独立的物理页。对尚无私有后备的匿名页，初次读取可以映射共享零页；首次写入需要可写私有后备，再按适用策略进行分配。示例因此在绑定 worker 后，对每个基础页跨度写一个非零字节，而不是只读零值。[mmap](https://man7.org/linux/man-pages/man2/mmap.2.html)、[匿名区域的读写分配](https://man7.org/linux/man-pages/man2/mbind.2.html)

若主线程在创建 worker 前已经清零这些页，之后 worker 再写就不再是同一次新页分配。分配器也可能直接提供仍有物理后备的旧内存。C++ new 描述对象创建及存储分配接口，没有规定 NUMA 节点；初始化、内存池复用和系统策略都可能改变实际页来源。[C++20 new-expression](https://timsong-cpp.github.io/cppwp/n4861/expr.new)

### 策略按作用域选择

Linux 区分适用的地址范围策略、线程策略和系统默认。某一层使用 default，意味着回退到适用的上层策略，并不等于永久固定到节点 0。线程策略由 set_mempolicy 设置，只直接影响调用线程，并由它之后创建的线程继承；已经存在的兄弟线程保留各自策略。mbind 则针对地址范围，作用还与映射类型有关。[NUMA memory policy](https://docs.kernel.org/admin-guide/mm/numa_memory_policy.html)、[set_mempolicy](https://man7.org/linux/man-pages/man2/set_mempolicy.2.html)

本地分配倾向于从分配发生时所在 CPU 的本地节点取得内存，条件不满足时可能回退。bind 约束允许的分配节点集合，interleave 按策略在节点间分布页。它们的目标不同，不能把所有策略统称为 first-touch 本地分配。默认策略变更主要影响之后的相关分配；对已有页，迁移需要另外处理。

### COW、文件页和已存在的页

MAP_PRIVATE 文件映射的读使用文件后备与页缓存，页可能早已由其他线程或进程建立。需要写时复制的写入会产生匿名私有后备页，不能用原文件页的来源解释新页的放置。fork 后的匿名页也可能暂时共享，之后只有需要建立新后备的 COW 路径才重新涉及分配；并非每次写入都会复制。[mbind 的映射类型边界](https://man7.org/linux/man-pages/man2/mbind.2.html)

线程换到另一个 CPU 后，页表仍可指向原来的后备页。sched_setaffinity 改变线程的可运行 CPU 集合，不会替已有地址逐页执行迁移。[sched_setaffinity](https://man7.org/linux/man-pages/man2/sched_setaffinity.2.html)

## 数据结构/系统内部实现

### 允许 CPU 和允许内存节点分别检查

系统在线资源与当前任务获准使用的资源不同。CPU 亲和性请求受在线 CPU 和 cpuset 限制；内存策略也在允许节点范围内工作。cgroup v2 的 `cpuset.cpus.effective`、`cpuset.mems.effective` 反映实际获准资源，可能与请求配置不同，且可能随热插拔或管理操作变化。[cgroup v2 cpuset](https://docs.kernel.org/admin-guide/cgroup-v2.html)

示例在 worker 内调用 sched_getaffinity，从返回集合选择一个 CPU，不假定 CPU0 可用。CPU mask 动态分配并按需增大，避免把固定 cpu_set_t 的容量当作所有机器的上限；探测也有明确的大小上限。绑定后再次读取实际集合并检查当前 CPU，结束时恢复原集合并核对结果。[动态 CPU 集合](https://man7.org/linux/man-pages/man3/CPU_SET.3.html)

### 放置证据和资源清理

示例申请 32 个系统基础页跨度，映射总量上限为 32 MiB。在首次写入前不读写映射内容，也不使用 MAP_POPULATE。它尝试对本次映射应用 MADV_NOHUGEPAGE，成功与失败都打印；这是本地址范围的大页建议，不是 NUMA 节点策略。[madvise](https://man7.org/linux/man-pages/man2/madvise.2.html)

首次写入后，move_pages 的 nodes 参数为空，只查询 32 个页对齐地址的当前节点，不迁移页面。整体调用失败时不解释 status；权限不足或接口不可用时打印 SKIP，其他异常错误作为程序失败。整体成功仍可能有逐页负状态，输出保留这些错误，不能把它们视为节点号。[move_pages 查询模式](https://man7.org/linux/man-pages/man2/move_pages.2.html)

亲和性保护对象在正常返回和异常展开时都尝试恢复原 CPU 集合。动态 cpuset 变化可能使精确恢复失败，此时报告错误。worker 结束后主线程 join，再读取验证映射中的字节；volatile 只用于保留示例的实际写操作，线程间同步由 join 提供。所有映射访问结束后才 munmap，清理错误也会报告。

### 运行期间位置仍可能变化

自动 NUMA balancing 可以根据访问模式迁移页，其采样和缺页处理也有开销。手动迁移、cgroup 内存节点调整及其他内存管理行为也可能改变位置。因此查询记录的是当时的页位置，不能解释为永久绑定。[自动 NUMA balancing](https://docs.kernel.org/admin-guide/sysctl/kernel.html#numa-balancing)

THP 会改变实际分配与映射粒度，并可能合并或拆分页面。基础页也未必是 4 KiB，不能把固定字节步长直接当作独立物理页数。即使记录了顶层 THP 模式，还要注意内核版本对应的每种页大小配置；本例成功应用 MADV_NOHUGEPAGE 后，才按这项局部限制解释自己的映射。[THP 文档](https://docs.kernel.org/admin-guide/mm/transhuge.html)

## C++ runnable demo

程序仅依赖 Linux 的 libc 接口及标准 C++20 线程支持，不需要安装 libnuma 开发包。它经 syscall 使用查询接口，没有调用 set_mempolicy、mbind 或迁移接口，也没有覆盖继承来的内存策略。

```cpp include=examples/numa-first-touch.cpp

```

编译时保留断言：

```bash
g++ -std=c++20 -O2 -pthread -Wall -Wextra -Wpedantic -Werror examples/numa-first-touch.cpp -o /tmp/numa-demo
/tmp/numa-demo
```

成功完成写读与清理后，末两行是：

```text
write/read checks passed; worker affinity restored
mapping cleanup passed; no placement or speed guarantee asserted
```

此前会打印页大小、映射大小、允许 CPU 数量、选中 CPU，以及节点样本或查询 SKIP。CPU 和节点号不作为固定预期输出；查询成功也没有断言所有样本必然在当前 CPU 的本地节点。

本次 WSL2 实测只暴露 node 0，CPU 与许可如下：

| 观察项                     | 本次结果                        |
| -------------------------- | ------------------------------- |
| online NUMA nodes          | 0                               |
| Cpus_allowed_list          | 0-15                            |
| Mems_allowed_list          | 0                               |
| 系统基础页大小             | 4096 字节                       |
| demo 映射                  | 131072 字节，32 个采样地址      |
| 缺省运行                   | 选中 CPU 0，32 项节点查询均为 0 |
| 进程限制为允许的 CPU 15 后 | 选中 CPU 15，32 项仍为节点 0    |
| THP 顶层配置               | always [madvise] never          |
| numa_balancing sysctl      | 文件不存在，未取得该配置值      |

两个 CPU 属于同一个暴露节点，这个对照只检查代码遵守允许集合，没有比较本地与远端访问。未取得 numa_balancing 配置也不能写成已经验证关闭。ASan/UBSan 运行通过用于检查有限执行；插桩会改变进程内存与运行行为，放置和性能实验应单独使用记录清楚的非插桩构建。

## 高频追问

### 多节点机器怎样复现实验？

先用系统拓扑和当前许可找出两个有内存且获准使用的节点，以及节点 A 内一个允许 CPU。若机器已安装 numactl，可先查看：

```bash
numactl --hardware
grep -E 'Cpus_allowed_list|Mems_allowed_list' /proc/self/status
```

把 CPU_A、NODE_A、NODE_B 设成实际核对过的编号，再运行全新进程：

```bash
numactl --physcpubind="$CPU_A" --localalloc /tmp/numa-demo
numactl --physcpubind="$CPU_A" --membind="$NODE_A" /tmp/numa-demo
numactl --physcpubind="$CPU_A" --membind="$NODE_B" /tmp/numa-demo
```

这里由外部工具设置进程启动策略，demo 自身仍只控制 worker 的 CPU 并查询页面。检查每条命令是否成功，保存节点样本；localalloc 结果受回退条件影响，不能只看命令参数就认定放置。三次运行都创建新映射，避免沿用先前已经触碰的页。这个步骤比较放置证据，没有测量远端访问速度。[numactl](https://man7.org/linux/man-pages/man8/numactl.8.html)

### move_pages 不可用，还有什么观察手段？

可读取目标进程的 `/proc/<pid>/numa_maps`，结合 maps 中的地址范围定位映射。numa_maps 的 N0、N1 等字段提供节点页数，但文件可能不可读，VMA 也可能合并；不能拿一行聚合数据冒充每个请求页的独立查询。策略文本描述与已经分配的节点统计也要分别解读。[numa_maps 说明](https://man7.org/linux/man-pages/man7/numa.7.html)

### 为什么按节点初始化后仍可能远端访问很多？

初始化者与长期访问者可能不同；分片跨线程传递后，页仍在原节点。小对象还可能共享同一基础页或大页，页级放置无法同时满足位于不同节点的多个所有者。应先查主要访问者、数据交接和共享写，再判断是否需要按节点分配池、复制只读表或显式迁移。

## 容易答错的点

| 说法                                | 修正                                                         |
| ----------------------------------- | ------------------------------------------------------------ |
| first-touch 就是谁先调用 malloc/new | 要区分分配器返回存储、已有页复用和真正建立后备页的访问       |
| 把匿名内存读一遍就本地化了          | 新匿名页的初次读可能使用共享零页，写入分配需要单独观察       |
| 绑核能把已有数据一起搬过去          | affinity 控制线程运行位置，页面迁移是另一项操作              |
| default 内存策略表示 node 0         | default 表示按作用域回退，系统正常运行时通常采用本地分配规则 |
| 节点 0 与 CPU0 一定可用             | 在线拓扑与任务获准集合不同，必须检查 cpuset 和返回值         |
| 32 个基础页跨度必然独立分配 32 次   | THP 等机制会影响分配粒度，样本地址数量不等于独立分配次数     |
| 节点查询失败仍能从 CPU 判断放置     | 缺少放置证据时明确 SKIP，不把调度位置当内存位置              |
| 页在本地就解决了并发访问问题        | 数据竞争和发布协议仍受 C++ 内存模型约束                      |

## 性能分析

本例只触碰有限页并查询节点，没有运行内存延迟或带宽基准。首次写入包含缺页处理、后备分配及清零等成本，不能把这一阶段的总耗时直接标成“远端内存延迟”。

做性能对照时，先确认 CPU 与页节点，再把初始化和稳态阶段分开。固定工作集、访问顺序、读写比例、并发度与 CPU 放置，记录内核、编译器、策略、THP、NUMA balancing 和内存压力。依赖式访问与多请求并行访问测到的瓶颈不同；工作集若大量命中 cache，也不能只用物理页位置解释差异。

局部放置可能降低远端流量，也可能把带宽压力集中到一个节点。interleave 适合评估聚合带宽的场景，但不保证单线程延迟更低。复制只读表可以换取局部性，代价是内存与版本发布成本。需要保存吞吐及端到端 p50/p99/p99.9、重复运行分布和观测时点，不给出通用纳秒表或固定加速倍数。

## Quant/Low-Latency 场景

行情解码、订单状态和风险分片如果长期由固定 worker 处理，可以让该 worker 在合适策略下初始化自己的页，并在启动后核对位置。主线程统一清零所有分片、共享内存池跨节点归还，以及任务在节点间迁移，都可能改变预期访问路径。

大型只读参数表同时服务多个节点时，需要在单份共享、interleave 与按节点复制之间选择。复制要求各份数据有一致的版本发布规则，NUMA 放置本身不提供同步。对于持续写入的数据，过多跨节点交接还可能叠加 cache coherence 流量；应结合所有权与访问分布调整数据分片。

重新平衡业务分片时，把数据搬迁量、暂停时间、允许内存容量和回退方案纳入计划。单纯把线程调到空闲 CPU，可能把原来的本地工作集变成远端工作集；这需要新的放置观察和端到端测量。

## 相关专题

- [CPU Cache 与 False Sharing](cpu-cache-false-sharing.md)：区分页级放置和 cache line 的共享写竞争。
- [C++ 内存模型](../concurrency/cpp-memory-model.md)：NUMA 局部性不替代数据发布与同步。
- [对象生命周期与布局](../cpp/object-lifetime-layout.md)：映射存储、对象建立及跨线程借用的前提。
- [RAII 与异常安全](../cpp/raii-exception-safety.md)：亲和性恢复、映射释放与失败报告。

## 分层面试题

回答时分别列出线程运行位置、有效内存策略、已经观察到的页节点和观察时间。工程题再说明工作集的长期所有者，以及证据不足时哪些结论需要保留。
