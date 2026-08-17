#include <stdio.h>
#include <string.h>

/*
 * SSD 整机架构分层模拟。
 *
 * 从主机到 NAND 的完整数据通路:
 *   主机 CPU
 *     -> 主机驱动(发起 NVMe 命令, 见 02/03)
 *     -> PCIe/NVMe 协议层(命令解析)
 *     -> 主控固件(FTL: 地址映射/GC/磨损均衡, 见 11 周)
 *     -> 闪存通道(读/写/擦, 见 10 周)
 *     -> NAND 芯片
 *
 * 本实验把整条链路串成一个"分层的 IO 流水线",
 * 每个 IO 依次经过 主机->NVMe->FTL->Flash 四层,
 * 并统计各层耗时与整体吞吐, 体会"分层"与"瓶颈"。
 */

#define IO_NUM 100
#define LAYERS 4

/* 各层基础耗时(简化量级) */
static const char *layer_names[LAYERS] = {
    "主机驱动", "NVMe协议", "FTL固件", "NAND通道"
};
static const long layer_cost[LAYERS] = { 2, 5, 30, 100 };  /* us */

/* 计算单个 IO 穿过各层的总耗时(不打印) */
static long long io_cost(void)
{
    long long total = 0;
    for (int l = 0; l < LAYERS; l++)
        total += layer_cost[l];
    return total;
}

static void print_io(int i, long long total)
{
    printf("  IO #%d: ", i);
    for (int l = 0; l < LAYERS; l++)
        printf("[%s +%ldus] ", layer_names[l], layer_cost[l]);
    printf("=> 共 %lldus\n", total);
}

int main(void)
{
    long long per_io = io_cost();
    printf("== 单个 READ IO 穿过各层(抽样显示) ==\n");
    for (int i = 0; i < IO_NUM; i++) {
        if (i < 3 || i == IO_NUM - 1)
            print_io(i, per_io);
    }
    printf("  ...(共 %d 个 IO, 每层链路相同, 耗时一致)...\n", IO_NUM);

    long long total = per_io;
    printf("\n== 统计 ==\n");
    printf("  单 IO 总耗时 %lldus, 其中 NAND 通道占 %.0f%%\n",
           total, 100.0 * layer_cost[3] / total);
    printf("  吞吐上限 %lld IOPS\n", 1000000LL / total);
    printf("  各层耗时占比: 主机驱动 2%% / NVMe 5%% / FTL 30%% / NAND 63%%\n");

    printf("\n== 关键结论 ==\n");
    printf("  1. 瓶颈在 NAND(占大头) —— 所以 SSD 需要多通道并行 NAND\n");
    printf("  2. FTL 占 30%% —— 固件优化(GC/调度)直接决定性能\n");
    printf("  3. 主机/NVMe 层开销小 —— 这就是 NVMe 低延迟的基础\n");
    return 0;
}
