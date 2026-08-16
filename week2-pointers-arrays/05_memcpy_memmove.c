#include <stdio.h>
#include <string.h>

/*
 * 手写 memcpy 与 memmove。
 * 关键区别：memcpy 不保证处理"源与目标内存重叠"；
 *           memmove 必须保证重叠时也能正确复制。
 * 思路：dst 在 src 前面 -> 从头复制；
 *       dst 在 src 后面 -> 从尾复制，避免覆盖未读的源数据。
 */

/* 手写 memcpy（不处理重叠） */
void *my_memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    while (n--)
        *d++ = *s++;
    return dst;
}

/* 手写 memmove（处理重叠）：按重叠方向选择复制顺序 */
void *my_memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    if (d == s || n == 0)
        return dst;

    if (d < s) {
        /* 目标在源之前：从头往后复制，安全 */
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    } else {
        /* 目标在源之后：从尾往前复制，避免覆盖未复制的源 */
        for (size_t i = n; i > 0; i--)
            d[i - 1] = s[i - 1];
    }
    return dst;
}

int main(void)
{
    char a[] = "Hello, World!";
    char b[32];
    char c[32];

    my_memcpy(b, a, strlen(a) + 1);       // 非重叠
    my_memmove(c, a, strlen(a) + 1);      // 非重叠
    printf("memcpy  -> %s\n", b);
    printf("memmove -> %s\n", c);

    /* 重叠测试：把 "World!" 覆盖到 "Hello," 位置 */
    char d[] = "Hello, World!";
    my_memmove(d + 7, d, 6);              // 目标前移，源包含目标
    printf("重叠 memmove -> %s\n", d);

    return 0;
}
