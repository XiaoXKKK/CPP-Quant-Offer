---
{
  'schemaVersion': 1,
  'id': 'memory-pool',
  'title': '内存池：槽位、对象寿命与归还协议',
  'description': '实现固定容量的 typed object pool，验证过对齐对象、构造异常回滚、陈旧句柄、重复归还和代次耗尽，并区分槽位复用与通用 allocator。',
  'category': 'design',
  'areas': ['Algorithm Coding', 'System Design'],
  'tags': ['memory-pool', 'object-lifetime', 'alignment', 'free-list', 'generation-handle'],
  'difficulty': 'L3',
  'roles': ['C++ Developer', 'Quant Developer', 'Low-Latency C++ Developer'],
  'companyTypes': ['高频交易', '量化私募', 'Trading Firm'],
  'status': 'published',
  'updated': '2026-09-19',
  'reviewed': '2026-09-19',
  'standard': 'C++20',
  'estimatedMinutes': 50,
  'prerequisites': ['对象生命周期与对齐', 'RAII 和构造异常', '数组索引与单链表'],
  'related':
    ['object-lifetime-layout', 'raii-exception-safety', 'aba-reclamation', 'benchmark-methodology'],
  'demo':
    {
      'file': 'examples/memory-pool.cpp',
      'platform': 'portable',
      'exercise': '在两个槽位均可用时连续注入两次构造失败，再验证容量、成员资源计数和成功构造/析构数；保留最早句柄，检查复用后仍被拒绝。不要通过解引用已归还指针测试失败路径。',
    },
  'references':
    [
      {
        'title': 'C++20 N4861: object lifetime',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/basic.life',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: object model and byte-array storage',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/intro.object',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: alignment requirements',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/basic.align',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: construct_at',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/specialized.construct',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: destroy_at',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/specialized.destroy',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: constructor exception cleanup',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/except.ctor',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: allocator requirements',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/allocator.requirements',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 N4861: pointer alignment',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/ptr.align',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'memory-pool-q01',
        'level': 'L1',
        'prompt': '内存池拿到一块足够大的内存后，T 对象是否已经可以使用？',
        'answer': '还要满足对齐和对象生命周期要求。本例每槽提供 alignas(T) 的字节存储，成功执行 construct_at 后才标为 live 并发出句柄。非平凡 Tracked 不能靠 reinterpret_cast 开始使用；它的构造函数还负责建立成员资源。C++20 的隐式对象创建规则也不允许任意字节缓冲自动成为任意类型。',
        'rubric': ['大小与对齐', '成功构造后发布', '不能将转换当构造'],
        'source':
          { 'kind': 'derived', 'rationale': '根据原始槽位与非平凡对象生命周期的区别设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q02',
        'level': 'L1',
        'prompt': '为什么槽位写成 alignas(T) 的字节数组，不能只预留 sizeof(T) 字节？',
        'answer': 'sizeof(T) 给出所需大小，却不能让任意缓冲起始地址满足 T 的对齐要求。alignas(T) 约束槽位中的存储地址，包含它的 Slot 及数组元素布局也要满足相应对齐。示例 Tracked 要求 64 字节对齐，本机高于 max_align_t；扩展对齐支持有实现边界，不能假定所有平台支持同一个值。',
        'rubric': ['地址也要对齐', 'Slot 数组保持对齐', '扩展对齐依实现'],
        'source':
          { 'kind': 'derived', 'rationale': '根据字节容量、地址对齐和过对齐类型的不同要求设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q03',
        'level': 'L1',
        'prompt': '容量耗尽时，try_emplace 如何返回？它会调用 T 的构造函数吗？',
        'answer': '示例先检查 free_head，无可用槽就返回 nullopt，尚未调用 T 构造函数，也不会偷偷扩容。若已有空槽而 T 构造抛异常，则恢复空闲链后向调用方传播异常。这两个结果分别表示容量不足和构造失败，业务可以据此选择拒绝、等待或上报。',
        'rubric': ['nullopt 表示容量不足', '满时不构造', '构造异常另行传播'],
        'source':
          { 'kind': 'derived', 'rationale': '根据有界分配入口的容量与构造失败两条路径设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q04',
        'level': 'L1',
        'prompt': 'destroy 成功后，内存是否已经还给操作系统？',
        'answer': '本例先调用对象析构函数，再把槽位放回池的空闲链，或因代次耗尽将它退役。槽位字节仍是 pool 的内联存储，直到 pool 自身存储结束才一起释放；其中原来 T 对象的寿命已经结束。析构函数还可能释放 T 自己拥有的其他资源，那是另一层所有权。',
        'rubric': ['对象析构与槽位归还', '内联存储仍存在', 'T 自有资源单独处理'],
        'source':
          { 'kind': 'derived', 'rationale': '根据对象、池存储和对象内部资源的三种寿命设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q05',
        'level': 'L1',
        'prompt': '为什么接口返回句柄，再通过 get 取得指针？',
        'answer': '句柄保存所属池、槽位索引和代次，get 能在池仍活着时拒绝默认句柄、跨池句柄以及已销毁或已复用的旧对象身份。裸指针只提供地址，不能携带这些检查。get 返回的指针仍是借用，后续 destroy 不会自动使调用方保存的指针变成 nullptr。',
        'rubric': ['池与槽位代次身份', 'get 时校验', '指针借用不会自动撤回'],
        'source':
          { 'kind': 'derived', 'rationale': '根据可检查对象身份与裸地址之间的能力差异设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q06',
        'level': 'L2',
        'prompt': '构造函数在成员资源建立后抛异常，池与 C++ 各负责清理什么？',
        'answer': '语言的异常展开销毁已完成构造的成员和基类；本例 Resource 的 unique_ptr 因而释放资源，未完成构造的 Tracked 不调用其自身析构函数。池的 catch 负责把预留槽位重新接回空闲链并恢复计数，不发布句柄。外部构造副作用未必能回滚，不能把这个承诺扩大为整个业务事务不变。',
        'rubric': ['已完成子对象展开', '失败的 Tracked 不析构', '池元数据回滚且不发布'],
        'source':
          { 'kind': 'derived', 'rationale': '根据构造异常展开与池自身事务边界的分工设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q07',
        'level': 'L2',
        'prompt': '同一个槽位被复用后，旧句柄为什么不能销毁新对象？',
        'answer': '成功销毁后本例递增该槽的代次，新对象获得新代次；旧句柄即使索引相同，也无法通过 matches。检查还包含 owner 和 live，防止跨存活池操作和尚未复用时的重复归还。这个证明依赖池存活、单线程和代次不回绕，不覆盖池地址在另一段寿命被重新使用的情况。',
        'rubric': ['索引相同但代次不同', 'owner 与 live 检查', '寿命与不回绕前提'],
        'source': { 'kind': 'derived', 'rationale': '根据槽位重复使用导致的陈旧身份风险设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q08',
        'level': 'L2',
        'prompt': '代次计数器到最大值后，为什么选择退役槽位？',
        'answer': '直接回绕可能让很久以前的句柄再次匹配。本例允许最大代次的对象完成本次寿命，但销毁后不再把该槽接回空闲链；retired 增加，容量永久减少。uint8 测试实际完成 255 轮后确认无法再分配。生产策略可以使用更宽代次或有证明的整体重建，但不能只把回绕称为概率很小就宣称绝对安全。',
        'rubric': ['回绕导致身份重合', '最大代次销毁后退役', '安全换取有效容量'],
        'source': { 'kind': 'derived', 'rationale': '根据有限代次空间与长期陈旧句柄的冲突设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q09',
        'level': 'L2',
        'prompt': 'get 校验成功后，可否无限期保留返回指针？',
        'answer': '调用方只能在该对象仍活着且未被销毁的期间借用。校验只是当前时点的检查，既不增加引用计数，也不阻止另一段代码 destroy。示例为单线程，调用方在归还前结束所有借用；并发扩展还需要锁住使用期间、所有权转移或其他回收协议，仅把 get 里的字段改成原子不够。',
        'rubric': ['校验不固定寿命', '归还前结束借用', '并发需保护整个使用区间'],
        'source':
          { 'kind': 'derived', 'rationale': '根据句柄校验与后续访问之间的生命周期间隔设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q10',
        'level': 'L2',
        'prompt': '这个池的创建、分配、归还复杂度分别是什么？',
        'answer': '初始化需建立 N 个槽位的空闲链，池析构也扫描 N 个槽位并销毁仍活着的对象。单次空闲链取出和归还只改固定数量的元数据，是 O(1)；try_emplace 与 destroy 的总成本还包括 T 的构造和析构。示例 T 内部做堆分配，因此不能据此承诺整个 API 无分配或延迟有固定时间上界。',
        'rubric': ['初始化和清理 O(N)', '槽位元数据 O(1)', '包括 T 的构造析构成本'],
        'source': { 'kind': 'derived', 'rationale': '根据空闲链操作与对象行为的复杂度边界设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q11',
        'level': 'L3',
        'prompt': '能否直接把 TypedPool 作为 std::vector 的 allocator 参数？',
        'answer': '不能直接替换。本例只提供单个 T 的构造与销毁，返回自定义句柄，不提供标准 allocator 所需的 allocate(n)、deallocate、相等性及相关类型约定。vector 还需要连续元素存储，空闲链中的几个独立槽并不满足这个要求。若要接入标准容器，应重新设计存储分配接口并逐项满足对应要求。',
        'rubric': ['API 不符合 allocator 合同', '连续 n 元素要求', '需完整适配而非改名'],
        'source':
          { 'kind': 'derived', 'rationale': '根据标准 allocator 存储接口与单对象池接口差异设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q12',
        'level': 'L3',
        'prompt': '线程本地池的对象交给另一个线程后，应由谁归还？',
        'answer': '先明确对象所有权及池所在的生命周期。本例没有同步，另一个线程不能直接访问池元数据。可以让远端把归还请求交回拥有池的线程，或者选择带同步的共享池，但请求队列、线程退出和借用结束都要有协议。线程本地存储减少部分竞争，也可能造成容量滞留和跨线程回收延迟。',
        'rubric': ['当前池不支持并发', '远端归还或同步共享方案', '队列容量与退出协议'],
        'source':
          { 'kind': 'derived', 'rationale': '根据线程归属变化对池元数据与回收路径的影响设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q13',
        'level': 'L3',
        'prompt': '交易组件如何为固定容量池定大小，又如何处理耗尽？',
        'answer': '预算应包含队列中对象、处理中的对象、异步完成前必须保留的对象，以及回收滞后和退役槽位。可用高水位与突发场景验证预算，平均速率不能覆盖所有尖峰。耗尽时必须按业务合同拒绝、背压或进入显式降级状态；若临时回退到通用分配器，也要追踪释放来源，并重新评估内存上界和延迟。',
        'rubric': ['覆盖在途及回收滞后', '突发和退役容量预算', '耗尽与回退合同'],
        'source':
          { 'kind': 'derived', 'rationale': '根据交易数据链路中的对象占用和有界资源约束设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q14',
        'level': 'L3',
        'prompt': '关闭组件时，pool 自动析构剩余对象是否足够？',
        'answer': '还要先停止新分配，结束生产者和消费者的访问，结清异步操作并结束所有借用，才能销毁池。示例按槽位顺序析构剩余对象，不保证逆创建顺序，依赖其他对象仍存活的析构函数需要单独安排清理顺序。自动析构提供资源清理，不能替代调用方的停机和依赖管理。',
        'rubric': ['关闭先于销毁', '借用及异步访问结束', '槽位顺序不等于依赖顺序'],
        'source': { 'kind': 'derived', 'rationale': '根据剩余对象清理与组件关闭的不同责任设计。' },
        'companies': [],
      },
      {
        'id': 'memory-pool-q15',
        'level': 'L3',
        'prompt': '怎样验证池确实改善了延迟，而没有把成本挪到别处？',
        'answer': '比较相同对象工作量与生命周期的方案，分别记录池建立、构造、归还、耗尽和关闭成本，并测量端到端分布。控制容量、访问顺序、线程及内存放置，记录原始样本和占用高水位；检查内存足迹及对象内部的分配。预期局部性或少一次通用分配需要实验支持，本例仅有正确性验证，不能给出加速倍数。',
        'rubric': ['等价工作量与完整成本', '分布和容量条件', '对象内部资源与无性能实测'],
        'source':
          { 'kind': 'derived', 'rationale': '根据优化实验的可归因要求和对象池成本转移风险设计。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

内存池预留并复用一组存储，减少反复向通用分配器申请存储的需要。对于带构造、析构的 C++ 对象，还要管理何时建立对象、何时结束借用、何时允许复用槽位。本章用 C++20 固定容量 typed object pool 演示这些步骤：空闲链提供槽位，construct_at 成功后发布带代次的句柄，destroy_at 后归还槽位。它是单线程、单类型的教学实现；容量耗尽明确失败，裸指针借用和 pool 自身寿命仍由调用方约束。

## 核心概念

| 概念      | 本章含义                                  | 需要区分的边界                     |
| --------- | ----------------------------------------- | ---------------------------------- |
| 存储槽位  | 能容纳一个 T 且满足 alignof(T) 的字节区域 | 有空间不代表非平凡 T 已构造        |
| live 对象 | 已成功构造、尚未开始析构的 T              | 归还之后槽位还在，旧对象寿命已结束 |
| 空闲链    | 用索引连接可重新构造对象的槽位            | retired 槽不在链内                 |
| 句柄      | owner、index 和 generation 组成的身份     | 只能在所属 pool 活着时参与操作     |
| 借用指针  | get 返回的当前 T 地址                     | 不增加引用计数，不阻止 destroy     |
| 退役      | 代次到最大值后永久停止复用该槽            | 避免回绕，代价是可用容量减少       |

本例不接受数组、const 或 volatile 的 T，并要求 T 可无异常析构。T 必须是可完整定义并满足本实现对齐要求的对象类型。pool 不可复制或移动，保持槽位地址与 owner 身份稳定；构造和析构 T 时不允许重入同一个 pool。这些是接口前提，并非类型系统已经证明调用方满足了全部要求。

语言语义以 C++20 草案 N4861 为准。对象寿命通常要求获得适当大小、对齐的存储并完成初始化；类对象寿命在析构函数调用开始时结束，构造和析构期间另有访问规则。[N4861 basic.life](https://timsong-cpp.github.io/cppwp/n4861/basic.life)

## 原理深入

### 先预留槽位，再发布对象

try_emplace 先从 free_head 取出一个索引并减少 free_count，然后在该槽的字节区域调用 construct_at。只有构造返回后，slot.object 才记录返回指针，live 才变为真，调用方才得到句柄。这样不会出现“句柄已可查到、对象还没构造完成”的正常可观察状态。

若构造抛异常，catch 将该索引接回空闲链并恢复计数，再传播原异常。此时没有发出新句柄，也不消耗代次。T 构造期间产生的外部副作用不一定能撤销；本例承诺恢复池的槽位状态，不能代替 T 自己的异常安全设计。

std::construct_at 的作用包含在指定位置构造对象。reinterpret_cast 在这里准备地址参数，未承担构造职责；get 始终返回成功构造所得的指针。[N4861 construct_at](https://timsong-cpp.github.io/cppwp/n4861/specialized.construct)

### 已构造成员与失败对象分别清理

Tracked 先构造持有 unique_ptr 的 Resource 成员，再根据 fail 参数抛异常。失败时 Resource 已完成构造，会在异常展开中被销毁；Tracked 自身没有完成构造，不调用其析构函数。因此计数应是一次失败尝试、零个新增 live 对象、零次新增 Tracked 析构，成员资源数回到失败前。

这也解释了池不能在 catch 中无条件对 T 调用 destroy_at。失败路径应交给语言规则清理已完成子对象，再由池恢复元数据。[N4861 constructor exceptions](https://timsong-cpp.github.io/cppwp/n4861/except.ctor)

### 归还同时结束对象身份

destroy 验证 owner、index、live 和 generation，失败返回 false 且不改状态。成功时先关闭该槽的 live 标记，调用 destroy_at，再递增代次并接回空闲链。代次已经最大则退役；该槽不会再用于构造。

本实现允许复制句柄，但所有副本代表同一对象身份。一个副本归还成功后，其余副本均失效。代次不能保护已经取出的 T*：调用方必须在归还前结束全部借用，也不能在 pool 销毁后继续调用 get 或 destroy。把同一地址用于另一个 pool 的新寿命，不会让旧句柄重新合法；本例不承诺检测这种违反前提的使用。

## 数据结构/系统内部实现

每个 Slot 含 alignas(T) 字节数组，以及独立的 next、generation、object 和 live 元数据。元数据不借用 T 的已占用字节，避免存活对象被空闲链指针覆盖；代价是额外空间和可能的填充。数组元素的步长由 Slot 布局决定，不手算为 sizeof(T)。

alignas(T) 使存储满足 T 的对齐要求。示例 alignas(64) 类型在本机属于过对齐类型，因为本机 alignof(max_align_t) 为 16；这些数值不是所有平台的保证。超过基础对齐的支持和适用环境依实现，目标平台不支持所请求对齐时需调整设计。[N4861 alignment](https://timsong-cpp.github.io/cppwp/n4861/basic.align)

字节数组提供存储，并不要求先默认构造 N 个 T。本例显式构造非平凡 Tracked；C++20 的隐式对象创建有类型和操作范围限制，不能据此省略任意 T 的构造。[N4861 object model](https://timsong-cpp.github.io/cppwp/n4861/intro.object)

在一次公开操作返回或传播构造异常后，满足：

```text
free_count + live_count + retired_count = N
空闲链中的每个槽恰好出现一次；live 与 retired 的槽不在空闲链中。
成功发出的句柄，只在 owner、index、live、generation 同时匹配时可用。
```

构造中的预留槽暂时不在 free 或 live 计数中，所以不变量限定在操作边界。单线程和禁止重入让调用方不能在这个中间状态再次操作本池。将来若要支持回调重入或并发，需要重新设计状态和同步，不能沿用这个假设。

pool 析构遍历所有槽，销毁仍 live 的 T。顺序是槽位顺序，可能不同于创建顺序，也不保证逆创建顺序。若 T 的析构依赖另一对象继续存活，调用方应先按依赖关系主动归还。destroy_at 负责调用对象析构，槽位存储的寿命由 pool 管理。[N4861 destroy_at](https://timsong-cpp.github.io/cppwp/n4861/specialized.destroy)

## C++ runnable demo

```cpp include=examples/memory-pool.cpp

```

保持断言启用；本测试有部分操作位于 assert 表达式内，不能加 -DNDEBUG：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/memory-pool.cpp -o /tmp/memory-pool
/tmp/memory-pool
```

WSL2 x86-64、GCC 13.3 的实际输出：

```text
lifecycle: 3 constructed, 3 destroyed, 1 constructor failure
generation: 255 lifetimes, 1 retired slot, no wrap
alignment: Tracked=64 max_align_t=16
all typed-pool checks passed
```

第一个测试池容量为 2。先保留值 11 的对象，再注入一次成员已构造后的失败，核对资源计数和可用容量恢复。随后放入值 22 的对象，确认满池请求不再调用构造函数；归还第一项后构造值 33 的对象，并检查旧句柄、重复归还、默认句柄和跨存活池句柄均被拒绝。离开作用域时，剩余两项由池析构，成功构造与析构数都为 3。

第二个测试把 Generation 改为 uint8_t，以有限执行到达代次边界。容量为 1 的槽完成 255 次成功构造和销毁，随后退役，下一次请求返回 nullopt。测试没有依靠计数器自然回绕，也没有运行悬空指针访问。

对齐断言用 std::align 核对现有对象地址已满足其要求；该工具调整或检查存储位置，不负责对象构造。[N4861 pointer alignment](https://timsong-cpp.github.io/cppwp/n4861/ptr.align)

严格编译和 ASan/UBSan 运行均通过。测试未创建线程；它没有证明并发回收、任意 T 的构造行为或任意扩展对齐都安全。Resource 内部故意使用 make_unique 检查异常展开，所以本程序的对象构造仍有堆分配，不能当作无分配热路径或性能对照。

## 高频追问

### 为什么不用 T objects[N]？

这会预先建立 N 个 T 对象，要求相应初始化合法，也让“空闲槽还没有对象”的状态难以表达。若实际需求允许全量预构造并反复 reset，预构造数组可能更简单，但 reset 的语义、失败处理和资源保留需要重新约定。本例测试每次取得槽位时构造、归还时析构。

### 为什么不直接返回 unique_ptr？

可以设计带自定义 deleter 的所有权封装，让作用域结束时归还池。deleter 仍需引用有效 pool，并保存足够的对象身份；所有权对象若活得比 pool 长，同样会出错。当前实现保留显式句柄，便于展示重复归还和陈旧身份检查，没有提供 RAII lease。

### 空闲链指针放进空槽会更省内存吗？

有些实现复用空槽的字节存放链接，省去一部分旁路元数据。它要同时满足链接与 T 的大小、对齐和生命周期规则，构造 T 前也要保存下一节点。这里把元数据独立存放，使失败回滚和代次验证更容易审查；是否值得压缩应先测空间与访问成本。

### 这是标准 allocator 吗？

本例没有 allocate(n)、deallocate 及 allocator 所需的类型、相等性等约定，也不承诺为 n 个元素提供连续存储。标准 allocator 的存储分配与元素构造需要按要求衔接；不能给 try_emplace 换个名字后就交给 vector 使用。[N4861 allocator requirements](https://timsong-cpp.github.io/cppwp/n4861/allocator.requirements)

## 容易答错的点

| 错误说法                                | 修正与原因                                        |
| --------------------------------------- | ------------------------------------------------- |
| memset 为零就等于初始化了 T             | 非平凡对象需要建立其生命周期和类不变量            |
| sizeof(T) 足够就能放在任意地址          | 起始地址还要满足 alignof(T)                       |
| 构造失败必须再调用 T 的析构函数         | 未完成对象不这样清理，已完成子对象由异常展开处理  |
| destroy 成功表示这块地址已释放          | 本例保留槽位，结束对象寿命后再复用或退役          |
| generation 使任何旧指针都安全           | generation 只在句柄检查时参与，裸指针借用另有期限 |
| uint64 代次足够大就等于不会回绕         | 工程概率与无回绕证明不同，仍须约定边界            |
| 一个槽位 O(1)，整个对象创建就是固定耗时 | T 构造、资源分配、缺页和调度都可能增加成本        |
| 改为原子空闲链就能支持并发              | 还要证明对象发布、使用期间保护与回收安全          |

## 性能分析

固定容量池的初始化需 O(N) 建立空闲链，析构扫描 O(N) 槽位，另外承担所有存活对象的析构成本。一次取槽和归还只更新固定数量元数据，复杂度为 O(1)；完整 API 的成本分别加上 T 构造或析构。构造失败还包括异常处理，本例不承诺该路径低延迟。

内存预算可以从 sizeof(TypedPool<T,N>) 开始，它包含槽位元数据、对齐填充和整池计数，通常大于 N×sizeof(T)。再加上 T 拥有的外部资源、等待归还请求和借用方保留的其他数据。固定槽位数只限制这一层对象数量；示例每个 Resource 的 make_unique 就是池外分配。

池可能降低通用分配调用次数，也可能改善局部性，但大块预留会占用工作集，闲置容量可能无法被其他池使用。退役策略还会降低长期有效容量，需监测 retired 与可用槽高水位，提前安排无借用状态下的安全重建。不能在仍有旧句柄可用时随意把 generation 清零。

性能实验应使用相同 T 和相同对象生存期分布，分别观察首次建池、预热后复用、构造失败和耗尽路径。记录占用、吞吐和端到端 p50/p99/p99.9，控制线程、CPU 和内存放置；对象内部资源策略也必须一致。优化构建用于计时，sanitizer 用于发现有限执行中的问题。本章未测性能，不给出池比 new/delete 更快的数字结论。

## Quant/Low-Latency 场景

行情解码器可以从固定池取得规范化事件，队列和下游处理持有其所有权，最后一个合法使用者结束后归还。预算应覆盖排队、处理和异步完成前保留的对象；只按队列长度配容量，可能漏掉已经出队但还在使用的部分。多个消费者共享对象时，还需要清楚的最终归还责任。

耗尽策略与数据合同有关。不可丢增量的组件不能默默忽略 nullopt，随后继续宣称状态完整；应按协议背压、使状态失效并恢复，或走有明确预算的备用路径。如果回退到通用分配，必须区分释放来源，并承认内存上界和延迟分布发生变化。

线程本地池适合单线程拥有其元数据的模型。跨线程传递对象后，可以把归还请求发回拥有池的线程，但要预算归还队列、消费者停顿和退出阶段。这个教学池不能被另一线程直接并发调用；加同步或采用其他回收机制属于单独的实现工作。

关闭时先停止新请求，结束所有借用和异步访问，再销毁池。池自动清理剩余 live 对象能回收资源，无法阻止仍在运行的组件访问已销毁存储。

## 相关专题

- [对象生命周期与布局](../cpp/object-lifetime-layout.md)：理解存储、构造、对齐和借用的语言基础。
- [RAII 与异常安全](../cpp/raii-exception-safety.md)：区分成员展开、池元数据回滚和外部副作用。
- [ABA 与内存回收](../concurrency/aba-reclamation.md)：代次身份检查之外，并发使用期间还需要回收保护。
- [Benchmark 方法](../performance/benchmark-methodology.md)：构造等价负载，检查对象池是否真的改善目标指标。

## 分层面试题

基础题检查存储与对象接口；机制题沿着失败、复用和借用路径审查不变量；设计题讨论标准容器适配、跨线程归还、容量与关闭合同。
