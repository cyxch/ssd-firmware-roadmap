#include <stdio.h>
#include <limits.h>

int main(void)
{
    // 1. 基本数据类型大小（嵌入式里 sizeof 很重要）
    printf("sizeof(int)    = %zu\n", sizeof(int));
    printf("sizeof(char)   = %zu\n", sizeof(char));
    printf("sizeof(float)  = %zu\n", sizeof(float));
    printf("sizeof(double) = %zu\n", sizeof(double));
    printf("int 范围: %d ~ %d\n", INT_MIN, INT_MAX);

    // 2. 整数 vs 浮点运算
    int a = 7, b = 2;
    printf("%d / %d = %d   (整数除法)\n", a, b, a / b);
    printf("%d %% %d = %d   (取余)\n", a, b, a % b);
    printf("%d / %d = %.2f (浮点除法，需强转)\n", a, b, (double)a / b);

    // 3. 溢出回绕（嵌入式寄存器常见坑）
    unsigned char uc = 255;
    printf("uc = %u, uc + 1 = %u (溢出回绕)\n", uc, uc + 1);

    // 4. 位运算：操作寄存器的基本功
    unsigned int reg = 0x00;
    reg |= (1u << 3);   // 置位 bit3
    reg &= ~(1u << 5);  // 清除 bit5
    reg ^= (1u << 7);   // 翻转 bit7
    printf("reg = 0x%X\n", reg);
    return 0;
}
