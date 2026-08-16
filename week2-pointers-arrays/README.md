# Week 2 · 指针与数组

SSD 固件方向学习计划 · 第 2 周（指针与数组）

## 本周目标

- 理解指针的本质：变量地址、指针变量、解引用
- 掌握指针与数组的关系、指针运算、指针数组
- 理解 `const` 与指针的三种组合
- 掌握二级指针
- 手写 `memcpy` / `memmove`（处理内存重叠）
- 数组反转、二分查找
- 自测：用指针交换两个 int

## 目录结构（按主题分类）

| 文件 | 主题 |
|------|------|
| `01_pointer_basics.c` | 指针基础与地址、解引用 |
| `02_pointers_arrays.c` | 指针与数组的关系、指针运算 |
| `03_const_pointers.c` | const 指针 / 常量指针 / 双重 const |
| `04_double_pointers.c` | 二级指针（修改指针本身） |
| `05_memcpy_memmove.c` | 手写 memcpy / memmove（重叠安全） |
| `06_array_ops.c` | 数组反转、二分查找 |
| `07_swap_test.c` | 自测：用指针交换两个 int |

## 编译运行

```bash
gcc -Wall -Wextra -o p 01_pointer_basics.c && ./p
```

## 本周自测（不看答案重写）

- [ ] `memcpy`（含内存重叠安全版）
- [ ] `memmove`
- [ ] 数组反转
- [ ] 用指针参数交换两个 int，并解释为什么必须用指针
