# Benchmark methodology 专题自审

日期：2026-09-19。类型：作者自审。主 agent 已逐行预审 canonical demo，并独立严格编译与运行 ASan/UBSan；正文等待最终验收。本文件保存一次优化构建的原始观察及其适用范围，不替代主 agent 的发布 hash。

## 交付

- `content/topics/performance/benchmark-methodology.md`：正文 11 节，15 题，L1/L2/L3 各 5 题。
- `examples/benchmark-methodology.cpp`：C++20 单文件，有界正确性验证及 GCC/Clang 计时路径。
- 本文件：来源核查、逐条技术自审、原始样本和验证记录。

已应用 cpp-quant-writing，遵循 WRITING_STYLE、CONTENT_STANDARD 和 taxonomy。仅修改本章三个文件，没有改已冻结章节、共享 manifest、roadmap、测试、skill 或提交 commit。没有引入外部测量库；Google Benchmark 和 LLVM 资料用于解释方法，不要求读者安装。

## 来源

九条 references 均已实际打开：

| 结论                                               | 核查资料                                                                                                                                                                     |
| -------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| steady_clock 保证与 high_resolution_clock 别名边界 | [C++20 N4861 steady_clock](https://timsong-cpp.github.io/cppwp/n4861/time.clock.steady)、[high_resolution_clock](https://timsong-cpp.github.io/cppwp/n4861/time.clock.hires) |
| 预热、重复、交错及 DoNotOptimize 的有限作用        | [Google Benchmark user guide](https://google.github.io/benchmark/user_guide.html)                                                                                            |
| memory clobber 约束编译器，但不阻止 CPU 推测读取   | [GCC extended asm](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html)                                                                                                     |
| 低噪声不足以排除偏差，保存构建与配置               | [LLVM benchmarking tips](https://llvm.org/docs/Benchmarking.html)                                                                                                            |
| 频率、调度、SMT、缓存和 NUMA 的方差来源            | [Google Benchmark reducing variance](https://google.github.io/benchmark/reducing_variance.html)                                                                              |
| 样本分位数有不同约定，本文使用经验分布逆函数       | [R stats quantile](https://stat.ethz.ch/R-manual/R-devel/library/stats/html/quantile.html)                                                                                   |
| 响应时间依赖的漏采与 expected-interval 校正        | [HdrHistogram 原作者 README](https://raw.githubusercontent.com/HdrHistogram/HdrHistogram/master/README.md)                                                                   |
| perf stat 事件、重复测量及计数器多路复用           | [perf-stat(1)](https://man7.org/linux/man-pages/man1/perf-stat.1.html)                                                                                                       |

除 C++20 固定草案外，工具文档为访问当日页面。没有采用 LLVM 文档中的具体噪声比例承诺，也没有照抄全局关闭安全或系统服务的操作。本文只记录本次实际配置，未修改 CPU governor、ASLR、SMT、调度优先级或系统级设置。

## 技术自审

1. 计时语义与单位分开。steady_clock 的非递减语义不代表纳秒精度；duration 转为 double 纳秒不补充真实分辨率，紧邻读数可以相同。
2. 明确 start 到 stop 的范围：每轮输入 barrier、函数指针调用、求和、checksum 累加、循环控制和结束结果 barrier。分配、生成、检查、打印与分位数排序均不在此区间。没有将这些值命名为纯加法指令成本。
3. A 为单累加器源码，B 为四路累加器源码及尾部。uint32_t 输入累加到 uint64_t；本次最大总和和 32 次 checksum 均有界，不依赖 signed overflow。
4. 长度 0 到 5 的固定数组使用独立列出的期望值，包含 UINT32_MAX、空输入及不足四项的尾部。完整输入用 17 与 1024 互质的置换性质得到闭式期望 33521664，不只比较 A 是否等于 B。
5. 非 GCC/Clang 构建仍可执行正确性验证，随后输出 SKIP timing。计时使用编译器扩展，没有伪装成纯 ISO C++ 的通用防优化保证；该备用分支未在另一编译器上实际运行。
6. 指针输入加 memory clobber 用于限制跨轮内存值复用，结束处结果操作数使 checksum 在结束时间戳前可用。没有使用 volatile 数据访问宣称全部优化已禁用，也没有声称该屏障会刷新 cache、序列化 CPU 或同步线程。
7. noinline 只限制相应内联行为；合法向量化、展开及其他变换仍可发生。作者已查看 GCC 13.3 `-O2` 汇编，measure 的两次 now 之间保留间接调用及累加循环。
8. 主 agent 另核查同配置汇编：sum_a 为标量 load/add 循环，sum_b 为 movdqu/punpck/paddq 等向量循环加尾部。这是本次构建观察，不能把差异单独归因于源码中四条标量累加链，也不能推广其他编译器和选项。
9. 4 对预热和 20 对记录均 AB/BA 交替，正式样本为 10 对 AB、10 对 BA。固定顺序便于复现，但周期噪声、共享 cache 与时间相关性仍可存在；正文没有把样本当作已证独立分布。
10. 20 对 Sample 在计时前预分配，全部原始数据保存后统一输出。每批 checksum 必须等于 1072693248，失败明确异常退出。不对 A 比 B 快或慢设断言。
11. nearest_rank 的样本数和分母限制为 1000，分子正且不大于分母，整数 rank 的乘法加法有界。p50 与 p99 的小数组例子有断言；20 个样本时 p99 和 p99.9 都选最大值。没有把这种经验分位数当可靠总体尾估计。
12. 统计对象为每批耗时除以 32 得到的批次均值。没有恢复单次调用分布，也没有把 20 批各 32 次虚报为 640 个延迟样本。扩大发送或样本数时需重新定义测量对象。
13. 紧邻时钟读数只保留原始值，没有机械扣除。批次摊薄计时相对成本的同时会改变重复访问状态，正文说明了这个取舍。
14. coordinated omission 限定独立到达模型与响应后再发的生成器不匹配的场景。真实闭环业务可以用闭环测试；本例无外部到达流，不生成虚构请求尾延迟。expected-interval 校正需假设，不能恢复任意真实请求历史。
15. perf 只作为后续归因方法，没有伪造运行结果。正文注明事件支持、权限、多路复用和整进程测量范围。没有因为一次耗时不同就给 cache、带宽或依赖链作唯一归因。
16. 15 题各层 5 题，均为 derived，companies 为空。L3 涉及生产替换、负载模型、删点、计数器归因与环境代表性；答案没有从小样本性能直接推出生产建议。

## 实际环境

作者在 WSL2 中读取到：

```text
Linux 6.18.33.2-microsoft-standard-WSL2 x86_64
g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz
16 logical CPUs; 8 cores; 2 threads per core; 1 socket
Microsoft hypervisor; full virtualization
NUMA node0 CPUs: 0-15
Cpus_allowed_list: 0-15
Mems_allowed_list: 0
L1d: 256 KiB total over 8 instances
L2: 2 MiB total over 8 instances
L3: 16 MiB over 1 instance
```

上述拓扑为 guest 可见结果，不声明已观测宿主完整干扰。未绑核、未设内存策略、未刷新 cache，未控制宿主负载、温度或睿频。查询 `/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor` 返回文件不存在，因此 governor 状态未知，不能写成已固定频率。该环境查询使组合 shell 命令最后返回 1；此前编译与 ASan/UBSan 程序已成功运行，并经 `&&` 进入后续 uname/lscpu 查询，失败原因不是 sanitizer。

输入 65536 项 uint32_t，共 262144 字节，同一数组反复读取。未证明数组在任何特定 cache 层驻留。生成函数、4 对预热、每批 32 次和 20 对 AB/BA 样本都固定在源码中。示例运行时间不构成实验机器的稳定性保证。

## 优化构建原始记录

作者编译参数：`-std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`，没有 `-march=native` 或 LTO。以下为本次完整输出，未筛除慢样本，未扣除计时器开销：

```text
correctness passed; expected_sum=33521664
elements=65536 bytes=262144 passes_per_batch=32 warmup_pairs=4 measured_pairs=20
compiler=13.3.0
timer_pair_ns,100.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,100.000,0.000,0.000,100.000,0.000,0.000,100.000,0.000,0.000,100.000,0.000
round,order,a_batch_ns,b_batch_ns,a_checksum,b_checksum
0,AB,719876.000,361988.000,1072693248,1072693248
1,BA,703177.000,347589.000,1072693248,1072693248
2,AB,679477.000,351389.000,1072693248,1072693248
3,BA,665779.000,321489.000,1072693248,1072693248
4,AB,692277.000,322589.000,1072693248,1072693248
5,BA,661578.000,326690.000,1072693248,1072693248
6,AB,678578.000,342589.000,1072693248,1072693248
7,BA,680977.000,329789.000,1072693248,1072693248
8,AB,687378.000,325489.000,1072693248,1072693248
9,BA,659878.000,326190.000,1072693248,1072693248
10,AB,672378.000,330889.000,1072693248,1072693248
11,BA,666778.000,318890.000,1072693248,1072693248
12,AB,669178.000,311090.000,1072693248,1072693248
13,BA,665479.000,320889.000,1072693248,1072693248
14,AB,659678.000,324889.000,1072693248,1072693248
15,BA,666278.000,311990.000,1072693248,1072693248
16,AB,690878.000,368988.000,1072693248,1072693248
17,BA,727476.000,377187.000,1072693248,1072693248
18,AB,688078.000,361188.000,1072693248,1072693248
19,BA,680077.000,318990.000,1072693248,1072693248
A batch_mean_ns_per_sum nearest_rank n=20 p50=21205.562 p99=22733.625 p99.9=22733.625
B batch_mean_ns_per_sum nearest_rank n=20 p50=10193.438 p99=11787.094 p99.9=11787.094
n=20: p99 and p99.9 both select max; no reliable tail estimate
batch means are not per-request latencies; no winner asserted
```

正文数值与这组优化构建输出一致，只描述一次运行。紧邻时钟读数中的 0 或 100 不足以证明所有时刻的有效分辨率固定为 100 ns，更不表示一次读取无成本。没有重复运行后只保留最有利的一组，没有计算或宣称普适加速倍数。

## 功能验证与格式

作者 ASan/UBSan 参数：`-std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie`。程序完成短数组、闭式期望、每批 checksum 与分位数检查，无 sanitizer 报告。主 agent 独立严格编译并复跑 ASan/UBSan 同样通过。sanitizer 输出的时间不作为性能证据，不与优化构建混入一张表。

未实际覆盖其他编译器的 timing SKIP 分支，未使用硬件计数器，未在独占裸机运行，未验证生产负载代表性。没有为了当前有限实验添加脆弱耗时阈值测试。

最终检查包括 Prettier、Markdownlint、内容 schema、11 节顺序、各层题数、canonical include、内部链接与 GFM 表格列数。结果和三文件 hash 随交付消息发送，发布验收由主 agent 完成。
