#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

/*
 * Linux / WSL 下观察进程虚拟内存布局：
 * 直接读取 /proc/self/maps（每个进程都有的"虚拟内存地图"）。
 * 在 Windows 上运行会提示改用 VirtualQuery（见 02）。
 *
 * 命令行更简单的方式：
 *   cat /proc/self/maps
 *   pmap $$          （pmap 需要先安装: sudo apt install procps）
 * 每一行的格式：
 *   起始-结束   权限  偏移  设备  索引   路径
 *   权限 rwx: r读 w写 x执行 p私有 s共享
 */

int main(void)
{
    FILE *f = fopen("/proc/self/maps", "r");
    if (f != NULL) {
        char line[512];
        printf("=== /proc/self/maps (本进程虚拟内存映射) ===\n");
        while (fgets(line, sizeof(line), f))
            fputs(line, stdout);
        fclose(f);
        return 0;
    }

    printf("当前不是 Linux/WSL 环境，无法读取 /proc/self/maps\n");
#ifdef _WIN32
    printf("在 Windows 下，请运行 02_virtualquery_map.c 观察虚拟地址空间。\n");
    printf("或在 WSL 里编译运行本文件即可看到真正的内存映射。\n");
#endif
    return 0;
}
