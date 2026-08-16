#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/*
 * 内存池（Memory Pool）—— 本周交付物 2
 * 原理：预分配一块大内存，切成固定大小的块，用"空闲链表"管理。
 * 优点：嵌入式/实时系统中无动态内存碎片、分配时间确定（O(1)）、无泄漏风险。
 * 缺点：块大小固定，可能浪费空间。
 */

#define POOL_BLOCKS   8u       /* 块数量 */
#define BLOCK_SIZE   32u       /* 每块字节数（不含头部） */

/* 空闲链表节点（放在每个空闲块头部，块空闲时才使用） */
typedef struct FreeNode {
    struct FreeNode *next;
} free_node_t;

/* 内存池结构（_Alignas 保证块内存按指针对齐，避免别名/对齐问题） */
typedef struct {
    _Alignas(free_node_t) uint8_t storage[POOL_BLOCKS][BLOCK_SIZE];
    free_node_t     *free_head;
    uint32_t         used_count;
} memory_pool_t;

static memory_pool_t g_pool;

void pool_init(memory_pool_t *pool)
{
    pool->free_head = NULL;
    pool->used_count = 0;

    /* 把所有块串成空闲链表 */
    for (uint32_t i = 0; i < POOL_BLOCKS; i++) {
        free_node_t *node = (free_node_t *)pool->storage[i];
        node->next = pool->free_head;
        pool->free_head = node;
    }
}

/* 从池中取一块（O(1)）；池空返回 NULL */
void *pool_alloc(memory_pool_t *pool)
{
    if (pool->free_head == NULL)
        return NULL;

    free_node_t *block = pool->free_head;
    pool->free_head = block->next;
    pool->used_count++;
    return (void *)block;
}

/* 归还一块（O(1)）；注意：必须是从本池 alloc 出来的地址 */
void pool_free(memory_pool_t *pool, void *ptr)
{
    if (ptr == NULL)
        return;

    free_node_t *node = (free_node_t *)ptr;
    node->next = pool->free_head;
    pool->free_head = node;
    pool->used_count--;
}

int main(void)
{
    pool_init(&g_pool);

    /* 申请 8 块（恰好用完） */
    void *blocks[POOL_BLOCKS] = {0};
    for (uint32_t i = 0; i < POOL_BLOCKS; i++) {
        blocks[i] = pool_alloc(&g_pool);
        printf("alloc[%u] = %p\n", i, blocks[i]);
    }

    printf("第 9 次分配 -> %s\n", pool_alloc(&g_pool) ? "ok" : "NULL (池已空)");

    /* 归还第 3 块后再取，验证复用同一地址 */
    pool_free(&g_pool, blocks[2]);
    void *re = pool_alloc(&g_pool);
    printf("复用归还块 -> %p  (与 blocks[2]=%p 相同? %s)\n",
           re, blocks[2], re == blocks[2] ? "是" : "否");

    /* 演示在池里存放数据（每块 32 字节足够放结构体） */
    if (re) {
        uint32_t *p = (uint32_t *)re;
        p[0] = 0xCAFE;
        p[1] = 0xBEEF;
        printf("池内存数据: 0x%X 0x%X\n", p[0], p[1]);
    }

    return 0;
}
