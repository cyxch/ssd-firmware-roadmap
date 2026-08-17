#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * 延迟分布与尾延迟(Tail Latency) —— P99 的意义。
 *
 * 平均延迟很有欺骗性:
 *   99% 的 IO 延迟很低(如 100us), 1% 的 IO 延迟很高(如 10ms),
 *   平均 200us 看起来很好, 但 1% 的"慢 IO"可能让应用超时。
 *
 * 企业级 SSD 的要求: P99/P99.9 延迟, 而不是平均延迟。
 * 造成尾延迟的原因: GC 暂停、磨损均衡触发、写缓存满等。
 *
 * 本实验: 模拟一批 IO 延迟, 统计平均 / P90 / P99 / P99.9, 观察差异。
 */

#define IO_NUM 10000

/* 模拟一个 IO 延迟: 大多数快, 少数慢(GC 或暂停导致) */
static int sample_latency(void)
{
    int r = rand() % 1000;
    if (r < 900)
        return 50 + rand() % 50;          /* 90% 的 IO: 50~100us */
    else if (r < 990)
        return 200 + rand() % 300;        /* 9% 的 IO: 200~500us */
    else
        return 2000 + rand() % 3000;      /* 1% 的 IO: 2~5ms(尾延迟) */
}

/* 比较函数用于 qsort */
static int cmp(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

int main(void)
{
    int lat[IO_NUM];
    double sum = 0;

    srand(12345);
    for (int i = 0; i < IO_NUM; i++) {
        lat[i] = sample_latency();
        sum += lat[i];
    }

    qsort(lat, IO_NUM, sizeof(int), cmp);

    double avg = sum / IO_NUM;
    int p50  = lat[IO_NUM * 50 / 100];
    int p90  = lat[IO_NUM * 90 / 100];
    int p99  = lat[IO_NUM * 99 / 100];
    int p999 = lat[IO_NUM * 999 / 1000];

    printf("批次 %d 个 IO 延迟统计:\n\n", IO_NUM);
    printf("  P50  (中位数) : %d us\n", p50);
    printf("  P90           : %d us\n", p90);
    printf("  P99           : %d us\n", p99);
    printf("  P99.9         : %d us\n", p999);
    printf("  平均          : %.0f us\n\n", avg);

    printf("对比: P99 = %d us, 平均 = %.0f us (P99 是平均的 %.0f 倍)\n",
           p99, avg, (double)p99 / avg);
    printf("P99.9 = %d us, 是平均的 %.0f 倍\n", p999, (double)p999 / avg);

    printf("\n教训: 平均延迟掩盖了 1%% 的慢 IO;\n");
    printf("      SSD 固件优化不能只看平均, 要关注 GC 暂停等尾延迟来源;\n");
    printf("      企业级 SLA 常要求 P99.9 < 1ms\n");
    return 0;
}