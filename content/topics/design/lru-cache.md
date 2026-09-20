---
{
  'schemaVersion': 1,
  'id': 'lru-cache',
  'title': 'LRUCache：链表顺序、哈希索引与异常回滚',
  'description': '用 list 与 unordered_map 实现固定整数键值的 LRU，证明命中、淘汰和插入失败后的双容器一致性，并用参考模型与分配故障验证。',
  'category': 'design',
  'areas': ['Algorithm Coding', 'System Design'],
  'tags': ['lru', 'cache', 'unordered-map', 'exception-safety'],
  'difficulty': 'L2',
  'roles': ['C++ Developer', 'Quant Developer', 'Low-Latency C++ Developer'],
  'companyTypes': ['高频交易', '量化私募', 'Trading Firm'],
  'status': 'published',
  'updated': '2026-09-19',
  'reviewed': '2026-09-19',
  'standard': 'C++20',
  'estimatedMinutes': 55,
  'prerequisites': ['std::list 与迭代器', '哈希表平均和最坏复杂度', 'RAII 与基本、强异常保证'],
  'related':
    ['raii-exception-safety', 'vector-invalidation', 'shared-mutex', 'cpu-cache-false-sharing'],
  'demo':
    {
      'file': 'examples/lru-cache.cpp',
      'platform': 'portable',
      'exercise': '增加不改变最近使用顺序的 peek，并在 vector 参考模型中独立实现；让 get 与 peek 交替出现，比较返回值和完整顺序，再确认分配失败不会提前淘汰旧条目。',
    },
  'references':
    [
      {
        'title': 'C++20 draft N4861: list splice operations',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/list.ops',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: list modifiers',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/list.modifiers',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: unordered associative container requirements',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/unord.req',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: unordered container exception guarantees',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/unord.req.except',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: standard hash specializations',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/unord.hash',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: allocator requirements',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/allocator.requirements',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'Redis documentation: key eviction and approximated LRU',
        'url': 'https://redis.io/docs/latest/develop/reference/eviction/',
        'kind': 'manual',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'lru-cache-q01',
        'level': 'L1',
        'prompt': 'LRU 按什么规则决定淘汰对象？',
        'answer': 'LRU 淘汰当前缓存中最久没有被使用的条目。本文把成功 get、新键 put 和已有键更新都算作使用；未命中 get 不改变顺序。链表头为最近使用，尾为下一次容量淘汰候选。',
        'rubric': ['按最近使用时间而非使用次数', '明确哪些操作更新顺序', '头尾含义'],
        'source': { 'kind': 'derived', 'rationale': '由 LRU 策略定义与本文访问契约推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q02',
        'level': 'L1',
        'prompt': '为什么同时使用 list 和 unordered_map？',
        'answer': 'unordered_map 按键平均常数时间找到 list 节点，list 通过已知迭代器常数时间调整顺序和删除节点。单独使用 list 查键要线性扫描，单独使用无序哈希表则没有最近使用顺序。',
        'rubric': ['哈希表按键定位', '链表维护顺序', '各自弥补的操作成本'],
        'source': { 'kind': 'derived', 'rationale': '由查找与重排两类操作的数据结构需求推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q03',
        'level': 'L1',
        'prompt': '容量为零和容量为一时，put 的语义是什么？',
        'answer': '本例容量零表示不保存条目，put 直接返回，get 始终未命中。容量一时更新已有键只改值，插入另一个键则在新条目准备成功后淘汰旧键；插入失败仍保留旧键。',
        'rubric': ['明确零容量契约', '更新不增加条目', '失败不能丢旧值'],
        'source':
          { 'kind': 'derived', 'rationale': '由容量边界和新增、更新路径的不同后置条件推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q04',
        'level': 'L1',
        'prompt': '更新已有键时，需要增加节点或淘汰旧条目吗？',
        'answer': '不需要。找到已有节点后更新 int 值，再把该节点移到链表头，缓存大小保持不变。先按新键路径插入会产生重复键或无谓淘汰，破坏索引与链表的一一对应。',
        'rubric': ['原节点改值并移到头部', '大小不变', '不能制造重复键'],
        'source': { 'kind': 'derived', 'rationale': '由更新命中语义和唯一键不变量推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q05',
        'level': 'L1',
        'prompt': '为什么 get 返回 optional<int> 而不约定 -1 表示未命中？',
        'answer': 'optional 把未命中与合法整数值区分开，因此 -1 等值也能被缓存。返回 int 的拥有副本还避免了条目被淘汰后悬空的引用；代价是值需要复制，本例 int 的复制很小且不抛出。',
        'rubric': ['缺失与合法值分离', '拥有副本的生命周期', '固定值类型的代价'],
        'source':
          { 'kind': 'derived', 'rationale': '由缓存缺失表示和值返回生命周期的接口选择推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q06',
        'level': 'L1',
        'prompt': '条目容量、内存上限与 TTL 是同一项约束吗？',
        'answer': '不同。本文容量只限制公开操作完成后的条目数量，不直接限制桶数组和节点开销；TTL 约束条目的有效时间。即使条目最近被访问，它仍可能按业务规则过期，LRU 本身没有判断数据是否新鲜。',
        'rubric': ['条目数不等于字节数', 'TTL 是独立有效性条件', 'LRU 不验证新鲜度'],
        'source':
          { 'kind': 'derived', 'rationale': '由缓存容量口径、分配成本与有效期条件的区别推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q07',
        'level': 'L2',
        'prompt': '怎样表述 list 与 unordered_map 之间的不变量？',
        'answer': '每个链表键唯一，索引中恰有一个相同键且映射到该节点；索引中也不能存在没有对应节点的条目。公开操作结束时两者大小相等且不超过容量，链表顺序必须符合已完成的访问历史。',
        'rubric': ['键与节点一一对应', '容量与大小一致', '顺序符合历史'],
        'source': { 'kind': 'derived', 'rationale': '由组合数据结构的完整性和 LRU 顺序语义推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q08',
        'level': 'L2',
        'prompt': '用 splice 提升命中节点有什么迭代器保证？',
        'answer': '同一 list 内移动单节点不会销毁或重建该元素，指向它的迭代器和引用仍然有效，索引不必重写。该操作是常数复杂度且不抛出；若节点已在目标位置，规定的相邻或重合情形保持原状。',
        'rubric': ['移动节点而非重建元素', '既有迭代器有效', '常数复杂度与不抛出'],
        'source': { 'kind': 'derived', 'rationale': '由 list 单元素 splice 的语义和复杂度推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q09',
        'level': 'L2',
        'prompt': 'unordered_map rehash 后，缓存索引中的 list 迭代器还能使用吗？',
        'answer': '能，rehash 失效的是 unordered_map 自身的迭代器，存储在 mapped value 中的 list 迭代器仍指向未被删除的链表节点。map 元素的引用和指针也不会因 rehash 失效；但不能跨可能 rehash 的 emplace 使用旧 map 迭代器。',
        'rubric':
          ['区分两种容器的迭代器', 'map 元素引用不因 rehash 失效', '不保留旧 map 迭代器跨插入'],
        'source':
          { 'kind': 'derived', 'rationale': '由无序容器重哈希规则和独立链表生命周期推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q10',
        'level': 'L2',
        'prompt': '新增 list 节点后，map 插入抛出 bad_alloc 应如何恢复？',
        'answer': '本例固定哈希与比较不抛出，单元素 map 插入因分配失败没有效果。catch 删除刚加到链表头的节点后重抛，旧值与旧顺序不变。只有两个容器都准备成功后才淘汰旧尾，避免失败时损失原条目。',
        'rubric': ['核对 map 无效果保证的前提', '删除新链表节点并重抛', '延后淘汰'],
        'source': { 'kind': 'derived', 'rationale': '由双容器插入的异常路径和延后淘汰顺序推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q11',
        'level': 'L2',
        'prompt': '为什么不能直接使用这个缓存的默认复制？',
        'answer': '复制 list 会创建新节点，但复制 map 的 mapped value 只会复制迭代器值，仍指向源 list。复制后的缓存可能操作源节点或在源对象销毁后持有悬空迭代器。正确复制要按新链表重建索引，本例直接删除复制和移动操作。',
        'rubric': ['新节点与复制迭代器不自动绑定', '指出跨对象或悬空风险', '重建索引或禁止复制'],
        'source': { 'kind': 'derived', 'rationale': '由跨成员保存迭代器时的逐成员复制行为推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q12',
        'level': 'L2',
        'prompt': '把 int 值改成任意 T 后，已有键更新还具有强保证吗？',
        'answer': '不一定。T 的赋值可能修改一部分状态再抛出，返回值复制也可能在顺序已更新后抛出。需要逐个规定赋值、复制、析构、哈希和比较的异常约束，或改用临时对象与不抛出的提交；不能只给类加模板参数。',
        'rubric': ['赋值可能部分修改', '返回值复制也有失败路径', '泛型化需重新证明保证'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由固定整数实现推广到可抛出值类型的新增失败路径推导。',
          },
        'companies': [],
      },
      {
        'id': 'lru-cache-q13',
        'level': 'L3',
        'prompt': '为什么共享锁不能直接保护这个缓存的 get？',
        'answer': 'get 命中后会用 splice 修改最近使用顺序，多个 get 同时执行也会写同一链表。可用独占锁覆盖索引、值和顺序更新，并在锁内复制返回值；也可改变策略减少顺序写入，但那需要另行定义并发语义。',
        'rubric': ['命中读取包含链表写操作', '锁覆盖完整不变量', '解锁前取得安全结果'],
        'source':
          { 'kind': 'derived', 'rationale': '由精确 LRU 的命中更新路径与共享访问条件推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q14',
        'level': 'L3',
        'prompt': '把缓存按 key 分片后，还能声称全局精确 LRU 吗？',
        'answer': '通常只能保证每片内部的 LRU。一个热点片可能已经满而其他片仍有空位，被淘汰的键未必是全局最久未使用。需要说明容量分配和热点倾斜，并测锁等待、每片命中率及负载；动态迁移还涉及所有权和同步。',
        'rubric': ['局部顺序不同于全局顺序', '容量倾斜', '测量与迁移成本'],
        'source': { 'kind': 'derived', 'rationale': '由分片局部顺序与全局淘汰条件的差异推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q15',
        'level': 'L3',
        'prompt': '如何验证淘汰顺序正确，而不只检查 get 的返回值？',
        'answer': '维护一个用 vector 线性查找和重排的独立参考模型，对同一轨迹逐步比较返回值与完整 MRU 到 LRU 顺序。每步再检查 map/list 一一对应，覆盖零容量、更新、未命中和删除；只看最终值可能漏掉延迟到下一次淘汰才暴露的顺序错误。',
        'rubric': ['独立简单模型', '逐步比较完整顺序', '组合结构不变量与边界'],
        'source':
          { 'kind': 'derived', 'rationale': '由缓存顺序错误的延迟显现特征与模型比较方法推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q16',
        'level': 'L3',
        'prompt': 'reserve(capacity) 能保证新增缓存条目完全不分配吗？',
        'answer': '不能，它主要准备哈希桶容量，list 节点和 map 节点仍可能逐次分配，本实现还会临时保留容量加一的条目。无动态分配的需求需要预建节点池和索引存储，并定义耗尽行为，不能只 reserve 后就声称满足硬内存上限。',
        'rubric': ['桶容量与节点分配分离', '临时超出条目数的准备空间', '预分配与耗尽策略'],
        'source':
          {
            'kind': 'derived',
            'rationale': '由桶数组、节点拥有关系与先插入后淘汰的内存需求推导。',
          },
        'companies': [],
      },
      {
        'id': 'lru-cache-q17',
        'level': 'L3',
        'prompt': '如何判断 LRU 是否适合某条低延迟数据路径？',
        'answer': '回放保留时间局部性和冷启动的访问轨迹，测命中率、淘汰率、分配次数、端到端尾延迟与未命中回源成本。容量之外还要扫描一次性访问流量，检查它是否挤掉热点；平均常数复杂度不足以说明延迟和整体收益。',
        'rubric': ['保留访问轨迹局部性', '缓存与回源合并测量', '扫描污染和尾延迟'],
        'source':
          { 'kind': 'derived', 'rationale': '由缓存策略、工作集变化和低延迟测量口径推导。' },
        'companies': [],
      },
      {
        'id': 'lru-cache-q18',
        'level': 'L3',
        'prompt': '交易系统能把订单状态放进普通 LRU 后任意淘汰吗？',
        'answer': '只有可按明确规则重建的副本才适合按缓存策略淘汰。若该条目是未完成订单的唯一状态，淘汰会丢失业务责任；需要可靠状态存储及恢复边界。即便缓存证券元信息，也需按版本或失效通知控制新鲜度，命中不代表数据仍有效。',
        'rubric': ['区分可重建副本和唯一业务状态', '可靠恢复边界', '新鲜度独立验证'],
        'source': { 'kind': 'derived', 'rationale': '由缓存可淘汰性与交易状态生命周期约束推导。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

LRU 在容量不足时淘汰最久未使用的条目。常见实现用哈希表按键找到双向链表节点，命中就把节点移到头部，尾部保留淘汰候选。查找、更新和删除平均 O(1)，哈希冲突下最坏可到 O(n)。C++ 实现还要保持两个容器一一对应：新链表节点建立后，哈希插入失败要删回该节点；全部准备成功才淘汰旧尾。命中会修改顺序，所以这个 get 也需要写同步。

## 核心概念

本文缓存固定为 `int` 键和 `int` 值，容量表示条目数。链表头是 MRU（最近使用），尾是 LRU（最久未使用）。`get` 命中、新键 `put` 和已有键更新都会把条目置于头部；查询缺失键不改变顺序。容量为零表示禁用存储，`put` 不保存数据。其他接口可以采用不同约定，但实现、测试和调用者必须使用同一份语义。

LRU 只用最近访问顺序决定淘汰。LFU 主要考虑访问频率，TTL 决定条目是否过期，数据版本则判断副本是否仍可使用。这些条件各有用途：刚被读取的旧配置仍可能失效，长时间未读的静态数据也未必过期。真实系统还可能采用近似策略；例如 [Redis 的淘汰文档](https://redis.io/docs/latest/develop/reference/eviction/) 说明其 LRU 通过采样选择候选，并不维护本文这样的全局精确顺序。

`get` 返回 `optional<int>`：无值表示未命中，有值表示一个独立整数副本。缓存内部节点被删除后，先前返回的值仍可使用。缓存不负责回源加载，也没有 TTL、持久化、并发控制或动态调整容量。

## 原理深入

### 两个容器要描述同一组条目

`order_` 保存 `(key, value)`，`index_` 保存 `key -> list::iterator`。公开操作完成后，应满足：每个链表键唯一；索引恰好指向同键节点；两个容器大小相等且不超过容量。链表顺序还必须与已完成的访问历史相符，仅检查大小不能发现错误的淘汰候选。

以容量 2 为例，表中左侧是 MRU：

| 操作         | 返回值 | 链表顺序     | 解释                   |
| ------------ | ------ | ------------ | ---------------------- |
| `put(1, 10)` | 无     | `1:10`       | 新条目最近使用         |
| `put(2, 20)` | 无     | `2:20, 1:10` | 缓存装满               |
| `get(1)`     | `10`   | `1:10, 2:20` | 只调整命中节点         |
| `put(3, 30)` | 无     | `3:30, 1:10` | 淘汰最久未用的键 2     |
| `put(1, 11)` | 无     | `1:11, 3:30` | 更新已有键，不增加条目 |

命中后用同一链表的单元素 `splice` 把节点移到头部。节点及其迭代器保持有效，因此索引中保存的 list 迭代器无需改写。该操作在满足前置条件时不抛出且为常数复杂度，见 [N4861 list.ops](https://timsong-cpp.github.io/cppwp/n4861/list.ops)。

### 新增操作把淘汰放在最后

新键 `put` 先在链表头构造节点，再将它的迭代器插入哈希索引。第一步可能因分配失败而抛出；[list 单元素插入](https://timsong-cpp.github.io/cppwp/n4861/list.modifiers) 此时没有效果。第二步如果失败，`catch` 删除刚添加的头节点并重抛，恢复旧顺序。

这个证明依赖具体类型。demo 的键和值为 `int`，哈希与比较不抛出，析构和释放不抛出；标准整数 `hash` 的异常条件见 [unord.hash](https://timsong-cpp.github.io/cppwp/n4861/unord.hash)。在这些前提下，哈希表单元素插入的分配异常没有效果，见 [unord.req.except](https://timsong-cpp.github.io/cppwp/n4861/unord.req.except)。如果允许哈希函数抛出，不能直接套用这条插入无效果保证。

两个容器都插入成功后，若超过容量，先删掉旧尾节点对应的哈希项，再弹出链表尾。淘汰阶段在本例的类型约束下不抛出，因而不再需要回滚。若先淘汰后分配新节点，分配失败就会让调用者失去原有条目。

因此强保证针对键值和最近使用顺序成立，但准备阶段会短暂存在 `capacity + 1` 个条目。这是条目上限在公开操作边界成立的实现，不能用于承诺任何时刻都不超出某个字节预算。

### 更新和删除也有顺序要求

更新已有键只赋值并调整节点位置。`int` 赋值不抛出；推广到任意值类型后，赋值可能改到一半再失败，返回值复制也可能引入异常路径。泛型缓存需要重新规定这些操作的保证。

删除先从索引找到 list 迭代器，然后删除索引项，最后销毁链表节点。这样不会留下指向已销毁节点的索引。demo 不执行用户回调，也不支持重入；如果加入淘汰通知，回调是否会抛出、是否再次访问缓存，都需要单独设计。

## 数据结构/系统内部实现

链表解决已知节点的重排，哈希表解决按键定位。标准保证的是操作语义与复杂度，节点字段布局、桶数组组织和哈希缓存方式由标准库实现决定。通常这两个容器都有节点及分配开销，不能把“双向链表加哈希”当作连续内存方案。

| 保存的对象                    | 能否跨 `index_` 的 rehash 使用 | 失效条件举例                   |
| ----------------------------- | ------------------------------ | ------------------------------ |
| `unordered_map` 迭代器        | 不能依赖仍有效                 | rehash，或对应索引项被删       |
| 指向 map 元素的指针或引用     | rehash 不使其失效              | 对应索引项被删                 |
| 索引中保存的 `list::iterator` | 可以，链表未被 rehash 操作修改 | 对应链表节点被删               |
| 指向缓存内部 value 的引用     | 本实现不向外提供               | 若提供，淘汰或删除该节点后失效 |

这些对象不能统称为“缓存迭代器”。[无序容器要求](https://timsong-cpp.github.io/cppwp/n4861/unord.req) 明确区分 rehash 对迭代器、指针和引用的影响。实现不保留 map 迭代器跨越可能扩容的 `emplace`；淘汰时从链表尾拿键，再查找并删除索引项。链表单节点移动不破坏索引中的 list 迭代器。

默认复制也有风险：新 list 有自己的节点，复制 map 却只复制了指向旧 list 的迭代器值。正确复制需要遍历新链表重建索引。本例删除了复制构造、复制赋值、移动构造与移动赋值。某些分配器条件下可以设计正确移动，但仍需处理两个成员的关联、失败路径和源对象状态，不能从单个容器可移动直接推出组合类的契约。

`FaultAllocator` 只为测试加入，它把实际分配转交给 `std::allocator`，并在指定的分配调用抛出 `bad_alloc`。list 和 map 的 allocator rebind 共享同一测试计数，释放不抛出；相关接口要求见 [allocator.requirements](https://timsong-cpp.github.io/cppwp/n4861/allocator.requirements)。全局探针仅用于有界单线程测试，不是生产内存池。

## C++ runnable demo

核心缓存后面附有线性 vector 参考模型、边界断言和故障注入。阅读时先检查 `get`、`put`、`erase`，再看 `CacheTestAccess::check` 如何验证索引与节点对应关系。参考模型不保存 list 迭代器，用独立表示降低两份实现复制同一错误的概率。

```cpp include=examples/lru-cache.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror examples/lru-cache.cpp -o /tmp/lru-demo
/tmp/lru-demo
```

程序使用 C++20 标准库，平台可移植；断言必须保持启用，不要定义 `NDEBUG`。预期输出：

```text
LRU boundaries, 10000 oracle steps, rehash and allocation rollback: OK
```

边界测试覆盖容量 0/1、命中更新、淘汰、重复删除、未命中，以及返回值在条目淘汰后仍然有效。模型测试对容量 0 至 4 各运行 2000 步，使用固定种子的有界整数轨迹，每步比较返回值和完整 MRU 到 LRU 序列。每隔 31 步请求一次不同桶数的 rehash，再检查双容器不变量；不假定实现选择某个精确桶数。

分配失败测试先测出一次新增实际经过多少次测试分配，然后每次重建同样的初态，依次令第 0、1 等次分配失败。空缓存和满缓存分别测试，失败后核对旧快照、未释放分配块数和结构一致性，再确认缓存仍可使用。计数随标准库分配策略变化，程序没有写死“新增必定分配三次”。

可另用 `-D_GLIBCXX_DEBUG` 在 libstdc++ 上检查迭代器误用，或启用 ASan/UBSan 检查有限执行中的内存问题。这些运行不证明所有访问轨迹都正确，也没有验证恶意哈希、任意 `T` 或并发访问。

## 高频追问

### get 为什么没有 const？

命中要改变最近使用顺序。若需求包含“只看值、不延长留存优先级”，应另设不改变顺序的 `peek`，并让测试区分这两种访问。简单把链表标成 `mutable` 可以满足语法上的 const，却不会消除共享写入。

### reserve 后是否就不会抖动？

`unordered_map::reserve` 针对桶容量，不等于预构造全部 map 节点，更不会替 list 准备节点。新增和淘汰仍有分配、析构与释放成本。本例还先保留新旧条目再淘汰，因此计划容量时需要留出临时准备空间。若要求操作期间不分配，需采用预建存储、可复用节点和确定的耗尽行为。

### 为什么不能把 list 换成 vector，再保存 vector 迭代器？

vector 的插入、删除和重分配有不同失效规则，移动到头部还要搬移中间元素。可以设计基于稳定槽位编号的数组链表，但那已经是另一种表示，需要维护空闲槽、前后链接和索引版本，不能直接替换容器类型。

### 完整缓存读取能一直使用共享锁吗？

本实现的命中会写链表，应该用同一把独占锁保护索引和值及顺序变化，并在解锁前取得安全返回值。分片可以分散竞争，但每片维护的只是局部 LRU。近似更新顺序可以减少共享写入，代价是淘汰语义变化，需要测它对命中率与延迟的影响。

## 容易答错的点

- 把 LRU 解释成淘汰访问次数最少的键。本文维护最近访问顺序，没有频次计数。
- 每次 `put` 都先淘汰。已有键更新不增加条目；新键也要准备成功后才淘汰，才能保留失败时的旧状态。
- 认为 rehash 会让索引中保存的 list 迭代器失效。rehash 操作的是哈希容器；list 节点生命周期由链表操作决定。
- 用哈希表 `operator[]` 查找缺失键。它可能插入默认值，本实现应使用 `find`，不能留下未关联有效节点的索引项。
- 直接复制含有跨成员迭代器的类。逐成员复制不会自动把旧链表迭代器映射到新节点。
- 把 `get` 的借用引用直接返回给解锁后的调用者。另一个线程可能立即淘汰对应节点；返回值副本或具有明确所有权的对象才便于约定生命周期。
- 把节点数上限称为严格内存上限。桶数组、节点头、分配器开销和准备阶段的新节点都需要计算。

## 性能分析

令 `n` 为当前条目数，`B` 为哈希桶数；固定整数键使哈希和比较本身为常数工作。

| 操作          | 平均复杂度 | 最坏复杂度 | 额外工作                         |
| ------------- | ---------- | ---------- | -------------------------------- |
| `get`         | O(1)       | O(n)       | 命中后常数时间 splice            |
| `put`         | O(1)       | O(n)       | 新节点分配、可能扩容、满时淘汰   |
| `erase`       | O(1)       | O(n)       | 哈希查找与删除，加链表单节点删除 |
| `snapshot`    | O(n)       | O(n)       | 分配并复制全部条目               |
| 显式 `rehash` | O(n)       | O(n²)      | 重建桶组织，测试专用访问器调用   |

复杂度按 [无序容器操作要求](https://timsong-cpp.github.io/cppwp/n4861/unord.req) 与 list 的单节点操作组合，不能只报链表移动的 O(1)。标准给出的平均复杂度也不代表某次操作的时间上界。测试检查器会遍历并查找全部条目，它的成本不属于核心 `get/put`，基准测试时不应把每步检查算进缓存 API 耗时。

空间应写成 O(n + B)，包括节点和桶。桶数会受 reserve/rehash 及实现策略影响，不能只用当前条目数推算实际内存。缓存满时，每个新键都经历新节点分配与旧节点释放，可能改变分配器状态和尾延迟；预留桶并不能消除这些动作。

评估策略时回放保留时间顺序的轨迹，覆盖冷启动、稳定热点和一次性扫描。只用均匀随机访问会掩盖时间局部性。分别记录命中率、淘汰率、每次分配、缓存操作 p50/p99/p99.9，以及未命中回源后的端到端延迟。若增加锁或分片，还需测锁等待和热点片负载。本文没有性能实测，不给出固定吞吐或加速倍数。

## Quant/Low-Latency 场景

可重建的证券元信息、格式化结果或历史查询结果可以作为缓存候选，但键要包含影响结果的版本或上下文。风控配置和证券状态变化时，需要失效通知或版本校验；“LRU 命中”只说明条目仍驻留，不能证明它对当前请求有效。

未完成订单的唯一状态、尚未确认发送结果的记录，不能因最近没有访问就被淘汰。缓存层只应保存有可靠来源的副本；发生未命中后是否能重建、重建期间是否允许继续处理，要在组件协议中说明。

市场数据回放可能突然扫描大量只用一次的键，将小工作集中的热点挤出。可以评估准入规则、分段策略或近似淘汰，但应将命中率、额外元数据和热路径写入放在同一条回放上比较。对极短处理链，list 指针访问、哈希节点与分配器开销可能已经超过省下的工作，需要先测缓存整体是否有收益。

## 相关专题

- [RAII 与异常安全](../cpp/raii-exception-safety.md)：本章插入失败回滚与延后淘汰的前置知识。
- [vector 失效边界](../cpp/vector-invalidation.md)：比较两种容器保存迭代器时的约束，也用于理解线性参考模型。
- [shared_mutex](../concurrency/shared-mutex.md)：检查命中路径的共享写入，以及解锁后结果的生命周期。
- [CPU cache / false sharing](../performance/cpu-cache-false-sharing.md)：继续分析节点布局、共享元数据和跨核写入成本。

## 分层面试题

L1 先说明访问语义和容量边界，L2 沿每次操作检查两个容器何时一致，L3 再加入异常、并发和真实负载约束。作答时至少手推一条包含命中提升与满容量插入的轨迹，并指出新节点分配失败时旧尾应保留在哪里。
