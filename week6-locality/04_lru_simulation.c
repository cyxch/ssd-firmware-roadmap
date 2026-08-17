#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * LRU（最近最少使用）缓存替换策略模拟。
 * 简化：直接映射 + 每路 LRU 的方式太复杂，这里用一个"全相联 LRU 缓存"
 * 演示命中率概念：容量有限时，访问模式决定命中率。
 *
 * 关键观察：
 *   - 顺序访问一个比缓存大的数组：命中率极低（每次都是新行）
 *   - 小循环（热点）：命中率极高（时间局部性）
 *   - 随机访问：命中率低（无局部性）
 */

#define CACHE_WAYS 64          /* 缓存能容纳的"行"数量 */

typedef struct {
    long tag;                  /* 行标签 */
    long last_used;            /* 最近使用时间戳（越大越新） */
} cache_line_t;

static cache_line_t cache[CACHE_WAYS];

static long evict_oldest(void)
{
    long oldest = 0;
    for (long i = 1; i < CACHE_WAYS; i++)
        if (cache[i].last_used < cache[oldest].last_used)
            oldest = i;
    return oldest;
}

/* 模拟一次访问，返回是否命中 */
static int access_one(long tag, long now)
{
    for (long i = 0; i < CACHE_WAYS; i++) {
        if (cache[i].tag == tag) {       /* 命中 */
            cache[i].last_used = now;
            return 1;
        }
    }
    /* 未命中：替换最久未用的行 */
    long slot = evict_oldest();
    cache[slot].tag = tag;
    cache[slot].last_used = now;
    return 0;
}

/* 运行一组访问序列，返回命中率 */
static double run_sequence(const long *seq, long n)
{
    memset(cache, 0, sizeof(cache));
    long hits = 0;
    for (long i = 0; i < n; i++)
        hits += access_one(seq[i], i);
    return (double)hits / n;
}

int main(void)
{
    const long N = 100000;
    long *seq = (long *)malloc(N * sizeof(long));
    if (seq == NULL)
        return 1;

    /* 模式1：顺序扫描（无时间局部性）—— 工作集 > 缓存 */
    for (long i = 0; i < N; i++)
        seq[i] = i % 100000;              /* 范围远大于 64 行 */
    printf("顺序扫描大数组  命中率 = %.1f%%\n", run_sequence(seq, N) * 100);

    /* 模式2：热点小循环（强时间局部性）—— 工作集 << 缓存 */
    for (long i = 0; i < N; i++)
        seq[i] = i % 8;                   /* 只在 8 个地址间循环 */
    printf("热点小循环      命中率 = %.1f%%\n", run_sequence(seq, N) * 100);

    /* 模式3：随机访问（无局部性） */
    for (long i = 0; i < N; i++)
        seq[i] = rand() % 100000;
    printf("随机访问        命中率 = %.1f%%\n", run_sequence(seq, N) * 100);

    free(seq);
    return 0;
}
