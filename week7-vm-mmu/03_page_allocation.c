#include <stdio.h>
#include <string.h>
#include <windows.h>

/*
 * 页大小 + 保留(Reserve)/提交(Commit)：
 * 虚拟内存的分配以"页"为单位。
 *  - Reserve：只是"预定"一段地址范围，不占物理内存，也不能访问
 *  - Commit ：才真正分配物理页/页表项，可以读写
 * 这种两阶段分配让"预留大空间、按需提交"成为可能（如线程栈、映射缓冲）。
 */

int main(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD page = si.dwPageSize;
    printf("页大小 = %lu 字节 (%lu KB)\n", page, page / 1024);

    /* 1. 保留 4 页地址空间（不提交，不能访问） */
    LPVOID base = VirtualAlloc(NULL, 4 * page, MEM_RESERVE, PAGE_NOACCESS);
    if (base == NULL) {
        printf("VirtualAlloc(Reserve) 失败\n");
        return 1;
    }
    printf("\n保留 4 页 @ %p (尚未提交，访问会崩溃)\n", base);
    printf("保留地址按页对齐: 地址 %% 页大小 = %lu\n",
           (unsigned long)((unsigned long long)base % page));

    /* 2. 只提交前 2 页为可读写 */
    LPVOID committed = VirtualAlloc(base, 2 * page, MEM_COMMIT, PAGE_READWRITE);
    printf("提交前 2 页 @ %p (与保留同址: %s)\n", committed,
           committed == base ? "是" : "否");

    /* 3. 现在可以写这 2 页了 */
    memset(committed, 0xAB, 2 * page);
    printf("写入 2 页成功，首字节 = 0x%02X\n", ((unsigned char *)committed)[0]);

    /* 4. 未提交的后 2 页仍不可访问（此处只展示概念，不去访问它） */

    /* 5. 释放整段（Release 会自动撤销提交） */
    BOOL ok = VirtualFree(base, 0, MEM_RELEASE);
    printf("释放整段: %s\n", ok ? "成功" : "失败");

    printf("\n结论: Reserve=划地皮(不花钱)，Commit=真正动工(才占内存)\n");
    return 0;
}
