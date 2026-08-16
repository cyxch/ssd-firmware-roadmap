#include <stdio.h>

/*
 * 数组常见操作：反转数组、二分查找（要求数组已排序）。
 */

/* 反转数组：用两个指针一前一后交换 */
void reverse_array(int *arr, int n)
{
    int *lo = arr;
    int *hi = arr + n - 1;
    while (lo < hi) {
        int tmp = *lo;
        *lo = *hi;
        *hi = tmp;
        lo++;
        hi--;
    }
}

/* 二分查找：返回下标，找不到返回 -1 */
int binary_search(const int *arr, int n, int target)
{
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;   // 避免 (lo+hi) 溢出
        if (arr[mid] == target)
            return mid;
        else if (arr[mid] < target)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}

void print_array(const int *arr, int n)
{
    printf("[");
    for (int i = 0; i < n; i++)
        printf("%s%d", i ? ", " : "", arr[i]);
    printf("]\n");
}

int main(void)
{
    int arr[] = {1, 3, 5, 7, 9, 11};
    int n = (int)(sizeof(arr) / sizeof(arr[0]));

    printf("原数组:  ");
    print_array(arr, n);

    int copy[] = {1, 3, 5, 7, 9, 11};
    reverse_array(copy, n);
    printf("反转后:  ");
    print_array(copy, n);

    int idx = binary_search(arr, n, 7);
    printf("查找 7  -> 下标 %d\n", idx);
    printf("查找 8  -> 下标 %d\n", binary_search(arr, n, 8));

    return 0;
}
