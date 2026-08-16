#include <stdio.h>

/*
 * 指针的本质：变量名只是"人的标签"，真正存放数据的是内存地址。
 * 指针变量存的就是另一个变量的地址。
 */
int main(void)
{
    int a = 42;
    int *p = &a;      // p 存 a 的地址；& 取地址运算符

    printf("a 的值      = %d\n", a);
    printf("a 的地址    = %p\n", (void *)&a);
    printf("p 存的内容  = %p  (就是 a 的地址)\n", (void *)p);
    printf("*p 解引用   = %d  (等价于读 a)\n", *p);

    // 通过指针修改 a
    *p = 100;
    printf("修改后 a     = %d\n", a);

    // 指针类型决定了读写宽度和步长
    printf("sizeof(int)  = %zu, sizeof(int*) = %zu\n", sizeof(int), sizeof(int *));

    // 指针的三种状态：有效地址 / NULL / 野指针
    int *null_p = NULL;              // 空指针
    printf("NULL 指针的值 = %p\n", (void *)null_p);
    // int *wild_p;                  // 未初始化 -> 野指针，禁止解引用！

    return 0;
}
