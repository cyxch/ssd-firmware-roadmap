#include <stdio.h>
#include <string.h>

/*
 * 垃圾回收(Garbage Collection, GC)。
 *
 * 为什么需要 GC:
 *   大量 out-of-place 更新后, invalid 页堆积, 空闲页耗尽(见 02)。
 *   此时必须回收"垃圾多"的块: 把块里还有用的有效页搬到别处,
 *   然后整块擦除, 重新变回空闲块。
 *
 * Victim 选择策略(本实验): 有效页最少优先 ——
 *   搬移的有效页越少, GC 开销越小, 写放大越低。
 *   注: 真实 FTL 还会综合考虑 P/E 磨损(冷块优先)等。
 */

#define BLOCKS 4
#define PAGES_PER_BLOCK 4

typedef struct {
    int valid;
    int invalid;
} block_t;

static block_t blocks[BLOCKS];
static int g_free_pages = 0;   /* 全局空闲页数 */

/* 构造一个"垃圾堆积"的场景 */
static void init_messy(void)
{
    memset(blocks, 0, sizeof(blocks));
    /* 块0: 3 有效 1 无效   块1: 2 有效 2 无效
       块2: 1 有效 3 无效   块3: 全空闲 */
    blocks[0].valid = 3;   blocks[0].invalid = 1;
    blocks[1].valid = 2;   blocks[1].invalid = 2;
    blocks[2].valid = 1;   blocks[2].invalid = 3;
    blocks[3].valid = 0;   blocks[3].invalid = 0;
    g_free_pages = PAGES_PER_BLOCK;   /* 块3 是空闲块 */
}

static void dump(const char *tag)
{
    printf("%s\n", tag);
    for (int b = 0; b < BLOCKS; b++)
        printf("  块 %d: 有效 %d, 无效 %d\n", b, blocks[b].valid, blocks[b].invalid);
    printf("  空闲页: %d\n\n", g_free_pages);
}

/* 选 victim: 有效页最少(且 >0)的块 */
static int pick_victim(void)
{
    int best = -1, min_valid = PAGES_PER_BLOCK + 1;
    for (int b = 0; b < BLOCKS; b++) {
        if (blocks[b].valid > 0 && blocks[b].valid < min_valid) {
            best = b;
            min_valid = blocks[b].valid;
        }
    }
    return best;
}

/* 执行一次 GC */
static void gc(void)
{
    int victim = pick_victim();
    if (victim < 0) {
        printf("  没有可回收的块\n");
        return;
    }

    int moves = blocks[victim].valid;
    printf("== 选择块 %d 作为回收目标: 有效页 %d 个 ==\n", victim, moves);

    printf("  步骤1 搬移: 把 %d 个有效页读出来写到空闲页\n", moves);
    g_free_pages -= moves;                    /* 搬移占用空闲页 */

    printf("  步骤2 擦除: 整块擦除, 恢复为全空闲\n");
    g_free_pages += PAGES_PER_BLOCK;

    blocks[victim].valid = 0;
    blocks[victim].invalid = 0;
    printf("  回收完成: 新增空闲页 %d 个, 当前空闲页 %d\n\n",
           PAGES_PER_BLOCK - moves, g_free_pages);
}

int main(void)
{
    init_messy();
    dump("== GC 前: invalid 页堆积, 空闲页紧张 ==");

    gc();
    dump("== GC 后 ==");

    printf("观察:\n");
    printf("  - 选中的是有效页最少的块 2, 只需搬 1 页(开销最小)\n");
    printf("  - 搬移的 %d 页是写放大来源: 写 1 页数据却实际写了几页\n", 1);
    printf("  - 若选块 0(3 有效), 开销会是 3 倍 —— 这就是 victim 策略的意义\n");
    return 0;
}
