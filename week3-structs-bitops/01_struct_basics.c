#include <stdio.h>
#include <string.h>

/*
 * 结构体：把不同类型的变量打包成一个复合类型。
 * 嵌入式里常用来描述"一个外设/一条消息/一个数据帧"。
 */

/* 定义一个传感器数据结构 */
struct Sensor {
    int id;                 // 传感器编号
    char name[16];          // 名称
    float value;            // 读数
    unsigned char status;   // 状态
};

int main(void)
{
    /* 1. 定义并初始化 */
    struct Sensor s1 = {1, "temp", 25.5f, 0};
    printf("s1: id=%d name=%s value=%.1f status=%u\n",
           s1.id, s1.name, s1.value, s1.status);

    /* 2. 用点号访问/修改成员 */
    s1.value = 26.8f;
    s1.status = 1;
    printf("修改后 s1.value = %.1f\n", s1.value);

    /* 3. 结构体指针 + 箭头运算符 */
    struct Sensor *p = &s1;
    p->id = 100;
    printf("p->id = %d  (*p).id = %d\n", p->id, (*p).id);

    /* 4. 结构体整体赋值（成员逐一拷贝） */
    struct Sensor s2;
    s2 = s1;
    printf("s2.name = %s\n", s2.name);

    /* 5. sizeof：结构体大小 >= 成员大小之和（对齐） */
    printf("sizeof(struct Sensor) = %zu\n", sizeof(struct Sensor));

    return 0;
}
