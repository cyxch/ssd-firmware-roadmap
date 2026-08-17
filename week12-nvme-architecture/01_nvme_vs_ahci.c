#include <stdio.h>

/*
 * NVMe vs AHCI —— 为什么 NVMe 快这么多?
 *
 * AHCI(SATA 时代的协议, 用于 SATA SSD):
 *   - 单命令队列: 最多 32 个 in-flight 命令, 深度浅
 *   - 每次命令要读/写寄存器, 有锁步开销
 *   - 命令搬移需要 CPU 参与更多
 *
 * NVMe(PCIe 时代协议):
 *   - 多队列: 一个核一条队列, 最多 64K 队列 x 64K 深度
 *   - 队列在主机内存(环形缓冲), 提交用"写寄存器门铃"即可
 *   - 命令直达, CPU 开销低, 中断可聚合(一个中断处理多命令)
 *
 * 本实验: 量化对比两种协议在"高并发"场景下的吞吐能力。
 */

/* 参数 */
#define CPU_CORES 4
#define AHCI_MAX_QD 32        /* AHCI 单队列最大深度 */
#define NVME_Q_PER_CORE 1     /* 每个核一条队列 */
#define NVME_Q_DEPTH 1024     /* 每条队列深度 */

#define AHCI_CMD_OVERHEAD_US 40   /* 每命令: 寄存器读写的 CPU/锁步开销 */
#define NVME_CMD_OVERHEAD_US 5    /* 每命令: 门铃 + 内存队列开销 */

/* 主机每秒发命令数(纯吞吐上限, 简化: 忽略介质耗时) */
static long long ahci_capacity(void)
{
    return 1000000LL / AHCI_CMD_OVERHEAD_US;   /* 1s / 40us */
}

static long long nvme_capacity(void)
{
    long long per_queue = 1000000LL / NVME_CMD_OVERHEAD_US;
    return per_queue * CPU_CORES * NVME_Q_PER_CORE;
}

int main(void)
{
    printf("== 队列机制对比 ==\n");
    printf("AHCI : 1 个队列, 深度最大 %d, 每命令寄存器开销 ~%dus\n",
           AHCI_MAX_QD, AHCI_CMD_OVERHEAD_US);
    printf("NVMe : %d 核 x %d 队列 x 深度 %d, 每命令开销 ~%dus\n\n",
           CPU_CORES, NVME_Q_PER_CORE, NVME_Q_DEPTH, NVME_CMD_OVERHEAD_US);

    long long a = ahci_capacity();
    long long n = nvme_capacity();
    printf("单核理论吞吐:\n");
    printf("  AHCI : %lld K IOPS\n", a / 1000);
    printf("  NVMe : %lld K IOPS\n", n / 1000);
    printf("  NVMe 约为 AHCI 的 %.0f 倍\n\n", (double)n / a);

    printf("并发能力:\n");
    printf("  AHCI : 单队列, 多核也要争抢一个队列, 深度只有 %d\n", AHCI_MAX_QD);
    printf("  NVMe : 每核独立队列, 互不争抢, 深度可到 %d\n", NVME_Q_DEPTH);
    printf("  多核场景 NVMe 优势更明显(队列与核 1:1 绑定)\n");

    printf("\n要点: 队列深度够, 才能把“随机读写小 IO”并发压满;\n");
    printf("      SSD 主控固件与 NVMe 队列交互是性能核心\n");
    return 0;
}
