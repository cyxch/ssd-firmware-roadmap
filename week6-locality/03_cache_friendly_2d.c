#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * 二维数组遍历方向：C 数组按"行优先"存储。
 *   arr[i][j] 的地址 = base + (i*COLS + j) * sizeof(int)
 * 行遍历: 内存连续 -> 空间局部性好 -> 快
 * 列遍历: 每次跨一整行 -> 缓存行被浪费 -> 慢
 * 这是"缓存友好"代码的最典型例子。
 */

#define ROWS 4096
#define COLS 4096

static double elapsed_sec(clock_t start, clock_t end)
{
    return (double)(end - start) / CLOCKS_PER_SEC;
}

int main(void)
{
    int (*m)[COLS] = (int (*)[COLS])malloc(ROWS * COLS * sizeof(int));
    if (m == NULL) {
        printf("malloc 失败\n");
        return 1;
    }
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            m[i][j] = (i + j) & 0xFF;

    printf("矩阵 %dx%d, %d MB\n", ROWS, COLS,
           (int)(ROWS * COLS * sizeof(int) / 1024 / 1024));

    volatile int acc = 0;

    /* 行优先遍历：连续内存 */
    clock_t s1 = clock();
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            acc += m[i][j];
    clock_t e1 = clock();

    /* 列优先遍历：跳着访问 */
    clock_t s2 = clock();
    for (int j = 0; j < COLS; j++)
        for (int i = 0; i < ROWS; i++)
            acc += m[i][j];
    clock_t e2 = clock();

    double t_row = elapsed_sec(s1, e1);
    double t_col = elapsed_sec(s2, e2);

    printf("行优先 = %.3f s\n", t_row);
    printf("列优先 = %.3f s\n", t_col);
    printf("列优先是行优先的 %.1f 倍慢\n", t_col / t_row);
    printf("(sum=%d)\n", acc);

    free(m);
    return 0;
}
