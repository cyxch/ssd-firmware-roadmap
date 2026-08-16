#include <stdio.h>
#include <stdint.h>

/*
 * 位域 bitfield：允许按"位"定义结构体成员，用于描述寄存器位字段。
 * 注意：位域的内存布局是"实现定义"的（依赖编译器/字节序），
 *       跨平台协议帧应避免直接用位域，改用显式位运算。
 */

/* 模拟一个 32 位状态寄存器（仅演示概念，非标准布局） */
struct StatusReg {
    uint32_t en      : 1;   // bit0 使能
    uint32_t mode    : 2;   // bit1-2 模式
    uint32_t rsvd    : 5;   // bit3-7 保留
    uint32_t err     : 1;   // bit8 错误标志
    uint32_t count   : 8;   // bit9-16 计数值
    uint32_t rsvd2   : 15;  // bit17-31
};

int main(void)
{
    struct StatusReg reg = {0};
    reg.en = 1;
    reg.mode = 3;
    reg.err = 1;
    reg.count = 200;

    printf("en=%u mode=%u err=%u count=%u\n",
           reg.en, reg.mode, reg.err, reg.count);

    /* 位域成员可以直接读写，可读性比位运算好 */
    reg.count++;
    printf("count 自增后 = %u\n", reg.count);

    printf("sizeof(struct StatusReg) = %zu (可能被优化为 4)\n",
           sizeof(struct StatusReg));

    return 0;
}
