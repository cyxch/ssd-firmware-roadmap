# Week 9 · 环形缓冲区与内存池

SSD 固件方向学习计划 · 第 9 周（嵌入式核心数据结构）

## 本周目标

- 掌握环形缓冲区（ring buffer）的实现、满/空判断与回绕
- 掌握 2 的幂掩码队列（NVMe 提交/完成队列同款做法）
- 掌握定长内存池：空闲链表、O(1) 分配/释放、避免碎片
- 量化对比 malloc 与内存池的性能差异
- 理解环形缓冲如何构成 NVMe 的 SQ/CQ 整条链路

## 目录结构

| 文件 | 说明 | 能否在本机编译 |
|------|------|--------------|
| `01_ring_buffer.c` | 基础环形缓冲：读写/满空/回绕 | ✅ gcc 可运行 |
| `02_ring_buffer_pow2.c` | 2 的幂掩码队列（留一格判满） | ✅ gcc 可运行 |
| `03_memory_pool.c` | **交付物**：定长内存池（空闲链表） | ✅ gcc 可运行 |
| `04_mempool_vs_malloc.c` | malloc vs 内存池性能对比 | ✅ gcc 可运行 |
| `05_nvme_queue_sim.c` | NVMe SQ/CQ 队列整链路模拟 | ✅ gcc 可运行 |

## 编译运行

```bash
gcc -O2 -Wall -Wextra -o r 01_ring_buffer.c && ./r
gcc -O2 -Wall -Wextra -o q 02_ring_buffer_pow2.c && ./q
gcc -O2 -Wall -Wextra -o m 03_memory_pool.c && ./m
gcc -O2 -Wall -Wextra -o b 04_mempool_vs_malloc.c && ./b
gcc -O2 -Wall -Wextra -o n 05_nvme_queue_sim.c && ./n
```

## 关键概念速查

- 环形缓冲满/空判断的两种方法：`count` 计数法 / 留一格法
- NVMe 队列深度必须为 2 的幂，索引回绕用 `& (size-1)`
- 内存池把空闲块指针存在块自身首个字中，零额外开销
- SSD 固件中：命令结构体、事件结构体常用内存池；SQ/CQ 用环形缓冲

## 本周自测

- [ ] 能写出环形缓冲满/空的两种判断方式并说出取舍
- [ ] 能解释为什么 NVMe 队列深度要求 2 的幂
- [ ] 能说出内存池相比 malloc 的三个优缺点
- [ ] 能画出 SQ/CQ 的生产者-消费者关系与 doorbell 交互
