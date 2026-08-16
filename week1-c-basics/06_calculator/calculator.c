#include <stdio.h>

/*
 * 综合本周所学：fgets + sscanf + switch + while 循环
 * 输入格式：5 + 3   输入 q 退出
 */
int main(void)
{
    char line[64];
    double a, b, result;
    char op;

    printf("简单计算器：输入  a op b  (如 5 + 3)\n");
    printf("支持 + - * / ；输入 q 退出\n");

    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin))
            break;
        if (line[0] == 'q' || line[0] == 'Q')
            break;

        if (sscanf(line, "%lf %c %lf", &a, &op, &b) != 3) {
            printf("输入格式错误，请重试\n");
            continue;
        }

        switch (op) {
            case '+': result = a + b; break;
            case '-': result = a - b; break;
            case '*': result = a * b; break;
            case '/':
                if (b == 0) { printf("除数不能为 0\n"); continue; }
                result = a / b;
                break;
            default:
                printf("未知运算符: %c\n", op);
                continue;
        }
        printf("= %.2f\n", result);
    }
    printf("再见\n");
    return 0;
}
