#include <stdio.h>

/*
 * QoS 加权轮询(Weighted Round Robin, WRR)与带宽分配。
 *
 * 场景: 多个租户/命名空间共享一块 SSD, 要按权重分配带宽
 *       (如 4:2:1), 不能让某个贪婪的租户把带宽全占了。
 *
 * WRR: 每轮按权重服务各队列 —— A 服务 4 个, B 服务 2 个, C 服务 1 个。
 * 长期看各队列获得的带宽比例 = 权重比。
 *
 * 本实验: 3 个队列, 固定运行 50 轮 WRR, 统计各队列分到的服务次数。
 * 观察队列 A(权重 4) 获得的服务次数约为队列 C(权重 1) 的 4 倍。
 */

#define QUEUES 3
#define RUNS 50

static const int weight[QUEUES] = { 4, 2, 1 };   /* A:B:C = 4:2:1 */
static int served[QUEUES];

static void wrr_one_round(void)
{
    for (int q = 0; q < QUEUES; q++)
        served[q] += weight[q];
}

int main(void)
{
    for (int r = 0; r < RUNS; r++)
        wrr_one_round();

    int total = served[0] + served[1] + served[2];

    printf("WRR 固定运行 %d 轮, 权重 A:B:C = %d:%d:%d\n\n", RUNS,
           weight[0], weight[1], weight[2]);

    printf("== 服务次数 ==\n");
    for (int q = 0; q < QUEUES; q++)
        printf("  队列 %c : %d 次 (占 %.0f%%)\n",
               'A' + q, served[q], 100.0 * served[q] / total);

    printf("\n== 理论权重比 ==\n");
    int total_w = weight[0] + weight[1] + weight[2];
    for (int q = 0; q < QUEUES; q++)
        printf("  队列 %c : 权重 %.0f%%\n",
               'A' + q, 100.0 * weight[q] / total_w);

    printf("\n要点: A 服务次数是 C 的 %.0f 倍(正好等于权重比 4:1);\n",
           (double)served[0] / served[2]);
    printf("      真实固件还会加“令牌桶”限速, 防止瞬时突发超限\n");
    return 0;
}