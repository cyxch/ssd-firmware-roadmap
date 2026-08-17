#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/*
 * 补码与溢出（CSAPP 第 2 章）：
 *  - 补码：最高位是符号位，n 位补码范围 [-2^(n-1), 2^(n-1)-1]
 *  - 溢出：结果超出范围，回绕到另一端（对无符号是取模）
 */

int main(void)
{
    /* 1. 有符号整数溢出：回绕 */
    int max = INT_MAX;
    printf("INT_MAX + 1 = %d  (溢出回绕为负数)\n", max + 1);

    int min = INT_MIN;
    printf("INT_MIN - 1 = %d  (下溢回绕为正数)\n", min - 1);

    /* 2. 无符号整数：取模回绕 */
    unsigned char uc = 255;
    printf("unsigned char 255 + 1 = %u\n", (unsigned)(uc + 1));

    /* 3. 补码解释：0xFF 在有符号/无符号下的不同含义 */
    int8_t  s8 = (int8_t)0xFF;          /* 有符号 8 位 */
    uint8_t u8 = (uint8_t)0xFF;         /* 无符号 8 位 */
    printf("0xFF 有符号 = %d, 无符号 = %u\n", (int)s8, (unsigned)u8);

    /* 4. 字节序：小端 vs 大端 */
    uint32_t v = 0x12345678;
    unsigned char *b = (unsigned char *)&v;
    printf("0x12345678 在内存(小端机)中: %02X %02X %02X %02X\n",
           b[0], b[1], b[2], b[3]);

    return 0;
}
