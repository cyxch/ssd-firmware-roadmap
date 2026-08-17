#include <stdio.h>
#include <string.h>

/*
 * 掉电保护(Power Loss Protection, PLP)。
 *
 * 原理: 主板上放电容/超级电容, 检测到掉电时电容还有电,
 * 给主控"最后一口气"(几十 ms ~ 几百 ms), 让固件把关键数据
 * (DRAM 里的 dirty 数据 / 映射表) 紧急刷到 NAND。
 *
 * 本实验模拟 PLP 的"紧急刷盘"流程:
 *   掉电 -> 电容供电 -> 固件在有限预算内抢救 dirty 数据
 *
 * 关键设计原则: 必须保证"已确认"(已向主机承诺)的数据不丢,
 * 未确认的数据丢了可接受(主机本就没得到成功返回)。
 */

#define DIRTY_NUM 8

typedef struct {
    int lba;
    int confirmed;      /* 1=已向主机确认 */
} dirty_t;

/* 模拟: 电容供电预算 budget 条, 优先保证已确认数据 */
static void plp_save(dirty_t *dirty, int n, int budget)
{
    int saved = 0;
    int saved_flag[DIRTY_NUM];

    memset(saved_flag, 0, sizeof(saved_flag));

    /* 第一遍: 先抢救"已确认"的数据(承诺必须兑现) */
    for (int i = 0; i < n && saved < budget; i++) {
        if (dirty[i].confirmed) {
            printf("  [保存] 已确认 LBA %d 刷入 NAND\n", dirty[i].lba);
            saved_flag[i] = 1;
            saved++;
        }
    }
    /* 第二遍: 有富余电量再刷未确认数据 */
    for (int i = 0; i < n && saved < budget; i++) {
        if (!dirty[i].confirmed) {
            printf("  [保存] 未确认 LBA %d 顺带刷入\n", dirty[i].lba);
            saved_flag[i] = 1;
            saved++;
        }
    }

    /* 统计: 哪些没保住(已确认的绝不能丢) */
    int lost_confirmed = 0, lost_unconfirmed = 0;
    for (int i = 0; i < n; i++) {
        if (!saved_flag[i]) {
            if (dirty[i].confirmed)
                lost_confirmed++;
            else
                lost_unconfirmed++;
            printf("  [丢弃] LBA %d (%s)\n",
                   dirty[i].lba, dirty[i].confirmed ? "已确认!!" : "未确认");
        }
    }

    printf("  保存 %d 条; 丢失: 已确认 %d 条(不允许!), 未确认 %d 条(可接受)\n",
           saved, lost_confirmed, lost_unconfirmed);
}

int main(void)
{
    dirty_t dirty[DIRTY_NUM];
    for (int i = 0; i < DIRTY_NUM; i++) {
        dirty[i].lba = i;
        dirty[i].confirmed = (i % 3 == 0);   /* 每 3 条有 1 条已确认 */
    }

    printf("DRAM 缓存 %d 条 dirty 数据:\n", DIRTY_NUM);
    for (int i = 0; i < DIRTY_NUM; i++)
        printf("  LBA %d -> %s\n", i, dirty[i].confirmed ? "已确认" : "未确认");

    printf("\n== 方案A: 无 PLP ==\n");
    printf("  掉电即全丢(含已确认) -> 数据损坏, 不可接受\n\n");

    printf("== 方案B: 有 PLP(电容供电, 预算 5 条) ==\n");
    plp_save(dirty, DIRTY_NUM, 5);

    printf("\n要点: PLP 用'已确认'语义保证不丢已承诺数据;\n");
    printf("      电容容量决定预算, 预算必须覆盖最大 dirty 量\n");
    return 0;
}
