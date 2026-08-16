#include <stdio.h>

/*
 * const 与指针的三种组合（嵌入式 API 里极常见）：
 *  1) const int *p;  指针可变，指向的值不可变   -> "指向常量的指针"
 *  2) int *const p;  指针不可变，指向的值可变   -> "常量指针"
 *  3) const int *const p; 两者都不可变
 */
int main(void)
{
    int a = 10;
    int b = 20;

    /* 1) const int *p：不能通过 p 改值，但 p 可以指向别处 */
    const int *p1 = &a;
    // *p1 = 99;            // 编译错误：p1 指向的值只读
    p1 = &b;                // 合法：指针本身可变
    printf("const int *p1 = %d\n", *p1);

    /* 2) int *const p2：指针本身不能改，但能改指向的值 */
    int *const p2 = &a;
    *p2 = 100;              // 合法：改值
    // p2 = &b;             // 编译错误：指针不可变
    printf("int *const p2 修改后 a = %d\n", a);

    /* 3) const int *const p3：都不能改 */
    const int *const p3 = &a;
    // *p3 = 1;  p3 = &b;   // 都编译错误
    printf("const int *const p3 = %d\n", *p3);

    /* 常见误用：函数形参用 const int * 是"我承诺只读" */
    return 0;
}
