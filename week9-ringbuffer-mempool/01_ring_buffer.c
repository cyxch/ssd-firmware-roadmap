#include <stdio.h>
#include <string.h>

/*
 * 环形缓冲区(ring buffer / circular buffer)：嵌入式最常用的数据结构之一。
 * 为什么不用普通数组？
 *   - 普通数组"取队首"需要把后续元素全部前移，O(n) 搬移浪费 CPU
 *   - 环形缓冲用 head/tail 两个游标，写入/读取都是 O(1)，天然 FIFO
 * 应用场景：串口接收队列、任务消息队列、DMA 接收缓冲、NVMe 的 SQ/CQ。
 *
 * 满/空判断（本文件用 count 计数法）：
 *   count == 0         -> 空
 *   count == BUF_SIZE  -> 满
 */

#define BUF_SIZE 8

typedef struct {
    unsigned char data[BUF_SIZE];
    int head;    /* 下一个可读位置 */
    int tail;    /* 下一个可写位置 */
    int count;   /* 当前已存字节数 */
} ringbuf_t;

static void rb_init(ringbuf_t *rb)
{
    memset(rb, 0, sizeof(*rb));
}

static int rb_is_empty(const ringbuf_t *rb) { return rb->count == 0; }
static int rb_is_full(const ringbuf_t *rb)  { return rb->count == BUF_SIZE; }

static int rb_write(ringbuf_t *rb, unsigned char c)
{
    if (rb_is_full(rb))
        return 0;                       /* 满：拒绝写入 */
    rb->data[rb->tail] = c;
    rb->tail = (rb->tail + 1) % BUF_SIZE;  /* 取模回绕 */
    rb->count++;
    return 1;
}

static int rb_read(ringbuf_t *rb, unsigned char *out)
{
    if (rb_is_empty(rb))
        return 0;                       /* 空：无数据可读 */
    *out = rb->data[rb->head];
    rb->head = (rb->head + 1) % BUF_SIZE;
    rb->count--;
    return 1;
}

static void rb_dump(const ringbuf_t *rb, const char *tag)
{
    printf("%s: head=%d tail=%d count=%d => 槽位[", tag,
           rb->head, rb->tail, rb->count);
    for (int i = 0; i < BUF_SIZE; i++)
        printf("%d ", rb->data[i]);
    printf("]\n");
}

int main(void)
{
    ringbuf_t rb;
    unsigned char c;

    rb_init(&rb);

    printf("步骤1：写入 8 个字节(0..7)，填满缓冲区\n");
    for (int i = 0; i < BUF_SIZE; i++)
        printf("  写 %d -> %s\n", i, rb_write(&rb, (unsigned char)i) ? "成功" : "失败");
    rb_dump(&rb, "填满后");

    printf("\n步骤2：缓冲区已满时再写 99 -> %s\n",
           rb_write(&rb, 99) ? "成功" : "被拒绝(满)");

    printf("\n步骤3：读出前 3 个字节(严格先入先出)\n");
    for (int i = 0; i < 3; i++) {
        if (rb_read(&rb, &c))
            printf("  读 -> %d\n", c);
    }
    rb_dump(&rb, "读3个后");

    printf("\n步骤4：再写 8,9,10,11，观察 tail 回绕绕过头(head)\n");
    rb_write(&rb, 8);
    rb_write(&rb, 9);
    rb_write(&rb, 10);
    rb_write(&rb, 11);
    rb_dump(&rb, "回绕后");

    printf("\n步骤5：把剩余数据全部读出，缓冲区回到空\n");
    while (!rb_is_empty(&rb)) {
        rb_read(&rb, &c);
        printf("  读 -> %d\n", c);
    }
    rb_dump(&rb, "读空后");

    printf("\n要点: head/tail 都是 0..7 循环，只要 count 正确，读写互不覆盖\n");
    return 0;
}
