#include <stdio.h>

/*
 * 指针与数组：数组名在表达式中会退化为指向首元素的指针。
 * 指针 + n 移动 n * sizeof(类型) 个字节。
 */
int main(void)
{
    int arr[5] = {10, 20, 30, 40, 50};

    printf("arr      = %p  (数组名，指向首元素)\n", (void *)arr);
    printf("&arr[0]  = %p  (首元素地址，相同)\n", (void *)&arr[0]);
    printf("arr[0]   = %d\n", arr[0]);
    printf("*arr     = %d  (等价于 arr[0])\n", *arr);

    // 指针运算：arr[i] 等价于 *(arr + i)
    for (int i = 0; i < 5; i++)
        printf("arr[%d]=%d  *(arr+%d)=%d  %p\n",
               i, arr[i], i, *(arr + i), (void *)(arr + i));

    // 指针遍历数组
    int *p = arr;
    printf("用指针遍历:");
    for (int i = 0; i < 5; i++)
        printf(" %d", *(p + i));
    printf("\n");

    // 指针自增逐个移动
    int *q = arr;
    printf("指针自增:");
    for (int i = 0; i < 5; i++)
        printf(" %d", *q++);
    printf("\n");

    return 0;
}
