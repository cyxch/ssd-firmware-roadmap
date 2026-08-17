#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/*
 * 写放大系数(Write Amplification Factor, WAF)测量。
 *
 * WAF = NAND 实际写入量 / 主机请求写入量
 *   - WAF = 1   : 理想, 主机写 1 页 NAND 就写 1 页
 *   - WAF > 1   : 主机写 1 页, NAND 实际写了好几页
 *                 (out-of-place 重写 + GC 搬移有效页造成的额外写入)
 *
 * 关键实现点(对照真实 FTL):
 *   - 低水位触发 GC: 空闲页降到一整个块以下时才 GC,
 *     保证 GC 有空间搬有效页(不能在空闲页=0 时才回收)
 *   - victim 选择: 优先选"有效页最少"的块; 全无效块(0 有效)是最佳目标
 *
 * 实验对比:
 *   - 顺序覆盖全盘: 块整体作废, GC 搬得少 -> WAF 低(≈1)
 *   - 随机写满全盘: 每个块都堆着有效页和作废页, GC 反复搬 -> WAF 高
 */

#define BLOCKS 4
#define PAGES_PER_BLOCK 8
#define N_PAGES (BLOCKS * PAGES_PER_BLOCK)
#define N_LBA 16
#define WRITES 800
#define LOW_WATER PAGES_PER_BLOCK   /* 空闲页低水位 */

typedef struct {
    int state;   /* 0=空闲 1=有效 2=作废 */
    int lba;
} page_t;

static page_t pages[N_PAGES];
static long long host_writes;   /* 主机请求写页数 */
static long long nand_writes;   /* NAND 实际编程页数(含 GC 搬移) */

static void reset(void)
{
    memset(pages, 0, sizeof(pages));
    host_writes = 0;
    nand_writes = 0;
}

static int free_pages(void)
{
    int n = 0;
    for (int i = 0; i < N_PAGES; i++)
        if (pages[i].state == 0)
            n++;
    return n;
}

/* 垃圾回收: 优先选"含无效页且有效页最少"的块, 搬有效页, 擦除 */
static void gc(void)
{
    int victim = -1, min_valid = PAGES_PER_BLOCK + 1;
    int found_invalid = 0;

    /* 第一遍: 只考虑含无效页的块(全无效块是最佳目标) */
    for (int b = 0; b < BLOCKS; b++) {
        int v = 0, inv = 0;
        for (int p = 0; p < PAGES_PER_BLOCK; p++) {
            int st = pages[b * PAGES_PER_BLOCK + p].state;
            if (st == 1) v++;
            if (st == 2) inv++;
        }
        if (inv > 0 && v < min_valid) {
            victim = b;
            min_valid = v;
            found_invalid = 1;
        }
    }
    /* 第二遍: 兜底 —— 全盘都有效时, 也选有效页最少的 */
    if (!found_invalid) {
        for (int b = 0; b < BLOCKS; b++) {
            int v = 0;
            for (int p = 0; p < PAGES_PER_BLOCK; p++)
                if (pages[b * PAGES_PER_BLOCK + p].state == 1)
                    v++;
            if (v < min_valid) { victim = b; min_valid = v; }
        }
    }
    if (victim < 0)
        return;

    int moves = 0;
    for (int p = 0; p < PAGES_PER_BLOCK; p++) {
        int idx = victim * PAGES_PER_BLOCK + p;
        if (pages[idx].state == 1) {
            /* 搬到空闲页 */
            int dest = -1;
            for (int i = 0; i < N_PAGES; i++)
                if (pages[i].state == 0) { dest = i; break; }
            if (dest < 0)
                break;      /* 无空间, 停止搬移(低水位下不应发生) */
            pages[dest].state = 1;
            pages[dest].lba = pages[idx].lba;
            nand_writes++;  /* 搬移 = 一次额外 NAND 写 */
            moves++;
        }
    }
    (void)moves;

    /* 擦除 victim: 恢复为全空闲 */
    for (int p = 0; p < PAGES_PER_BLOCK; p++) {
        int idx = victim * PAGES_PER_BLOCK + p;
        pages[idx].state = 0;
        pages[idx].lba = -1;
    }
}

static void ftl_write(int lba)
{
    /* out-of-place: 旧页作废 */
    for (int i = 0; i < N_PAGES; i++)
        if (pages[i].state == 1 && pages[i].lba == lba)
            pages[i].state = 2;

    /* 低水位触发 GC(留空间搬有效页) */
    if (free_pages() <= LOW_WATER)
        gc();

    for (int i = 0; i < N_PAGES; i++) {
        if (pages[i].state == 0) {
            pages[i].state = 1;
            pages[i].lba = lba;
            host_writes++;
            nand_writes++;
            return;
        }
    }
}

/* mode: 0=顺序覆盖全盘 1=随机写满全盘 */
static void run(const char *label, int mode)
{
    reset();
    srand(12345);

    for (int i = 0; i < WRITES; i++) {
        int lba;
        if (mode == 0)
            lba = i % N_LBA;              /* 顺序扫过所有 LBA */
        else
            lba = rand() % N_LBA;         /* 随机写满全地址空间 */
        ftl_write(lba);
    }

    double waf = (double)nand_writes / host_writes;
    printf("%s\n", label);
    printf("  主机写入 %lld 页, NAND 实际写入 %lld 页\n", host_writes, nand_writes);
    printf("  写放大系数 WAF = %.2f\n\n", waf);
}

int main(void)
{
    run("场景1: 顺序覆盖全盘(顺序写)", 0);
    run("场景2: 随机写满全盘(随机写)", 1);

    printf("原因分析:\n");
    printf("  顺序写: 每个块被连续填满后整体作废, GC 时全无效块被回收, 搬移≈0\n");
    printf("  随机写: 16 个 LBA 都有有效页散布在各块, GC 每次要搬走有效页\n");
    printf("  => 写放大高会加速 NAND 磨损、占带宽、发热, 所以 SSD 会优化写入模式\n");
    return 0;
}
