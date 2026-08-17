#include <stdio.h>
#include <string.h>

/*
 * NVMe 提交队列(SQ)/完成队列(CQ)完整流转 + doorbell 模拟。
 *
 * 真实 NVMe 交互(本实验模拟):
 *   1. 主机写命令到 SQ(内存环形缓冲) -> 更新 SQ Tail doorbell
 *   2. 控制器读命令执行 -> 写完成项到 CQ -> 更新 CQ Head doorbell
 *   3. 主机读 CQ 完成项 -> 更新 CQ Tail doorbell 归还槽位
 *
 * doorbell 用"内存映射寄存器地址"模拟: 读/写该地址即表示通知对方。
 * 关键概念: SQ 满/空用 head/tail 判断, CQ 也是。
 */

#define QD 4          /* 队列深度(2 的幂) */
#define MASK (QD - 1)

typedef struct {
    unsigned char slot[QD][64];   /* 槽位: 简化 64 字节 */
    unsigned int head;            /* 消费者读位置 */
    unsigned int tail;            /* 生产者写位置 */
} queue_t;

typedef struct {
    unsigned char opc;    /* 1=READ 2=WRITE */
    unsigned int  cid;    /* 命令 ID */
    unsigned int  nsid;
    unsigned long long lba;
    unsigned int  count;
} nvme_cmd_t;

typedef struct {
    unsigned int  cid;    /* 命令 ID */
    unsigned short status;/* 0=成功 */
} nvme_cqe_t;

/* doorbell 模拟: 用数组地址代表寄存器 */
static unsigned int doorbell[2];   /* [0]=SQ tail, [1]=CQ head */

static int q_empty(const queue_t *q) { return q->head == q->tail; }

static void sq_push(queue_t *sq, const nvme_cmd_t *c)
{
    memcpy(sq->slot[sq->tail], c, sizeof(*c));
    sq->tail = (sq->tail + 1) & MASK;
    doorbell[0] = sq->tail;   /* 写 SQ Tail doorbell 通知控制器 */
}

static void sq_pop(queue_t *sq, nvme_cmd_t *c)
{
    memcpy(c, sq->slot[sq->head], sizeof(*c));
    sq->head = (sq->head + 1) & MASK;
}

static void cq_push(queue_t *cq, const nvme_cqe_t *e)
{
    memcpy(cq->slot[cq->tail], e, sizeof(*e));
    cq->tail = (cq->tail + 1) & MASK;
    doorbell[1] = cq->tail;   /* 更新 CQ Head doorbell */
}

static void cq_pop(queue_t *cq, nvme_cqe_t *e)
{
    memcpy(e, cq->slot[cq->head], sizeof(*e));
    cq->head = (cq->head + 1) & MASK;
}

int main(void)
{
    queue_t sq, cq;
    nvme_cmd_t cmd;
    nvme_cqe_t cqe;

    memset(&sq, 0, sizeof(sq));
    memset(&cq, 0, sizeof(cq));

    printf("== 主机提交 3 条命令到 SQ ==\n");
    for (int i = 0; i < 3; i++) {
        nvme_cmd_t c = { 1, (unsigned int)i, 1, (unsigned long long)(0x1000 + i * 0x100), 8 };
        sq_push(&sq, &c);
        printf("  提交 cmd%d: READ nsid=1 lba=0x%llx +8 块\n",
               i, (unsigned long long)c.lba);
    }
    printf("  SQ tail doorbell = %u (已通知控制器)\n", doorbell[0]);

    printf("\n== 控制器逐个执行并产生完成项 ==\n");
    int n = 0;
    while (!q_empty(&sq)) {
        sq_pop(&sq, &cmd);
        nvme_cqe_t e = { cmd.cid, 0 };
        cq_push(&cq, &e);
        printf("  执行 cmd%d 完成, 写入 CQ (cid=%u)\n", e.cid, e.cid);
        n++;
    }
    printf("  控制器更新 CQ head doorbell = %u\n", doorbell[1]);

    printf("\n== 主机读取完成项 ==\n");
    while (!q_empty(&cq)) {
        cq_pop(&cq, &cqe);
        printf("  主机读到完成: cid=%u status=%u\n", cqe.cid, cqe.status);
    }
    printf("  主机归还 CQ 槽位 (更新 CQ tail), 共处理 %d 条\n", n);

    printf("\n要点: 队列在内存, 交互全靠 doorbell 寄存器,\n");
    printf("      这就是 NVMe 相比 AHCI 低延迟高并发的关键\n");
    return 0;
}
