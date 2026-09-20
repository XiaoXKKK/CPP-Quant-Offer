# Price Ladder 专题自审交接

- 日期：2026-09-19。
- 类型：作者自审，主 agent 独立验收。
- 范围：`content/topics/design/price-ladder.md`、`examples/price-ladder.cpp`。
- 结论：固定 tick、有界聚合量 ladder 通过独立 map 对照与整数边界验证；不是完整 MBO、变 tick 表或行情接入。

## 内容与来源

已应用 cpp-quant-writing、写作约定及内容质量标准。正文 11 节、15 题，各层 5 题，全部 derived、companies 为空。解释整数单位、网格锚点、档位索引与市场合法性之间的区别，没有以某个产品规则代表所有交易所。

2026-09-19 实际打开并核对：

| 资料                                                                                   | 核查内容                                                                                           |
| -------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------- |
| [JPX Tick Size](https://www.jpx.co.jp/english/equities/trading/domestic/07.html)       | 按产品类别和价格区间变化的 tick 表，以及页面列出的后续规则变更；正文仅用作分段与版本条件的实际对照 |
| [N4861 bit.count](https://timsong-cpp.github.io/cppwp/n4861/bit.count)                 | countl_zero/countr_zero 的无符号类型条件，对零返回位宽                                             |
| [N4861 basic.fundamental](https://timsong-cpp.github.io/cppwp/n4861/basic.fundamental) | 有符号溢出与无符号模算术，转换后价格差的推理前提                                                   |

未引用实际 tick 数值作为示例配置，避免把教学 0.01 单位和 0.05 tick 冒充某产品规则。打开过 CME 产品页面，但工具返回内容没有可核查的目标 tick 字段，未将其列为论据。

## 算术和数据结构审查

- parse_fixed_2 完全按十进制数字累积 magnitude，不经 double。语法接受可选负号、至少一个整数位、点和恰好两位小数；前导零与负零允许，正号、空白、指数、额外精度和舍入不支持。
- 每步乘十加数字之前与正负各自的 magnitude 上限比较；INT64_MIN 直接返回，不先生成不可表示的正有符号幅度，也不取最小负数绝对值。
- grid tick 必须正，档数 1..64。先检查跨度乘法，跨度强制不超过 INT64_MAX，再检查 minimum+span 上界。允许极小 minimum，但不会产生向下溢出。
- 正文明确这一跨度限制会排除部分数学上首末端点可表示的大范围，不声称覆盖全 int64 价域。
- price 输入先按首末边界检查，再用 uint64(price)-uint64(minimum) 得到已证明范围内的非负差；整除失败拒绝，不截断到邻档。
- 反向映射检查 index<levels，乘法受构造的 span 上界覆盖，转为 signed 后相加可表示。
- set 是绝对量替换。先从 total 移除旧值，再检查新值加法；失败不会修改数量、总量或位图。
- 位移使用 uint64 1，index 最大 63；verify 同样只遍历 0..63，没有 1<<64 或有符号高位移位。
- lowest/highest 先判 occupied 是否零；标准 bit 函数对零有定义，但此时不能把返回位宽作为索引。
- verify 检查数量非零与占用位等价、配置外槽位为空、总量守恒。完整 set 含 64 槽扫描，正文说明推广 N 档时验证 O(N)，不只报告核心更新成本。
- 当前所有读写是单线程普通访问，没有把单独位图原子化当作完整并发方案。价格单位、产品合法性、快照与序列有效性属于外部配置和接入层责任。

## 独立模型与边界

| 场景            | 实际证据                                                                              |
| --------------- | ------------------------------------------------------------------------------------- |
| 独立 map        | 400 次确定性更新，参考键按 10000+index×5 独立计算，未调用被测网格生成 oracle 键       |
| 完整比对        | 每一步检查最低最高档、总量与全部 64 档，共 25600 次档位检查                           |
| 非法更新        | 每一步附加 10001 的 off-tick 更新，确认位图和 total 不变                              |
| 小数解析        | 正负、前导零、负零、精确 int64 两端文本，边界外和非法语法拒绝                         |
| 网格 round-trip | -100..100、tick=5 的 41 档双向映射，外部和 off-tick 价格失败                          |
| 极端位置        | 贴近 INT64_MIN/MAX 的 64 档；最大值上的单档且 tick 本身也取最大正值                   |
| 构造失败        | tick=0/-1、levels=0/65、span 超限、最后价格溢出、反向 index 越界                      |
| 位图            | bit63 单独存在，加入 bit0，删除最高档后最优价移动，再删除全部变空                     |
| 数量            | 单档 UINT64_MAX 成功，加另一档 1 失败不修改；重复绝对设置不加倍，先降低旧档再容纳新量 |
| 返回语义        | 合法空档 quantity=0，网格外 quantity=nullopt，单档最低和最高相同                      |

map 仅作正确性 oracle，没有用其耗时作性能比较。测试数量有界，不依赖调度、随机生成器或真实行情。

## 执行记录

作者在 WSL GCC 13.3.0、C++20、断言启用条件下执行：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  examples/price-ladder.cpp -o .artifacts/price-ladder-review/demo
.artifacts/price-ladder-review/demo

g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  examples/price-ladder.cpp -o .artifacts/price-ladder-review/san
.artifacts/price-ladder-review/san
```

均退出 0，sanitizer 无诊断。主 agent 全文代码预审并独立复跑严格 warning、ASan/UBSan，反馈通过。末行输出 `fixed tick, bounded price domain, absolute level quantities only`。

内容校验本章零错误，11 节、15 题、L1/L2/L3 各 5。正文与交接使用 Prettier，最终 markdownlint 和 hash 在交接消息报告。未修改其他章节、共享 manifest、roadmap 或测试，等待主 agent 最终登记。
