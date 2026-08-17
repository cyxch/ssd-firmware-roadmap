#include <stdio.h>
#include <string.h>

/*
 * SSD 掉电风险点分析。
 *
 * SSD 内部有三个"易失"的地方，掉电时可能丢数据:
 *   1. DRAM 缓存: 主机写的数据先缓存在 DRAM, 掉电就丢
 *                 (dirty cache / write back 缓冲)
 *   2. 映射表(L2P): 页级映射表在 DRAM, 掉电丢失
 *                 -> 若没有持久化, 开机后无法知道数据在哪
 *   3. 进行中的写/擦: 正在写页/擦块时掉电, 页数据不完整
 *
 * 本实验: 模拟一次掉电, 观察"哪些数据会丢/哪些能保住",
 * 建立对掉电风险的整体认识(下个实验讲如何保护)。
 */

#define CACHE_MAX 4

/* 主机写入序列: 是否被确认(ack)给主机 */
typedef struct {
    int lba;
    int confirmed;   /* 1=已确认(承诺持久化), 0=还在缓存未确认 */
} pending_t;

/* 模拟一次掉电: 返回丢了多少条未确认数据 */
static int simulate_power_loss(const pending_t *pending, int n)
{
    int lost = 0;
    for (int i = 0; i < n; i++) {
        if (!pending[i].confirmed) {
            printf("  [丢失] LBA %d 尚未确认(在 DRAM 缓存), 掉电丢失!\n",
                   pending[i].lba);
            lost++;
        } else {
            printf("  [保住] LBA %d 已确认(已落盘/已持久化)\n", pending[i].lba);
        }
    }
    return lost;
}

int main(void)
{
    /* 模拟主机连续写 6 次, 前 2 次已确认, 后 4 次还在缓存 */
    pending_t writes[6];
    for (int i = 0; i < 6; i++) {
        writes[i].lba = i;
        writes[i].confirmed = (i < 2) ? 1 : 0;
    }

    printf("主机写入了 6 次(前 2 次已确认, 后 4 次在 DRAM 缓存未确认)\n\n");
    printf("== 突然掉电 ==\n");
    int lost = simulate_power_loss(writes, 6);

    printf("\n丢失 %d 条数据, 保住 %d 条\n", lost, 6 - lost);
    printf("\n风险总结:\n");
    printf("  1. DRAM 里的 dirty 数据(未确认)掉电必丢\n");
    printf("  2. 映射表在 DRAM, 掉电后无法定位数据(需从 NAND 重建)\n");
    printf("  3. 正在写的页可能“半写”不完整(需 ECC/冗余校验)\n");
    printf("\n对策: 掉电保护(PLP) + 日志(journal) + 映射表持久化(见后文)\n");
    return 0;
}
