# Week 7 · 虚拟内存与 MMU

SSD 固件方向学习计划 · 第 7 周（虚拟内存与 MMU）

## 本周目标

- 理解虚拟地址、页表、MMU 映射
- 用 Windows `VirtualQuery` 观察进程虚拟地址空间（Windows 版 `/proc/self/maps`）
- 理解页大小、保留(Reserve)与提交(Commit)的区别
- 观察栈向下、堆向上的增长方向
- 掌握 Linux/WSL 下用 `/proc/self/maps`、`pmap` 观察布局

## 目录结构

| 文件 | 主题 |
|------|------|
| `01_address_space.c` | 代码/数据/堆/栈对象地址分布 |
| `02_virtualquery_map.c` | **核心**：VirtualQuery 遍历虚拟地址空间 |
| `03_page_allocation.c` | 页大小 + VirtualAlloc 保留/提交 |
| `04_stack_heap_growth.c` | 栈向下、堆向上增长方向 |
| `05_linux_maps_viewer.c` | Linux/WSL /proc/self/maps 查看器 |

## 编译运行（Windows 版）

```bash
gcc -O0 -Wall -Wextra -o vq 02_virtualquery_map.c && ./vq
gcc -O0 -Wall -Wextra -o pg 03_page_allocation.c && ./pg
gcc -O0 -Wall -Wextra -o g 04_stack_heap_growth.c && ./g
```

## Linux/WSL 下观察

```bash
gcc -O0 -Wall -Wextra -o m 05_linux_maps_viewer.c && ./m
# 或者直接用系统命令：
cat /proc/self/maps      # 当前 shell 的内存映射
pmap $$                  # 当前进程内存映射
```

## 本周自测

- [ ] 能解释"虚拟地址 ≠ 物理地址，由 MMU + 页表转换"
- [ ] 能画出自己进程的虚拟内存分布图
- [ ] 能解释 Reserve 与 Commit 的区别
