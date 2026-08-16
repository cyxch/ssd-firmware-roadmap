#include <stdio.h>

/*
 * 自测题：用函数交换两个 int。
 * 结论：C 是值传递，形参是实参的副本。
 * 想在函数里改变调用方的变量，必须传地址（指针）。
 */

/* 错误示范：只交换了副本，对调用方无效 */
void swap_wrong(int a, int b)
{
    int tmp = a;
    a = b;
    b = tmp;
}

/* 正确写法：传入地址，通过解引用交换 */
void swap(int *pa, int *pb)
{
    int tmp = *pa;
    *pa = *pb;
    *pb = tmp;
}

int main(void)
{
    int x = 5, y = 9;

    printf("交换前: x=%d y=%d\n", x, y);

    swap_wrong(x, y);
    printf("swap_wrong 后: x=%d y=%d  (没变，因为传的是副本)\n", x, y);

    swap(&x, &y);
    printf("swap(&x,&y) 后: x=%d y=%d  (成功)\n", x, y);

    return 0;
}
