#include <stdio.h>
#include <stdint.h>

/*
 * 自测题：
 *  1) 结构体成员顺序如何影响 sizeof？
 *  2) 下面的结构体为什么是 8 而不是 5？
 *  3) 用宏实现"读取寄存器某字段"。
 */

/* 自测 2：char + int + char，默认对齐 */
struct Quiz {
    char a;
    int  b;
    char c;
};

/* 自测 3：字段定义 */
#define REG_CFG_BIT  0u
#define REG_CFG_MASK (0xFu << REG_CFG_BIT)

#define READ_FIELD(reg, bit, width) \
    (((reg) >> (bit)) & ((1u << (width)) - 1u))

int main(void)
{
    /* 2) 对齐导致 sizeof 变大 */
    printf("sizeof(struct Quiz) = %zu  (char+int+char=6，对齐后为 8)\n",
           sizeof(struct Quiz));

    /* 3) 宏读取字段 */
    uint32_t reg = 0x000000A5;      /* bit0-3 = 0x5 */
    printf("读取字段 = 0x%X\n", READ_FIELD(reg, REG_CFG_BIT, 4));

    /* 思考题：为什么位域不能用于跨平台协议帧？ */
    return 0;
}
