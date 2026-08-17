#include <stdio.h>

/*
 * 函数调用栈：每次调用函数都会在栈上创建"栈帧"，包含：
 *   局部变量、参数、返回地址、保存的旧栈指针。
 * 递归/深层调用会耗尽栈空间 -> 栈溢出。
 * 用地址对比观察"栈从高地址向低地址增长"。
 */

void fun_a(int x)
{
    int local_a = x + 1;
    printf("[A] 参数 x=%d 栈上局部变量地址 %p\n", x, (void *)&local_a);
}

void fun_b(void)
{
    int local_b = 42;
    printf("[B] 局部变量地址 %p\n", (void *)&local_b);
    fun_a(local_b);
}

/* 递归演示：打印栈帧地址递减，直观看到"栈"在生长 */
void recurse(int n)
{
    int frame = n;
    printf("递归深度 %2d  帧地址 %p\n", n, (void *)&frame);
    if (n > 0)
        recurse(n - 1);
}

int main(void)
{
    int main_var = 0;
    printf("[main] 局部变量地址 %p\n", (void *)&main_var);

    fun_b();

    printf("--- 递归栈帧 ---\n");
    recurse(5);

    return 0;
}
