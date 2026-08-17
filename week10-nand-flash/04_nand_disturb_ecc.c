#include <stdio.h>
#include <string.h>

/*
 * NAND 干扰与 ECC（单比特纠错 SEC 模拟）。
 *
 * 干扰类型:
 *   - Program Disturb  : 写一个页时，相邻页单元电荷被轻微扰动
 *   - Read Disturb     : 反复读一个页时，相邻页电荷被轻微扰动
 *   - Retention Loss   : 随电子泄漏，电荷电平漂移，久了会不可读
 *
 * ECC 纠错原理:
 *   编程时把"校验位"写入 OOB 区；读时重新计算校验，与存储的比较。
 *   单比特纠错需要能"定位"错误发生在哪一位 —— 本实验用
 *   4x4 矩阵 + 行列校验：
 *     行校验错 => 错误在某一行；列校验错 => 错误在某一列
 *     行列交叉处即为错误位，翻转它即完成纠正。
 *   (这就是汉明码/Hamming code 的直观思想)
 */

#define GRID 4    /* 4x4 = 16 个数据位 */

typedef struct {
    unsigned char bit[GRID][GRID];   /* 数据位 0/1 */
} grid_t;

/* 计算行校验和列校验 */
static void compute_parity(const grid_t *d,
                           unsigned char rowp[GRID],
                           unsigned char colp[GRID])
{
    for (int r = 0; r < GRID; r++) {
        rowp[r] = 0;
        for (int c = 0; c < GRID; c++)
            rowp[r] ^= d->bit[r][c];          /* 行内异或 */
    }
    for (int c = 0; c < GRID; c++) {
        colp[c] = 0;
        for (int r = 0; r < GRID; r++)
            colp[c] ^= d->bit[r][c];          /* 列内异或 */
    }
}

/* 校验: 返回 0=无错, 1=单比特错误(纠正), -1=多比特(无法定位) */
static int ecc_check_and_correct(grid_t *d,
                                 const unsigned char rowp[GRID],
                                 const unsigned char colp[GRID])
{
    int bad_row = -1, bad_col = -1;
    int row_cnt = 0, col_cnt = 0;
    unsigned char rp[GRID], cp[GRID];

    compute_parity(d, rp, cp);

    for (int r = 0; r < GRID; r++)
        if (rp[r] != rowp[r]) { row_cnt++; bad_row = r; }
    for (int c = 0; c < GRID; c++)
        if (cp[c] != colp[c]) { col_cnt++; bad_col = c; }

    if (row_cnt == 0 && col_cnt == 0)
        return 0;                     /* 无错 */

    /* 只有"恰好一行 + 恰好一列"错，交叉处才是唯一错误位 */
    if (row_cnt == 1 && col_cnt == 1) {
        d->bit[bad_row][bad_col] ^= 1;    /* 翻转纠正 */
        return 1;
    }

    /* 多行/多列错(多比特)或仅校验位错: 超出单比特纠错能力 */
    return -1;
}

/* 打印网格 */
static void grid_print(const grid_t *d, const char *tag)
{
    printf("%s\n", tag);
    for (int r = 0; r < GRID; r++) {
        printf("  ");
        for (int c = 0; c < GRID; c++)
            printf("%d ", d->bit[r][c]);
        printf("\n");
    }
}

int main(void)
{
    grid_t original, readback;
    unsigned char rowp[GRID], colp[GRID];

    /* 构造 16 位数据 */
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++)
            original.bit[r][c] = (unsigned char)((r * GRID + c) % 2);

    compute_parity(&original, rowp, colp);
    printf("== 编程: 数据 + 校验位写入 OOB ==\n");
    grid_print(&original, "写入的数据(16 位):");

    /* ---- 场景 1: 单比特错误(读干扰/编程干扰) ---- */
    readback = original;
    readback.bit[2][1] ^= 1;   /* 人为翻转第(2,1)位 */
    printf("\n== 场景1: 读回时 (2,1) 位发生翻转(干扰) ==\n");
    grid_print(&readback, "读回的数据:");

    int r1 = ecc_check_and_correct(&readback, rowp, colp);
    if (r1 == 1)
        printf("  ECC: 定位并纠正了 (2,1) 位错误!\n");
    printf("  与原始数据一致: %s\n\n",
           memcmp(&readback, &original, sizeof(grid_t)) == 0 ? "是" : "否");

    /* ---- 场景 2: 无错误 ---- */
    readback = original;
    int r2 = ecc_check_and_correct(&readback, rowp, colp);
    printf("== 场景2: 无错误数据 ==\n  ECC 判定: %s\n\n",
           r2 == 0 ? "数据完好, 无需纠正" : "有错");

    /* ---- 场景 3: 两个比特错误(超出单比特纠错能力) ---- */
    readback = original;
    readback.bit[0][0] ^= 1;
    readback.bit[3][3] ^= 1;
    printf("== 场景3: 两个比特错误 (0,0)(3,3) ==\n");
    grid_print(&readback, "读回的数据:");
    int r3 = ecc_check_and_correct(&readback, rowp, colp);
    printf("  ECC 判定: %s\n",
           r3 == -1 ? "检测到错误但无法定位(多比特) -> 需更强 ECC(如 BCH/LDPC)"
                    : "可纠正");
    printf("  与原始数据一致: %s\n",
           memcmp(&readback, &original, sizeof(grid_t)) == 0 ? "是" : "否");

    printf("\n要点: 真实 SSD 用 BCH/LDPC 码, 可纠正几十~几百比特;\n");
    printf("      TLC/QLC 电荷窗口小更容易出错, 需要更强 ECC\n");
    return 0;
}
