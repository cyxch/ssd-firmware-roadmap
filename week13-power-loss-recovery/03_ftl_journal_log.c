#include <stdio.h>
#include <string.h>

/*
 * 日志(Journal/Log)机制与崩溃恢复 —— 掉电时保住元数据。
 *
 * 问题: 掉电时映射表更新可能只写了一半(页半写), 或还没来得及写。
 * 方案: 写数据前先记一条"日志"(journal entry)到 NAND,
 *       记录"我要把 LBA X 放到物理页 P"。
 *       即使掉电, 开机时重放(playback)日志, 即可重建映射。
 *
 * 关键: 日志必须先于数据落盘(先写日志, 再写数据), 才能保证一致性。
 * 这叫 write-ahead logging (WAL)。
 *
 * 本实验模拟: 若干条日志 -> 中途掉电 -> 开机重放 -> 恢复映射表。
 */

#define LOG_MAX 16

typedef struct {
    int valid;      /* 该条日志是否有效 */
    int lba;
    int pba;
    int seq;        /* 日志序号, 用于判断先后 */
} log_entry_t;

static log_entry_t journal[LOG_MAX];
static int next_seq = 0;

/* 追加一条日志(模拟写入 NAND 日志区) */
static void log_append(int lba, int pba)
{
    for (int i = 0; i < LOG_MAX; i++) {
        if (!journal[i].valid) {
            journal[i].valid = 1;
            journal[i].lba = lba;
            journal[i].pba = pba;
            journal[i].seq = next_seq++;
            return;
        }
    }
}

/* 开机恢复: 重放日志, 重建"LBA -> PBA"映射 */
static void replay(int map[/* 逻辑空间 */8])
{
    /* 按 seq 从小到大重放; 相同 LBA 取最新(seq 大)的一条 */
    printf("== 开机: 重放日志(playback)重建映射 ==\n");
    for (int pass = 0; pass < LOG_MAX; pass++) {
        /* 找到 seq 最小的有效日志 */
        int best = -1, best_seq = 0x7fffffff;
        for (int i = 0; i < LOG_MAX; i++) {
            if (journal[i].valid && journal[i].seq < best_seq) {
                best = i;
                best_seq = journal[i].seq;
            }
        }
        if (best < 0)
            break;
        journal[best].valid = 0;   /* 消费掉 */
        /* 重放: 更新映射(同 LBA 后写覆盖先写) */
        if (journal[best].lba >= 0 && journal[best].lba < 8)
            map[journal[best].lba] = journal[best].pba;
        printf("  重放 seq=%d: LBA %d -> 物理页 %d\n",
               journal[best].seq, journal[best].lba, journal[best].pba);
    }
}

int main(void)
{
    int map[8];
    memset(map, 0, sizeof(map));   /* 初始全映射到 0 */

    printf("== 正常工作: 写数据 + 记日志 ==\n");
    log_append(0, 10);
    log_append(1, 20);
    log_append(0, 30);   /* LBA 0 更新: 最新映射应指向 30 */
    log_append(2, 40);

    printf("  已写入 4 条日志(最后一条 seq=%d)\n\n", next_seq - 1);

    printf("== 突然掉电! 映射表还没来得及刷盘 ==\n");
    printf("  但日志区已先落盘(WAL) -> 可恢复\n\n");

    replay(map);

    printf("\n== 恢复后的映射表 ==\n");
    for (int l = 0; l < 8; l++)
        printf("  LBA %d -> 物理页 %d\n", l, map[l]);

    printf("\n要点: WAL 先写日志后写数据, 掉电重放即可恢复;\n");
    printf("      相同 LBA 多条日志取最后一条(seq 最大) —— 一致性保证\n");
    return 0;
}
