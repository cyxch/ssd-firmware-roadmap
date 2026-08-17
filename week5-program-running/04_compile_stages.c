#include <stdio.h>

/*
 * 编译四阶段（用下面的命令亲手验证每一步的产物）：
 *   1) 预处理  gcc -E 04_compile_stages.c -o stage1.i
 *                展开 #include/#define，可 grep "#define MUL" 看宏还在不在
 *   2) 编译    gcc -S stage1.i -o stage2.s
 *                生成汇编文件
 *   3) 汇编    gcc -c stage2.s -o stage3.o
 *                生成目标文件（机器码，还不能运行）
 *   4) 链接    gcc stage3.o -o stage4
 *                链接库与启动代码，生成可执行文件
 *
 * 可执行文件与 .o 的区别：.o 里符号地址是 0，链接后才有真实地址。
 */

#define MUL(x, y) ((x) * (y))   /* 预处理阶段会被展开 */

static int compute(int a, int b)
{
    return MUL(a, b) + a - b;
}

int main(void)
{
    printf("compute(6,7) = %d\n", compute(6, 7));
    return 0;
}
