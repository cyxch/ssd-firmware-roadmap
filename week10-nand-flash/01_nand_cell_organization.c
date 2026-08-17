#include <stdio.h>

/*
 * NAND Flash 物理层级结构与容量计算。
 *
 * 层级(从大到小): 芯片(die) -> 平面(plane) -> 块(block) -> 页(page) -> 单元(cell)
 *   - 单元(cell) : 用一个浮栅晶体管存电荷，SLC 存 1bit / MLC 2bit / TLC 3bit / QLC 4bit
 *   - 页(page)   : 最小"读写"单位（如 16KB 主区 + OOB 备用区）
 *   - 块(block)  : 最小"擦除"单位（如 512 页/块）
 *   - plane      : 可并行操作的一组块
 *   - die        : 一颗芯片，多 die 可交错(interleave)提升带宽
 *
 * 关键：同一颗 die 的"单元总数"是固定的。按 SLC 跑容量最小、
 * 按 QLC 跑容量最大 —— 但位数越多，可靠性/寿命/速度越差。
 */

/* 一颗 die 在 SLC 模式下 1 页 = 16KB 数据，即每页 16K*8 个单元 */
#define CELLS_PER_PAGE    (16 * 1024 * 8)
#define PAGES_PER_BLOCK   512
#define BLOCKS_PER_PLANE  1024
#define PLANES_PER_DIE    2
#define DIES_PER_DEVICE   2

typedef struct {
    const char *name;
    int bits_per_cell;
} cell_mode_t;

/* 总容量(字节) = 单元数 x 每单元位数 / 8 */
static long long device_capacity_bytes(int bits_per_cell)
{
    return (long long)CELLS_PER_PAGE * bits_per_cell / 8
           * PAGES_PER_BLOCK * BLOCKS_PER_PLANE
           * PLANES_PER_DIE * DIES_PER_DEVICE;
}

int main(void)
{
    static const cell_mode_t modes[] = {
        {"SLC(1bit/单元)", 1},
        {"MLC(2bit/单元)", 2},
        {"TLC(3bit/单元)", 3},
        {"QLC(4bit/单元)", 4},
    };
    const int n = (int)(sizeof(modes) / sizeof(modes[0]));

    printf("NAND 物理层级参数:\n");
    printf("  单元 cell       : 存 1~4 bit 电荷\n");
    printf("  页 page         : 最小读写单位\n");
    printf("  块 block        : 最小擦除单位, %d 页/块\n", PAGES_PER_BLOCK);
    printf("  plane           : %d 块/plane\n", BLOCKS_PER_PLANE);
    printf("  die(芯片)       : %d plane/die, 共 %d die\n\n",
           PLANES_PER_DIE, DIES_PER_DEVICE);

    printf("同一颗 die 按不同模式运行, 单元数不变, 容量不同:\n");
    for (int i = 0; i < n; i++) {
        long long bytes = device_capacity_bytes(modes[i].bits_per_cell);
        printf("  %-15s 每页%4dKB  整盘%7.1f GB\n",
               modes[i].name,
               CELLS_PER_PAGE * modes[i].bits_per_cell / 8 / 1024,
               (double)bytes / (1024.0 * 1024.0 * 1024.0));
    }

    printf("\n要点(SSD 选型核心权衡):\n");
    printf("  位数越多 -> 容量越大, 但 P/E 寿命缩短、可靠性下降、写更慢\n");
    printf("  SLC 寿命最长(约 TLC 的 30 倍), 企业级常用 SLC 缓存 + TLC 大容量\n");
    return 0;
}
