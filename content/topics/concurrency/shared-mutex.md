---
{
  'schemaVersion': 1,
  'id': 'shared-mutex',
  'title': 'shared_mutex：读多写少的边界',
  'description': '从共享所有权、锁升级与快照生命周期，判断读写锁什么时候值得用，什么时候会增加尾延迟。',
  'category': 'concurrency',
  'areas': ['Multithreading / Atomic / Lock-Free'],
  'tags': ['shared-mutex', 'synchronization', 'snapshot'],
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
  'estimatedMinutes': 35,
  'prerequisites': ['RAII 与 unique_lock', '数据竞争', '临界区与对象生命周期'],
  'related': ['cpp-memory-model', 'cpu-cache-false-sharing'],
  'demo':
    {
      'file': 'examples/shared-mutex.cpp',
      'platform': 'portable',
      'exercise': '在示例中新增第二个 writer，让它读取快照后按 version 条件更新；记录失败重试数，并与普通 mutex 比较读临界区长度变化的影响。',
    },
  'references':
    [
      {
        'title': 'C++ shared mutex requirements',
        'url': 'https://eel.is/c++draft/thread.sharedmutex.requirements',
        'kind': 'standard',
        'accessed': '2026-09-18',
      },
      {
        'title': 'C++ shared lock class',
        'url': 'https://eel.is/c++draft/thread.lock.shared',
        'kind': 'standard',
        'accessed': '2026-09-18',
      },
      {
        'title': 'GCC libstdc++ shared_mutex implementation',
        'url': 'https://raw.githubusercontent.com/gcc-mirror/gcc/releases/gcc-13.2.0/libstdc++-v3/include/std/shared_mutex',
        'kind': 'implementation',
        'accessed': '2026-09-18',
      },
      {
        'title': 'C++20 draft N4861: associative container invalidation',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/associative.reqmts',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: unordered container rehash and invalidation',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/unord.req',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: data races',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/intro.races',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'shared-mutex-q01',
        'level': 'L1',
        'prompt': 'shared_mutex 允许哪些并发持锁方式？',
        'answer': '允许多个线程同时拥有共享锁，或一个线程拥有独占锁。共享与独占互斥。读取代码必须遵守无写入约定，锁不会自动识别被保护对象。',
        'rubric': ['共享与独占', '访问协议'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q02',
        'level': 'L1',
        'prompt': 'shared_lock 和 unique_lock 有什么区别？',
        'answer': 'shared_lock 管理共享所有权，适合真实只读访问；unique_lock 管理独占所有权，用于修改。两者都利用 RAII 管理释放，但允许的并发关系不同。',
        'rubric': ['所有权模式', 'RAII'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q03',
        'level': 'L1',
        'prompt': 'const 方法持共享锁就一定安全吗？',
        'answer': '不一定。const 方法可修改 mutable 成员，内部库也可能懒初始化。应审计真实写入和返回对象生命周期，而非仅看方法签名。',
        'rubric': ['隐藏写入', '生命周期'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q04',
        'level': 'L1',
        'prompt': '在 shared_lock 下调用 map[] 有问题吗？',
        'answer': 'operator[] 在键不存在时会插入，从而修改容器，与其他读者竞争。只读路径应用 find 或 at，并明确缺失键处理，不能依赖键大概率存在。',
        'rubric': ['可能插入', '只读查找'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q05',
        'level': 'L1',
        'prompt': '标准保证读写锁不会饿死写者吗？',
        'answer': '没有通用的公平性或无饥饿保证。不能把某个平台的读者或写者优先策略当成可移植契约，延迟要求需结合实现与压力测试。',
        'rubric': ['无公平承诺', '实现与测量'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q06',
        'level': 'L2',
        'prompt': '如何安全地从读阶段转到写阶段？',
        'answer': '结束共享所有权，再获取独占锁，在独占区重新检查读阶段的前提，例如 version。不能持有读锁递归请求写锁，也不能假设间隙没有其他写者。',
        'rubric': ['释放后重验', '非原子升级'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q07',
        'level': 'L2',
        'prompt': '为什么不应该返回受锁保护容器的引用？',
        'answer': '函数返回后锁已释放，其他线程可能修改、删除或重排元素，引用不再安全。可在锁内复制值，或设计延续锁寿命/不可变快照的显式接口。',
        'rubric': ['解锁后的修改', '安全返回策略'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q08',
        'level': 'L2',
        'prompt': '读写锁为什么在纯读负载也会争用？',
        'answer': '共享锁实现通常维护读者计数等同步元数据，获取和释放都可能写同一 cache line。业务数据只读，不代表锁的管理状态没有写竞争。',
        'rubric': ['读者元数据', '一致性流量'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q09',
        'level': 'L2',
        'prompt': 'version 条件更新解决什么问题？',
        'answer': '它让写者确认读阶段所依据的状态仍然有效，防止在锁切换窗口覆盖其他写者的更新。若失败则重算或拒绝，不应盲目继续提交。',
        'rubric': ['检查前提', '失败处理'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q10',
        'level': 'L2',
        'prompt': '锁内调用用户回调会有什么风险？',
        'answer': '回调耗时不受控，可能获取其他锁、递归进入当前对象或执行 I/O，引发尾延迟和死锁。通常复制必要数据后在锁外回调，并规定一致性语义。',
        'rubric': ['锁顺序和重入', '临界区边界'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q11',
        'level': 'L3',
        'prompt': '如何判断风控配置表该不该用 shared_mutex？',
        'answer': '测量读取时长、更新频率、读者数量和读写等待分位数，与普通 mutex 及不可变快照比较。风控热路径对等待的约束可能比平均 QPS 更关键。',
        'rubric': ['可测指标', '方案比较'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q12',
        'level': 'L3',
        'prompt': '不可变 shared_ptr 快照是否免费？',
        'answer': '不是。引用计数可能引发共享写流量，构建新快照有复制/分配成本，最后一个引用释放还可能在读线程触发析构。需要分析这些成本所在的线程。',
        'rubric': ['引用计数', '回收位置'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q13',
        'level': 'L3',
        'prompt': '怎样构造能暴露 writer 延迟的 benchmark？',
        'answer': '在持续读流量下保留真实写者，记录写请求到获得锁的延迟分布，同时扫描临界区长度与线程绑定。只测纯读吞吐不会发现写等待问题。',
        'rubric': ['持续读写负载', '写等待分位数'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q14',
        'level': 'L3',
        'prompt': '交易热路径采用单写者有什么代价？',
        'answer': '可减少共享写入与锁竞争，但跨线程读取要通过消息或快照获取状态，还需处理排队、背压和快照滞后。低竞争不等于没有端到端延迟。',
        'rubric': ['所有权转移', '排队与滞后'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'shared-mutex-q15',
        'level': 'L3',
        'prompt': '采用 epoch 回收替代锁需证明什么？',
        'answer': '必须证明旧对象回收前所有可能访问它的读者都离开相关 epoch，且停滞读者的处理有界或可观测。仅发布新指针并不能安全释放旧对象。',
        'rubric': ['读者生命周期', '停滞与回收'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

shared_mutex 允许多个共享读者或一个独占写者。共享区必须真正只读，返回数据还要保证解锁后的生命周期。标准没有原子升级或公平性保证；读后改应释放读锁、获取写锁并重验。短临界区的读写锁可能比 mutex 更慢，需测读写等待与尾延迟。

## 核心概念

| 操作         | 正确搭配                 | 约束                         |
| ------------ | ------------------------ | ---------------------------- |
| 只读共享对象 | shared_lock              | 不得藏有懒更新或非原子统计写 |
| 修改共享对象 | unique_lock              | 所有访问遵循同一同步约定     |
| 返回数据     | 锁内复制快照             | 解锁后不可借用可能失效的引用 |
| 读后改       | 释放共享锁，再独占锁重验 | 中间有竞争窗口               |

shared_mutex 不提供递归所有权。不要在持有同一把共享锁时再请求独占锁，也不要把“本机实现似乎可行”当成标准保证。规范边界见 [shared mutex requirements](https://eel.is/c++draft/thread.sharedmutex.requirements)。

## 原理深入

共享锁的解锁与后续成功获取之间存在标准规定的同步关系，使受保护对象的读写可以排序；关键是所有访问都遵守协议。const 成员函数不代表绝对无写入：mutable 缓存、内部 lazy initialization 或访问计数可能仍有写操作。

版本检查模式把“读到的状态”与“准备提交的修改”联系起来。线程先取快照，再获得独占锁；若版本改变，放弃或重算。如果只解锁后再上写锁而不重验，就可能覆盖其他写者的新值。它不是原子升级，而是乐观读、受锁保护的条件提交。

返回容器元素引用时，需要区分引用与迭代器的失效条件。`std::map` 没有 `rehash`；`std::unordered_map::rehash` 会使该容器的迭代器失效，但不会使元素引用或指针失效。两种容器删除对应元素后，指向该元素的引用都会失效，见 [关联容器要求](https://timsong-cpp.github.io/cppwp/n4861/associative.reqmts) 与 [无序容器要求](https://timsong-cpp.github.io/cppwp/n4861/unord.req)。

引用仍有效也不代表解锁后可以任意访问：另一个线程可能删除该元素，或与调用者发生未同步的冲突读写，后者可能构成 [data race](https://timsong-cpp.github.io/cppwp/n4861/intro.races)。demo 在锁内复制两个字段，解锁后使用独立的值；大对象可研究不可变快照与 shared_ptr，但不能忽略引用计数和回收代价。

## 数据结构/系统内部实现

概念实现可以使用“活跃读者数、写者活动标志、等待队列”。不同标准库、平台、构建配置可能委托 pthread rwlock，或使用 mutex + condition_variable；公平和唤醒策略不是可移植契约。

即使没有业务写者，每次共享锁的获取/释放也可能修改共享读者计数，导致多个 CPU 争夺该计数所在的 cache line。读者共享数据不等于同步元数据只读。底层布局与策略需对照具体 [libstdc++ 实现](https://raw.githubusercontent.com/gcc-mirror/gcc/releases/gcc-13.2.0/libstdc++-v3/include/std/shared_mutex)。

## C++ runnable demo

四个读者读取 version/checksum 一致快照，一个写者提交 5000 次更新。最后检查过期版本拒绝。线程数量不影响正确性证明，但没有证明任何公平或延迟上界。

```cpp include=examples/shared-mutex.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pthread examples/shared-mutex.cpp -o /tmp/shared-demo
/tmp/shared-demo
```

该示例只用小整数，checksum 是验证字段一致性的教学不变量，不是安全校验。断言检查 `version + checksum == 0`，没有用某个精确调度次序作为测试前提。

## 高频追问

“读多写少，读写锁一定更好？”不一定。若临界区只有两次内存读取，并发读取带来的收益可能低于锁元数据竞争。先用 mutex 作为基线，逐渐加长读取工作，找到真实负载下的交叉点。

“如何避免 writer 长时间等待？”标准 shared_mutex 没有公平性参数；可以更换具有已知策略的实现、限制读者批次，或改用单写者发布不可变快照，但每种方案都需要回收、内存和延迟方面的测量。

## 容易答错的点

- map 的 operator[] 可能插入元素，不能把它放进所谓只读共享区；查找用 find/at 并处理不存在。
- shared_lock 允许并发访问，不允许数据竞争。
- 释放读锁再获取写锁不是原子升级，必须重验状态。
- 公平性依赖实现，不能宣称所有平台写者优先。
- 不要在锁内做网络 I/O、日志磁盘写入或不受控回调。

## 性能分析

固定数据集和线程绑定，扫描读写比例、读取时长、写入批量及读线程数。分别统计读等待、写等待、持锁时间及 p99.9，而不是只报总 QPS。测试里保留真实写者，纯读 benchmark 无法暴露 writer starvation 风险。

同一物理核 SMT 与跨 NUMA 两种放置会改变锁元数据通信成本。不可变快照可减少读侧加锁，但 shared_ptr 引用计数仍会写共享状态，且大对象复制及最后一个读者触发的析构可能造成抖动。

## Quant/Low-Latency 场景

低频更新的风控配置、证券元信息表可考虑读写锁，前提是读取开销足够大且允许等待。每笔订单必须触达的热路径，优先评估单线程所有权或不可变版本快照；不要在未测量前直接套用 RCU。撤回旧版本需要证明所有读者已不再使用。

## 相关专题

- [C++ memory model](cpp-memory-model.md)：锁与原子的共同正确性语言。
- [CPU cache / false sharing](../performance/cpu-cache-false-sharing.md)：共享计数的硬件成本。
- 后续规划：RCU / epoch、mutex 基线、可线性化快照、线程安全 LRU。

## 分层面试题

回答时分清标准保证、实现策略和业务约束。题目来源为知识推导，不代表任何公司的面试记录。
