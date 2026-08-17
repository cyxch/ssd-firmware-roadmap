#include <stdio.h>

/*
 * 中断向量表模拟：模拟一个小型 MCU 的中断系统。
 * 概念：
 *   - 向量表：存放各中断源处理函数(ISR)地址的表格
 *   - 每个中断源有 挂起位(pending)/使能位(enabled)
 *   - CPU 检测到挂起的中断 -> 查向量表 -> 调用 ISR
 * 真实 MCU(如 Cortex-M)：NVIC 管理中断，向量表在固件启动时建立。
 */

#define IRQ_NUM 4

/* ISR 类型：无参无返回值 */
typedef void (*isr_t)(void);

/* 中断源控制块 */
typedef struct {
    int      pending;   /* 挂起位 */
    int      enabled;   /* 使能位 */
    int      priority;  /* 优先级(数值小优先) */
    isr_t    handler;   /* 指向向量表中的 ISR */
} irq_t;

static irq_t irqs[IRQ_NUM];

/* 三个示例 ISR */
static void isr_uart_rx(void) { printf("    [ISR] UART 收到数据\n"); }
static void isr_timer(void)   { printf("    [ISR] 定时器溢出\n"); }
static void isr_dma_done(void){ printf("    [ISR] DMA 搬运完成\n"); }

/* 注册 ISR 到向量表 */
static void irq_register(int irq, isr_t handler)
{
    irqs[irq].handler = handler;
    irqs[irq].enabled = 1;
    irqs[irq].pending = 0;
}

/* 硬件置挂起位(相当于触发一次中断请求) */
static void irq_raise(int irq)
{
    irqs[irq].pending = 1;
}

/* CPU 中断入口：扫描挂起且使能的中断，按优先级分发 */
static void cpu_dispatch(void)
{
    for (int pass = 0; pass < IRQ_NUM; pass++) {
        int best = -1;
        int best_pri = 0x7fffffff;
        for (int i = 0; i < IRQ_NUM; i++) {
            if (irqs[i].pending && irqs[i].enabled &&
                irqs[i].priority < best_pri) {
                best = i;
                best_pri = irqs[i].priority;
            }
        }
        if (best < 0)
            break;
        irqs[best].pending = 0;          /* 清挂起 */
        if (irqs[best].handler)
            irqs[best].handler();        /* 调用向量表里的 ISR */
    }
}

int main(void)
{
    irq_register(0, isr_uart_rx);
    irq_register(1, isr_timer);
    irq_register(2, isr_dma_done);
    irqs[0].priority = 2;
    irqs[1].priority = 0;   /* 定时器优先级最高 */
    irqs[2].priority = 1;

    printf("同时触发 UART 与 定时器 两个中断:\n");
    irq_raise(0);
    irq_raise(1);
    cpu_dispatch();

    printf("\n再次触发 DMA 完成中断:\n");
    irq_raise(2);
    cpu_dispatch();

    printf("\n观察: 优先级高的定时器先被处理(尽管后触发)\n");
    return 0;
}
