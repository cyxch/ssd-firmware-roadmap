#include <stdio.h>

int main(void)
{
    // 1. if / else if
    int score = 85;
    if (score >= 90)
        printf("优秀\n");
    else if (score >= 60)
        printf("及格\n");
    else
        printf("不及格\n");

    // 2. for：1~10 求和
    int sum = 0;
    for (int i = 1; i <= 10; i++)
        sum += i;
    printf("1~10 求和 = %d\n", sum);

    // 3. while：统计二进制中 1 的个数
    unsigned int n = 0b10110100;
    int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    printf("1 的个数 = %d\n", count);

    // 4. switch
    int day = 3;
    switch (day) {
        case 1: printf("Monday\n");    break;
        case 2: printf("Tuesday\n");   break;
        case 3: printf("Wednesday\n"); break;
        default: printf("Other\n");    break;
    }
    return 0;
}
