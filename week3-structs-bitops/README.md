# Week 3 · 结构体、联合体与位运算

SSD 固件方向学习计划 · 第 3 周（结构体、联合体与位运算）

## 本周目标

- 掌握结构体的定义、初始化、访问、指针操作
- 理解内存对齐（alignment）与 `#pragma pack`
- 掌握联合体（union）与枚举（enum）
- 掌握位域（bitfield）在寄存器场景的应用
- 用宏封装位运算（SET_BIT / CLEAR_BIT / GET_BIT）
- 综合项目：用 union + 位域模拟一个 32 位寄存器

## 目录结构（按主题分类）

| 文件 | 主题 |
|------|------|
| `01_struct_basics.c` | 结构体定义/初始化/访问/指针 |
| `02_alignment_packing.c` | 内存对齐与 #pragma pack |
| `03_union_enum.c` | 联合体与枚举 |
| `04_bitfields.c` | 位域与寄存器 |
| `05_bit_ops_macros.c` | 位运算宏封装 |
| `06_register_simulator.c` | 综合项目：union+位域模拟寄存器 |
| `07_self_test.c` | 自测题 |

## 编译运行

```bash
gcc -Wall -Wextra -o r 06_register_simulator.c && ./r
```

## 本周自测

- [ ] 解释 `#define` 与 `#pragma pack` 对结构体大小的影响
- [ ] 用 union + 位域实现 32 位寄存器字段读写
- [ ] 说出结构体成员顺序与 sizeof 的关系
