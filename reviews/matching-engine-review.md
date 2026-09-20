# Matching Engine 自审交接

日期：2026-09-19。类型为作者自审，主代理负责独立技术与页面验收。仅新增 `content/topics/trading/matching-engine.md`、`examples/matching-engine.cpp` 和本文件，没有修改共享 manifest、roadmap、harness 或旧专题。

## 内容与来源

本章按 cpp-quant-writing skill 和已读取的 WRITING_STYLE、CONTENT_STANDARD 写作。roadmap F 明确列出 Matching Engine，taxonomy 选择 trading 与 Matching Engine/Order Book/Trading System。正文 11 节、15 道 derived 题，L1/L2/L3 各 5 道，所有题目有答案和评分点，companies 为空。

5 条 references 实际打开并核对：Coinbase Exchange matching engine 页面支持该场所 price-time、静止单价格与 STP 的具体规则；同场所订单文档的 Time In Force 段支持 GTC/IOC/FOK 基本区别；CME 算法概览与 Globex 算法说明用于证明分配算法存在产品/规则差异；C++20 N4861 array 支持固定元素结构说明。

最初尝试的 CME matching-algorithm 路径和 Nasdaq TradingSystem 路径无法访问，随后搜索并打开有效 CME 官方页面与其 Client Systems Wiki。没有把无法打开的资料列作已读来源。资料只做公开阅读，没有调用任何下单 API 或进行真实交易。

正文将规则显式标为教学模型：单品种连续限价，价格优先、同价 FIFO，按 maker 价格，GTC/IOC，无市价、FOK、改单、账户、STP、风险或跨会话恢复。没有因为部分规则与 Coinbase 一致，就声称实现了该场所。

## 核心状态与边界

Engine 有 8 个固定活单槽位、65 项 seen 与成功接收计数。输入 ID 1..64，价格 1..1000，数量 1..8，Side/Tif 非法枚举均拒绝。先校验字段，再检测重复身份，避免越界访问 seen。拒绝不消费身份，成功接收包括没有成交的 IOC；成功身份终身保留至该实例结束。

accepted_count 不超过 64，priority 使用该计数加一，且新单必须拥有未使用合法 ID 才能增加计数，因此不会在身份耗尽后继续增长或回绕。活单容量与 seen 生命周期分开：即便 64 张 IOC 全部未成交，活单为零但身份域已经耗尽。正文明确没有回用协议。

选择候选时先对侧与交叉价格，再按买来单选最低卖价/卖来单选最高买价，最后同价按 priority。部分成交保留 maker priority；清空槽位时 Resting 全部归零，复用槽位不会取代逻辑时间排序。

每次执行量是两个非零残量的 min，同时扣减来单和静止单并记录相同数量。一次迭代至少耗尽一张 maker 或来单，最多 8 笔成交，固定 Result 足够。单笔与本次累计成交不超过来单上限 8，没有价格乘数量的金额运算，也没有将整数价格解释为实际货币精度。

GTC 在候选状态撮合后才检查残量槽位，IOC 无需残量槽。8 张活单的入口状态不等于必须拒绝：来单可以消费对手单且完全成交，或释放槽位后挂剩余。容量拒绝返回空成交结果，正式 State 不变。

## 异常与结果发布

submit 在局部 State 副本上完成全部撮合，并构造固定 Result。最终 State 赋值和 Result 复制/移动均通过 nothrow 类型特征静态断言；核心没有 vector、分配或回调。自审检查了 result.trade_count 的上界推理，不依赖希望分配永不失败。

故障注入只在提交前抛出专用空异常类型。分别测试候选状态中 maker 部分扣减，以及 maker 耗尽并准备来单挂残量两种情况，完整 State（含 seen 与计数）都与提交前相等。之后使用相同 ID 正常重试并校验成交，证明未提前占用身份。

cancel 是有界扫描和简单值重置，声明 noexcept，移除全部剩余数量而保留 seen。未知、已成、已撤及零 ID 返回零。它没有分配或外部通知，不需要复制整个 State。

内存提交保证不等于并发原子性。Engine 只有单 owner，其他线程不得无同步读写 State。返回后结果消费者失败、进程崩溃、日志落盘与网络送达均未实现；正文和 L3 明确 seen 不足以支持恢复原结果的幂等回复。本章不在拒绝前外发任何成交消息。

## 独立 oracle 与测试覆盖

主实现按整张订单逐轮扫描最佳 maker。UnitOracle 把每张订单展开为单位 lot，收集全部符合条件的单位并排序，逐单位从另一份状态删除，再合并连续同 maker 的成交。它不调用主实现的候选选择逻辑；有独立字段校验表达式、容量计算与取消表示。两者共享公开命令/结果值类型和教学规则，不能据此认为杜绝了共同规格错误。

每条命令后核对完整 Result、按 ID 归一后的活单、全部 seen、accepted_count，以及最高买价小于最低卖价。accepted 命令验证 executed+resting+canceled 等于输入量；拒绝验证完整 State 不变、trade_count 为零。

显式测试覆盖：

- 买卖两方向的价格改善、同价 FIFO、部分成交和跨价位 IOC 残量。
- GTC 残量挂单，未知与重复撤单，已成/已撤 ID 重用拒绝。
- ID 0/65、价格 0/1001、数量 0/9、非法 Side/Tif。
- 合法价格和数量边界、ID 64、空簿 IOC。
- 满 8 活单拒绝、取消后以原被拒 ID 重试、8 个 maker 的结果容量上界。
- 删除数组前槽再插新单，确认未越过同价旧单。
- 64 个成功 IOC 身份全部耗尽，旧 ID duplicate、域外 ID invalid。
- 两类提交前故障后 State 完全不变，并可重试。

此外使用固定种子的 uint32_t LCG，128 条轨迹各 128 个命令，共 16384 个命令逐条交叉验证。轨迹有限，包含重号与取消，不声称均匀分布、穷举全部交错或生产输入代表性。oracle vector 的分配失败没有注入；其分配属于测试基础设施，不属于 Engine 的无分配声明。

## 实际执行证据

本机 WSL2 x86-64，GCC `13.3.0`。在独有 `/tmp/cpp-quant-matching-review` 目录编译，未运行共享全量 harness。实际 Linux 命令如下，由 PowerShell 调用 WSL bash 执行。

```bash
mkdir -p /tmp/cpp-quant-matching-review
src=/mnt/c/Users/ADMIN/Documents/ChatGPT/CPP-Quant-offer/examples/matching-engine.cpp
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  "$src" -o /tmp/cpp-quant-matching-review/demo
/tmp/cpp-quant-matching-review/demo
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  "$src" -o /tmp/cpp-quant-matching-review/demo-sanitize
/tmp/cpp-quant-matching-review/demo-sanitize
```

普通与 ASan/UBSan 均成功，无 sanitizer 报告；输出如下。初版测试通过后补充 8 笔成交、槽位复用、双故障形态及身份耗尽，最新版本再次按上述命令编译执行通过。

```text
Price/time, maker price, IOC/GTC, cancel, rollback: OK
128 traces x 128 commands agree with unit-lot oracle.
```

主代理反馈另行独立严格编译与 ASan/UBSan 通过、完成核心代码审读。该反馈不计为作者本人测试。未测耗时、吞吐、尾延迟、线程争用或持久化恢复。

## 正文与题目自审

参数化复杂度写为每次最优候选扫描 O(N)、最多 N 张 maker，因此最坏 O(N²)；固定 N=8 不被包装成通用 O(1)。完整性能讨论包含结果生成、队列与持久化，并说明本章没有数字结论。

15 题逐一检查规则来源及例子的数量/价格。L3 覆盖提交与交付区别、场所规则扩展、多索引一致性、权威序列与恢复测试，不只重复定义。按 skill 通读后保留技术条件，去掉无证据的速度或生产能力评价。内部相关链接指向现有 order-book、RAII、UDP 与 SPSC 专题。

格式、schema 和 lint 于冻结前检查；最终 hash 通过交付消息提供。全量构建、页面与共享审查记录由主代理维护。
