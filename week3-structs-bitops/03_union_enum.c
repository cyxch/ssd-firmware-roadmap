#include <stdio.h>

/*
 * 联合体 union：所有成员共享同一块内存，大小 = 最大成员。
 * 常用于：类型转换、寄存器整体/位字段访问、协议载荷复用。
 * 枚举 enum：给整型常量起名字，增加可读性。
 */

/* 联合体：同一个 4 字节既可按整体看，也可按字节看 */
union IntBytes {
    int value;
    unsigned char bytes[4];
};

/* 枚举：状态机/错误码常用 */
enum ErrorCode {
    ERR_NONE = 0,
    ERR_TIMEOUT,
    ERR_CRC,
    ERR_FULL
};

/* 使用枚举作为函数返回值，可读性更高 */
enum ErrorCode do_something(void)
{
    return ERR_CRC;
}

int main(void)
{
    /* 1. union 的基本用法 */
    union IntBytes u;
    u.value = 0x12345678;
    printf("u.value = 0x%X\n", u.value);
    printf("字节序(小端, PC 上):");
    for (int i = 0; i < 4; i++)
        printf(" %02X", u.bytes[i]);
    printf("\n");
    printf("sizeof(union IntBytes) = %zu\n", sizeof(union IntBytes));

    /* 2. 修改一个字节会影响整个值 */
    u.bytes[0] = 0xAA;
    printf("改后 u.value = 0x%X\n", u.value);

    /* 3. enum 使用 */
    enum ErrorCode e = do_something();
    if (e == ERR_CRC)
        printf("错误：CRC 校验失败 (code=%d)\n", e);

    return 0;
}
