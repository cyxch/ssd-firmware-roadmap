# Week 5 · 程序如何运行

SSD 固件方向学习计划 · 第 5 周（程序如何运行）

## 本周目标

- 理解补码、整数表示与溢出（CSAPP 第 2 章核心）
- 理解函数调用栈与栈帧（push/pop、返回地址）
- 会用 `objdump` 反汇编，看懂简单汇编指令
- 亲手跑一遍编译四阶段（预处理→编译→汇编→链接）
- 观察可执行文件的段（.text/.data/.bss）与内存地址

## 目录结构

| 文件 | 主题 |
|------|------|
| `01_integer_representation.c` | 补码、溢出、字节序 |
| `02_call_stack.c` | 函数调用栈与栈帧 |
| `03_asm_view.c` | 反汇编看懂汇编 |
| `04_compile_stages.c` | 编译四阶段产物对比 |
| `05_elf_sections.c` | 可执行文件段与地址 |

## 编译运行

```bash
gcc -Wall -Wextra -o r 01_integer_representation.c && ./r

# 反汇编
gcc -g -O0 -c 03_asm_view.c -o asm.o
objdump -d asm.o

# 查看段
gcc -g 05_elf_sections.c -o sec
objdump -h sec          # 段表
nm sec                  # 符号地址
```

## 本周自测

- [ ] 解释"为什么 32 位 int 最大值是 2147483647，再加 1 变负数"
- [ ] 画图说明函数调用的压栈/弹栈过程
- [ ] 用 objdump 找到 add() 的汇编并对照 C 源码
