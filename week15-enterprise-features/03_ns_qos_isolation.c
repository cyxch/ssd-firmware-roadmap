#include <stdio.h>

/*
 * 命名空间级 QoS 隔离。
 *
 * 企业场景: 多个租户(各占一个 NS)共享一块 SSD。
 * 问题: 一个租户的"写风暴"(极端突发)会占满 NAND 带宽,
 *       把其他租户饿死 —— 这就是"无隔离"。
 * 方案: 每个 NS 设带宽配额(每时隙最多服务 X 个), 超限排队。
 *
 * 本实验: 4 个 NS, NS0 疯狂写, 其他 3 个正常(各 4/时隙),
 * 总带宽 40/时隙, 跑 1000 时隙, 对比有无隔离的吞吐。
 */

#define NS_NUM 4
#define SLOTS 1000
#define CAP_PER_SLOT 40        /* 总带宽 */
#define FAIR_SHARE (CAP_PER_SLOT / NS_NUM)   /* 公平配额 10 */
#define GREEDY_DEMAND 40       /* NS0 每时隙需求 */
#define NORMAL_DEMAND 4        /* 其他 NS 每时隙需求 */

static long long served_none[NS_NUM];   /* 无隔离 */
static long long served_qos[NS_NUM];    /* 有隔离 */

int main(void)
{
    /* 无隔离: 谁需求大谁先拿(贪婪者占满) */
    for (int s = 0; s < SLOTS; s++) {
        int left = CAP_PER_SLOT;
        /* NS0 先吃 */
        int take0 = GREEDY_DEMAND < left ? GREEDY_DEMAND : left;
        served_none[0] += take0;
        left -= take0;
        /* 剩下的给其他 */
        for (int i = 1; i < NS_NUM; i++) {
            int t = NORMAL_DEMAND < left ? NORMAL_DEMAND : left;
            served_none[i] += t;
            left -= t;
        }
    }

    /* 有隔离: 每 NS 最多拿公平配额, 超出的排队(这里简化为丢弃超额) */
    for (int s = 0; s < SLOTS; s++) {
        for (int i = 0; i < NS_NUM; i++) {
            int demand = (i == 0) ? GREEDY_DEMAND : NORMAL_DEMAND;
            int t = demand < FAIR_SHARE ? demand : FAIR_SHARE;
            served_qos[i] += t;
        }
    }

    printf("== 无 QoS 隔离: NS0 写风暴占满带宽 ==\n");
    for (int i = 0; i < NS_NUM; i++)
        printf("  NS%d : %5lld 次 (占 %.0f%%)%s\n",
               i, served_none[i],
               100.0 * served_none[i] / (SLOTS * CAP_PER_SLOT),
               (i != 0 && served_none[i] == 0) ? " <- 被饿死!" : "");

    printf("\n== 有 QoS 隔离: 每 NS 最多用 %d/时隙 ==\n", FAIR_SHARE);
    for (int i = 0; i < NS_NUM; i++)
        printf("  NS%d : %5lld 次 (占 %.0f%%)%s\n",
               i, served_qos[i],
               100.0 * served_qos[i] / (SLOTS * CAP_PER_SLOT),
               (i != 0) ? " <- 不受影响" : " <- 被限流");

    printf("\n结论: 无隔离时 3 个租户 0 吞吐(被饿死);\n");
    printf("      有隔离后每个租户获得保底带宽, 贪婪者被限制在配额内\n");
    return 0;
}
