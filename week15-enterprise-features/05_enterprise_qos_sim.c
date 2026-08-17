#include <stdio.h>

/*
 * 企业级综合 QoS 调度。
 *
 * 把本周知识点串起来: 多命名空间(02) x 优先级(14-02) x WRR(14-03) x 配额(03)
 *
 * 场景: 一块企业盘服务 4 个租户(NS), 各有权重(带宽份额)与
 *       延迟敏感度(高优先级先服务)。
 * 调度: 每轮 WRR 按权重分配服务次数; 延迟敏感 NS 拥有高权重(天然优先)。
 * 统计: 各 NS 分到的带宽占比, 是否达到其需求。
 *
 * 本实验是"企业级 QoS 控制面"的简化模型。
 */

#define NS_NUM 4
#define ROUNDS 100
#define IO_PER_NS 200     /* 每个 NS 待处理的 IO 数 */

static const int weight[NS_NUM] = { 4, 2, 2, 1 };  /* 带宽权重 */
static int served[NS_NUM];

int main(void)
{
    /* WRR: 每轮按权重服务各 NS */
    for (int r = 0; r < ROUNDS; r++)
        for (int i = 0; i < NS_NUM; i++)
            served[i] += weight[i];

    int total = 0;
    for (int i = 0; i < NS_NUM; i++)
        total += served[i];

    printf("企业盘 4 个租户, 权重 %d:%d:%d:%d, 跑 %d 轮 WRR\n\n",
           weight[0], weight[1], weight[2], weight[3], ROUNDS);

    printf("== 各租户带宽分配 ==\n");
    int total_w = 0;
    for (int i = 0; i < NS_NUM; i++)
        total_w += weight[i];

    for (int i = 0; i < NS_NUM; i++) {
        int ok = served[i] >= IO_PER_NS;   /* 是否满足其需求 */
        printf("  租户%d(权重%d): 服务 %d 次, 占 %.0f%%  需求 %d -> %s\n",
               i + 1, weight[i], served[i],
               100.0 * served[i] / total, IO_PER_NS,
               ok ? "满足" : "未满(还能吸收更多)");
    }

    printf("\n== 与权重的一致性 ==\n");
    printf("  实际占比   : %d%% / %d%% / %d%% / %d%%\n",
           (int)(100.0 * served[0] / total), (int)(100.0 * served[1] / total),
           (int)(100.0 * served[2] / total), (int)(100.0 * served[3] / total));
    printf("  理论权重占比: %d%% / %d%% / %d%% / %d%%\n",
           (int)(100.0 * weight[0] / total_w), (int)(100.0 * weight[1] / total_w),
           (int)(100.0 * weight[2] / total_w), (int)(100.0 * weight[3] / total_w));

    printf("\n要点: 企业级 QoS = 多命名空间隔离 + 权重带宽 + 延迟优先;\n");
    printf("      固件通过 WRR/令牌桶/优先级把这些承诺兑现给每个租户\n");
    return 0;
}
