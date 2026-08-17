# Week 10 · NAND Flash 原理

SSD 固件方向学习计划 · 第 10 周（NAND 物理特性）

## 本周目标

- 掌握 NAND 物理层级：die / plane / block / page / cell
- 理解 SLC/MLC/TLC/QLC 与容量的关系、寿命权衡
- 理解三大物理约束：页读写 / 块擦除 / 写前必擦
- 掌握坏块来源与坏块表（BBT）管理
- 理解干扰（编程/读干扰）与 ECC 纠错原理
- 理解块生命周期与 P/E 循环、磨损均衡的必要性

## 目录结构

| 文件 | 说明 | 本机编译 |
|------|------|---------|
| `01_nand_cell_organization.c` | 物理层级 + SLC/MLC/TLC/QLC 容量计算 | ✅ gcc 可运行 |
| `02_nand_ops_timing.c` | 页读/页写/块擦时序与写前必擦 | ✅ gcc 可运行 |
| `03_nand_bad_block.c` | 坏块管理 + 坏块表(BBT)位图 | ✅ gcc 可运行 |
| `04_nand_disturb_ecc.c` | 干扰 + 4x4 矩阵单比特纠错(SEC) | ✅ gcc 可运行 |
| `05_nand_lifecycle_sim.c` | 块生命周期 + P/E 循环 + 磨损均衡对比 | ✅ gcc 可运行 |

## 编译运行

```bash
gcc -O2 -Wall -Wextra -o c 01_nand_cell_organization.c && ./c
gcc -O2 -Wall -Wextra -o t 02_nand_ops_timing.c && ./t
gcc -O2 -Wall -Wextra -o b 03_nand_bad_block.c && ./b
gcc -O2 -Wall -Wextra -o e 04_nand_disturb_ecc.c && ./e
gcc -O2 -Wall -Wextra -o l 05_nand_lifecycle_sim.c && ./l
```

## 关键概念速查

- 最小读写单位 = 页；最小擦除单位 = 块
- 写（编程）只能 1→0，不能 0→1，所以改写必须整块擦除
- 坏块：出厂坏块（工厂标记）+ 运行坏块（磨损/干扰）
- ECC：BCH/LDPC 码纠正随机比特错误，TLC/QLC 需要更强 ECC
- P/E 上限：SLC~100k / MLC~10k / TLC~3k，磨损均衡延长整盘寿命

## 本周自测

- [ ] 能画出 die→plane→block→page→cell 的层级并说出各单位作用
- [ ] 能解释为什么"改 1 字节"实际要做整块擦除
- [ ] 能说出坏块来源与 BBT 在启动时的加载流程
- [ ] 能解释 ECC 为什么能定位单比特错误（行列校验思想）
- [ ] 能说明磨损不均为什么会让 SSD 提前报废
