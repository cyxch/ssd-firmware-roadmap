#include <stdio.h>
#include <stdint.h>

/*
 * 综合项目：用 union + 位域模拟一个 32 位寄存器。
 * 这是嵌入式读寄存器最经典的做法：
 *   - 通过 union 让"整体 32 位"和"逐位字段"共用同一块内存
 *   - 读取/写入整体用 reg->value，操作字段用 reg->bits
 */

/* 模拟一个外设控制寄存器 CTRL_REG
 *   bit0    : EN      使能
 *   bit1-2  : SPEED   速度 0-3
 *   bit3    : RST     复位(写1)
 *   bit4-7  : PRESCALE 预分频
 *   bit8    : TX_EMPTY 发送缓冲空(只读)
 */
union CtrlReg {
    uint32_t value;                 /* 整体访问 */
    struct {
        uint32_t en       : 1;
        uint32_t speed    : 2;
        uint32_t rst      : 1;
        uint32_t prescale : 4;
        uint32_t tx_empty : 1;
        uint32_t rsvd     : 23;
    } bits;                         /* 位字段访问 */
};

static union CtrlReg g_ctrl;        /* 模拟硬件寄存器 */

void print_reg(const char *tag, const union CtrlReg *r)
{
    printf("%-14s value=0x%08X  en=%u speed=%u rst=%u prescale=%u tx_empty=%u\n",
           tag, r->value,
           r->bits.en, r->bits.speed, r->bits.rst,
           r->bits.prescale, r->bits.tx_empty);
}

int main(void)
{
    /* 整体清零 */
    g_ctrl.value = 0;
    print_reg("复位后", &g_ctrl);

    /* 通过位字段配置 */
    g_ctrl.bits.en = 1;
    g_ctrl.bits.speed = 2;
    g_ctrl.bits.prescale = 9;
    print_reg("配置后", &g_ctrl);

    /* 直接写整体（例如硬件把 TX_EMPTY 置 1） */
    g_ctrl.value |= (1u << 8);
    print_reg("硬件更新", &g_ctrl);

    /* 通过位字段读状态 */
    printf("tx_empty = %u\n", g_ctrl.bits.tx_empty);

    /* 验证：整体值和位字段指向同一内存 */
    printf("&g_ctrl.value  = %p\n", (void *)&g_ctrl.value);
    printf("&g_ctrl.bits   = %p\n", (void *)&g_ctrl.bits);

    return 0;
}
