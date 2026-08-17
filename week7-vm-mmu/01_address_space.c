#include <stdio.h>
#include <stdlib.h>

/*
 * 进程虚拟地址空间：程序里的每个对象都落在不同的虚拟地址区。
 * 注意：这些是"虚拟地址"，由 MMU + 页表映射到物理内存；
 *       不同进程可以各自拥有完全相同的虚拟地址而互不干扰。
 */

int g_data = 5;                 /* .data 段（已初始化全局） */
int g_bss;                      /* .bss  段（零初始化全局） */
static const char s_rodata[] = "rodata-text";   /* .rodata 只读 */

int main(void)
{
    int stack_var = 0;          /* 栈区 */
    int *heap_var = (int *)malloc(64 * sizeof(int));  /* 堆区 */

    printf("代码 main()   : %p   (.text 只读)\n", (void *)main);
    printf("只读 rodata   : %p   (.rodata)\n", (void *)s_rodata);
    printf("全局 g_data   : %p   (.data)\n", (void *)&g_data);
    printf("全局 g_bss    : %p   (.bss)\n", (void *)&g_bss);
    printf("堆   heap_var : %p   (malloc)\n", (void *)heap_var);
    printf("栈   stack_var: %p   (局部变量)\n", (void *)&stack_var);

    printf("\n观察要点:\n");
    printf("  - 代码/数据/堆/栈分处不同的虚拟地址范围(Windows 布局与教科书示意可能不同)\n");
    printf("  - 32 位程序虚拟地址空间约 4GB；64 位约 128TB(用户态)\n");
    printf("  - 这些地址是虚拟的，物理页由 MMU 按需分配(缺页时才真正占内存)\n");
    printf("  - 具体布局可用 02_virtualquery_map.c 遍历查看\n");

    free(heap_var);
    return 0;
}
