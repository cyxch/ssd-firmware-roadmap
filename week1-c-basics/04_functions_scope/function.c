#include <stdio.h>

// 全局变量：所有函数都能访问
static int g_count = 0;

// 函数声明（定义在 main 之后时必须有）
int add(int a, int b);
void increment(void);

int main(void)
{
    int x = 5, y = 3;
    printf("add(%d, %d) = %d\n", x, y, add(x, y));

    increment();
    increment();
    printf("g_count = %d\n", g_count);
    return 0;
}

int add(int a, int b)
{
    return a + b;   // 值传递：a、b 是调用方的副本
}

void increment(void)
{
    g_count++;          // 修改全局变量，main 里能看到
    static int s = 0;   // 静态局部变量：只初始化一次，跨调用保持
    s++;
    printf("local s = %d\n", s);
}
