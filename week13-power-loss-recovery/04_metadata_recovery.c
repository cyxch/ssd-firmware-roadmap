#include <stdio.h>
#include <string.h>

/*
 * 映射表重建与一致性校验（开机恢复）。
 *
 * 开机时 DRAM 里的映射表是空的, 需要从 NAND 重建:
 *   1. 扫描每个物理页的 OOB(页尾元数据): 里面记录了"该页属于哪个 LBA"
 *   2. 用 OOB 信息重建 L2P 表
 *
 * 一致性校验(CRC): 每个数据页存 CRC, 读回时校验。
 * 若 CRC 错 -> 该页"半写"(掉电中断) -> 丢弃该页, 不影响其他页。
 *
 * 本实验模拟: 扫描所有物理页 -> 重建映射 -> 用 CRC 检测掉电半写页。
 */

#define PAGES 12
#define LBA_NUM 8

typedef struct {
    int lba;            /* OOB: 该页属于哪个 LBA, -1=空闲 */
    unsigned char crc;  /* OOB: 数据的校验值 */
    unsigned char data; /* 数据区 */
} page_t;

static page_t nand[PAGES];

/* 简单 CRC: 把所有字节异或 */
static unsigned char calc_crc(unsigned char d)
{
    return d;   /* 简化: 数据本身即校验(演示用), 真实是多项式 CRC */
}

/* 模拟 OOB: 写页时把 lba + crc 存进页元数据 */
static void program_page(int p, int lba, unsigned char data)
{
    nand[p].lba = lba;
    nand[p].data = data;
    nand[p].crc = calc_crc(data);
}

int main(void)
{
    int map[LBA_NUM];
    int bad_pages = 0;

    memset(nand, 0, sizeof(nand));
    for (int i = 0; i < PAGES; i++)
        nand[i].lba = -1;          /* 默认全部是空闲页 */
    for (int i = 0; i < LBA_NUM; i++)
        map[i] = -1;

    /* 模拟盘上已有数据(某些页掉电半写, CRC 损坏) */
    program_page(0, 0, 0x11);
    program_page(1, 1, 0x22);
    program_page(2, 0, 0x33);   /* LBA0 更新到页2 */
    program_page(3, 2, 0x44);
    program_page(4, 3, 0x55);
    /* 页5: 掉电半写, data 被破坏但 OOB 声称 LBA4 */
    program_page(5, 4, 0x66);
    nand[5].data = 0xFF;        /* 半写导致数据损坏, CRC 失配 */

    printf("== 开机: 扫描所有物理页, 从 OOB 重建映射 ==\n");
    for (int p = 0; p < PAGES; p++) {
        if (nand[p].lba < 0)
            continue;   /* 空闲页 */
        /* CRC 校验: 半写页会被揪出来 */
        if (calc_crc(nand[p].data) != nand[p].crc) {
            printf("  页 P%d: CRC 校验失败(掉电半写)! 丢弃该页\n", p);
            bad_pages++;
            continue;
        }
        /* 有效页: 更新映射(同 LBA 后面的页覆盖前面的) */
        map[nand[p].lba] = p;
        printf("  页 P%d: 有效, LBA %d -> P%d\n", p, nand[p].lba, p);
    }

    printf("\n== 重建完成的映射表 ==\n");
    for (int l = 0; l < LBA_NUM; l++) {
        if (map[l] >= 0)
            printf("  LBA %d -> P%d\n", l, map[l]);
        else
            printf("  LBA %d -> 未映射\n", l);
    }

    printf("\n发现半写坏页 %d 个(已隔离, 不影响其他数据)\n", bad_pages);
    printf("\n要点: 开机重建映射表靠扫描 OOB; CRC 揪出半写页;\n");
    printf("      掉电最坏情况只丢“最后一条未完成写”, 不会损坏已有数据\n");
    return 0;
}
