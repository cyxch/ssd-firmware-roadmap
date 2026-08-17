#include <stdio.h>
#include <signal.h>

/*
 * 用"信号"模拟中断：signal() 注册的处理函数等价于 ISR，
 * raise() 等价于硬件触发一次中断。
 *
 * 关键点——共享变量：
 *   ISR(信号处理)与主程序共享的变量应声明为
 *   `volatile sig_atomic_t`（保证读写在原子层面、且编译器不会缓存）。
 * 如果去掉 volatile 且开优化(-O2)，编译器可能把"主循环读取"缓存到寄存器，
 * 导致主程序永远看不到 ISR 的修改 —— 嵌入式经典 bug。
 */

static volatile sig_atomic_t g_irq_count = 0;   /* ISR 与 main 共享 */

/* 这个"ISR"：收到信号(SIGINT)时执行 */
static void my_isr(int sig)
{
    (void)sig;
    /*
     * 重新安装处理函数(re-arm)：
     * Windows 上 signal() 安装的一次性处理函数在触发后会复位，
     * 若不重新安装，下一次 SIGINT 将执行默认动作(终止进程)。
     * 对应真实 MCU：一次性中断(如边沿触发)需在 ISR 里重新使能。
     */
    signal(SIGINT, my_isr);
    g_irq_count++;                      /* 修改共享变量 */
    printf("    [ISR] 收到中断，累计 %d 次\n", g_irq_count);
    fflush(stdout);                     /* 立即刷出，避免缓冲干扰观察 */
}

int main(void)
{
    /* 把 SIGINT 绑到我们的 ISR（真实 MCU 里这是写 NVIC + 向量表） */
    signal(SIGINT, my_isr);

    printf("主程序：准备触发 3 次软件中断 (raise)\n");
    raise(SIGINT);                      /* 触发中断 */
    raise(SIGINT);
    raise(SIGINT);

    printf("主程序：g_irq_count = %d（应为 3）\n", g_irq_count);

    printf("\n实验：把 g_irq_count 去掉 volatile 再用 -O2 编译，\n");
    printf("观察主循环是否还能看到 ISR 的修改 —— 理解为什么必须 volatile\n");
    return 0;
}
