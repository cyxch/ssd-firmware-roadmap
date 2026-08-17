# Week 8 · 中断与 DMA

SSD 固件方向学习计划 · 第 8 周（中断与 DMA）

## 本周目标

- 理解中断机制：向量表、ISR、中断入口/退出流程
- 量化对比三种外设接收方式：**轮询 / 中断 / DMA**
- 理解 ISR 与主循环共享变量的 `volatile` 陷阱
- 掌握 STM32 串口中断接收 + DMA 收发（硬件参考代码）

## 目录结构

| 文件 | 说明 | 能否在本机编译 |
|------|------|--------------|
| `01_interrupt_vector_sim.c` | 中断向量表 + 注册/触发/分发模拟 | ✅ gcc 可运行 |
| `02_polling_vs_interrupt_vs_dma.c` | **核心**：三种接收方式 CPU 开销对比 | ✅ gcc 可运行 |
| `03_signal_isr_sim.c` | 用信号模拟 ISR + 共享变量 volatile | ✅ gcc 可运行 |
| `04_stm32_usart_it_reference.c` | STM32 串口中断接收（HAL）参考 | ❌ 需开发板+ARM工具链 |
| `05_stm32_dma_loopback_reference.c` | STM32 串口 DMA 回环（HAL）参考 | ❌ 需开发板+ARM工具链 |

## 本机可运行实验

```bash
gcc -O2 -Wall -Wextra -o v 01_interrupt_vector_sim.c && ./v
gcc -O2 -Wall -Wextra -o p 02_polling_vs_interrupt_vs_dma.c && ./p
gcc -O2 -Wall -Wextra -o s 03_signal_isr_sim.c && ./s
```

## 硬件实验（有 STM32 板时）

把 `04/05` 放入 STM32CubeMX 生成的工程，串口连接 PC：
- `04`：PC 发什么，板子中断接收并回显
- `05`：DMA 回环，观察 DMA 搬运

## 本周自测

- [ ] 能画出中断响应流程（触发→向量表→ISR→返回）
- [ ] 能说出轮询/中断/DMA 各自的优缺点与适用场景
- [ ] 能解释为什么 ISR 与主循环共享的变量要加 `volatile`
