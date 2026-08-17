#include <stdio.h>

/*
 * 反汇编入门：本文件用于配合 objdump 学习汇编。
 * 指令对照（x86-64 AT&T/Intel）：
 *   push/pop  入栈/出栈
 *   mov        数据搬运（复制值）
 *   lea        计算地址
 *   add/sub    加减
 *   call/ret   调用/返回（call 会把返回地址压栈，ret 弹出跳回）
 *
 * 用法：
 *   gcc -g -O0 -c 03_asm_view.c -o asm.o
 *   objdump -d asm.o
 * 找到 <add> 符号，把汇编与下面的 C 对照看。
 */

static int add(int a, int b)
{
    return a + b;
}

static int mul2(int x)
{
    return x * 2;
}

int main(void)
{
    int r1 = add(3, 4);
    int r2 = mul2(r1);
    printf("r1=%d r2=%d\n", r1, r2);
    return 0;
}
