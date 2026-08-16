#include <stdio.h>

/*
 * 第 1 周 · 第一个 C 程序
 * 编译：gcc 01_hello_world/hello.c -o hello
 * 运行：./hello  （Windows 下为 hello.exe）
 *
 * 编译四阶段（分别用参数验证）：
 *   1) 预处理  gcc -E  展开 #include / #define
 *   2) 编译    gcc -S  生成汇编文件 .s
 *   3) 汇编    gcc -c  生成目标文件 .o
 *   4) 链接    gcc -o  合并成可执行文件
 */
int main(void)
{
    printf("Hello, World!\n");
    printf("Welcome to Embedded C!\n");
    return 0;
}
