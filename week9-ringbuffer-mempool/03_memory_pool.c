#include <stdio.h>
#include <string.h>
#include <stdint.h>

/*
 * ============================================================
 * 定长内存池(memory pool) —— 本周【交付物】
 * ============================================================
 * 为什么嵌入式/SSD 固件要自己管内存，而不是 malloc/free？
 *   1. 碎片化   ：malloc 频繁分配/释放小对象会产生内存碎片，
 *                 长时间运行后即使总内存足够也分配不出大块连续内存
 *   2. 不确定   ：malloc 可能触发堆扩展/系统调用，耗时不可控，破坏实时性
 *   3. 崩溃风险 ：堆损坏通常难定位
 * 内存池的方案：启动时一次性划分一块静态区域为 N 个定长块，
 * 用"空闲链表"管理；分配/释放都是 O(1) 链表操作，无碎片、时间确定。
 *
 * 关键技术：空闲块的"下一指针"就存在块自身的第一个字里(复用负载区)，
 * 不额外占用内存。这也是很多 RTOS 内存池的标准实现。
 *
 * 缺点：所有块等长(定长池)，按最大对象取块大小会浪费小对象空间；
 *       需要按需求设计多级池或变长池。
 */

#define POOL_BLOCK_NUM 8
#define POOL_BLOCK_SIZE 32

/* 块结构：data 是给用户用的负载区，必须放在首位(方便 指针<->块 互转) */
typedef struct {
    unsigned char data[POOL_BLOCK_SIZE];
} block_t;

static block_t g_pool[POOL_BLOCK_NUM];   /* 静态内存池，不用 malloc 也可用 */
static block_t *g_free_list;             /* 空闲链表头 */
static int g_used = 0;                   /* 当前已用块数 */
static int g_max_used = 0;               /* 峰值已用块数(评估池大小是否够) */

static void pool_init(void)
{
    g_free_list = NULL;
    /* 倒序插入，使首次分配的地址从低到高，方便观察 */
    for (int i = POOL_BLOCK_NUM - 1; i >= 0; i--) {
        *(block_t **)g_pool[i].data = g_free_list;  /* 把 next 存进块内 */
        g_free_list = &g_pool[i];
    }
    g_used = 0;
    g_max_used = 0;
}

static void *pool_alloc(void)
{
    if (g_free_list == NULL)
        return NULL;                    /* 池耗尽 */
    block_t *b = g_free_list;
    g_free_list = *(block_t **)b->data; /* 取下个空闲块 */
    g_used++;
    if (g_used > g_max_used)
        g_max_used = g_used;
    return b->data;
}

static void pool_free(void *p)
{
    if (p == NULL)
        return;
    /* 校验：指针是否落在本池范围内(用 uintptr_t 做整数运算，避免指针比较未定义行为) */
    uintptr_t base = (uintptr_t)g_pool;
    uintptr_t off  = (uintptr_t)p - base;
    if (off >= (uintptr_t)(POOL_BLOCK_NUM * sizeof(block_t)) ||
        off % sizeof(block_t) != 0) {
        printf("  [错误] 释放的指针 0x%p 不在本池内\n", p);
        return;
    }
    block_t *b = (block_t *)p;
    *(block_t **)b->data = g_free_list; /* 压回空闲链表 */
    g_free_list = b;
    g_used--;
}

static void pool_stats(const char *tag)
{
    printf("%s: 空闲=%d 已用=%d 峰值=%d\n", tag,
           POOL_BLOCK_NUM - g_used, g_used, g_max_used);
}

int main(void)
{
    void *a, *b, *c;

    pool_init();
    printf("内存池: %d 块 x %d 字节，地址范围 %p..%p\n\n",
           POOL_BLOCK_NUM, POOL_BLOCK_SIZE,
           (void *)g_pool, (void *)(g_pool + POOL_BLOCK_NUM));

    printf("== 分配 3 个块 ==\n");
    a = pool_alloc();
    b = pool_alloc();
    c = pool_alloc();
    if (a && b && c) {
        memcpy(a, "hello pool", 10);
        memcpy(b, "second", 6);
        memcpy(c, "third", 5);
        printf("  a=%p -> %s\n", a, (char *)a);
        printf("  b=%p -> %s\n", b, (char *)b);
        printf("  c=%p -> %s\n", c, (char *)c);
    }
    pool_stats("分配后");

    printf("\n== 释放 b，再分配 1 个(应复用 b 的地址，证明链表复用) ==\n");
    pool_free(b);
    pool_stats("释放后");
    void *d = pool_alloc();
    printf("  新块 d=%p (与 b 相同=%s)\n", d, d == b ? "是" : "否");
    pool_stats("再分配后");

    printf("\n== 耗尽测试：把池全部分光 ==\n");
    pool_free(a);
    pool_free(c);
    pool_free(d);
    void *all[POOL_BLOCK_NUM];
    for (int i = 0; i < POOL_BLOCK_NUM; i++) {
        all[i] = pool_alloc();
        printf("  第 %d 块 -> %s\n", i + 1, all[i] ? "成功" : "池已空(NULL)");
    }
    pool_stats("耗尽后");
    printf("  再多分配一个 -> %s\n", pool_alloc() ? "成功" : "NULL(池空，正确)");

    printf("\n== 越界释放防护测试(故意传错指针) ==\n");
    unsigned char fake[8];
    pool_free(fake);

    printf("\n要点: 分配/释放均为 O(1) 链表操作；峰值=8 说明池大小配置恰好够用\n");
    return 0;
}
