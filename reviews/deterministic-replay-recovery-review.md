# 确定性回放与容灾专题自审交接

- 日期：2026-09-19。
- 类型：作者自审，主 agent 独立验收。
- 范围：`content/topics/design/deterministic-replay-recovery.md`、`examples/deterministic-replay-recovery.cpp`。
- 结论：有界内存日志和输出接收模型通过验证；没有文件 I/O、fsync、持久化、真实选主或完整灾备实现。

## 内容与来源

已应用 cpp-quant-writing、写作约定与内容质量标准。正文 11 节，15 题，L1/L2/L3 各 5，全部 derived、companies 为空。区分确定性状态转移、日志提交与持久性、外部交付、旧主隔离，没有将内存数组称作 WAL 持久化实现。

2026-09-19 实际打开并核对四个一手来源：

| 来源                                                                 | 核查内容                                                                               |
| -------------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| [Raft extended paper](https://raft.github.io/raft.pdf)               | 第 2 节复制状态机及日志一致性职责；第 8 节提交后丢回复引起的客户端重试、序列身份和去重 |
| [Linux fsync(2)](https://man7.org/linux/man-pages/man2/fsync.2.html) | 同步完成、错误、文件与目录项同步的区别                                                 |
| [SQLite WAL](https://www.sqlite.org/wal.html)                        | WAL 与 checkpoint、同步配置影响断电持久性，不能仅凭名称判断确认保证                    |
| [etcd v3.6 recovery](https://etcd.io/docs/v3.6/op-guide/recovery/)   | 快照恢复后的新逻辑集群身份、完整性检查、revision 与下游缓存问题                        |

正文没有给出实际集群操作命令，也没有实现这些系统的协议。FNV、40 字节 frame、四条输入及 Sink 都是教学设计。

## 模型核查

- frame 为 10 个大端 32 位字段，共 40 字节；逐字段编解码，没有对象表示、未对齐读取或指针类型转换。
- 入口限制日志最多 8 frame，先验证长度再按固定边界读取，get_word/put_word 保留边界断言。测试必须启用断言。
- format、schema、config 分别检查，固定规则使用记录的 logical_time、draw、amount。回放不调用真实时钟或随机源，不读取外部配置。
- 时间非递减、draw 范围 0..9、amount 为正。amount 先扩为 64 位再乘二；余额增加前检查剩余空间。
- 只有 input.sequence 大于候选序号才执行 candidate.sequence+1，因此不会在最大值时递增回绕。最大减一到最大可以成功，随后低序号输入不作为新轮处理。
- FNV32 校验和使用无符号回绕；前一校验值关联编码顺序，但两者均不是认证或无碰撞证明。重算后仍能过校验，因此另测语义检查。
- snapshot 与 target 是外部可信输入。可信性包括余额、水位、hash、pending 一致并对应完整已提交前缀；本例只检查少量结构字段，不编码或认证快照文件，也不发现 commit index。
- 候选复制包含 pending，保留水位之前未确认输出。全部验证后才由单 owner 普通赋值替换 published，没有跨线程原子发布承诺。
- 本轮已见同号完整 frame 相同才跳过；同号冲突拒绝。快照以前的原始载荷不在 seen 中，覆盖输入返回 duplicate_unknown，不假装已证明是重复。
- 缺口、版本错误、损坏、容量或 target 不匹配均整次失败。没有自动修补日志、扫描下一 magic 或静默丢尾逻辑。
- Sink 的稳定业务键为 stream+sequence，发送授权 epoch 单独校验。初始 epoch=0 无权限；activate 只接受更大值，但其可信授权、选主和一致性是外部前提。
- Sink 的去重和效果更新在同一个串行内存方法里，没有两者共同持久化的实现。Sink 崩溃会丢失内存记录，不能宣称端到端 exactly-once。
- pending 没有确认删除接口，容量 8 后停止；历史记录和输入均有上限，不是完整生产 outbox 生命周期。

## 独立期望和边界

| 测试               | 实际结果                                                                                                                      |
| ------------------ | ----------------------------------------------------------------------------------------------------------------------------- |
| 手算输出           | 四条增量常量为 7、12、2、10，余额为 31；水位 2 快照余额为 19                                                                  |
| 快照后缀           | 从头回放与水位 2 快照加后两条回放 State 完全相等，包括 pending 四条                                                           |
| 重复               | 在完整日志后追加完全相同第二帧，结果不变；同号改内容并重算校验，整次失败                                                      |
| 160 个短前缀       | 长度 0..159 全部失败，发布 sentinel 不变；长度 0/40/80/120 是整帧边界，因无法达到 target 而失败，其余是 truncated             |
| 160 次损坏         | 每个字节单独翻最低位，全部失败且不发布；不是 160×8 全位枚举，也不是碰撞测试                                                   |
| 合法校验下语义故障 | 格式版本、stream、倒退时间、越界 draw、config、previous_checksum 修改后重新 seal，仍失败                                      |
| 顺序与 target      | 1 后跳 3、从 3 起读、覆盖快照旧帧、target=3/5 配到 4 的日志，均失败                                                           |
| 算术和容量         | 最大余额溢出、已有 8 个 pending 后追加第 9 个、超长日志、未知 schema 失败；空日志到 target=0 成功                             |
| 序号末端           | 最大减一的可信基线重放最后序号成功，余额 3；再输入低序号拒绝                                                                  |
| 未知交付和隔离     | 第一条已应用后模拟 Ack 丢失，新 epoch 重发得到 duplicate；旧 epoch 第二条 fenced；新 epoch 完成剩余三条，最终 4 次效果总量 31 |
| 输出冲突           | 相同业务键不同金额返回 conflict；未授权 epoch 和零序号意图拒绝                                                                |

手算常量没有由转移函数产生。截断测试固定目标为 4，不把无法达到 target 的整帧前缀误报成解码截断。

## 执行记录

WSL GCC 13.3.0，C++20，断言启用：

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
  examples/deterministic-replay-recovery.cpp \
  -o .artifacts/deterministic-replay-recovery-review/demo
.artifacts/deterministic-replay-recovery-review/demo

g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie \
  examples/deterministic-replay-recovery.cpp \
  -o .artifacts/deterministic-replay-recovery-review/san
.artifacts/deterministic-replay-recovery-review/san
```

作者两次均退出 0，sanitizer 无诊断。主 agent 另行通读代码并复跑严格 warning 与 ASan/UBSan，反馈通过。末行输出声明 `memory model only: no durability, consensus or authenticated log`。

内容检查本章零错误，11 节、15 题、各层 5。Prettier 已格式化，最终 markdownlint 和 hash 在交接消息报告。未改共享文件、旧章节或其他 agent 文件，等待主 agent 终验登记。
