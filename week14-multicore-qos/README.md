# Week 14 · 多核固件架构与 QoS

SSD 固件方向学习计划 · 第 14 周（多核架构与服务质量）

## 本周目标

- 理解多核流水线(pipeline)并行与加速比
- 掌握 IO 调度器：FIFO vs 优先级队列
- 掌握加权轮询(WRR)与带宽分配
- 理解命令分派与多核负载均衡
- 理解延迟分布与尾延迟(P99)的意义

## 目录结构

| 文件 | 说明 | 本机编译 |
|------|------|---------|
| `01_multi_core_pipeline.c` | 多核流水线 vs 单核吞吐对比 | ✅ gcc 可运行 |
| `02_io_scheduler.c` | FIFO vs 优先级队列（QoS 基础） | ✅ gcc 可运行 |
| `03_qos_wrr.c` | 加权轮询与带宽分配 | ✅ gcc 可运行 |
| `04_command_steering.c` | 命令分派与多核无锁设计 | ✅ gcc 可运行 |
| `05_latency_tail_latency.c` | 延迟分布与尾延迟 P99 | ✅ gcc 可运行 |

## 编译运行

```bash
gcc -O2 -Wall -Wextra -o p 01_multi_core_pipeline.c && ./p
gcc -O2 -Wall -Wextra -o s 02_io_scheduler.c && ./s
gcc -O2 -Wall -Wextra -o w 03_qos_wrr.c && ./w
gcc -O2 -Wall -Wextra -o c 04_command_steering.c && ./c
gcc -O2 -Wall -Wextra -o l 05_latency_tail_latency.c && ./l
```

## 关键概念速查

- 流水线并行：拆阶段，满流水时吞吐 = 1/最慢阶段耗时
- 优先级调度：高优先级命令先出，避免被普通 IO 堵死
- WRR：按权重轮询，保障各租户/命名空间的带宽比例
- 多核无锁设计：每核独立队列，命令按 hash 分派，消除锁竞争
- 尾延迟：P99/P99.9 比平均延迟更能反映真实体验，GC 暂停是主要来源

## 本周自测

- [ ] 能画出流水线 4 核的时序图并算出加速比
- [ ] 能说出优先级调度为什么不能只用 FIFO
- [ ] 能解释 WRR 如何保证带宽分配
- [ ] 能说明多核无锁设计的关键原则
- [ ] 能解释为什么 P99 比平均延迟更重要