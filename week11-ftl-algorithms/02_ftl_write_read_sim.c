#include <stdio.h>
#include <string.h>

/*
 * 完整 FTL 写/读流程模拟（out-of-place 更新）。
 *
 * 核心思想:
 *   写 LBA 时"写到新页"(out-of-place), 旧页作废(invalid) ——
 *   因为 NAND 不能原地改, 这是 FTL 存在的根本原因。
 *   读 LBA 时查 L2P 表找到当前物理页。
 *
 * 观察重点:
 *   - L2P 表随写入不断变化
 *   - invalid 页不断积累 -> 最终无空闲页 -> 必须垃圾回收(见 03)
 */

#define BLOCKS 2
#define PAGES_PER_BLOCK 4
#define N_PAGES (BLOCKS * PAGES_PER_BLOCK)
#define N_LBA 4

typedef struct {
    int state;      /* 0=空闲 1=有效 2=作废(invalid) */
    int lba;        /* 该页存放的 LBA */
    unsigned char data;
} page_t;

static page_t pages[N_PAGES];
static int l2p[N_LBA];      /* 逻辑页 -> 物理页, -1 = 未映射 */

static int alloc_free_page(void)
{
    for (int i = 0; i < N_PAGES; i++)
        if (pages[i].state == 0)
            return i;
    return -1;              /* 无空闲页 */
}

static void ftl_write(int lba, unsigned char data)
{
    int old = l2p[lba];
    if (old >= 0) {
        printf("  写 LBA%d(0x%02X): 旧页 P%d 作废", lba, data, old);
        pages[old].state = 2;
        printf(", out-of-place 换新页\n");
    } else {
        printf("  写 LBA%d(0x%02X): 首次写\n", lba, data);
    }

    int p = alloc_free_page();
    if (p < 0) {
        printf("  [阻塞] 无空闲页! 需要垃圾回收(见 03)\n");
        return;
    }
    pages[p].state = 1;
    pages[p].lba = lba;
    pages[p].data = data;
    l2p[lba] = p;
    printf("          -> 分配到 P%d, L2P[%d]=%d\n", p, lba, p);
}

static int ftl_read(int lba)
{
    int p = l2p[lba];
    if (p < 0) {
        printf("  读 LBA%d: 未映射(返回 0)\n", lba);
        return 0;
    }
    printf("  读 LBA%d: 查表 P%d -> 数据 0x%02X\n", lba, p, pages[p].data);
    return pages[p].data;
}

static void dump(void)
{
    printf("  页状态:");
    for (int i = 0; i < N_PAGES; i++)
        printf(" P%d=%s", i, pages[i].state == 1 ? "V"
                                : (pages[i].state == 2 ? "I" : "F"));
    printf("\n  L2P表  :");
    for (int l = 0; l < N_LBA; l++) {
        if (l2p[l] < 0)
            printf(" L%d=- ", l);
        else
            printf(" L%d=P%d ", l, l2p[l]);
    }
    printf("\n");
}

int main(void)
{
    memset(pages, 0, sizeof(pages));
    for (int i = 0; i < N_LBA; i++)
        l2p[i] = -1;

    printf("== 首次写 LBA0..3 ==\n");
    ftl_write(0, 0xAA);
    ftl_write(1, 0xBB);
    ftl_write(2, 0xCC);
    ftl_write(3, 0xDD);
    dump();

    printf("\n== 重写 LBA0, LBA1(out-of-place) ==\n");
    ftl_write(0, 0x11);
    ftl_write(1, 0x22);
    dump();

    printf("\n== 读回验证(读到最新数据) ==\n");
    ftl_read(0);
    ftl_read(1);

    printf("\n== 继续重写 LBA2, LBA3 ==\n");
    ftl_write(2, 0x33);
    ftl_write(3, 0x44);
    dump();

    printf("\n== 再写一次 LBA0 -> 无空闲页, 触发 GC 需求 ==\n");
    ftl_write(0, 0x55);

    printf("\n要点: 每次重写都产生一个 invalid 页, 它们只能靠垃圾回收\n");
    printf("      腾出空间; invalid 越多, 回收时要搬移的有效页越少越划算\n");
    return 0;
}
