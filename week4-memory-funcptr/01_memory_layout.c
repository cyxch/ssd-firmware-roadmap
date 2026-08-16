#include <stdio.h>
#include <stdlib.h>

/*
 * 内存四区（C 程序地址空间，从高到低）：
 *   栈区    : 局部变量、函数调用帧，自动分配/释放，小且快
 *   堆区    : malloc/free 动态分配，手动管理
 *   全局/静态区: 全局变量、static 变量，程序生命周期内存在
 *   代码区  : 函数指令（只读）
 * 嵌入式通常没有"操作系统堆"，常见做法就是内存池（见 05）。
 */

int g_global = 10;          /* 全局区 */
static int s_static = 20;   /* 全局区(静态) */

void func(void)
{
    int local = 30;         /* 栈区 */
    printf("  func 内 local = %d (栈)\n", local);
}

int main(void)
{
    int stack_var = 5;              /* 栈区 */
    int *heap_var = (int *)malloc(sizeof(int));  /* 堆区 */

    if (heap_var == NULL) {
        printf("malloc 失败\n");
        return 1;
    }
    *heap_var = 99;

    printf("g_global  地址 %p (全局区)\n", (void *)&g_global);
    printf("s_static  地址 %p (静态区)\n", (void *)&s_static);
    printf("stack_var 地址 %p (栈区)\n", (void *)&stack_var);
    printf("heap_var  地址 %p (堆区)\n", (void *)heap_var);

    func();

    free(heap_var);                 /* 用完释放，避免泄漏 */

    return 0;
}
