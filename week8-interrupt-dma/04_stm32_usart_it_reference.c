/*
 * ============================================================
 * 【参考代码 · 需要真实硬件】STM32 串口中断接收
 * ============================================================
 * 运行环境：STM32F1/F4 + STM32CubeMX 生成的 HAL 工程 + 串口连 PC
 * 编译工具链：arm-none-eabi-gcc（本机未安装，此文件未在本机编译）
 *
 * 功能：PC 发送数据，板子通过"串口中断"逐字节接收，回显到 PC。
 * 流程：中断到达 -> 进入 UARTx_IRQHandler -> HAL 调用回调
 *      -> 在回调里把收到的字节存进环形缓冲区/回显。
 *
 * 如何搭：CubeMX 勾选 USART1 + NVIC 使能全局中断，
 *        串口接线 TX->RX, RX->TX, GND 相连。
 */

#include "main.h"
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

/* 收到的字节 */
static volatile uint8_t g_rx_byte = 0;

/* 启动"中断接收一个字节"：硬件收到 1 字节后触发中断 */
void app_uart_start_rx(void)
{
    HAL_UART_Receive_IT(&huart1, &g_rx_byte, 1);
}

/*
 * 中断回调：在 UART 中断上下文里被调用（等价于 ISR 主体）。
 * HAL 已经在 UARTx_IRQHandler 里做了向量跳转，我们只需实现回调。
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        /* 回显收到的字节 */
        HAL_UART_Transmit(&huart1, &g_rx_byte, 1, 100);
        /* 继续接收下一个字节（否则只收一次） */
        HAL_UART_Receive_IT(&huart1, &g_rx_byte, 1);
    }
}

/* 主循环用法（示意） */
void app_loop(void)
{
    app_uart_start_rx();
    while (1) {
        /* 主循环可处理其他任务；接收由中断驱动，无需轮询 */
    }
}
