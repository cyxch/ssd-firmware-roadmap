#include <stdio.h>

/*
 * 二级指针：指向"指针变量"的指针。
 * 用途：函数内要修改调用方的指针变量本身时，必须传一级指针的地址。
 */
/* 静态存储，保证指针出函数后依然有效 */
static int g_storage = 777;

/* 通过二级指针让调用方的指针变量指向 g_storage */
void init_pointer(int **pp)
{
    *pp = &g_storage;
}

int main(void)
{
    int x = 5;
    int *ptr = &x;          // 一级指针
    int **pptr = &ptr;      // 二级指针指向 ptr

    printf("ptr   = %p, &x  = %p\n", (void *)ptr, (void *)&x);
    printf("*pptr = %p (就是 ptr 的值)\n", (void *)*pptr);
    printf("**pptr = %d (就是 x 的值)\n", **pptr);

    /* 用二级指针修改一级指针的指向 */
    init_pointer(&ptr);
    printf("调用后 *ptr = %d\n", *ptr);

    return 0;
}
