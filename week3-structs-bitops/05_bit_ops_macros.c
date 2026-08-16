#include <stdio.h>
#include <stdint.h>

/*
 * 位运算宏封装：操作寄存器的标准写法（嵌入式必会）。
 * 示例寄存器 32 位：
 *   bit0-3  : 模式 mode (4 位)
 *   bit4    : 使能 en
 *   bit5    : 中断标志 irq
 */

#define REG_MODE_BIT   0u
#define REG_MODE_MASK  (0xFu << REG_MODE_BIT)     /* 0b1111 << 0 */
#define REG_EN_BIT     4u
#define REG_EN_MASK    (1u << REG_EN_BIT)
#define REG_IRQ_BIT    5u
#define REG_IRQ_MASK   (1u << REG_IRQ_BIT)

/* 置位：把 bit 置 1 */
#define SET_BIT(reg, bit)    ((reg) |= (1u << (bit)))
/* 清除：把 bit 置 0 */
#define CLEAR_BIT(reg, bit)  ((reg) &= ~(1u << (bit)))
/* 翻转：bit 取反 */
#define TOGGLE_BIT(reg, bit) ((reg) ^= (1u << (bit)))
/* 读取：得到 bit 的值 (0/1) */
#define GET_BIT(reg, bit)    (((reg) >> (bit)) & 1u)

/* 多字段：写 value 到 [bit, bit+width) 字段 */
#define SET_FIELD(reg, bit, width, value) \
    ((reg) = ((reg) & ~(((1u << (width)) - 1u) << (bit))) | \
             (((value) & ((1u << (width)) - 1u)) << (bit)))

#define GET_FIELD(reg, bit, width) \
    (((reg) >> (bit)) & ((1u << (width)) - 1u))

int main(void)
{
    uint32_t reg = 0x00000000;

    /* 单 bit 操作 */
    SET_BIT(reg, REG_EN_BIT);
    printf("置位 en  后 reg = 0x%08X\n", reg);

    CLEAR_BIT(reg, REG_EN_BIT);
    printf("清除 en  后 reg = 0x%08X\n", reg);

    SET_BIT(reg, REG_IRQ_BIT);
    TOGGLE_BIT(reg, REG_IRQ_BIT);
    printf("翻转 irq 后 reg = 0x%08X\n", reg);

    /* 多字段：mode 写 0xA */
    SET_FIELD(reg, REG_MODE_BIT, 4, 0xAu);
    printf("写 mode=0xA 后 reg = 0x%08X\n", reg);

    printf("读取 mode = 0x%X\n", GET_FIELD(reg, REG_MODE_BIT, 4));
    printf("读取 irq  = %u\n", GET_BIT(reg, REG_IRQ_BIT));

    return 0;
}
