#include <stdio.h>
#include <string.h>
#include <math.h>

/*
 * 磨损均衡(Wear Leveling) —— 动态 + 静态。
 *
 * 问题: 某些 LBA 是"热点"(频繁改写), 某些是"冷数据"(写完不动)。
 *   - 只有动态均衡: 热点数据在热块间轮换, 冷数据块永远不被磨损
 *                   -> 热块先到 P/E 上限报废, 冷块寿命白白浪费
 *   - 动态+静态:    后台把冷数据搬到磨损较重的块, 让冷块也参与磨损
 *                   -> 所有块 P/E 趋于均匀, 整盘寿命最大化
 *
 * 本实验统计三种策略下 P/E 分布的 max 与标准差:
 *   max 越小 -> 首个报废的块来得越晚, 寿命越长
 *   标准差越小 -> 磨损越均匀
 */

#define BLOCKS 8
#define HOT_NUM 5          /* 前 5 块放热点数据 */
#define PE_LIMIT 100
#define WRITES 520         /* 主机热点写轮次 */
#define STATIC_EVERY 5     /* 每几次写做一次静态均衡 */

static int pe[BLOCKS];
static int cold[BLOCKS];   /* 1 = 该块当前持冷数据 */

/* 动态均衡分配器: 在"非冷数据"块里选 P/E 最少的 */
static int alloc_warm(void)
{
    int best = -1;
    for (int i = 0; i < BLOCKS; i++)
        if (!cold[i] && (best < 0 || pe[i] < pe[best]))
            best = i;
    return best;
}

/* 静态均衡一步: 冷数据迁移, 让冷块也磨损一次 */
static void static_wl_step(void)
{
    int c = -1, h = -1;
    for (int i = 0; i < BLOCKS; i++)
        if (cold[i] && (c < 0 || pe[i] < pe[c])) c = i;   /* 最冷的冷块 */
    for (int i = 0; i < BLOCKS; i++)
        if (!cold[i] && (h < 0 || pe[i] > pe[h])) h = i;  /* 最热的热块 */
    if (c < 0 || h < 0)
        return;
    pe[c]++;              /* 冷块被擦除重用: 磨损 +1 */
    cold[c] = 0;          /* 它进入热区参与后续写 */
    pe[h]++;              /* 目标块写入冷数据: 磨损 +1 */
    cold[h] = 1;          /* 目标块变成冷块 */
}

/* mode: 0=无均衡 1=仅动态 2=动态+静态 */
static void run(int mode, const char *label)
{
    memset(pe, 0, sizeof(pe));
    memset(cold, 0, sizeof(cold));
    for (int i = HOT_NUM; i < BLOCKS; i++)
        cold[i] = 1;      /* 初始: 后 3 块持冷数据 */

    for (int w = 0; w < WRITES; w++) {
        int b;
        if (mode == 0)
            b = 0;                          /* 无均衡: 总写块 0 */
        else
            b = alloc_warm();               /* 动态: 选磨损最少的热块 */
        pe[b]++;
        if (mode == 2 && (w + 1) % STATIC_EVERY == 0)
            static_wl_step();
    }

    int max = 0, min = 1 << 30;
    double sum = 0, sum2 = 0;
    for (int i = 0; i < BLOCKS; i++) {
        if (pe[i] > max) max = pe[i];
        if (pe[i] < min) min = pe[i];
        sum += pe[i];
        sum2 += (double)pe[i] * pe[i];
    }
    double avg = sum / BLOCKS;
    double sd = sqrt(sum2 / BLOCKS - avg * avg);

    printf("%s\n", label);
    printf("  各块 P/E : ");
    for (int i = 0; i < BLOCKS; i++)
        printf("%d ", pe[i]);
    printf("\n  max=%d min=%d  标准差=%.0f\n", max, min, sd);
    printf("  是否已出现报废(>=%d): %s\n\n", PE_LIMIT,
           max >= PE_LIMIT ? "是, 数据有丢失风险!" : "否");
}

int main(void)
{
    printf("场景: 5 个热块 + 3 个冷数据块, 主机热点写 %d 轮, P/E 上限 %d\n\n",
           WRITES, PE_LIMIT);

    run(0, "策略1: 无磨损均衡");
    run(1, "策略2: 仅动态均衡");
    run(2, "策略3: 动态 + 静态均衡");

    printf("结论:\n");
    printf("  无均衡: 热点块过早报废, 冷块寿命浪费\n");
    printf("  仅动态: 热块间均衡了, 但冷块不参与, 热块仍先报废\n");
    printf("  动+静  : 所有块一起老化, max 最低, 寿命最长\n");
    return 0;
}
