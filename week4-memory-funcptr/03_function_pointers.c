#include <stdio.h>

/*
 * 函数指针：指向函数的指针。函数名在表达式中退化为函数地址。
 * 回调机制：把"函数指针"作为参数传给另一个函数，由它来调用。
 * 嵌入式典型场景：中断回调、驱动注册、协议解析派发。
 */

/* 三个业务处理函数（签名一致才能用同一个函数指针） */
int cmd_led_on(int arg)   { printf("LED 打开, arg=%d\n", arg); return 0; }
int cmd_led_off(int arg)  { printf("LED 关闭, arg=%d\n", arg); return 0; }
int cmd_read(int arg)     { printf("读取寄存器, arg=%d\n", arg); return 0; }

/* 函数指针类型定义 */
typedef int (*cmd_handler_t)(int);

/* 命令表：名字 + 处理函数（面向表驱动的调度） */
struct CmdEntry {
    const char     *name;
    cmd_handler_t   handler;
};

/* 简易字符串比较（避免额外 include，展示核心逻辑） */
int strcmp_ish(const char *a, const char *b);

/* 表驱动分发：按名字查表并调用回调 */
int dispatch(const struct CmdEntry *table, int n, const char *name, int arg)
{
    for (int i = 0; i < n; i++) {
        if (strcmp_ish(table[i].name, name) == 0)
            return table[i].handler(arg);   /* 通过函数指针回调 */
    }
    printf("未知命令: %s\n", name);
    return -1;
}

/* 简易字符串比较（避免额外 include，展示核心逻辑） */
int strcmp_ish(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int main(void)
{
    struct CmdEntry table[] = {
        {"on",   cmd_led_on},
        {"off",  cmd_led_off},
        {"read", cmd_read},
    };
    int n = (int)(sizeof(table) / sizeof(table[0]));

    dispatch(table, n, "on", 1);
    dispatch(table, n, "read", 0x4000);
    dispatch(table, n, "unknown", 0);

    return 0;
}
