#include <stdio.h>
#include <string.h>

/*
 * 命令分派与多核负载均衡（Command Steering）。
 *
 * NVMe 多队列的好处之一: 每个核绑定一条 SQ/CQ(见 12 周),
 * 命令天然按队列分派到对应核, 各核处理自己的队列 -> 无需加锁。
 *
 * 本实验对比两种架构:
 *   方案A: 单队列 + 多核争抢(需要锁, 有竞争开销, 核间共享)
 *   方案B: 多队列, 命令按 hash 分派到核, 每核独立队列(无锁)
 * 统计各核处理量分布与总耗时。
 */

#define CORES 4
#define CMD_NUM 1000

/* 每命令处理开销(us) + 加锁竞争开销 */
#define PROC_US 5
#define LOCK_US 4

static void run_shared_queue(void)
{
    long long cost = 0;
    int per_core[CORES] = {0};

    printf("== 方案A: 单队列, 4 核争抢(需锁) ==\n");
    /* 命令进同一队列, 各核取命令时要拿锁 */
    for (int i = 0; i < CMD_NUM; i++) {
        int core = i % CORES;      /* 简化: 各核轮流取 */
        cost += PROC_US + LOCK_US; /* 每次取命令都有锁竞争 */
        per_core[core]++;
    }
    printf("  总耗时 %lld us (每个命令都带锁竞争开销)\n", cost);
    printf("  各核处理: ");
    for (int c = 0; c < CORES; c++)
        printf("核%d=%d ", c, per_core[c]);
    printf("\n\n");
}

static void run_per_core_queues(void)
{
    long long cost = 0;
    int per_core[CORES] = {0};

    printf("== 方案B: 每核独立队列, 命令按 hash 分派(无锁) ==\n");
    for (int i = 0; i < CMD_NUM; i++) {
        /* 按命令哈希(如 LBA)分派到核, 核内处理无需锁 */
        int core = i % CORES;
        cost += PROC_US;
        per_core[core]++;
    }
    printf("  总耗时 %lld us (无锁竞争, 核间完全并行)\n", cost);
    printf("  各核处理: ");
    for (int c = 0; c < CORES; c++)
        printf("核%d=%d ", c, per_core[c]);
    printf("\n\n");
}

int main(void)
{
    run_shared_queue();
    run_per_core_queues();

    printf("要点: 每核独立队列(方案B)消除了锁竞争,\n");
    printf("      总耗时从 %d 降到 %d us —— 这就是 NVMe 多队列架构的意义\n",
           CMD_NUM * (PROC_US + LOCK_US), CMD_NUM * PROC_US);
    return 0;
}
