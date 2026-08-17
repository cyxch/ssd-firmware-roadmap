#include <stdio.h>
#include <string.h>

/*
 * 坏块管理(Bad Block Management)与坏块表(BBT)。
 *
 * 现实：NAND 出厂就有"出厂坏块"，且使用中因 P/E 循环、
 * 干扰、保持错误等会不断产生"新增坏块"。SSD 必须：
 *   1. 扫描坏块
 *   2. 建坏块表
 *   3. 读写遇坏块时"跳过"(用预留的替换块)
 *
 * 本实验模拟：128 个块中有 5 个出厂坏块 + 3 个使用中新增坏块；
 * 坏块表用位图(bitmap)存储，节省空间；读写路由跳过坏块。
 */

#define BLOCK_NUM 128
#define BBT_WORDS  ((BLOCK_NUM + 31) / 32)   /* 32 位对齐 */

/* 坏块表: 1 = 坏块, 0 = 好块 */
static unsigned int g_bbt[BBT_WORDS];

static void bbt_set(int blk)
{
    g_bbt[blk / 32] |= (1u << (blk % 32));
}

static int bbt_is_bad(int blk)
{
    return (g_bbt[blk / 32] >> (blk % 32)) & 1u;
}

static void bbt_dump(const char *tag)
{
    int bad = 0;
    for (int i = 0; i < BLOCK_NUM; i++)
        if (bbt_is_bad(i)) bad++;
    printf("%s: 坏块数=%d, 坏块表总位=%d\n", tag, bad, BLOCK_NUM);

    /* 打印具体坏块编号(只打前 20 个避免刷屏) */
    printf("  坏块编号: ");
    int printed = 0;
    for (int i = 0; i < BLOCK_NUM; i++) {
        if (bbt_is_bad(i)) {
            if (printed++ >= 20) { printf("..."); break; }
            printf("%d ", i);
        }
    }
    printf("\n");
}

/* 模拟一次读写: 遇坏块则跳过 */
static int read_block(int blk)
{
    if (bbt_is_bad(blk)) {
        printf("  读块 %d: 跳过(坏块)\n", blk);
        return -1;
    }
    printf("  读块 %d: 成功\n", blk);
    return 0;
}

int main(void)
{
    /* 1. 模拟出厂坏块(制造时随机坏的) */
    int factory_bad[] = {7, 17, 33, 67, 99};
    printf("出厂坏块扫描(Bad Block Scan):\n");
    for (int i = 0; i < (int)(sizeof(factory_bad)/sizeof(factory_bad[0])); i++)
        bbt_set(factory_bad[i]);
    bbt_dump("出厂扫描后");

    /* 2. 使用一段时间后新增坏块 */
    int grown_bad[] = {23, 55, 88};
    printf("\n使用中新增坏块检测:\n");
    for (int i = 0; i < (int)(sizeof(grown_bad)/sizeof(grown_bad[0])); i++)
        bbt_set(grown_bad[i]);
    bbt_dump("运行一段时间后");

    /* 3. 读写路由：跳过坏块(用周边好块代替) */
    printf("\n读写路由(跳过坏块):\n");
    for (int i = 65; i < 72; i++)
        read_block(i);

    printf("\n要点: 坏块表在固件启动时从 NAND 前几个块加载到内存;\n");
    printf("      SSD 预留 2%%~5%% 的替换块(over-provisioning)来替换坏块;\n");
    return 0;
}