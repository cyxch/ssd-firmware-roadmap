# Week 1 · C 语言筑基

SSD 固件方向学习计划 · 第 1 周（C 语言筑基）

## 本周目标

- 搭建 GCC + VS Code 开发环境
- 掌握 C 语言基本语法、类型、运算符、控制流、函数与作用域
- 理解编译四阶段（预处理 → 编译 → 汇编 → 链接）
- 手写 `strlen` / `strcpy` / `atoi`
- 综合小项目：命令行计算器

## 目录结构（按主题分类）

| 目录 | 内容 | 主题 |
|------|------|------|
| `01_hello_world/` | `hello.c` | 第一个程序 + 编译四阶段 |
| `02_types_operators/` | `types.c` | 数据类型、运算符、位运算 |
| `03_control_flow/` | `control.c` | if/for/while/switch |
| `04_functions_scope/` | `function.c` | 函数、作用域、static |
| `05_handwritten_lib/` | `my_string.c` | 手写 strlen/strcpy/atoi |
| `06_calculator/` | `calculator.c` | 综合小项目：计算器 |

## 编译运行

```bash
gcc 01_hello_world/hello.c -o hello
./hello          # Windows 下为 hello.exe
```

验证编译四阶段：

```bash
gcc -E 01_hello_world/hello.c   # 预处理
gcc -S 01_hello_world/hello.c   # 编译 → .s 汇编
gcc -c 01_hello_world/hello.c   # 汇编 → .o 目标文件
gcc -o hello 01_hello_world/hello.c  # 链接
```

## 学习记录

- [ ] 完成环境搭建（MSYS2 / MinGW-w64 + VS Code）
- [ ] 完成 6 个练习文件
- [ ] 手写 strlen / strcpy / atoi 不看答案
- [ ] 用 gcc -E/-S/-c 验证编译四阶段
