#include <stdio.h>
#include <string.h>

/*
 * 2 的幂掩码环形缓冲：NVMe 提交队列/完成队列的标准做法。
 * 队列深度必须为 2^n，索引回绕用 (idx+1) & (size-1) —— 只一条与指令，比取模快。
 *
 * 满/空判断（本文件用"留一格"法，经典做法，无需额外 count 变量）：
 *   head == tail                        -> 空
 *   ((tail + 1) & MASK) == head         -> 满
 * 代价：容量为 size 的队列实际只能存 size-1 个元素。
 *
 * volatile 的作用：单生产者单消费者(SPSC)下，生产者只写 tail、
 * 消费者只写 head，各自读对方，不需要锁；volatile 防止编译器把
 * 对方可能修改的索引缓存到寄存器。真实 MCU 上是硬件写 doorbell 寄存器。
 */

#define Q_SIZE 16           /* 必须是 2 的幂 */
#define Q_MASK (Q_SIZE - 1)

typedef struct {
    unsigned char slot[Q_SIZE];
    volatile unsigned int head;   /* 消费者读位置 */
    volatile unsigned int tail;   /* 生产者写位置 */
} spsc_q_t;

static void q_init(spsc_q_t *q)
{
    memset(q, 0, sizeof(*q));
}

static int q_empty(const spsc_q_t *q) { return q->head == q->tail; }

static int q_full(const spsc_q_t *q)
{
    return ((q->tail + 1) & Q_MASK) == q->head;
}

static int q_push(spsc_q_t *q, unsigned char c)
{
    if (q_full(q))
        return 0;
    q->slot[q->tail] = c;
    q->tail = (q->tail + 1) & Q_MASK;   /* 与掩码回绕，不用取模 */
    return 1;
}

static int q_pop(spsc_q_t *q, unsigned char *out)
{
    if (q_empty(q))
        return 0;
    *out = q->slot[q->head];
    q->head = (q->head + 1) & Q_MASK;
    return 1;
}

int main(void)
{
    spsc_q_t q;
    unsigned char c;
    int pushed = 0;

    q_init(&q);

    printf("队列深度=%d(2 的幂)，留一格判满 -> 实际可存 %d 个元素\n\n",
           Q_SIZE, Q_SIZE - 1);

    printf("== 生产者连续压入 15 个字节(0..14) ==\n");
    for (int i = 0; i < Q_SIZE; i++)
        if (q_push(&q, (unsigned char)i))
            pushed++;
    printf("成功压入 %d 个\n", pushed);

    printf("队列满后再压一个 -> %s\n\n", q_push(&q, 255) ? "成功" : "被拒绝(满)");

    printf("== 消费者取出前 10 个 ==\n");
    for (int i = 0; i < 10; i++) {
        if (q_pop(&q, &c))
            printf("  pop -> %d\n", c);
    }

    printf("\n索引回绕原理: 自增后直接 & %d 即可回到 0 开始(无需 if/取模)\n", Q_MASK);
    printf("当前 head=%u tail=%u(两者之差即队列中元素数)\n",
           (unsigned)q.head, (unsigned)q.tail);

    return 0;
}
