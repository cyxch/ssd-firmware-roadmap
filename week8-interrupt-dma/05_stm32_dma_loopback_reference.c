/*
 * ============================================================
 * 【参考代码 · 需要真实硬件】STM32 串口 DMA 回环
 * ============================================================
 * 运行环境：STM32F1/F4 + STM32CubeMX HAL 工程 + 串口连 PC
 * 编译工具链：arm-none-eabi-gcc（本机未安装，此文件未在本机编译）
 *
 * 功能：DMA 把缓冲区内容通过串口发出；同时用另一路 DMA 接收，
 *      实现"收发都靠硬件搬运"，CPU 只在中途做极少处理。
 *
 * 关键点（对比 04 的中断逐字节方式）：
 *   - 中断方式：每个字节触发一次 CPU 中断
 *   - DMA 方式：整段数据由 DMA 硬件搬运，完成才触发一次中断
 *
 * CubeMX 配置：USART1 的 TX/RX 都开启 DMA，NVIC 使能 UART+DMA 中断。
 */

#include "main.h"
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart1_rx;

/* 回环缓冲区 */
static uint8_t g_tx_buf[64] = "hello dma loopback!";
static uint8_t g_rx_buf[64];

/* DMA 发送完成回调（一次中断处理整段） */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        /* 发送完成，可在此处理后续 */
    }
}

/* DMA 接收完成回调 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        /* 收满 64 字节后才进来（不是逐字节！） */
        HAL_UART_Transmit_DMA(&huart1, g_rx_buf, 64);   /* 回显 */
        HAL_UART_Receive_DMA(&huart1, g_rx_buf, 64);    /* 继续收 */
    }
}

/* 主循环用法（示意） */
void app_loop(void)
{
    /* 启动 DMA 发送：硬件自动搬 g_tx_buf -> 串口 */
    HAL_UART_Transmit_DMA(&huart1, g_tx_buf, 64);
    /* 启动 DMA 接收：硬件自动把串口数据搬进 g_rx_buf */
    HAL_UART_Receive_DMA(&huart1, g_rx_buf, 64);

    while (1) {
        /* CPU 几乎无事可做：搬运全交给 DMA */
    }
}
