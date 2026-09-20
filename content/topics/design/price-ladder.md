---
{
  'schemaVersion': 1,
  'id': 'price-ladder',
  'title': 'Price Ladder：整数 tick、占用位图与最优档',
  'description': '把固定 tick 的有界价域映射为数组，用占用位图寻找非空档位，并用独立 map 模型核对价格、数量和整数边界。',
  'category': 'design',
  'areas': ['Algorithm Coding', 'System Design'],
  'tags': ['price-ladder', 'fixed-point', 'tick-size', 'bitmap', 'market-data'],
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
  'estimatedMinutes': 45,
  'prerequisites': ['整数表示与溢出', '数组、map 和位运算', 'MBP 与价格档'],
  'related':
    ['order-book', 'snapshot-recovery', 'cpu-cache-false-sharing', 'benchmark-methodology'],
  'demo':
    {
      'file': 'examples/price-ladder.cpp',
      'platform': 'portable',
      'exercise': '增加返回前 k 个非空档位的只读方法，在局部位图副本上逐次清除当前最低位；分别检查 k=0、超过非空档数和第 63 位，比较 map 参考结果，并验证原 ladder 不变。',
    },
  'references':
    [
      {
        'title': 'JPX: Tick Size for Domestic Stocks',
        'url': 'https://www.jpx.co.jp/english/equities/trading/domestic/07.html',
        'kind': 'protocol',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: bit counting',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/bit.count',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
      {
        'title': 'C++20 draft N4861: fundamental integer types',
        'url': 'https://timsong-cpp.github.io/cppwp/n4861/basic.fundamental',
        'kind': 'standard',
        'accessed': '2026-09-19',
      },
    ],
  'questions':
    [
      {
        'id': 'price-ladder-q01',
        'level': 'L1',
        'prompt': 'Price ladder 与完整订单簿有什么区别？',
        'answer': '本例 ladder 只按价格保存聚合数量并查找非空档位，没有订单 ID、档内时间优先级或撮合。完整 MBO 订单簿还要维护订单索引和队列，并处理事件语义。不能从聚合数量还原具体订单排队位置。',
        'rubric': ['价格聚合', '无订单身份和优先级', '不能还原队列'],
        'source': { 'kind': 'derived', 'rationale': '根据本例聚合量模型与 Order Book 边界推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q02',
        'level': 'L1',
        'prompt': '价格的小数位数与 tick 是同一回事吗？',
        'answer': '不是。scale 定义整数的计量单位，tick 定义允许价格间隔。本例两位小数意味着 12.35 编码为 1235，tick=5 则代表 0.05 的间隔；能精确解析某价格不等于它落在合法网格上。',
        'rubric': ['计量单位', '允许间隔', '解析与网格分开'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 parse_fixed_2 与 PriceGrid 的分层设计推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q03',
        'level': 'L1',
        'prompt': '固定 tick 下怎样把价格映射到数组下标？',
        'answer': '先确定网格起点 p0、正 tick t 和档数 N，检查价格位于覆盖范围且 p-p0 能被 t 整除，再取下标 (p-p0)/t。p0 是配置的网格锚点，不能随便取一个窗口下界就认为所有档位符合交易规则。',
        'rubric': ['范围与整除', '下标公式', '锚点语义'],
        'source': { 'kind': 'derived', 'rationale': '根据 PriceGrid 映射条件推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q04',
        'level': 'L1',
        'prompt': '聚合量设为零后应怎样更新最优档？',
        'answer': '将该价格档数量置零并清掉对应占用位。后续最优档查询从剩余位图重新找最低或最高非空位，全部清空则返回无档位。不能只改数量而让旧 best 指向一个空档。',
        'rubric': ['数量与位图同步', '重新找非空档', '空簿结果'],
        'source': { 'kind': 'derived', 'rationale': '根据 bit0、bit63 删除和空 ladder 测试推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q05',
        'level': 'L1',
        'prompt': '最低价格档与最高价格档分别对应哪一侧最优价？',
        'answer': '正常按价格比较时，卖侧最低非空价是 best ask，买侧最高非空价是 best bid。本例提供 lowest/highest，但不带买卖方向，不检查双边交叉状态，也没有定义竞价或交易所撮合规则。',
        'rubric': ['买高卖低', '只查非空', '示例范围'],
        'source': { 'kind': 'derived', 'rationale': '根据方向无关的档位查询接口推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q06',
        'level': 'L2',
        'prompt': '为什么不能直接用 signed price-minimum 计算偏移？',
        'answer': '两个价格各自可表示，差值却可能超出有符号范围。本例先约束构造跨度不超过 INT64_MAX，再检查价格边界，最后用 uint64 转换后的减法得到非负数学差值；它没有依赖有符号溢出的行为。',
        'rubric': ['端点可表示不代表差可表示', '先限制范围', 'unsigned 差的前提'],
        'source': { 'kind': 'derived', 'rationale': '根据负价网格和整数极值测试推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q07',
        'level': 'L2',
        'prompt': 'std::countr_zero(0) 是否是未定义行为？',
        'answer': 'C++20 标准函数对零返回类型位宽，本例 uint64 为 64。程序仍先判空，因为 64 不是有效档位下标，highest 的 63-countl_zero(0) 也不能用作索引。不要与某些编译器内建函数的前提混淆。',
        'rubric': ['标准零值定义', '下标仍无效', '区分 builtin'],
        'source': { 'kind': 'derived', 'rationale': '根据 N4861 bit.count 与空位图保护推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q08',
        'level': 'L2',
        'prompt': '替换一个档位数量时，怎样安全更新总量？',
        'answer': '先从旧总量减去旧档数量，得到 without_old，再检查新数量是否大于 UINT64_MAX-without_old。通过后才写回档位、总量和位图。本例 set 是绝对值替换，相同输入重做不会把数量再次累加。',
        'rubric': ['移除旧值', '加法前检查', '绝对值更新'],
        'source': { 'kind': 'derived', 'rationale': '根据总量最大值和失败不修改测试推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q09',
        'level': 'L2',
        'prompt': '为什么两位小数解析没有使用 double 再乘 100？',
        'answer': '直接按十进制数字累积整数，可避免二进制浮点近似与后续取整策略混在一起。本例要求恰好两位小数，接受前导零和负零，拒绝加号、指数和多余精度；不替调用方猜测舍入方向。',
        'rubric': ['精确十进制转换', '明确语法', '不隐式舍入'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 exact grammar 与 INT64 极值解析测试推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q10',
        'level': 'L2',
        'prompt': '位图查询只要几步，整个 set 就一定是 O(1) 吗？',
        'answer': '固定 64 档模型有固定上界，但当前 set 还调用 verify 扫描 64 个槽。推广到 N 档时，完整验证成本是 O(N)，不能只报核心数组赋值和位操作成本。更大的位图查找还要计算多字扫描或层级索引的代价。',
        'rubric': ['固定模型范围', '完整 API 含验证', '多字扩展成本'],
        'source': { 'kind': 'derived', 'rationale': '根据 verify 实现和位宽边界推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q11',
        'level': 'L3',
        'prompt': '什么时候适合 dense ladder，什么时候应考虑稀疏结构？',
        'answer': '已知窄价域且较密集时，数组直接映射简单并有良好连续布局；价域大、活跃档稀疏或 tick 规则复杂时，map、分页数组等可减少空槽。比较时要看占用密度、范围变化、更新分布和恢复成本，不能只凭大 O 判断实际速度。',
        'rubric': ['价域与密度', '稀疏方案', '实测维度'],
        'source': { 'kind': 'derived', 'rationale': '根据有界数组模型与参考 map 对照推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q12',
        'level': 'L3',
        'prompt': '价格跑出当前数组窗口后，可以取模覆盖旧槽吗？',
        'answer': '不可以直接覆盖，否则不同绝对价格会别名到同一槽并丢数据。需要明确扩容、迁移、分页或失效恢复策略，搬迁时保留绝对价格与配置版本，避免旧下标引用新档。本例对窗口外价格直接拒绝，不实施滚动窗口。',
        'rubric': ['别名与丢数据', '显式迁移策略', '下标寿命和版本'],
        'source': { 'kind': 'derived', 'rationale': '根据网格固定范围与无取模设计推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q13',
        'level': 'L3',
        'prompt': '变 tick 表能否只替换一个 tick 常量？',
        'answer': '不能。不同价格区间可能采用不同步长，边界包含规则和档位计数会变化；同一标的规则还可能按日期变化。需要按实际协议构造分段合法价域并验证边界，配置切换时重建索引。本例仅支持一个固定 tick。',
        'rubric': ['分段规则', '边界与日期', '重建索引'],
        'source': { 'kind': 'derived', 'rationale': '根据 JPX tick 表和固定网格范围对照推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q14',
        'level': 'L3',
        'prompt': '如何证明位图优化没有改变业务结果？',
        'answer': '使用独立 map 保存绝对价格与数量，按相同更新序列比较全部档位、最低最高档和总量，再覆盖空、单档、最高位、非法价格及算术失败。比较模型应独立计算价格，避免和被测网格共享同一错误映射。',
        'rubric': ['独立参考模型', '完整结果比对', '边界和失败'],
        'source':
          { 'kind': 'derived', 'rationale': '根据 400 次 map 对照和 25600 次档位检查推导。' },
        'companies': [],
      },
      {
        'id': 'price-ladder-q15',
        'level': 'L3',
        'prompt': '把 ladder 接到行情 feed 还缺什么？',
        'answer': '需要验证消息序列、快照有效性、事件是绝对量还是增减量，并按产品绑定价格单位与 tick 版本。出现 gap 或非法范围不能只丢更新后继续声称簿完整。还需定义读写所有权、失效通知和恢复，本例没有这些接入层。',
        'rubric': ['消息语义与序列', '配置身份', '失效和所有权'],
        'source':
          { 'kind': 'derived', 'rationale': '根据聚合存储与完整 feed 状态的责任边界推导。' },
        'companies': [],
      },
    ],
}
---

## 30 秒面试回答

Price ladder 把价格映射为档位，保存每档数据并支持最优档查询。固定 tick、窄价域可以用数组直接定位，再用占用位图跳过空档。实现前要定义整数价格单位、网格锚点和允许范围，检查 off-tick 与算术溢出；位图必须始终与数量一致。它只是一种价格索引结构，不能代替订单身份、行情连续性或完整撮合规则。

## 核心概念

scale 决定价格整数的单位，tick 决定允许间隔。示例解析器将 `12.35` 精确转换为整数 1235，即单位为 0.01；如果网格 tick 为 5，那么档位间隔为 0.05。解析成功只说明金额可表示，还要检查价格是否在当前网格中。

| 对象             | 本例定义                                     |
| ---------------- | -------------------------------------------- |
| PriceGrid        | 固定起点 p0、正 tick、1..64 个档位           |
| PriceLadder      | 每档绝对聚合数量，数量零表示空档             |
| occupied         | 一个 uint64 位图，第 i 位对应第 i 档是否非空 |
| lowest / highest | 最低 / 最高非空价格档；空时返回 nullopt      |
| quantity(price)  | 网格内空档返回 0，非法价格返回 nullopt       |

网格起点也是对齐锚点。本例允许任意满足整数条件的 p0，是否符合某产品的合法价格集合由配置层负责。解析器支持负数，只表示数据结构能安全表示负价格，不意味着所有交易品种都接受负报价。

本例类似 MBP 的聚合量存储，没有订单 ID、档内 FIFO、数量增量去重或撮合。set 接收绝对数量；对相同价格重复 set 同一值不会加倍，但乱序的旧绝对量仍可能覆盖新量，接入层必须先验证事件顺序。

## 原理深入

对合法价格 p，网格下标为 `(p-p0)/tick`。成立条件包括 p 位于首末档之间，且差值能被 tick 整除。直接做整数除法再截断，会把 off-tick 价格误落到邻档；用取模循环映射窗口外价格，则可能让两个绝对价格占同一个槽。

构造时先检查 `(N-1)*tick` 的跨度不超过 INT64_MAX，再检查 `p0+span` 可表示。这个选择使后续偏移可以安全转回有符号整数，但也有意拒绝某些数学上首末端点都可表示、跨度却大于 INT64_MAX 的大范围。它不是覆盖整个 int64 价格域的通用索引。

查询先检查 p 的首末边界，再计算 `uint64(p)-uint64(p0)`。在本例跨度限制下，数学差值非负且不超过 INT64_MAX；无符号模运算得到的结果正是该差值，不依赖有符号减法溢出。反向映射的 `index*tick` 也被构造时跨度约束覆盖。[C++20 整数规则](https://timsong-cpp.github.io/cppwp/n4861/basic.fundamental)

位图不变量是：第 i 位为 1，当且仅当该档数量非零；配置范围之外的位和数量始终为零。因此非空时最低位对应最低价，最高位对应最高价。卖侧通常选 lowest，买侧选 highest，但本例不维护双边市场状态，也不判断交叉报价是否符合某种交易阶段。

## 数据结构/系统内部实现

`parse_fixed_2` 接受可选负号、至少一个整数位、小数点和恰好两位小数。它按十进制数字构建无符号 magnitude，每次乘十加数字前检查范围。负数允许的幅度比正数大一，因此 INT64_MIN 单独返回，避免先转成不可表示的正数再取负。前导零和负零可接受，加号、空白、指数、多余小数和隐式舍入都不在语法内。

`set` 先验证价格，计算 `without_old=total-old_quantity`，再检查新数量能否加入。全部通过后才修改数量、总量和占用位；失败保留旧状态。使用 `uint64_t{1}<<index`，且 index 严格小于 64，所以第 63 位合法，不会出现移位 64 位。

`lowest` 在非空位图上调用 countr_zero，`highest` 使用 `63-countl_zero`。C++20 标准函数对零返回位宽 64，程序先判空是为了避免把结果当成档位下标，不是因为这些标准函数对零有 UB。标准没有承诺它们在所有 CPU 上对应同一条指令。[N4861 bit.count](https://timsong-cpp.github.io/cppwp/n4861/bit.count)

verify 扫描全部 64 个槽，检查数量与位图一致、范围外为空以及总量守恒。所有读写均为单线程普通访问，当前代码没有并发快照或跨线程发布协议。若用在单 owner 行情处理器中，需要另行设计读者获取一致视图的方式。

## C++ runnable demo

```cpp include=examples/price-ladder.cpp

```

使用 C++20，测试无需行情连接。断言中包含更新操作，验证时不能定义 `NDEBUG`。

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  examples/price-ladder.cpp -o price-ladder
./price-ladder

g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  examples/price-ladder.cpp -o price-ladder-san
./price-ladder-san
```

参考模型用 `std::map<Price,Quantity>` 保存绝对价格，只保留非零项。测试自行按 `10000+index*5` 构造输入，不调用 PriceGrid 的反向映射来生成 oracle 键。400 次确定性更新后，每一步都比对最低最高档、总量和全部 64 档，共 25600 次档位检查。

| 测试         | 实际覆盖                                                                     |
| ------------ | ---------------------------------------------------------------------------- |
| 精确解析     | 两位小数、负值、前导零、负零，INT64_MIN/MAX 对应文本；超出边界和错误语法拒绝 |
| 网格双向映射 | -100..100、tick=5 的全部 41 档 round-trip，范围外和 off-tick 拒绝            |
| 极值价域     | 贴近 INT64_MIN/MAX 的 64 档，以及最大价格上的单档                            |
| 构造失败     | 非正 tick、档数 0/65、跨度过大、末档溢出和反向索引越界                       |
| 位图边界     | bit0 与 bit63、只有最高档、删除最高档、全部清空、单档网格                    |
| 总量         | UINT64_MAX 单档可表示；新增一单位失败且不改状态，替换旧值后释放空间再更新    |
| 独立模型     | 400 次 map 对照，非法价格更新后位图与总量保持原值                            |

正常严格 warning 构建与 ASan/UBSan 运行通过，程序输出 `map_oracle_updates=400 level_checks=25600 passed`。这些是功能测试，没有测量实际更新延迟，也没有证明位图比树结构在所有工作负载上更快。

## 高频追问

### 如果某产品按价格区间使用不同 tick 呢？

此时单个 `(p-p0)/tick` 不足以描述整个合法价域。需要按协议定义分段边界、每段起点、步长和累计档数，特别核对临界价格属于哪一段。JPX 官方 tick 规则按标的类别和价格区间列出不同步长，并包含规则变更信息；配置必须带适用范围与版本。本例不实现该表，也不把两位小数解析器当成通用交易所价格格式。[JPX Tick Size](https://www.jpx.co.jp/english/equities/trading/domestic/07.html)

### 价格超过数组窗口时能否直接移动基准？

移动 p0 会改变每个下标的绝对价格含义。需要迁移仍有效的数据或使用明确分页结构，并处理旧下标、迭代器或句柄。不能只更新基准却保留原数量数组。本例选择拒绝越界，调用方据协议决定扩展、重建还是标记数据失效。

### 绝对量更新是不是天然不怕重传？

同一更新重复应用的数量效果相同，但旧值晚到仍会覆盖新值，其他附带统计也未必幂等。恢复器需要先确认序列与状态有效，再把可应用的绝对量送入 ladder；这里的存储接口没有足够信息自行判断消息新旧。

## 容易答错的点

- 价格能被解析为整数，不等于符合 tick 或位于当前存储窗口。
- 两个有符号价格都可表示，不保证它们的差值可表示。
- std::countr_zero 对零有定义，但返回的位宽不能当作本例有效下标。
- 数量置零时必须同步清位；从空位图取 best 必须得到无结果。
- 总量更新要先移除旧值再检查新值，不能把绝对设置误作累加。
- 位图保存非空档身份，不能保存 MBO 订单排队优先级或行情完整性。

## 性能分析

固定 64 档下，核心价格映射、单档修改和一个机器字内的最优档查询都有固定工作上界，存储也是固定数组。当前完整 set 包括 verify 的 64 槽扫描；推广到 N 档时，这部分是 `O(N)`，不能只报核心位操作的代价。解析长度为 D 的文本需要 `O(D)` 时间，生产入口还应设置文本长度预算。

更大价域需要多个位图字。逐字扫描与层级摘要位图有不同成本，维护摘要本身也会增加更新工作。dense 数组的空间随覆盖档数增长，即使大多数档为空仍要付出存储成本；稀疏树结构主要随活跃档数增长，但节点布局和查找路径不同。

比较实现时固定价格范围、活跃密度、更新与查询比例、极端价格迁移和输入成本。记录内存、分配、缓存 miss、更新与 best 查询的分布，并把恢复和范围变化成本计入完整链路。当前 map 仅作为结果 oracle，不是性能基线实验。

## Quant/Low-Latency 场景

行情处理器可以在已确认的固定价域内维护聚合量，再向策略提供最优档。收到 off-tick 或范围外更新时，应记录原始价格、产品配置和事件序号，判断是规则变化、解析错误还是窗口不足；直接丢弃后继续发布 valid 簿会掩盖状态缺失。

配置变更会影响价格合法性与档位映射。切换 tick 或单位时应绑定配置版本、明确切点，并重建或验证已有档位，不应让新旧解释混在同一个无版本数组里。历史回放也需要使用事件对应的规则版本。

若多个策略读取同一 ladder，不能仅把 occupied 改成原子就认为数量数组也安全。读者可能看到位图和数量的不同版本，需要单 owner 输出快照、消息传递或其他完整同步方案。存储结构省下的查询成本，不代替发布与生命周期设计。

## 相关专题

- [Order Book](../trading/order-book.md)：从聚合价格档扩展到订单身份和档内顺序。
- [快照与增量恢复](../trading/snapshot-recovery.md)：确定哪些更新可以进入有效 ladder。
- [CPU cache / false sharing](../performance/cpu-cache-false-sharing.md)：分析连续布局、共享与缓存成本。
- [基准测试方法](../performance/benchmark-methodology.md)：设计可比较的更新和最优档查询实验。

## 分层面试题

L1 解释价格单位、tick 和档位，L2 检查整数与位图边界，L3 选择价域结构并处理配置和恢复。写出几个临界价格，再给出它们的整数值、合法性和下标，能直接检查映射是否严密。
