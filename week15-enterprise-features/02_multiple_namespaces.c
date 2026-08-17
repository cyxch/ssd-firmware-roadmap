#include <stdio.h>
#include <string.h>

/*
 * 多命名空间(Namespace) —— 一个控制器管理多个独立逻辑盘。
 *
 * 概念: 一块物理 SSD 可以被分成多个"命名空间"(Namespace, NS),
 * 每个 NS 有:
 *   - 独立的 LBA 地址空间(从 0 开始, 与别的 NS 互不干扰)
 *   - 独立的容量
 *   - 独立的逻辑块大小(512B / 4K)
 * 主机命令里的 NSID 字段(见 12 周)决定操作哪个 NS。
 *
 * 用途: 多租户隔离、一块盘当多块用、不同应用不同块大小。
 * 本实验: 创建 3 个 NS, 演示 (nsid, lba) 唯一定位数据。
 */

#define NS_NUM 3

typedef struct {
    int nsid;
    const char *name;
    long long capacity;     /* 逻辑块数 */
    int lba_size;           /* 每块字节 */
    int active;             /* 是否在线 */
} ns_t;

static ns_t nss[NS_NUM];

static void ns_init(void)
{
    nss[0].nsid = 1; nss[0].name = "租户A-系统盘";  nss[0].capacity = 5000000; nss[0].lba_size = 512; nss[0].active = 1;
    nss[1].nsid = 2; nss[1].name = "租户B-数据库";  nss[1].capacity = 10000000; nss[1].lba_size = 4096; nss[1].active = 1;
    nss[2].nsid = 3; nss[2].name = "租户C-备份";    nss[2].capacity = 2000000;  nss[2].lba_size = 512;  nss[2].active = 0;
}

/* 把 (nsid, lba) 转成"全局物理地址"(简化: 各 NS 从 0 独立编号) */
static void resolve(int nsid, long long lba)
{
    int found = -1;
    for (int i = 0; i < NS_NUM; i++)
        if (nss[i].nsid == nsid) found = i;
    if (found < 0) {
        printf("  NSID %d 不存在!\n", nsid);
        return;
    }
    if (!nss[found].active) {
        printf("  NSID %d 未上线(offline)\n", nsid);
        return;
    }
    if (lba >= nss[found].capacity) {
        printf("  NSID %d 的 LBA %lld 越界(容量 %lld 块)\n",
               nsid, lba, nss[found].capacity);
        return;
    }
    printf("  NSID %d -> LBA %lld 有效: 属于 %s, 块大小 %d B\n",
           nsid, lba, nss[found].name, nss[found].lba_size);
}

int main(void)
{
    ns_init();

    printf("== 控制器下的命名空间 ==\n");
    for (int i = 0; i < NS_NUM; i++)
        printf("  NSID=%d %-16s 容量 %8lld 块  块大小 %5d B  %s\n",
               nss[i].nsid, nss[i].name, nss[i].capacity, nss[i].lba_size,
               nss[i].active ? "在线" : "离线");

    printf("\n== 地址解析: 同一个 LBA 在不同 NS 是不同位置 ==\n");
    resolve(1, 100);
    resolve(2, 100);   /* 与 NSID 1 的 LBA100 完全不同 */
    resolve(2, 99999999); /* 越界 */
    resolve(3, 0);        /* 离线 */
    resolve(9, 0);        /* 不存在 */

    printf("\n要点: 物理一块盘, 逻辑多块盘; (nsid, lba) 唯一寻址;\n");
    printf("      多命名空间是企业级多租户隔离的基础\n");
    return 0;
}
