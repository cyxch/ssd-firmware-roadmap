#include <stdio.h>
#include <stdlib.h>

/*
 * malloc / free 与常见陷阱：
 *   - 泄漏：分配后没 free（长时间运行的程序会耗尽内存）
 *   - 悬垂指针：free 之后还继续用（未定义行为）
 *   - 双重释放：同一个指针 free 两次（崩溃）
 *   - 内存碎片：频繁小分配/释放会产生碎片（嵌入式用内存池解决）
 */

int main(void)
{
    /* 正确用法：分配 -> 检查 -> 使用 -> 释放 -> 置 NULL */
    int *p = (int *)malloc(5 * sizeof(int));
    if (p == NULL) {
        printf("malloc 失败\n");
        return 1;
    }

    for (int i = 0; i < 5; i++)
        p[i] = i * 10;

    printf("p[3] = %d\n", p[3]);

    free(p);
    p = NULL;               /* 释放后置 NULL，防止悬垂访问 */

    /* 泄漏演示（仅展示概念，这里故意不 free 会告警，可忽略） */
    /* int *leak = malloc(1024);  泄漏，见注释即可 */
    /* 推荐：用 Valgrind / ASan 检测泄漏：
     *   gcc -fsanitize=address -o t 02_malloc_free.c && ./t
     */

    return 0;
}
