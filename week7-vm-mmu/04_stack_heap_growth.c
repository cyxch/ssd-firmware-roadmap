#include <stdio.h>
#include <stdlib.h>

/*
 * 栈向下、堆向上的增长方向观察。
 * 栈：每次调用在栈顶压入新帧，地址不断减小（向下生长）。
 * 堆：malloc 分配的块地址不断增大（向上生长）。
 * 两者相向生长，中间是"空隙"；若耗尽则栈溢出或堆耗尽。
 */

#define DEPTH 5000   /* 递归深度（每帧很小，安全范围） */

static void first_frame(void *addr)
{
    /* 到达最深处 */
    int deepest;
    printf("栈帧地址: 首帧 %p  ->  最深帧 %p\n", addr, (void *)&deepest);
    printf("栈生长方向: 地址减小 = 向下生长 (delta ≈ %ld KB)\n",
           (long)(((char *)addr - (char *)&deepest) / 1024));
}

static void recurse(int n, void *first)
{
    int frame;
    if (n <= 0) {
        first_frame(first);
        return;
    }
    (void)frame;
    recurse(n - 1, first);
}

int main(void)
{
    /* 栈：向下 */
    int stack_base;
    printf("=== 栈 (向下生长) ===\n");
    recurse(DEPTH, &stack_base);

    /* 堆：向上 */
    printf("\n=== 堆 (向上生长) ===\n");
    void *prev = NULL;
    for (int i = 0; i < 8; i++) {
        void *p = malloc(1024 * 1024);   /* 每次 1 MB */
        printf("  malloc #%d @ %p%s\n", i, p,
               prev ? (p > prev ? "  (增大↑)" : "  (非单调)") : "");
        if (p == NULL) {
            printf("malloc 失败\n");
            break;
        }
        prev = p;
    }
    printf("堆地址总体向上增长（分配器可能复用/合并块，偶有波动）\n");
    return 0;
}
