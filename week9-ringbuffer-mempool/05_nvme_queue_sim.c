#include <stdio.h>
#include <string.h>

/*
 * NVMe 队列模型模拟：提交队列(SQ) + 完成队列(CQ)。
 * 真实 SSD 里，这两个队列就是位于主机内存的环形缓冲区：
 *
 *   SQ(Submission Queue)：主机是"生产者"，写命令；控制器是"消费者"，读命令。
 *                          主机写完更新 SQ Tail doorbell(门铃寄存器)通知控制器。
 *   CQ(Completion Queue) ：控制器是"生产者"，写完成项；主机是"消费者"，读完成项。
 *                          主机读完更新 CQ Head doorbell 释放槽位。
 *
 * 队列深度必须是 2 的幂，索引回绕用掩码(见 02)。
 * 本程序用通用环形缓冲把"提交 -> 执行 -> 完成"整条链路串起来，观察双方协作。
 */

#define QD 4                 /* 队列深度(2 的幂) */
#define Q_MASK (QD - 1)
#define SLOT_BYTES 64        /* 每个槽位最大负载字节 */

/* 一个简化 NVMe 命令(真实为 64 字节的 nvme_command) */
typedef struct {
    unsigned char opc;              /* 1=READ 2=WRITE */
    unsigned int  nsid;             /* 命名空间 ID */
    unsigned long long lba;         /* 起始逻辑块地址 */
    unsigned int  count;            /* 传输块数 */
} nvme_cmd_t;

/* 完成项(真实为 16 字节的 nvme_completion) */
typedef struct {
    nvme_cmd_t cmd;                 /* 携带被完成的命令 */
    unsigned short status;          /* 0 = 成功 */
} nvme_cqe_t;

/* 通用环形缓冲(按 NVMe 用 2 的幂深度 + 掩码回绕) */
typedef struct {
    unsigned char slot[QD][SLOT_BYTES];
    volatile unsigned int head;
    volatile unsigned int tail;
    unsigned int item_size;
} queue_t;

static void q_init(queue_t *q, unsigned int item_size)
{
    memset(q, 0, sizeof(*q));
    q->item_size = item_size;
}

static int q_full(const queue_t *q)
{
    return ((q->tail + 1) & Q_MASK) == q->head;
}

static int q_empty(const queue_t *q) { return q->head == q->tail; }

static int q_push(queue_t *q, const void *item)
{
    if (q_full(q))
        return 0;
    memcpy(q->slot[q->tail], item, q->item_size);
    q->tail = (q->tail + 1) & Q_MASK;
    return 1;
}

static int q_pop(queue_t *q, void *out)
{
    if (q_empty(q))
        return 0;
    memcpy(out, q->slot[q->head], q->item_size);
    q->head = (q->head + 1) & Q_MASK;
    return 1;
}

static const char *op_name(unsigned char opc)
{
    return opc == 1 ? "READ" : "WRITE";
}

int main(void)
{
    queue_t sq, cq;
    nvme_cmd_t cmds[4] = {
        {1, 1, 0x1000, 8},
        {1, 1, 0x2000, 16},
        {2, 1, 0x3000, 4},
        {1, 2, 0x4000, 32},
    };
    nvme_cmd_t cmd;
    nvme_cqe_t cqe;
    int done = 0;

    q_init(&sq, sizeof(nvme_cmd_t));
    q_init(&cq, sizeof(nvme_cqe_t));

    printf("== 1) 主机提交 4 条命令到 SQ(生产者=主机) ==\n");
    for (int i = 0; i < 4; i++) {
        printf("  提交[%d] opc=%s nsid=%u lba=0x%llx +%u 块 -> %s\n",
               i, op_name(cmds[i].opc), cmds[i].nsid,
               cmds[i].lba, cmds[i].count,
               q_push(&sq, &cmds[i]) ? "入队" : "SQ 已满");
    }
    printf("  主机写 SQ Tail doorbell(门铃寄存器), 通知控制器处理\n");

    printf("\n== 2) 控制器取命令执行，并把完成项写入 CQ(生产者=控制器) ==\n");
    while (q_pop(&sq, &cmd)) {
        cqe.cmd = cmd;
        cqe.status = 0;                     /* SUCCESS */
        printf("  执行 %s nsid=%u lba=0x%llx +%u 块 -> 完成, 推入 CQ\n",
               op_name(cmd.opc), cmd.nsid, cmd.lba, cmd.count);
        q_push(&cq, &cqe);
    }
    printf("  控制器更新 SQ Head doorbell, 表示 SQ 已有空槽\n");

    printf("\n== 3) 主机读取 CQ 完成项(消费者=主机)，读完更新 CQ Head ==\n");
    while (q_pop(&cq, &cqe))
        done++;
    printf("  主机共取出 %d 个完成项, 更新 CQ Head doorbell 释放槽位\n", done);

    printf("\n注意: 队列深度 QD=%d(留一格) 实际只能容纳 %d 条在途命令，\n", QD, QD - 1);
    printf("      第 4 条因 SQ 满被拒，需等控制器处理完、SQ 有空槽后主机再提交\n");
    printf("      —— 这正是队列深度限制在途命令数的真实含义\n");
    return 0;
}
