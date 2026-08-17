# Week 6 · 存储层级与局部性

SSD 固件方向学习计划 · 第 6 周（存储层级与局部性）

## 本周目标

- 理解存储层级（寄存器→Cache→主存→磁盘/SSD）
- 掌握时间/空间局部性原理（SSD 固件的性能命脉）
- 亲手实验：**顺序访问 vs 跳步访问**（为什么顺序读快）
- 二维数组遍历方向对缓存的影响
- 模拟 LRU 缓存替换策略

## 目录结构

| 文件 | 主题 |
|------|------|
| `01_memory_hierarchy.c` | 存储层级总览（金字塔） |
| `02_locality_order_vs_stride.c` | **核心实验**：顺序 vs 跳步访问计时 |
| `03_cache_friendly_2d.c` | 二维数组行/列遍历对比 |
| `04_lru_simulation.c` | LRU 缓存命中率模拟 |

## 编译运行（02/03 务必用 -O0，避免优化掩盖缓存效果）

```bash
gcc -O0 -Wall -Wextra -o exp 02_locality_order_vs_stride.c && ./exp
gcc -O0 -Wall -Wextra -o m2 03_cache_friendly_2d.c && ./m2
gcc -O0 -Wall -Wextra -o lru 04_lru_simulation.c && ./lru
```

> 优化级别会显著改变结果：`-O2` 可能让跳步访问也很快（编译器向量化/预取），
> 实验要用 `-O0` 才能看到真实的缓存失效代价。

## 本周自测

- [ ] 能解释"为什么顺序读快"（局部性 + cache line）
- [ ] 能画出存储层级金字塔并标注大小/速度/成本
- [ ] 能解释 LRU 替换策略如何工作
