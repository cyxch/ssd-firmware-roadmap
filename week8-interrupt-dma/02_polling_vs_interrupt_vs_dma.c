#include <stdio.h>

/*
 * 核心实验：轮询 / 中断 / DMA 三种接收方式的 CPU 开销对比。
 * 场景：外设(UART)慢速到达 N 个字节，比较三种方式各消耗多少 CPU 周期。
 *
 * 结论预期：
 *   - 轮询 : CPU 一直忙等状态位，字节越多浪费越多
 *   - 中断 : 每个字节有进入/退出中断的开销，但等待期间 CPU 可干别的
 *   - DMA  : 硬件自己搬运，CPU 只需一次配置 + 一次完成中断，几乎免费
 */

#define N 1000   /* 接收字节数 */
#define WAIT_PER_BYTE 100   /* 模拟外设每字节到达需要等待的"周期" */

/* 抽象的 CPU 周期计数器 */
static long g_cycles = 0;

/* 模拟读取一个数据寄存器 */
static int read_data_reg(void)
{
    return 0x41;   /* 假装收到字符 'A' */
}

/* ---------- 方式 1：轮询 ---------- */
static long do_polling(void)
{
    long start = g_cycles;
    int data[N];

    for (int i = 0; i < N; i++) {
        /* 忙等：反复读状态寄存器直到 RXNE 置位（这里用 WAIT 模拟轮询开销） */
        g_cycles += WAIT_PER_BYTE;
        data[i] = read_data_reg();   /* 读数据 */
        g_cycles += 5;               /* 存到数组 */
    }
    (void)data;
    return g_cycles - start;
}

/* ---------- 方式 2：中断 ---------- */
static long do_interrupt(void)
{
    long start = g_cycles;
    int data[N];

    for (int i = 0; i < N; i++) {
        /* 外设到达 -> 触发中断; 等待期间 CPU 可做别的事(这里不占周期) */
        g_cycles += WAIT_PER_BYTE;   /* 等待期间 CPU 在跑其他任务(不计阻塞) */

        /* 进入中断: 压栈/保存现场/向量跳转/弹栈 */
        g_cycles += 12;
        data[i] = read_data_reg();   /* ISR 里读数据 */
        g_cycles += 5;
    }
    (void)data;
    return g_cycles - start;
}

/* ---------- 方式 3：DMA ---------- */
static long do_dma(void)
{
    long start = g_cycles;
    int data[N];

    /* 一次性配置 DMA 描述符(源地址=外设寄存器, 目的=数组, 长度=N) */
    g_cycles += 20;
    /* 硬件自动搬运 N 字节，每字节 0 CPU 周期 */
    for (int i = 0; i < N; i++)
        data[i] = read_data_reg();    /* 这是"硬件"在做，不占 CPU */
    /* DMA 完成 -> 一次完成中断 */
    g_cycles += 12;
    (void)data;
    return g_cycles - start;
}

int main(void)
{
    long c_poll = do_polling();
    long c_int  = do_interrupt();
    long c_dma  = do_dma();

    printf("接收 %d 字节，三种方式 CPU 周期开销:\n", N);
    printf("  轮询 : %ld 周期 (CPU 全程忙等)\n", c_poll);
    printf("  中断 : %ld 周期 (等待期 CPU 可做别的事)\n", c_int);
    printf("  DMA  : %ld 周期 (几乎只有配置+完成中断)\n", c_dma);

    printf("\n对比倍数:\n");
    printf("  轮询是 DMA 的 %.0f 倍\n", (double)c_poll / c_dma);
    printf("  中断是 DMA 的 %.0f 倍\n", (double)c_int / c_dma);
    printf("  轮询是中断的 %.1f 倍\n", (double)c_poll / c_int);

    printf("\n要点: 周期开销只是其一;\n");
    printf("  轮询还会“卡死”CPU(无法响应实时事件)，中断/DMA 才能保障实时性\n");
    return 0;
}
