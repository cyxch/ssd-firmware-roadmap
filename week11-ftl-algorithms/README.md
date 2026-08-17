# Week 11 · FTL 核心算法

SSD 固件方向学习计划 · 第 11 周（FTL 核心算法）

## 本周目标

- 理解 FTL 为什么要做地址映射，以及页级/块级映射的取舍
- 掌握 out-of-place 更新与 L2P 映射表读写流程
- 掌握垃圾回收（GC）：victim 选择与有效页搬移
- 掌握动态/静态磨损均衡的区别与意义
- 理解写放大系数（WAF）及其对寿命的影响

## 目录结构

| 文件 | 说明 | 本机编译 |
|------|------|---------|
| `01_ftl_address_mapping.c` | 页级/块级映射与映射表大小计算 | ✅ gcc 可运行 |
| `02_ftl_write_read_sim.c` | 完整写/读流程 + out-of-place 更新 | ✅ gcc 可运行 |
| `03_ftl_garbage_collection.c` | 垃圾回收 + victim 选择 | ✅ gcc 可运行 |
| `04_ftl_wear_leveling.c` | 无/动态/动态+静态磨损均衡对比 | ✅ gcc 可运行 |
| `05_ftl_wa_measure.c` | 写放大系数 WAF 测量（顺序 vs 热点） | ✅ gcc 可运行 |

## 编译运行

```bash
gcc -O2 -Wall -Wextra -o m 01_ftl_address_mapping.c && ./m
gcc -O2 -Wall -Wextra -o w 02_ftl_write_read_sim.c && ./w
gcc -O2 -Wall -Wextra -o g 03_ftl_garbage_collection.c && ./g
gcc -O2 -Wall -Wextra -o l 04_ftl_wear_leveling.c && ./l
gcc -O2 -Wall -Wextra -o a 05_ftl_wa_measure.c && ./a
```

## 关键概念速查

- FTL（Flash Translation Layer）：LBA → PBA 映射 + 磨损均衡 + GC
- 页级映射表是内存大户，掉电需持久化，开机重建
- out-of-place 更新产生 invalid 页 → GC 回收整块
- GC 搬有效页 = 写放大来源；victim 选有效页最少的块
- 静态磨损均衡让冷数据块也参与磨损，最大化整盘寿命
- WAF = NAND 实际写 / 主机写；顺序写 WAF 低，热点重写 WAF 高

## 本周自测

- [ ] 能解释为什么 NAND 必须有 FTL 而不是直接映射
- [ ] 能说出页级 vs 块级映射的优缺点
- [ ] 能画出 out-of-place 更新 + invalid 页 + GC 的完整链路
- [ ] 能说明动态均衡为什么不够、静态均衡补了什么
- [ ] 能解释 WAF 高为什么缩短 SSD 寿命
