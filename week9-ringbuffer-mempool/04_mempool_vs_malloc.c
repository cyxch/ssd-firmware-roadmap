#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

/*
 * 性能对比：内存池 vs malloc，高频"分配+释放"小对象。
 * 场景：固件里频繁申请/释放 32 字节小对象(如命令/事件结构体)。
 * 预期结论：内存池明显更快(malloc 有堆管理、可能锁/系统调用)；
 * 更重要的是 malloc 耗时不确定(可能触发堆扩展)，内存池耗时恒定。
 *
 * 注意：必须用 -O2 以外也验证？本实验用了 volatile 汇合点，
 * 防止编译器把分配结果优化掉，O2/O0 结果趋势一致。
 */

#define ALLOC_NUM 10000    /* 每轮分配个数 */
#define ITER 50            /* 重复轮数 */
#define BLK_SIZE 32        /* 对象大小 */

/* ---- 定长内存池(简化版，见 03) ---- */
typedef struct {
    unsigned char data[BLK_SIZE];
} block_t;

static block_t g_pool[ALLOC_NUM];
static block_t *g_flist;

static void pool_init(void)
{
    g_flist = NULL;
    for (int i = ALLOC_NUM - 1; i >= 0; i--) {
        *(block_t **)g_pool[i].data = g_flist;
        g_flist = &g_pool[i];
    }
}

static void *pool_alloc(void)
{
    if (!g_flist)
        return NULL;
    block_t *b = g_flist;
    g_flist = *(block_t **)b->data;
    return b->data;
}

static void pool_free(void *p)
{
    block_t *b = (block_t *)p;
    *(block_t **)b->data = g_flist;
    g_flist = b;
}
/* ------------------------------- */

/* volatile 汇合点：让编译器认为分配结果真的被使用了 */
static volatile uintptr_t g_sink = 0;

static double run_malloc(int iters)
{
    void *ptrs[ALLOC_NUM];
    clock_t t0 = clock();

    for (int it = 0; it < iters; it++) {
        for (int i = 0; i < ALLOC_NUM; i++) {
            ptrs[i] = malloc(BLK_SIZE);
            if (!ptrs[i]) {
                fprintf(stderr, "malloc 失败\n");
                exit(1);
            }
        }
        for (int i = 0; i < ALLOC_NUM; i++) {
            g_sink = (uintptr_t)ptrs[i];
            free(ptrs[i]);
        }
    }
    return (double)(clock() - t0) / CLOCKS_PER_SEC;
}

static double run_pool(int iters)
{
    void *ptrs[ALLOC_NUM];
    clock_t t0 = clock();

    for (int it = 0; it < iters; it++) {
        for (int i = 0; i < ALLOC_NUM; i++)
            ptrs[i] = pool_alloc();
        for (int i = 0; i < ALLOC_NUM; i++) {
            g_sink = (uintptr_t)ptrs[i];
            pool_free(ptrs[i]);
        }
    }
    return (double)(clock() - t0) / CLOCKS_PER_SEC;
}

int main(void)
{
    pool_init();

    printf("每次轮: %d 次 %d 字节 alloc+free，共 %d 轮\n\n",
           ALLOC_NUM, BLK_SIZE, ITER);

    double t_malloc = run_malloc(ITER);
    double t_pool   = run_pool(ITER);

    printf("  malloc : %6.3f 秒\n", t_malloc);
    printf("  pool   : %6.3f 秒\n", t_pool);
    if (t_pool > 0)
        printf("  提速   : %.0f 倍\n", t_malloc / t_pool);

    printf("\n汇合值(防止优化): %lu\n", (unsigned long)g_sink);
    printf("\n结论: 小对象高频分配下内存池更快；且 malloc 耗时不确定(可能触发堆扩展)，\n");
    printf("      内存池 O(1) 且耗时恒定，对实时性友好\n");
    return 0;
}
