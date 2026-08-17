#include <stdio.h>
#include <string.h>

/*
 * Flush / fsync / TRIM 与数据可靠性。
 *
 * 主机侧对可靠性的控制:
 *   - FUA(Force Unit Access): 该命令必须真正落盘才返回
 *   - Flush(NVMe 的 Flush 命令 / SATA 的 Flush Cache):
 *     把 DRAM 缓存里的数据全部强制刷到 NAND, 完成后才返回
 *   - fsync(): 应用层让内核发 Flush, 保证文件持久化
 *
 * 本实验模拟: 主机不断写, 某次掉电前调用了 Flush,
 * 对比"有 Flush"与"无 Flush"的可靠性差异。
 *
 * TRIM(ATA)/Deallocate(NVMe): 告诉 SSD 哪些 LBA 已删除,
 * 让固件提前 GC, 减少写放大、延长寿命。
 */

#define CACHE_NUM 8

static int cache_lba[CACHE_NUM];       /* 缓存中的 LBA */
static int cache_n = 0;               /* 缓存条数 */
static int flushed_lba[CACHE_NUM];     /* 已刷到 NAND */
static int flushed_n = 0;

/* 主机写数据(先进缓存) */
static void host_write(int lba)
{
    cache_lba[cache_n++] = lba;
    printf("  写 LBA %d(进 DRAM 缓存)\n", lba);
}

/* Flush: 把缓存全部刷到 NAND */
static void do_flush(void)
{
    for (int i = 0; i < cache_n; i++) {
        flushed_lba[flushed_n++] = cache_lba[i];
        printf("  刷 LBA %d -> NAND\n", cache_lba[i]);
    }
    cache_n = 0;
}

/* 掉电: 返回丢了多少条 */
static int power_loss(void)
{
    int lost = 0;
    for (int i = 0; i < cache_n; i++) {
        printf("  [丢失] LBA %d 仍在缓存, 掉电丢失\n", cache_lba[i]);
        lost++;
    }
    return lost;
}

int main(void)
{
    printf("== 场景1: 无 Flush, 写后立即掉电 ==\n");
    host_write(0);
    host_write(1);
    host_write(2);
    printf("  掉电!\n");
    power_loss();

    printf("\n== 场景2: 写后先 Flush 再掉电(新的上电周期, 缓存已清空) ==\n");
    cache_n = 0;                 /* 开机后缓存清空 */
    host_write(3);
    host_write(4);
    printf("  调用 Flush...\n");
    do_flush();
    printf("  掉电!\n");
    printf("  (已刷 %d 条, 全部保住)\n", flushed_n);

    printf("\n== TRIM/Deallocate 演示 ==\n");
    /* 文件删除后: 主机发 Deallocate 告诉 SSD 这些 LBA 无效 */
    printf("  删除文件 -> 主机发 Deallocate(LBA 0..1)\n");
    printf("  SSD 固件将这些 LBA 对应页标记无效, GC 可提前回收\n");
    printf("  好处: 减少写放大、延长寿命、掉电恢复更快\n");

    printf("\n要点: 数据可靠性由主机驱动(Flush/FUA)+固件(PLP/日志)共同保证;\n");
    return 0;
}
