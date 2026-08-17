#include <stdio.h>
#include <string.h>

/*
 * NAND 三种操作的时序特性(量级, 简化):
 *   - 页读  tR    ~ 100 us  (读最快)
 *   - 页写  tPROG ~ 1 ms    (写比读慢约 10 倍)
 *   - 块擦  tBERS ~ 3 ms    (最慢, 且只能整块擦)
 *
 * 三大物理约束(本实验演示):
 *   1. 最小读写单位是"页", 最小擦除单位是"块"
 *   2. 写(编程)只能把 bit 从 1 变 0, 不能 0 变 1
 *      -> 想改数据必须先整块擦除(变回全 1)
 *   3. 同一页擦后只能编程一次, 重复编程会失败
 */

#define PAGES_PER_BLOCK 512
#define PAGE_DATA_BYTES 8
#define TREAD_US   100    /* 页读 100us */
#define TPROG_US   1000   /* 页写 1ms  */
#define TERASE_US  3000   /* 块擦 3ms  */

typedef struct {
    int valid[PAGES_PER_BLOCK];                 /* 该页是否已编程 */
    unsigned char data[PAGES_PER_BLOCK][PAGE_DATA_BYTES];
} nand_block_t;

static long long g_time_us = 0;   /* 累计耗时(us) */

/* 编程一页: 失败返回 -1 */
static int page_program(nand_block_t *b, int page, unsigned char val)
{
    if (b->valid[page]) {
        printf("    [失败] 页 %d 已编程过: 1->0 只能一次, 0->1 做不到, 需先擦块\n",
               page);
        return -1;
    }
    g_time_us += TPROG_US;
    memset(b->data[page], val, sizeof(b->data[page]));
    b->valid[page] = 1;
    return 0;
}

/* 擦除整块: 全部页恢复为"未编程"态 */
static void block_erase(nand_block_t *b)
{
    g_time_us += TERASE_US;
    memset(b, 0, sizeof(*b));
}

int main(void)
{
    nand_block_t blk;
    memset(&blk, 0, sizeof(blk));

    printf("== 操作时序对比(典型量级) ==\n");
    printf("  页读  tR    : %d us\n", TREAD_US);
    printf("  页写  tPROG : %d us (写比读慢 %d 倍)\n", TPROG_US, TPROG_US / TREAD_US);
    printf("  块擦  tBERS : %d us (一次擦 %d 页)\n\n", TERASE_US, PAGES_PER_BLOCK);

    printf("== 写满一个块(512 页) ==\n");
    for (int i = 0; i < PAGES_PER_BLOCK; i++) {
        if (page_program(&blk, i, (unsigned char)(i & 0xff)) < 0)
            return 1;
    }
    printf("  写满耗时 %lld us\n", g_time_us);

    printf("\n== 想覆盖页 10 的数据(不改数据区无法直接改) ==\n");
    page_program(&blk, 10, 0xAA);   /* 会失败: 已编程过 */

    printf("\n== 必须先整块擦除才能重写 ==\n");
    block_erase(&blk);
    printf("  擦除耗时 %lld us, 块恢复为全 1(未编程)态\n", g_time_us);
    printf("  擦后重写页 10 -> %s\n",
           page_program(&blk, 10, 0xAA) == 0 ? "成功" : "失败");

    printf("\n要点: 因为“改小块要先擦大块”，直接覆盖会引发\n");
    printf("      “读旧页+搬数据+擦块+写新页”，即写放大与垃圾回收的根源\n");
    printf("      —— 这就是下周 FTL 要解决的核心问题\n");
    return 0;
}
