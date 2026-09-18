---
{
  'schemaVersion': 1,
  'id': 'order-book',
  'title': 'Order Book：从事件到价格档位',
  'description': '实现可验证的 MBO 订单簿，明确订单索引、FIFO、撤单、数量不变量与行情恢复边界。',
  'category': 'trading',
  'areas': ['Order Book', 'Market Data', 'Feed Handler'],
  'tags': ['order-book', 'market-data', 'mbo'],
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
  'updated': '2026-09-18',
  'reviewed': '2026-09-18',
  'standard': 'C++20',
  'estimatedMinutes': 45,
  'prerequisites': ['map / list / unordered_map', '整数溢出', '事件驱动系统'],
  'related': ['epoll-lt-et', 'cpp-memory-model'],
  'demo':
    {
      'file': 'examples/order-book.cpp',
      'platform': 'portable',
      'exercise': '为 demo 加入 replace(old_id, new_id, price, qty)，先写清优先级规则和失败时是否回滚，再用独立事件回放测试验证数量守恒。',
    },
  'references':
    [
      {
        'title': 'Nasdaq TotalView ITCH 5.0 specification',
        'url': 'https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf',
        'kind': 'protocol',
        'accessed': '2026-09-18',
      },
      {
        'title': 'C++ associative container requirements',
        'url': 'https://eel.is/c++draft/associative.reqmts',
        'kind': 'standard',
        'accessed': '2026-09-18',
      },
      {
        'title': 'C++ unordered container requirements',
        'url': 'https://eel.is/c++draft/unord.req',
        'kind': 'standard',
        'accessed': '2026-09-18',
      },
    ],
  'questions':
    [
      {
        'id': 'order-book-q01',
        'level': 'L1',
        'prompt': 'MBO 和 MBP 的信息有什么不同？',
        'answer': 'MBO 包含订单级标识与变更，MBP 通常是价格档汇总。仅凭 MBP 不能重建真实订单队列及个体顺序，不能把汇总量拆成虚构订单。',
        'rubric': ['信息粒度', '不可还原队列'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q02',
        'level': 'L1',
        'prompt': '订单簿与撮合引擎有什么区别？',
        'answer': '行情簿依外部事件维护视图，撮合引擎依据市场规则决定成交及剩余挂单。簿应用 execution 消息并不表示它本身执行了撮合决策。',
        'rubric': ['状态视图', '撮合决策'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q03',
        'level': 'L1',
        'prompt': '价格为什么常用整数 tick 存储？',
        'answer': '整数避免浮点表达误差作为键时带来的问题，也便于校验步长；仍需处理不同产品 tick schedule、转换舍入、范围和溢出，不能只乘一个固定系数。',
        'rubric': ['精确表示', '转换与范围'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q04',
        'level': 'L1',
        'prompt': 'best bid 与 best ask 如何定义？',
        'answer': 'best bid 是当前非空买档最高价，best ask 是非空卖档最低价。空侧应返回明确的缺失状态；不要用可成为有效值的价格作隐式哨兵。',
        'rubric': ['两侧方向', '空簿表示'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q05',
        'level': 'L1',
        'prompt': 'FIFO 能代表所有交易所优先规则吗？',
        'answer': '不能。规则可能涉及价格时间优先、比例分配或特殊订单属性。本例 list 表示教学队列，实际优先级与更改后是否保留位置需要协议依据。',
        'rubric': ['规则差异', '教学假设'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q06',
        'level': 'L2',
        'prompt': '为什么需要 order-id 到订单位置的索引？',
        'answer': '撤单和执行通常按 ID 到达。索引避免扫描全部订单；保存稳定迭代器可直接定位档内节点，但树查找和哈希最坏情况仍要计入整条路径。',
        'rubric': ['直接定位', '完整复杂度'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q07',
        'level': 'L2',
        'prompt': '部分执行或减少数量应维护哪些不变量？',
        'answer': '订单剩余量与档位总量同步减少，拒绝非正或超剩余的减少量；订单归零删除索引和节点，最后节点删除后还要移除价格档。',
        'rubric': ['数量守恒', '空档删除'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q08',
        'level': 'L2',
        'prompt': '重复 ID 或未知撤单应如何处理？',
        'answer': '本例返回 false 且不改变状态；生产系统需按协议判断是否重复传输、乱序、会话错误或 gap，必要时令簿无效并恢复，不能总是静默忽略。',
        'rubric': ['无副作用失败', '协议错误分类'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q09',
        'level': 'L2',
        'prompt': '为什么禁止默认复制含迭代器索引的簿？',
        'answer': '复制容器后原索引保存的迭代器不会自动重定位到新容器，可能指回旧对象。应禁止复制或从复制后的节点重建索引，并验证独立生命周期。',
        'rubric': ['迭代器归属', '重建或禁止'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q10',
        'level': 'L2',
        'prompt': '该 map/list/hash 方案撤单是 O(1) 吗？',
        'answer': '当前方案还需按价格查找 map，为 O(log P)，ID 哈希期望 O(1) 但最坏 O(N)，档内 list 迭代器删除才是常数级。应说明整个操作路径。',
        'rubric': ['树查询成本', '期望与最坏'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q11',
        'level': 'L3',
        'prompt': '行情 sequence gap 后可以继续使用旧簿吗？',
        'answer': '应把视图标为无效，按策略停止依赖该簿的决策，并通过快照和连续增量恢复。旧簿可作诊断展示，但不能冒充完整最新状态。',
        'rubric': ['有效性状态', '恢复协议'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q12',
        'level': 'L3',
        'prompt': '快照和实时增量如何衔接？',
        'answer': '记录快照基准序号，缓冲增量，只应用该基准之后连续且去重的消息。必须确保会话和序列域一致，避免重复应用或跨过缺口。',
        'rubric': ['序号边界', '连续与去重'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q13',
        'level': 'L3',
        'prompt': '什么时候用数组或 bitmap 替代价格树？',
        'answer': '价格范围有界、tick 密集且内存预算允许时可考虑数组与非空 bitmap；宽范围稀疏市场可能浪费内存。要测最佳价跳跃、重置与极端输入。',
        'rubric': ['密度与内存', '边界负载'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q14',
        'level': 'L3',
        'prompt': '如何验证优化后的订单簿没改坏语义？',
        'answer': '使用独立的简单参考模型，逐事件回放对比订单集合、档位总量和 best，加入重复、超量、溢出、空档等边界，再比较性能。',
        'rubric': ['独立oracle', '逐事件不变量'],
        'source':
          {
            'kind': 'derived',
            'rationale': '根据本专题机制、正确性边界和目标岗位工程职责推导；不声称为公司真题。',
          },
        'companies': [],
      },
      {
        'id': 'order-book-q15',
        'level': 'L3',
        'prompt': '为什么单写者簿仍可能有延迟尖峰？',
        'answer': '分配器、哈希 rehash、cache miss、页错误和下游队列背压都可造成尖峰。单写者减少锁竞争，不消除内存与调度成本，需记录分位数和容量行为。',
        'rubric': ['非锁成本', '容量与背压'],
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

Order Book 按买卖方向与价格组织挂单。MBO 需要价格档、档内队列和订单 ID 索引；MBP 不能还原订单排队位置。行情簿依消息重建状态，撮合引擎决定成交。实现要维护数量、空档和最佳价不变量，并在序号缺口后标记无效、执行恢复。

## 核心概念

| 对象         | 本例定义                 | 边界                             |
| ------------ | ------------------------ | -------------------------------- |
| Price        | 正整数 tick，int64       | 不处理负价产品或多 tick schedule |
| Order ID     | 会话内唯一的 uint64      | 重连后是否复用依协议而定         |
| Level        | 同一方向同价格的一组订单 | total 等于订单剩余量之和         |
| Best bid/ask | 买方最高价 / 卖方最低价  | 空簿用 optional，不用价格 0 冒充 |
| MBO / MBP    | 订单级 / 档位级          | 信息粒度不同                     |

FIFO 是本例队列组织和教学假设，真实市场可能采用不同优先规则；不能把本地加入顺序视为所有交易所的实际排队位置。

## 原理深入

add 检查 ID、价格与数量，创建价格档，加入队尾并建立索引。reduce 根据 ID 定位，只允许正数量且不超过剩余量；减少订单和档位总量，订单归零时删除，价格档为空时删除。cancel 是按当前剩余量执行完整 reduce。

核心不变量：每个 ID 只有一个位置；所有订单数量为正；档位非空；total 精确等于档内数量和；best 来自非空档位的有序边界。duplicate add、unknown reduce 和 over-reduction 在本例返回 false，且状态不变。生产中如何处理异常事件要依据 feed 协议及会话状态，不能静默忽略后仍标为有效。

公开协议区分 add、execution、cancel、delete、replace 等消息，不能把所有字段变化归为相同操作。本文仅选择可测试的 add/reduce/cancel 子集，不声称实现完整 [ITCH 协议](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf)。

## 数据结构/系统内部实现

每侧用 `map<Price, Level>` 保存价格顺序，Level 内 `list<Order>` 保存稳定迭代器，unordered_map 把 ID 映射到方向、价格和订单迭代器。买侧取 rbegin，卖侧取 begin。删除订单使用保存的迭代器，不做档内线性扫描。

设 P 是价格档数量，N 是订单数。add 和 reduce/cancel 的当前实现都需要 O(log P) 树查找，加上 ID 哈希表期望 O(1) 查询；哈希表最坏可退化 O(N)。取最佳价格 O(1)，内存 O(P+N)。不能因为有 ID 索引就把整条撤单路径声称为 O(1)。保留 Level 迭代器可省去部分查找，但需要重新审计失效规则。

对象含有内部迭代器，默认复制会让新对象的索引指回旧对象，故禁止复制。本例还对数量加法做溢出检查，并在索引插入分配失败时回滚新订单；它仍使用普通堆分配，不是生产低延迟容器。

## C++ runnable demo

断言覆盖空簿、同价 FIFO、部分减少、重复 ID、未知订单、超量减少、最后订单删除与数量溢出。该组件不产生交易、不交叉撮合、不解析二进制报文。

```cpp include=examples/order-book.cpp

```

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pthread examples/order-book.cpp -o /tmp/book-demo
/tmp/book-demo
```

练习前写出 replace 的优先级规则：修改数量是否丢时间优先级、改价是否相当于撤单加单。不要在没有协议依据的情况下给出统一答案。

## 高频追问

“连续价格范围很小，为什么还用 map？”map 是容易验证的基线；密集 tick 可使用数组加非空 bitmap，稀疏广域价格则要考虑分段结构。比较时计入重置、最优价移动、内存占用和极端价格输入。

“订单簿能保证不交叉吗？”这个组件不执行撮合，因此允许传入交叉买卖价。实际可见簿是否短暂交叉取决于市场、会话与消息语义；不能在所有 feeds 上硬套 best_bid 小于 best_ask 的不变量。

## 容易答错的点

- MBP 深度不能直接推出真实订单 FIFO 或自身排队位置。
- 用 double 当价格键会引入表达和比较问题；tick 转换要校验范围和步长。
- unordered_map 查询是期望复杂度，不是最坏 O(1)。
- 行情重建簿不等于撮合引擎；“执行消息减少数量”也不等于自己决定成交。
- 缺一条更新可能使簿永久错误，不能只靠后续消息修复。

## 性能分析

用可回放、固定种子的事件流测 add/reduce/cancel 分布，包含高撤单率、热点价格、价格跳跃及重建。记录每事件耗时分位数、分配次数、内存峰值、cache miss 和最终状态摘要。以简单参考模型对照每一步结果，再做内存池或平坦存储优化。

热路径避免不受控 rehash 和逐订单分配可改善稳定性，但预留容量不是无限保证，必须处理容量耗尽。缓存优化与字段布局可以接着看 [false sharing](../performance/cpu-cache-false-sharing.md)，单线程簿尤其要先关注 locality 而非并发锁技巧。

## Quant/Low-Latency 场景

feed handler 校验会话与序号，将可解释事件送入单写者簿。检测 gap 后转为 invalid，缓冲增量并获取快照；只应用快照序号之后、连续的增量，恢复到 valid 才允许下游依赖该视图。快照版本与增量序号必须来自同一协议的可比较序列域。

这个恢复状态机是系统集成要求，demo 没有实现。演示数据并非交易所真实流量，也没有据此声称纳秒级延迟。

## 相关专题

- [epoll LT vs ET](../network/epoll-lt-et.md)：行情传输与分帧入口。
- [C++ memory model](../concurrency/cpp-memory-model.md)：向策略线程发布事件。
- 后续规划：Matching Engine、Feed Handler、snapshot recovery、OMS / pre-trade risk。

## 分层面试题

L1 检查术语，L2 检查不变量与容器边界，L3 检查 feed 语义和系统恢复。全部是岗位知识推导题。
