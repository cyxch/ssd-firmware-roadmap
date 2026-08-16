#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * 环形缓冲区（Ring Buffer）—— 本周交付物 1
 * 生产-消费模型：写指针(写满/写位置)与读指针，绕过数组尾时回绕。
 * 常用于：串口收发、DMA 与主循环之间传递数据（嵌入式高频使用）。
 *
 * 设计：读/写指针均"前进后取模"。用 size 计数避免"满/空"歧义。
 */

#define RB_SIZE 8u

typedef struct {
    uint8_t  buf[RB_SIZE];
    uint32_t head;          /* 写位置 */
    uint32_t tail;          /* 读位置 */
    uint32_t count;         /* 当前数据个数 */
} ring_buffer_t;

void rb_init(ring_buffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

bool rb_is_empty(const ring_buffer_t *rb) { return rb->count == 0; }
bool rb_is_full(const ring_buffer_t *rb)  { return rb->count == RB_SIZE; }

/* 写入一个字节；满则失败 */
bool rb_put(ring_buffer_t *rb, uint8_t byte)
{
    if (rb_is_full(rb))
        return false;

    rb->buf[rb->head] = byte;
    rb->head = (rb->head + 1) % RB_SIZE;
    rb->count++;
    return true;
}

/* 读出一个字节；空则失败 */
bool rb_get(ring_buffer_t *rb, uint8_t *out)
{
    if (rb_is_empty(rb))
        return false;

    *out = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % RB_SIZE;
    rb->count--;
    return true;
}

uint32_t rb_available(const ring_buffer_t *rb) { return rb->count; }

int main(void)
{
    ring_buffer_t rb;
    rb_init(&rb);

    /* 写入 8 个字节（恰好填满） */
    for (int i = 0; i < (int)RB_SIZE; i++)
        printf("put %d -> %s\n", i, rb_put(&rb, (uint8_t)(i + 1)) ? "ok" : "full");

    printf("满后再 put  -> %s\n", rb_put(&rb, 0xFF) ? "ok" : "full (拒绝)");
    printf("当前数据个数 = %u\n", rb_available(&rb));

    /* 读出 3 个 */
    uint8_t v;
    for (int i = 0; i < 3; i++) {
        if (rb_get(&rb, &v))
            printf("get -> %u\n", v);
    }

    /* 再写 2 个，验证回绕 */
    rb_put(&rb, 100);
    rb_put(&rb, 101);

    printf("清空剩余:");
    while (rb_get(&rb, &v))
        printf(" %u", v);
    printf("\n");

    printf("空后 get -> %s\n", rb_get(&rb, &v) ? "ok" : "empty (拒绝)");

    return 0;
}
