#include <stdio.h>
#include <windows.h>

/*
 * 核心实验：用 VirtualQuery 遍历进程的虚拟地址空间。
 * 这是 Windows 版的 "/proc/self/maps"：列出每一段已映射(提交)的虚拟内存区。
 * 可以看到：代码段、数据段、堆、栈、加载的 DLL 等都在不同的虚拟地址范围。
 * 这正是"页表 + MMU"管理下，进程看到的虚拟内存视图。
 *
 * 概念对照：
 *   - MEM_COMMIT  已提交（已分配物理页或页表项）—— 真正可访问
 *   - MEM_RESERVE 仅保留地址范围，未占物理内存
 *   - MEM_FREE    未映射
 */

static const char *protect_name(DWORD p)
{
    switch (p & 0xFF) {
        case PAGE_EXECUTE_READ:       return "RX";
        case PAGE_EXECUTE_READWRITE:  return "RWX";
        case PAGE_READONLY:           return "R";
        case PAGE_READWRITE:          return "RW";
        case PAGE_NOACCESS:           return "--";
        default:                      return "??";
    }
}

int main(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    printf("系统信息:\n");
    printf("  页大小           = %lu KB\n", si.dwPageSize / 1024);
    printf("  分配粒度         = %lu KB\n", si.dwAllocationGranularity / 1024);
    printf("  用户态最小地址   = %p\n", si.lpMinimumApplicationAddress);
    printf("  用户态最大地址   = %p\n", si.lpMaximumApplicationAddress);

    printf("\n进程已提交的虚拟内存区 (地址 / 大小 / 权限 / 类型):\n");

    unsigned char *addr = (unsigned char *)si.lpMinimumApplicationAddress;
    unsigned char *max  = (unsigned char *)si.lpMaximumApplicationAddress;
    int shown = 0;

    while (addr < max && shown < 40) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(addr, &mbi, sizeof(mbi)) == 0)
            break;

        if (mbi.State == MEM_COMMIT) {
            const char *type = (mbi.Type == MEM_IMAGE) ? "IMAGE"
                              : (mbi.Type == MEM_MAPPED) ? "MAPPED" : "PRIVATE";
            printf("  %p  %8lu KB   %-3s  %-7s\n",
                   mbi.BaseAddress,
                   mbi.RegionSize / 1024,
                   protect_name(mbi.Protect),
                   type);
            shown++;
        }

        /* 跳到下一段（步进整段，避免逐页遍历慢） */
        if (mbi.RegionSize == 0)
            break;
        addr += mbi.RegionSize;
    }

    printf("\n(共显示 %d 段已提交区域)\n", shown);
    printf("观察: 代码段(RX/IMAGE)、数据段(RW)、堆、栈分处不同虚拟地址范围\n");
    return 0;
}
