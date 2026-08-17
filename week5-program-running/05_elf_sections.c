#include <stdio.h>

/*
 * 可执行文件段与内存地址：
 *   .text   代码段（只读）—— 函数
 *   .rodata 只读数据 —— 字符串常量
 *   .data   已初始化数据 —— 有初值的全局/静态变量
 *   .bss    未初始化数据 —— 零初始化，运行时占内存但不占文件空间
 *   stack   栈 —— 局部变量（高地址，向下增长）
 *   heap    堆 —— malloc（向高地址增长）
 *
 * 用命令查看：
 *   gcc -g 05_elf_sections.c -o sec
 *   objdump -h sec        # 段表
 *   nm sec                # 符号表（可看到 _start/main/全局变量地址）
 *   size sec              # 各段大小
 */

int g_data = 100;           /* .data */
int g_bss;                  /* .bss */
static int s_bss;           /* .bss */

static const char msg[] = "hello";   /* .rodata */

static int g_heap_area[64];
static int *malloc_small(void);      /* 前置声明 */

int main(void)
{
    int stack_local = 5;                    /* 栈 */
    int *heap_p = (int *)malloc_small();    /* 堆，见下 */

    printf("g_data (data)  = %p\n", (void *)&g_data);
    printf("g_bss  (bss)   = %p\n", (void *)&g_bss);
    printf("s_bss  (bss)   = %p\n", (void *)&s_bss);
    printf("msg    (rodata)= %p\n", (void *)msg);
    printf("main   (text)  = %p\n", (void *)main);
    printf("stack  (栈)    = %p\n", (void *)&stack_local);
    printf("heap   (堆)    = %p\n", (void *)heap_p);
    return 0;
}

/* 简化堆分配：用静态数组模拟，避免引入 stdlib 的 malloc 以便聚焦地址 */
static int *malloc_small(void)
{
    return g_heap_area;
}
