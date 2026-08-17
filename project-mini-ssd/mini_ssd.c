#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* ==================================================================
 * Mini SSD 模拟器 —— 综合项目【交付物】
 *
 * 把 15 周的知识串成一条完整可运行的 SSD 固件链路:
 *   NAND 层     (第10周) : 块/页模型, 页读写擦, 出厂坏块
 *   FTL 层      (第11周) : L2P 映射, out-of-place 写, 垃圾回收(GC),
 *                          动态+静态磨损均衡, 写放大(WAF)
 *   NVMe 队列   (第12周) : 命令接口(READ/WRITE/FLUSH), 优先级队列
 *   DRAM 缓存+PLP(第13周): 写回缓存, 掉电丢失未提交数据, 上电重建映射
 *   QoS 调度    (第14周) : 高优先级命令(FLUSH/管理)插队
 *   SMART 遥测  (第15周) : 健康状态, PE 分布, 剩余寿命, WAF
 *
 * 编译: gcc -O2 -Wall -Wextra -o ssd mini_ssd.c && ./ssd
 * ================================================================== */

/* ---------------- 1. 配置 ---------------- */
#define BLOCKS 32              /* 物理块数 */
#define PAGES_PER_BLOCK 8      /* 每块页数 */
#define PG_SIZE 16             /* 每页字节(简化) */
#define N_PAGES (BLOCKS * PAGES_PER_BLOCK)   /* 256 物理页 */
#define N_LBA 128              /* 逻辑地址空间(128 个 LBA) */
#define CACHE_CAP 16           /* DRAM 写缓存条数 */
#define PE_LIMIT 100           /* 块 P/E 上限 */

enum { PAGE_FREE, PAGE_VALID, PAGE_INVALID };

/* ---------------- 2. 数据结构 ---------------- */
typedef struct {
    int state;                 /* 空闲/有效/作废 */
    int lba;                   /* 存放的 LBA, -1=无 */
    unsigned char data[PG_SIZE];
    unsigned char crc;         /* 数据完整性校验 */
} page_t;

typedef struct {
    page_t pages[PAGES_PER_BLOCK];
    int pe;                    /* 擦写次数 */
    int bad;                   /* 坏块 */
} block_t;

static block_t nand[BLOCKS];
static int l2p[N_LBA];         /* LBA -> 物理页, -1 = 未映射 */

typedef struct {
    int valid;
    int lba;
    unsigned char data;        /* 每 LBA 一个字节数据(简化) */
} cache_t;
static cache_t cache[CACHE_CAP];

/* ---------------- 3. 统计(供 SMART) ---------------- */
static long long g_host_writes;   /* 主机写入次数 */
static long long g_nand_writes;   /* NAND 实际编程次数(含 GC 搬移) */
static long long g_reads;         /* 主机读次数 */
static long long g_cache_hits;    /* 缓存命中(写后立即读) */
static long long g_ecc_fail;      /* 完整性校验失败 */
static long long g_gc_moves;      /* GC 搬移的有效页数 */

/* ---------------- 4. NAND 层 ---------------- */
static void nand_init(void)
{
    memset(nand, 0, sizeof(nand));
    for (int b = 0; b < BLOCKS; b++)
        for (int p = 0; p < PAGES_PER_BLOCK; p++) {
            nand[b].pages[p].state = PAGE_FREE;
            nand[b].pages[p].lba = -1;
        }
    /* 模拟 2 个出厂坏块 */
    nand[5].bad = 1;
    nand[21].bad = 1;
}

/* 擦除整块: 恢复为全空闲, P/E +1 */
static void erase_block(int b)
{
    for (int p = 0; p < PAGES_PER_BLOCK; p++) {
        nand[b].pages[p].state = PAGE_FREE;
        nand[b].pages[p].lba = -1;
        memset(nand[b].pages[p].data, 0xFF, PG_SIZE);
        nand[b].pages[p].crc = 0;
    }
    nand[b].pe++;
}

static int free_pages(void)
{
    int n = 0;
    for (int b = 0; b < BLOCKS; b++) {
        if (nand[b].bad)
            continue;
        for (int p = 0; p < PAGES_PER_BLOCK; p++)
            if (nand[b].pages[p].state == PAGE_FREE)
                n++;
    }
    return n;
}

/* ---------------- 5. FTL 层 ---------------- */
/* 动态磨损均衡: 在有空闲页的块里选 P/E 最少的, 返回物理页号 */
static int find_free_page(void)
{
    int best_b = -1, best_pe = 1 << 30;
    for (int b = 0; b < BLOCKS; b++) {
        if (nand[b].bad)
            continue;
        int has_free = 0;
        for (int p = 0; p < PAGES_PER_BLOCK; p++)
            if (nand[b].pages[p].state == PAGE_FREE) { has_free = 1; break; }
        if (!has_free)
            continue;
        if (nand[b].pe < best_pe) { best_pe = nand[b].pe; best_b = b; }
    }
    if (best_b < 0)
        return -1;
    for (int p = 0; p < PAGES_PER_BLOCK; p++)
        if (nand[best_b].pages[p].state == PAGE_FREE)
            return best_b * PAGES_PER_BLOCK + p;
    return -1;
}

/* 垃圾回收: 优先回收"含无效页且有效页最少"的块, 搬有效页, 擦除 */
static void gc(void)
{
    int victim = -1, min_valid = PAGES_PER_BLOCK + 1, found = 0;
    for (int b = 0; b < BLOCKS; b++) {
        if (nand[b].bad)
            continue;
        int v = 0, inv = 0;
        for (int p = 0; p < PAGES_PER_BLOCK; p++) {
            int st = nand[b].pages[p].state;
            if (st == PAGE_VALID) v++;
            if (st == PAGE_INVALID) inv++;
        }
        if (inv > 0 && v < min_valid) { victim = b; min_valid = v; found = 1; }
    }
    /* 兜底: 全盘都有效时也选有效页最少的 */
    if (!found) {
        for (int b = 0; b < BLOCKS; b++) {
            if (nand[b].bad)
                continue;
            int v = 0;
            for (int p = 0; p < PAGES_PER_BLOCK; p++)
                if (nand[b].pages[p].state == PAGE_VALID)
                    v++;
            if (v > 0 && v < min_valid) { victim = b; min_valid = v; }
        }
    }
    if (victim < 0)
        return;

    int moved = 0;
    for (int p = 0; p < PAGES_PER_BLOCK; p++) {
        page_t *pg = &nand[victim].pages[p];
        if (pg->state != PAGE_VALID)
            continue;
        int pba = find_free_page();
        if (pba < 0)
            break;
        page_t *np = &nand[pba / PAGES_PER_BLOCK].pages[pba % PAGES_PER_BLOCK];
        np->state = PAGE_VALID;
        np->lba = pg->lba;
        memcpy(np->data, pg->data, PG_SIZE);
        np->crc = pg->crc;
        l2p[pg->lba] = pba;      /* 搬移后更新映射 */
        g_nand_writes++;         /* 搬移 = 额外 NAND 写 */
        moved++;
        pg->state = PAGE_INVALID;
    }
    erase_block(victim);         /* 擦除后恢复为全空闲 */
    g_gc_moves += moved;
}

/* FTL 写: out-of-place 提交(数据真正落 NAND) */
static int ftl_commit(int lba, unsigned char data)
{
    if (lba < 0 || lba >= N_LBA)
        return -1;
    /* out-of-place: 旧页作废 */
    if (l2p[lba] >= 0) {
        page_t *old = &nand[l2p[lba] / PAGES_PER_BLOCK]
                           .pages[l2p[lba] % PAGES_PER_BLOCK];
        if (old->state == PAGE_VALID)
            old->state = PAGE_INVALID;
    }
    /* 低水位触发 GC */
    if (free_pages() <= PAGES_PER_BLOCK)
        gc();
    int pba = find_free_page();
    if (pba < 0) {
        gc();
        pba = find_free_page();
    }
    if (pba < 0)
        return -1;
    page_t *pg = &nand[pba / PAGES_PER_BLOCK].pages[pba % PAGES_PER_BLOCK];
    pg->state = PAGE_VALID;
    pg->lba = lba;
    memset(pg->data, 0, PG_SIZE);
    pg->data[0] = data;
    pg->crc = data;              /* 简化完整性校验 */
    l2p[lba] = pba;
    g_nand_writes++;
    return 0;
}

/* FTL 读: 查映射表, 校验 CRC */
static int ftl_read(int lba, unsigned char *out)
{
    if (lba < 0 || lba >= N_LBA)
        return -1;
    int pba = l2p[lba];
    if (pba < 0)
        return -1;
    page_t *pg = &nand[pba / PAGES_PER_BLOCK].pages[pba % PAGES_PER_BLOCK];
    if (pg->state != PAGE_VALID)
        return -1;
    if (pg->crc != pg->data[0]) { g_ecc_fail++; return -1; }
    *out = pg->data[0];
    g_reads++;
    return 0;
}

/* 静态磨损均衡: 把最冷(PE 最低)的有效数据块迁移并擦除, 抬升其 PE */
static void static_wl_cycle(void)
{
    int coldest = -1;
    for (int b = 0; b < BLOCKS; b++) {
        if (nand[b].bad)
            continue;
        int has_valid = 0;
        for (int p = 0; p < PAGES_PER_BLOCK; p++)
            if (nand[b].pages[p].state == PAGE_VALID) { has_valid = 1; break; }
        if (!has_valid)
            continue;
        if (coldest < 0 || nand[b].pe < nand[coldest].pe)
            coldest = b;
    }
    if (coldest < 0)
        return;
    for (int p = 0; p < PAGES_PER_BLOCK; p++) {
        page_t *pg = &nand[coldest].pages[p];
        if (pg->state != PAGE_VALID)
            continue;
        int pba = find_free_page();
        if (pba < 0)
            break;
        page_t *np = &nand[pba / PAGES_PER_BLOCK].pages[pba % PAGES_PER_BLOCK];
        np->state = PAGE_VALID;
        np->lba = pg->lba;
        memcpy(np->data, pg->data, PG_SIZE);
        np->crc = pg->crc;
        l2p[pg->lba] = pba;
        g_nand_writes++;
        pg->state = PAGE_INVALID;
    }
    erase_block(coldest);        /* 冷块经历一次 P/E */
}

/* ---------------- 6. DRAM 写缓存 + 掉电保护 ---------------- */
/* 前向声明 */
static int flush_cache(void);

static int cache_write(int lba, unsigned char data)
{
    /* 命中已有: 就地更新 */
    for (int i = 0; i < CACHE_CAP; i++)
        if (cache[i].valid && cache[i].lba == lba) {
            cache[i].data = data;
            return 0;
        }
    /* 新增条目 */
    for (int i = 0; i < CACHE_CAP; i++)
        if (!cache[i].valid) {
            cache[i].valid = 1;
            cache[i].lba = lba;
            cache[i].data = data;
            g_host_writes++;
            return 0;
        }
    /* 缓存满: 自动刷出(write-back 缓存的 eviction)后重试 */
    flush_cache();
    for (int i = 0; i < CACHE_CAP; i++)
        if (!cache[i].valid) {
            cache[i].valid = 1;
            cache[i].lba = lba;
            cache[i].data = data;
            g_host_writes++;
            return 0;
        }
    return -1;               /* 异常: 理论上不会到这里 */
}

static int cache_read(int lba, unsigned char *out)
{
    for (int i = 0; i < CACHE_CAP; i++)
        if (cache[i].valid && cache[i].lba == lba) {
            *out = cache[i].data;
            g_cache_hits++;
            return 0;
        }
    return -1;
}

/* 主机读: 先查缓存(写后立即读), 再查 NAND */
static int read_lba(int lba, unsigned char *out)
{
    if (cache_read(lba, out) == 0)
        return 0;
    return ftl_read(lba, out);
}

/* FLUSH: 把缓存全部提交到 NAND */
static int flush_cache(void)
{
    int n = 0;
    for (int i = 0; i < CACHE_CAP; i++)
        if (cache[i].valid) {
            if (ftl_commit(cache[i].lba, cache[i].data) == 0)
                n++;
            cache[i].valid = 0;
        }
    return n;
}

/* 掉电: 未提交的缓存数据丢失(未确认语义) */
static void power_loss(void)
{
    int lost = 0;
    for (int i = 0; i < CACHE_CAP; i++)
        if (cache[i].valid) {
            printf("  [掉电] LBA %d 未提交, 丢失!\n", cache[i].lba);
            lost++;
            cache[i].valid = 0;
        }
    printf("  [掉电] 共丢失 %d 条未提交数据(已确认数据不受影响)\n", lost);
}

/* 上电: 清空缓存, 扫描 NAND 重建映射表 */
static void power_on_recovery(void)
{
    memset(cache, 0, sizeof(cache));
    for (int l = 0; l < N_LBA; l++)
        l2p[l] = -1;
    int rebuilt = 0;
    for (int b = 0; b < BLOCKS; b++) {
        if (nand[b].bad)
            continue;
        for (int p = 0; p < PAGES_PER_BLOCK; p++) {
            page_t *pg = &nand[b].pages[p];
            if (pg->state != PAGE_VALID)
                continue;
            if (pg->crc != pg->data[0]) { g_ecc_fail++; continue; } /* 半写页 */
            l2p[pg->lba] = b * PAGES_PER_BLOCK + p;
            rebuilt++;
        }
    }
    printf("  [上电] 扫描重建映射表: 恢复 %d 个有效页\n", rebuilt);
}

/* ---------------- 7. 命令队列 + QoS 优先级 ---------------- */
/* 前向声明 */
static void smart_report(void);
#define CMDQ_CAP 16
typedef struct {
    int opc;      /* 0=READ 1=WRITE 2=FLUSH 3=POWER_LOSS 4=POWER_ON 5=SMART */
    int lba;
    unsigned char data;
    int pri;      /* 0=高 1=普通 */
} cmd_t;

static cmd_t cmdq[CMDQ_CAP];
static int cmdq_n;

/* 前向声明 */
static void cmd_dispatch(void);

/* 提交命令: 高优先级插到队首(QoS); 队列满时先处理一批 */
static int cmd_submit(int opc, int lba, unsigned char data, int pri)
{
    if (cmdq_n >= CMDQ_CAP)
        cmd_dispatch();          /* 队列满: 先执行已有命令腾出空间 */
    if (cmdq_n >= CMDQ_CAP)
        return -1;
    int pos = cmdq_n;
    if (pri == 0) {
        for (int i = cmdq_n; i > 0; i--)
            cmdq[i] = cmdq[i - 1];
        pos = 0;
    }
    cmdq[pos].opc = opc;
    cmdq[pos].lba = lba;
    cmdq[pos].data = data;
    cmdq[pos].pri = pri;
    cmdq_n++;
    return 0;
}

static int cmd_execute(const cmd_t *c)
{
    switch (c->opc) {
    case 0: {
        unsigned char v;
        int ok = read_lba(c->lba, &v);
        if (ok == 0)
            printf("    读 LBA %d -> 0x%02X\n", c->lba, v);
        return ok;
    }
    case 1:
        return cache_write(c->lba, c->data);
    case 2:
        return flush_cache();
    case 3:
        power_loss();
        return 0;
    case 4:
        power_on_recovery();
        return 0;
    case 5:
        smart_report();
        return 0;
    default:
        return -1;
    }
}

/* 分发执行队列中的命令(FIFO, 但高优先级已在队首) */
static void cmd_dispatch(void)
{
    while (cmdq_n > 0) {
        cmd_t c = cmdq[0];
        for (int i = 0; i < cmdq_n - 1; i++)
            cmdq[i] = cmdq[i + 1];
        cmdq_n--;
        cmd_execute(&c);
    }
}

/* ---------------- 8. SMART 遥测 ---------------- */
static void smart_report(void)
{
    int bad = 0, min_pe = 1 << 30, max_pe = 0;
    double sum = 0, sum2 = 0, n = 0;
    for (int b = 0; b < BLOCKS; b++) {
        if (nand[b].bad) { bad++; continue; }
        if (nand[b].pe < min_pe) min_pe = nand[b].pe;
        if (nand[b].pe > max_pe) max_pe = nand[b].pe;
        sum += nand[b].pe;
        sum2 += (double)nand[b].pe * nand[b].pe;
        n++;
    }
    double avg = sum / n;
    double variance = sum2 / n - avg * avg;
    if (variance < 0) variance = 0;
    double waf = g_host_writes ? (double)g_nand_writes / g_host_writes : 0;

    printf("\n========== SMART 健康报告 ==========\n");
    printf("  主机读次数      : %lld\n", g_reads);
    printf("  主机写次数      : %lld\n", g_host_writes);
    printf("  NAND 编程次数   : %lld (含 GC 搬移 %lld)\n", g_nand_writes, g_gc_moves);
    printf("  写放大系数 WAF  : %.2f\n", waf);
    printf("  缓存命中        : %lld 次\n", g_cache_hits);
    printf("  完整性校验失败  : %lld 次%s\n", g_ecc_fail, g_ecc_fail ? " (!)" : "");
    printf("  坏块            : %d / %d\n", bad, BLOCKS);
    printf("  块 P/E 分布      : min=%d max=%d 平均=%.0f 方差=%.0f\n",
           min_pe, max_pe, avg, variance);
    printf("  剩余寿命(按 P/E) : %.0f%%\n",
           100.0 * (PE_LIMIT - max_pe) / PE_LIMIT);
    printf("======================================\n");
}

/* 校验一组 LBA 的读回值是否等于期望 */
static void verify_range(const char *tag, int from, int to,
                         const unsigned char *expect)
{
    int bad = 0;
    for (int l = from; l <= to; l++) {
        unsigned char v = 0;
        if (expect[l] == 0xFF)
            continue;              /* 该 LBA 未写过, 跳过 */
        if (read_lba(l, &v) != 0 || v != expect[l])
            bad++;
    }
    printf("  %s: 校验 LBA %d..%d -> %s\n", tag, from, to,
           bad ? "有错误!" : "全部通过");
}

/* ---------------- main: 端到端工作负载 ---------------- */
int main(void)
{
    unsigned char expect[N_LBA];

    srand(2026);
    memset(expect, 0xFF, sizeof(expect));
    nand_init();
    for (int l = 0; l < N_LBA; l++)
        l2p[l] = -1;
    memset(cache, 0, sizeof(cache));

    printf("========== Mini SSD 模拟器 ==========\n");
    printf("  NAND : %d 块 x %d 页 = %d 物理页 (2 个出厂坏块)\n",
           BLOCKS, PAGES_PER_BLOCK, N_PAGES);
    printf("  逻辑 : %d 个 LBA, DRAM 缓存 %d 条, P/E 上限 %d\n\n",
           N_LBA, CACHE_CAP, PE_LIMIT);

    /* ---- 命令路径演示: 缓存命中 + QoS 插队 ---- */
    printf("\n== 命令路径演示 ==\n");
    cmd_submit(1, 30, 0x5A, 1);
    cmd_dispatch();                         /* 写进 DRAM 缓存 */
    cmd_submit(0, 30, 0, 1);
    cmd_dispatch();                         /* 立即读: 缓存命中 */
    cmd_submit(1, 20, 0xCC, 1);             /* 普通写(先进队) */
    cmd_submit(1, 21, 0xDD, 1);             /* 普通写 */
    cmd_submit(0, 30, 0, 0);                /* 高优先级读(插到队首) */
    cmd_dispatch();
    printf("  (上一条 READ 高优先级, 越过 2 条排队中的普通 WRITE 先执行)\n");

    /* ---- 阶段1: 顺序写 + 读回校验 ---- */
    printf("\n== 阶段1: 顺序写 LBA 0..63(经 DRAM 缓存) ==\n");
    for (int l = 0; l < 64; l++) {
        expect[l] = (unsigned char)(l & 0xFF);
        cmd_submit(1, l, expect[l], 1);
    }
    cmd_dispatch();
    cmd_submit(2, 0, 0, 1);        /* FLUSH(普通优先级, 按序执行) */
    cmd_dispatch();
    verify_range("顺序写后", 0, 63, expect);

    /* ---- 阶段2: 随机写 + 读回校验 ---- */
    printf("\n== 阶段2: 随机写 LBA 0..127 ==\n");
    for (int i = 0; i < 128; i++) {
        int l = rand() % N_LBA;
        expect[l] = (unsigned char)(rand() & 0xFF);
        cmd_submit(1, l, expect[l], 1);
    }
    cmd_dispatch();
    cmd_submit(2, 0, 0, 1);
    cmd_dispatch();
    verify_range("随机写后", 0, 127, expect);

    /* ---- 阶段3: 大量随机重写, 触发 GC, 观察 WAF ---- */
    printf("\n== 阶段3: 大量随机重写(触发垃圾回收) ==\n");
    for (int pass = 0; pass < 10; pass++) {
        for (int i = 0; i < N_LBA; i++) {
            int l = rand() % N_LBA;
            expect[l] = (unsigned char)(rand() & 0xFF);
            cmd_submit(1, l, expect[l], 1);
            if ((pass * N_LBA + i) % 32 == 31)
                cmd_submit(2, 0, 0, 1);   /* 定期 FLUSH(按序) */
        }
        cmd_submit(2, 0, 0, 1);
    }
    cmd_dispatch();
    verify_range("重写后", 0, 127, expect);
    printf("  写放大 WAF = %.2f (NAND 编程 %lld / 主机写 %lld)\n",
           (double)g_nand_writes / g_host_writes, g_nand_writes, g_host_writes);

    /* ---- 阶段4: 静态磨损均衡(冷数据块) ---- */
    printf("\n== 阶段4: 静态磨损均衡 ==\n");
    /* 模拟一块"冷数据": 写入后不再更新, PE 停留在低位 */
    for (int l = 100; l < 108; l++) {
        expect[l] = 0xAB;
        cmd_submit(1, l, expect[l], 1);
    }
    cmd_submit(2, 0, 0, 1);    /* FLUSH(按序提交冷数据) */
    cmd_dispatch();
    printf("  执行 10 轮静态磨损均衡(迁移冷块数据并擦除冷块)\n");
    for (int i = 0; i < 10; i++)
        static_wl_cycle();
    verify_range("冷数据后", 100, 107, expect);

    /* ---- 阶段5: 掉电 + 上电恢复 ---- */
    printf("\n== 阶段5: 掉电保护与恢复 ==\n");
    /* 新写入进缓存但不 FLUSH(掉电时会丢失, 属未确认数据) */
    for (int l = 0; l < 8; l++)
        cmd_submit(1, l, (unsigned char)(0xE0 + l), 1);
    cmd_dispatch();
    printf("  突然掉电...\n");
    cmd_submit(3, 0, 0, 0);     /* 掉电: 未提交缓存丢失 */
    cmd_dispatch();
    cmd_submit(4, 0, 0, 0);     /* 上电: 扫描重建映射表 */
    cmd_dispatch();
    printf("  校验已提交数据(阶段2/3 写的数据)仍完好:\n");
    verify_range("恢复后", 0, 99, expect);

    /* ---- 阶段6: SMART 报告 ---- */
    smart_report();

    printf("\n========== 模拟器运行结束 ==========\n");
    return 0;
}
