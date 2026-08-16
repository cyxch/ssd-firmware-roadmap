#include <stdio.h>
#include <stddef.h>

/*
 * 内存对齐：编译器为了让 CPU 高效读取，会把成员放到"对齐边界"上。
 * 因此结构体大小通常 > 各成员大小之和（存在填充字节 padding）。
 * #pragma pack(1) 可强制紧凑布局（代价是访问变慢，跨平台有风险）。
 */

/* 默认对齐：int 后面会有填充，char 也可能被对齐到 4 字节边界 */
struct DefaultLayout {
    char    a;      // 0
    int     b;      // 4..7    (跳过 1..3 的填充)
    char    c;      // 8
    double  d;      // 16..23  (跳过 9..15)
};

/* pack(1)：紧凑排列，无填充 */
#pragma pack(push, 1)
struct PackedLayout {
    char    a;      // 0
    int     b;      // 1..4
    char    c;      // 5
    double  d;      // 6..13
};
#pragma pack(pop)

int main(void)
{
    printf("sizeof(struct DefaultLayout) = %zu\n", sizeof(struct DefaultLayout));
    printf("sizeof(struct PackedLayout)  = %zu\n", sizeof(struct PackedLayout));

    printf("offsetof(DefaultLayout.a) = %zu\n", offsetof(struct DefaultLayout, a));
    printf("offsetof(DefaultLayout.b) = %zu\n", offsetof(struct DefaultLayout, b));
    printf("offsetof(DefaultLayout.d) = %zu\n", offsetof(struct DefaultLayout, d));

    /* 说明：pack 常用于"协议帧 / 寄存器映射"，保证内存布局可预测 */
    return 0;
}
