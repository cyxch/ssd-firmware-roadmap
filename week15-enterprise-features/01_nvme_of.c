#include <stdio.h>

/*
 * NVMe over Fabrics (NVMe-oF) —— 远程访问 SSD。
 *
 * 架构: 应用 -> NVMe 驱动 -> 传输层(RDMA/FC/TCP) -> 网络
 *       -> 远端目标控制器 -> 本地 SSD
 *
 * 价值: 主机可以像访问本地盘一样访问"网络上的"SSD,
 *       实现集中管理、动态扩容、多机共享(数据中心场景)。
 * 代价: 每跳命令多一次网络往返(RTT), 延迟增加。
 *
 * 本实验: 对比本地 NVMe 与 NVMe-oF(RDMA/TCP) 的延迟分解。
 */

int main(void)
{
    /* 本地 NVMe 各阶段耗时(us, 量级) */
    const long drv = 2;    /* 主机驱动 */
    const long que = 5;    /* 队列交互 */
    const long ctrl = 20;  /* 控制器处理 */
    const long nand = 100; /* NAND 媒体 */
    /* 网络 RTT(us, 量级) */
    const long rdma_rtt = 10;   /* RDMA 低延迟网络 */
    const long tcp_rtt  = 100;  /* TCP 普通网络 */

    long local = drv + que + ctrl + nand;
    long of_rdma = local + rdma_rtt;
    long of_tcp  = local + tcp_rtt;

    printf("== 本地 NVMe 延迟分解 ==\n");
    printf("  驱动 %ld + 队列 %ld + 控制器 %ld + NAND %ld = %ld us\n\n",
           drv, que, ctrl, nand, local);

    printf("== NVMe-oF(每个命令多一次网络往返) ==\n");
    printf("  RDMA 传输: %ld + RTT %ld = %ld us (+%.0f%%)\n",
           local, rdma_rtt, of_rdma, 100.0 * rdma_rtt / local);
    printf("  TCP  传输: %ld + RTT %ld = %ld us (+%.0f%%)\n",
           local, tcp_rtt, of_tcp, 100.0 * tcp_rtt / local);

    printf("\n== 权衡 ==\n");
    printf("  本地 : 延迟最低, 但容量/管理受限于本机\n");
    printf("  NVMe-oF: 用网络延迟换取集中化、弹性扩展、共享\n");
    printf("  RDMA 适合延迟敏感场景; TCP 适合通用/远距离\n");

    printf("\n要点: NVMe-oF 是数据中心存储池化的核心,\n");
    printf("      固件工程师要理解“命令在网络上的封装与往返”\n");
    return 0;
}
