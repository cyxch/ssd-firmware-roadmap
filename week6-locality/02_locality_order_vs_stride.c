#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * 核心实验：顺序访问 vs 跳步访问
 * 原理：CPU 以"缓存行(cache line, 通常 64 字节)"为单位搬数据。
 *   - 顺序访问(stride=1): 每取一个缓存行，16 个 int 全部命中 -> 快
 *   - 跳步访问(stride 大): 每取一个缓存行只用 1 个 int，其余浪费 -> 慢
 * 结论：同样的访问次数，顺序访问可能快 10-50 倍。
 *
 * 编译务必用 -O0：
 *   gcc -O0 -Wall -Wextra -o exp 02_locality_order_vs_stride.c && ./exp
 */

#define N      (1 << 22)   /* 数组元素数：4M 个 int = 16 MB，超过 L3 */
#define ITERS  (1 << 24)   /* 每个 stride 的访问次数：16M 次 */

static double elapsed_sec(clock_t start, clock_t end)
{
    return (double)(end - start) / CLOCKS_PER_SEC;
}

int main(void)
{
    int *arr = (int *)malloc(N * sizeof(int));
    if (arr == NULL) {
        printf("malloc 失败\n");
        return 1;
    }

    /* 初始化，防止稀疏虚拟页导致首次访问开销干扰 */
    for (int i = 0; i < N; i++)
        arr[i] = i & 0xFF;

    printf("数组大小 = %d * %d B = %d MB (超过 L3，主要走主存)\n",
           N, (int)sizeof(int), (int)(N * sizeof(int) / 1024 / 1024));
    printf("每个 stride 访问次数 = %d\n", ITERS);
    printf("\n stride |   耗时(s) | 相对顺序访问倍数\n");
    printf("--------+-----------+-------------------\n");

    /* 先热身，把内存页调到缓存可比较的状态 */
    volatile int acc = 0;
    for (int i = 0; i < (1 << 16); i++)
        acc += arr[i];

    int strides[] = {1, 4, 16, 64, 256, 1024};
    double base = 0.0;

    for (int s = 0; s < 6; s++) {
        int stride = strides[s];
        clock_t start = clock();

        for (int k = 0; k < ITERS; k++)
            acc += arr[(k * stride) & (N - 1)];   /* 取模保持下标在范围内 */

        clock_t end = clock();
        double t = elapsed_sec(start, end);
        if (s == 0)
            base = t;

        printf(" %6d | %9.3f | %15.1fx\n",
               stride, t, t / base);
    }

    /* 防止 acc 被优化掉 */
    printf("(sum=%d, 防止优化)\n", acc);
    free(arr);
    return 0;
}
