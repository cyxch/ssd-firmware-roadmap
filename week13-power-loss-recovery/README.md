# Week 13 · 掉电保护与数据恢复

SSD 固件方向学习计划 · 第 13 周（掉电保护与数据恢复）

## 本周目标

- 理解 SSD 掉电的数据风险点（DRAM 缓存 / 映射表 / 半写页）
- 理解掉电保护（PLP）机制与"已确认"语义
- 掌握日志（WAL）机制与崩溃重放恢复
- 掌握开机映射表重建与 CRC 一致性校验
- 理解 Flush / FUA / TRIM 与数据可靠性的关系

## 目录结构

| 文件 | 说明 | 本机编译 |
|------|------|---------|
| `01_power_loss_risks.c` | 掉电风险点分析 | ✅ gcc 可运行 |
| `02_power_loss_protection.c` | PLP 掉电保护 + 紧急刷盘 | ✅ gcc 可运行 |
| `03_ftl_journal_log.c` | 日志（WAL）+ 崩溃重放恢复 | ✅ gcc 可运行 |
| `04_metadata_recovery.c` | 映射表重建 + CRC 半写页检测 | ✅ gcc 可运行 |
| `05_flush_fsync_sim.c` | Flush / TRIM 与数据可靠性 | ✅ gcc 可运行 |

## 编译运行

```bash
gcc -O2 -Wall -Wextra -o r 01_power_loss_risks.c && ./r
gcc -O2 -Wall -Wextra -o p 02_power_loss_protection.c && ./p
gcc -O2 -Wall -Wextra -o j 03_ftl_journal_log.c && ./j
gcc -O2 -Wall -Wextra -o m 04_metadata_recovery.c && ./m
gcc -O2 -Wall -Wextra -o f 05_flush_fsync_sim.c && ./f
```

## 关键概念速查

- 掉电风险：DRAM dirty 数据 / DRAM 映射表 / 半写页
- PLP：电容供电让固件把关键数据紧急刷到 NAND
- WAL：先写日志再写数据，掉电重放日志重建映射
- 开机恢复：扫描 OOB 重建映射表，CRC 揪出半写页
- 数据可靠性：主机 Flush/FUA + 固件 PLP/日志 共同保证
- TRIM/Deallocate：提前告知无效 LBA，降低写放大

## 本周自测

- [ ] 能说出掉电会丢哪三类东西
- [ ] 能解释 PLP 为什么只保证"已确认"数据
- [ ] 能画出 WAL 的写序与掉电重放流程
- [ ] 能说明 CRC 如何检测半写页
- [ ] 能区分 Flush 与 TRIM 的作用
