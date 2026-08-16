#include <stdio.h>
#include <string.h>

/*
 * 综合项目：命令解释器（本周所学大整合）
 * 用 结构体(命令表) + 函数指针(回调) + 循环/字符串解析。
 * 这是"表驱动"设计的雏形，SSD 固件中大量用于命令派发（如 NVMe 命令）。
 */

typedef int (*handler_t)(const char *args);

static int cmd_help(const char *args)   { (void)args; printf("可用命令: help, sum, hi, exit\n"); return 0; }
static int cmd_sum(const char *args)
{
    int a, b;
    if (sscanf(args, "%d %d", &a, &b) != 2) {
        printf("用法: sum <a> <b>\n");
        return -1;
    }
    printf("sum = %d\n", a + b);
    return 0;
}
static int cmd_hi(const char *args)     { printf("你好! (参数: %s)\n", args[0] ? args : "(无)"); return 0; }
static int cmd_exit(const char *args)   { (void)args; printf("再见\n"); return 1; }

struct Cmd {
    const char *name;
    handler_t   handler;
};

static const struct Cmd cmd_table[] = {
    {"help", cmd_help},
    {"sum",  cmd_sum},
    {"hi",   cmd_hi},
    {"exit", cmd_exit},
};

#define CMD_N ((int)(sizeof(cmd_table) / sizeof(cmd_table[0])))

int main(void)
{
    char line[64];

    printf("简易命令解释器：输入 help 查看命令，输入 exit 退出\n");

    while (1) {
        printf("> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        /* 去掉末尾换行 */
        line[strcspn(line, "\n")] = '\0';

        /* 拆出命令名与参数 */
        char *name = line;
        char *args = strchr(line, ' ');
        if (args) {
            *args = '\0';
            args++;
        }

        int exit_flag = 0;
        int found = 0;
        for (int i = 0; i < CMD_N; i++) {
            if (strcmp(name, cmd_table[i].name) == 0) {
                found = 1;
                int r = cmd_table[i].handler(args ? args : "");
                if (r == 1)
                    exit_flag = 1;
                break;
            }
        }
        if (!found)
            printf("未知命令: %s\n", name);
        if (exit_flag)
            break;
    }
    return 0;
}
