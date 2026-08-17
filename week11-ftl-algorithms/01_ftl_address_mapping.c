#include <stdio.h>

/*
 * FTL 地址映射：把主机看到的逻辑地址(LBA)翻译成 NAND 物理地址(PBA)。
 *
 * 为什么必须有映射?
 *   1. 主机把 SSD 当块设备用, LBA 从 0 连续编号
 *   2. NAND 不能原地覆盖(写前必擦), 数据要写到别处(out-of-place)
 *      -> 同一个 LBA 的物理位置随时在变, 必须用映射表记住
 *
 * 映射粒度是 FTL 最重要的设计决策:
 *   - 页级映射: 每个 4KB 逻辑页独立映射到任意物理页
 *               -> 灵活、写放大低, 但表大(常驻内存)
 *   - 块级映射: 整块映射, 块内偏移固定
 *               -> 表小, 但更新小块数据也要搬整块, 写放大严重
 */

#define DISK_GB  64
#define PAGE_KB  4
#define PAGES_PER_BLOCK 256

int main(void)
{
    long long pages_total = (long long)DISK_GB * 1024 * 1024 / PAGE_KB;
    long long blocks_total = pages_total / PAGES_PER_BLOCK;

    printf("SSD 参数: %d GB, 逻辑页 %dKB, %d 页/块\n\n",
           DISK_GB, PAGE_KB, PAGES_PER_BLOCK);
    printf("  总逻辑页数 = %lld, 总块数 = %lld\n\n", pages_total, blocks_total);

    printf("【页级映射表】(每个逻辑页一项, PBA 用 4 字节):\n");
    printf("  表项数 = %lld, 表大小 = %.1f MB\n\n",
           pages_total, (double)pages_total * 4 / (1024 * 1024));

    printf("【块级映射表】(每个逻辑块一项, 块号用 2 字节):\n");
    printf("  表项数 = %lld, 表大小 = %.2f MB\n\n",
           blocks_total, (double)blocks_total * 2 / (1024 * 1024));

    printf("对比: 页级表是块级表的约 %.0f 倍\n\n",
           (double)(pages_total * 4) / (blocks_total * 2));

    printf("优缺点:\n");
    printf("  页级: 表大(内存开销), 但每个 LBA 独立映射, 更新无需搬整块\n");
    printf("        -> 写放大低、性能好, 高端/主流 SSD 采用\n");
    printf("  块级: 表小(可放 SRAM/低成本主控), 但更新 4KB 也搬整块\n");
    printf("        -> 写放大严重、性能差, 多见于早期/低端方案\n");

    printf("\n要点: 映射表是 FTL 的内存大户, 掉电需保存在 NAND 再开机恢复\n");
    return 0;
}
