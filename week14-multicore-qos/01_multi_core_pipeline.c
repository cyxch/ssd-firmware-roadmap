#include <stdio.h>

/*
 * 多核固件架构：流水线(pipeline)并行 vs 单核串行。
 *
 * SSD 主控固件把 IO 处理拆成多个阶段:
 *   取命令 -> 命令解析 -> FTL 映射 -> 媒体访问(NAND)
 * 单核: 一个 IO 走完 4 阶段, 下一个才能开始(串行, 吞吐 = 1/总耗时)
 * 多核: 每阶段一个核, 第 2 个 IO 进入阶段1 时, 阶段2 正在处理第 1 个
 *       -> 满流水后每个单位时间完成一个 IO(吞吐 ≈ 1/单阶段耗时)
 *
 * 本实验: 均衡 4 阶段(每阶段 10us), 对比单核与 4 核流水线的总耗时。
 */

#define STAGES 4
#define IO_NUM 1000

static const int stage_cost[STAGES] = { 10, 10, 10, 10 };  /* us */

/* 单核: 每个 IO 串行走完 4 阶段 */
static long long single_core_total(void)
{
    return (long long)IO_NUM * STAGES * stage_cost[0];
}

/* 4 核流水线: 首个 IO 走完 4 阶段后, 之后每阶段耗时出一个 IO */
static long long pipeline_total(void)
{
    int bottleneck = stage_cost[0];
    for (int i = 1; i < STAGES; i++)
        if (stage_cost[i] > bottleneck)
            bottleneck = stage_cost[i];
    return (long long)IO_NUM * bottleneck + STAGES * stage_cost[0] - bottleneck;
}

int main(void)
{
    long long single = single_core_total();
    long long pipe = pipeline_total();

    printf("IO 处理 4 阶段, 每阶段 %dus, 共 %d 个 IO\n", stage_cost[0], IO_NUM);
    printf("\n单核串行 : %lld us (每个 IO 都完整走 4 阶段)\n", single);
    printf("4核流水线: %lld us (满流水后每个阶段时间出一个 IO)\n", pipe);
    printf("加速比   : %.1f 倍\n", (double)single / pipe);

    printf("\n关键点:\n");
    printf("  1. 流水线把吞吐从 1/总耗时 提升到 1/最慢阶段耗时\n");
    printf("  2. 阶段越均衡, 加速比越接近核数(4 阶段均衡 -> ~4 倍)\n");
    printf("  3. 真实 SSD: 主控多核 + 多通道 NAND 并行,\n");
    printf("     NAND 是最慢阶段, 所以要多通道并行来消除瓶颈\n");
    return 0;
}
