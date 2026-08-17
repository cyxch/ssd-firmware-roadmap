#include <stdio.h>
#include <string.h>

/*
 * NVMe 完成项(Completion Queue Entry, CQE)与中断聚合。
 *
 * 真实 CQE: 16 字节, 关键字段:
 *   - DW0 : Command Specific(如完成字节数)
 *   - DW1 : SQ Head Pointer(该 SQ 已被消费到哪, 通知主机可再提交)
 *   - DW2 : SQ Identifier(哪个 SQ) | Status Field(状态)
 *   - DW3 : Command ID
 *
 * 中断聚合(Interrupt Coalescing / Aggregation):
 *   每次完成一个命令就中断一次 -> 中断风暴, CPU 被打满。
 *   所以 NVMe 支持"聚合": 攒够 N 个完成项, 或等 T 微秒, 才发一次中断。
 *   本实验统计: 聚合前后主机被中断的次数差异。
 */

#define QD 8
#define MASK (QD - 1)
#define TOTAL_CMDS 1000

/* 简化 CQE */
typedef struct {
    unsigned int  sq_head;   /* SQ 头指针 */
    unsigned short sqid;     /* SQ 编号 */
    unsigned short status;   /* 状态: 0=成功 */
    unsigned int  cid;       /* 命令 ID */
} cqe_t;

static cqe_t cq[QD];
static unsigned int cq_head, cq_tail;

static int q_empty(void) { return cq_head == cq_tail; }

/* 产生一个完成项(控制器写入 CQ) */
static void produce_completion(unsigned int cid, unsigned short sqid)
{
    cqe_t e;
    e.sq_head = 0;
    e.sqid = sqid;
    e.status = 0;
    e.cid = cid;
    cq[cq_tail] = e;
    cq_tail = (cq_tail + 1) & MASK;
}

/* 主机消费完成项并清空队列 */
static int drain_cq(void)
{
    int n = 0;
    while (!q_empty()) {
        /* 读到 CQE 并处理(这里只计数) */
        cq_head = (cq_head + 1) & MASK;
        n++;
    }
    return n;
}

int main(void)
{
    /* ---- 场景1: 每完成一个命令就中断一次 ---- */
    long long interrupts = 0;
    for (int i = 0; i < TOTAL_CMDS; i++) {
        produce_completion((unsigned int)i, 1);
        drain_cq();           /* 立即中断处理 */
        interrupts++;
    }
    printf("== 中断聚合前(每命令一次中断) ==\n");
    printf("  完成 %d 条命令, 主机被中断 %lld 次\n", TOTAL_CMDS, interrupts);

    /* ---- 场景2: 聚合 -- 攒满队列(8个)才中断一次 ---- */
    long long irq2 = 0;
    memset(&cq, 0, sizeof(cq));
    cq_head = cq_tail = 0;
    int pending = 0;
    for (int i = 0; i < TOTAL_CMDS; i++) {
        produce_completion((unsigned int)i, 1);
        pending++;
        if (pending >= (QD - 1)) {   /* 攒满(留一格)才中断 */
            pending = 0;
            drain_cq();
            irq2++;
        }
    }
    if (pending > 0) {                /* 收尾: 最后不足一队的也处理 */
        pending = 0;
        drain_cq();
        irq2++;
    }

    printf("\n== 中断聚合后(攒满 %d 个才中断一次) ==\n", QD - 1);
    printf("  完成 %d 条命令, 主机被中断 %lld 次\n", TOTAL_CMDS, irq2);
    printf("  中断次数降为原来的 %.0f 分之一\n", (double)interrupts / irq2);

    printf("\n要点: 聚合显著降低 CPU 中断负载, 但牺牲一点延迟;\n");
    printf("      高性能 SSD 固件会精细调聚合阈值(延迟 vs 吞吐)\n");
    return 0;
}
