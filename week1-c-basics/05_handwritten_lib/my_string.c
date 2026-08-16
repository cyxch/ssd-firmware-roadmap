#include <stdio.h>

/* 手写 strlen：返回字符串长度（不含 '\0'） */
size_t my_strlen(const char *s)
{
    size_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* 手写 strcpy：把 src 复制到 dst，返回 dst */
char *my_strcpy(char *dst, const char *src)
{
    char *p = dst;
    while (*src != '\0') {
        *p = *src;
        p++;
        src++;
    }
    *p = '\0';   // 结束符不能漏
    return dst;
}

/* 手写 atoi：字符串转整数，支持正负号、跳过前导空格 */
int my_atoi(const char *s)
{
    int sign = 1, num = 0;

    while (*s == ' ')
        s++;
    if (*s == '-' || *s == '+') {
        if (*s == '-')
            sign = -1;
        s++;
    }
    while (*s >= '0' && *s <= '9') {
        num = num * 10 + (*s - '0');
        s++;
    }
    return num * sign;
}

int main(void)
{
    char buf[32];
    printf("strlen(\"hello\") = %zu\n", my_strlen("hello"));
    my_strcpy(buf, "world");
    printf("strcpy -> %s\n", buf);
    printf("atoi(\"-123\")   = %d\n", my_atoi("-123"));
    printf("atoi(\"  42abc\") = %d\n", my_atoi("  42abc"));
    return 0;
}
