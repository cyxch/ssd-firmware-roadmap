#include <stdio.h>
#include <string.h>

/*
 * 块的完整生命周期与 P/E(擦写)循环。
 *
 * 一块 NAND 块从出厂到退役的过程:
 *   出厂(可用) -> 反复擦写(P/E) -> 磨损老化 -> 达到 P/E 上限 -> 坏块
 *
 * P/E 上限(粗略): SLC ~100k 次, MLC ~3k-10k, TLC ~1k-3k, QLC <1k
 *
 * 本实验演示"磨损不均"的危害:
 *   热点数据反复写同一块 -> 该块先耗光 P/E 寿命 -> 整块变坏 -> 容量缩水。
 *   反之磨损均衡(wear leveling, 下周 FTL 的内容)让所有块同步老化,
 *   整盘寿命大大延长。
 */

#define BLOCK_NUM  8
#define PE_LIMIT   100    /* 简化寿命上限: 100 次擦写 */

typedef struct {
    int pe;         /* 已擦写次数 */
    int bad;        /* 是否已坏 */
} block_t;

static block_t blocks[BLOCK_NUM];

static void init_blocks(void)
{
    memset(blocks, 0, sizeof(blocks));
}

/* 向某块执行一次 P/E(擦+写)循环 */
static void pe_cycle(int blk)
{
    if (blocks[blk].bad)
        return;
    blocks[blk].pe++;
    if (blocks[blk].pe >= PE_LIMIT) {
        blocks[blk].bad = 1;
        printf("  !! 块 %d 达到 P/E 上限(%d 次), 报废!\n", blk, PE_LIMIT);
    }
}

/* 每次写操作: 数据落到哪个块 (每 20 轮打印一次, 避免刷屏) */
static void write_to(int blk, int round)
{
    if (round % 20 == 0)
        printf("  写第 %d 轮 -> 块 %d\n", round, blk);
    pe_cycle(blk);
}

static void show_status(const char *tag)
{
    int bad = 0;
    for (int i = 0; i < BLOCK_NUM; i++)
        if (blocks[i].bad) bad++;
    printf("%s: 坏块 %d/%d, 剩余可用 %d\n", tag, bad, BLOCK_NUM, BLOCK_NUM - bad);
    printf("  各块 P/E: ");
    for (int i = 0; i < BLOCK_NUM; i++) {
        if (blocks[i].bad)
            printf("X    ");          /* 已报废 */
        else
            printf("%-4d ", blocks[i].pe);
    }
    printf("\n");
}

int main(void)
{
    /* ---- 场景 1: 无磨损均衡, 热点全写块 0 ---- */
    init_blocks();
    printf("== 场景1: 热点数据总是写块 0(无磨损均衡) ==\n");
    for (int round = 0; round < PE_LIMIT; round++)
        write_to(0, round);
    show_status("结果");

    /* ---- 场景 2: 磨损均衡(所有块轮着写) ---- */
    init_blocks();
    printf("\n== 场景2: 磨损均衡(数据轮换写到各块) ==\n");
    for (int round = 0; round < PE_LIMIT; round++)
        write_to(round % BLOCK_NUM, round);
    show_status("结果");

    printf("\n对比: 无均衡时块 0 报废, 容量剩 7/8;\n");
    printf("      均衡后各块 P/E 均匀, 整盘寿命约为单块寿命 x 块数\n");
    return 0;
}