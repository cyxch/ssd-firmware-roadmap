#include <stdio.h>

/*
 * 存储层级总览：越靠近 CPU 越快、越小、越贵。
 * 程序员的黄金法则：让数据访问尽可能"局部"（时间/空间局部性）。
 * SSD 固件为何追求"顺序写"？因为闪存/FTL 对顺序访问的 GC 与磨损管理更友好，
 * 本质也是"局部性"在介质层的体现。
 */

int main(void)
{
    printf("┌─────────────────────────────────────────────┐\n");
    printf("│  存储层级金字塔（自上而下: 快→慢, 小→大, 贵→廉）│\n");
    printf("├─────────────────────────────────────────────┤\n");
    printf("│  寄存器 寄存器        ~1 ns     ~百字节       │\n");
    printf("│  L1 Cache 片内        ~1 ns     ~32-64 KB    │\n");
    printf("│  L2 Cache 片内/片外   ~4 ns     ~256-512 KB  │\n");
    printf("│  L3 Cache 共享        ~10 ns    ~几 MB       │\n");
    printf("│  主存 DRAM            ~100 ns   ~几 GB       │\n");
    printf("│  SSD NAND             ~us-ms    ~几百 GB-TB  │\n");
    printf("│  磁盘 HDD             ~ms       ~几 TB       │\n");
    printf("└─────────────────────────────────────────────┘\n\n");

    printf("关键数字（量级对比）:\n");
    printf("  DRAM 访问 ~100ns，NAND 读 ~几十us，HDD ~几ms\n");
    printf("  一次 HDD 寻道时间 ≈ 数百万次寄存器操作\n");
    printf("  => 程序性能瓶颈通常不在 CPU，而在『memory wall』（访存）\n\n");

    printf("两个局部性:\n");
    printf("  时间局部性: 刚访问过的数据很快会再访问  -> 循环/热点\n");
    printf("  空间局部性: 访问某地址后邻近地址会被访问 -> 数组/顺序流\n\n");

    printf("=> 本目录的实验(02/03)将用量化数据证明这两点。\n");
    return 0;
}
