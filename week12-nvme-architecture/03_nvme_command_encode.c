#include <stdio.h>
#include <string.h>

/*
 * NVMe 命令编码与解码（模拟）。
 *
 * 真实 NVMe 命令: 固定 64 字节, 构成:
 *   - DW0 : Dword0, 含 OPC(opcode 操作码) / CID(命令ID) / FUSE 等
 *   - DW1 : NSID(命名空间 ID)
 *   - DW2-DW3 : MPTR(元数据指针) / PRP1 等
 *   - DW4-DW5 : PRP1/PRP2(物理区域页指针, 指向数据所在内存)
 *   - DW6-DW9 : CDW10~13(命令特有字段, 如 LBA / 长度 / 特性)
 *
 * 本实验用"手写位操作"把一个简化命令装进 64 字节数组,
 * 再按字段位置解码出来, 理解固件侧如何解析主机发来的命令。
 */

#define CMD_SIZE 64

/* 把一个命令编码进 64 字节 */
static void encode_cmd(unsigned char buf[CMD_SIZE],
                       unsigned char opc, unsigned short cid,
                       unsigned int nsid,
                       unsigned long long lba, unsigned int count)
{
    memset(buf, 0, CMD_SIZE);

    /* DW0 = OPC(低 8bit) | CID(高 16bit) | 其他位 */
    unsigned int dw0 = (unsigned int)opc
                     | ((unsigned int)cid << 16);
    buf[0]  = (unsigned char)(dw0 & 0xFF);
    buf[1]  = (unsigned char)((dw0 >> 8) & 0xFF);
    buf[2]  = (unsigned char)((dw0 >> 16) & 0xFF);
    buf[3]  = (unsigned char)((dw0 >> 24) & 0xFF);

    /* DW1 = NSID */
    buf[4]  = (unsigned char)(nsid & 0xFF);
    buf[5]  = (unsigned char)((nsid >> 8) & 0xFF);
    buf[6]  = (unsigned char)((nsid >> 16) & 0xFF);
    buf[7]  = (unsigned char)((nsid >> 24) & 0xFF);

    /* CDW10(字节 40..43) = LBA 低 32 位 */
    buf[40] = (unsigned char)(lba & 0xFF);
    buf[41] = (unsigned char)((lba >> 8) & 0xFF);
    buf[42] = (unsigned char)((lba >> 16) & 0xFF);
    buf[43] = (unsigned char)((lba >> 24) & 0xFF);

    /* CDW12(字节 48..51) = count */
    buf[48] = (unsigned char)(count & 0xFF);
    buf[49] = (unsigned char)((count >> 8) & 0xFF);
    buf[50] = (unsigned char)((count >> 16) & 0xFF);
    buf[51] = (unsigned char)((count >> 24) & 0xFF);
}

/* 从 64 字节里解码出各字段 */
static void decode_cmd(const unsigned char buf[CMD_SIZE],
                       unsigned char *opc, unsigned short *cid,
                       unsigned int *nsid,
                       unsigned long long *lba, unsigned int *count)
{
    unsigned int dw0 = (unsigned int)buf[0]
                     | ((unsigned int)buf[1] << 8)
                     | ((unsigned int)buf[2] << 16)
                     | ((unsigned int)buf[3] << 24);
    *opc = (unsigned char)(dw0 & 0xFF);
    *cid = (unsigned short)((dw0 >> 16) & 0xFFFF);

    *nsid = (unsigned int)buf[4]
          | ((unsigned int)buf[5] << 8)
          | ((unsigned int)buf[6] << 16)
          | ((unsigned int)buf[7] << 24);

    *lba = (unsigned long long)buf[40]
         | ((unsigned long long)buf[41] << 8)
         | ((unsigned long long)buf[42] << 16)
         | ((unsigned long long)buf[43] << 24);

    *count = (unsigned int)buf[48]
           | ((unsigned int)buf[49] << 8)
           | ((unsigned int)buf[50] << 16)
           | ((unsigned int)buf[51] << 24);
}

int main(void)
{
    unsigned char cmd[CMD_SIZE];
    unsigned char opc;
    unsigned short cid;
    unsigned int nsid;
    unsigned long long lba;
    unsigned int count;

    /* 编码: READ, cid=7, nsid=1, lba=0x12345678, 16 块 */
    encode_cmd(cmd, 0x02, 7, 1, 0x12345678ull, 16);

    printf("== 主机发送的 64 字节命令(前 16 字节 + 相关 DW) ==\n");
    printf("  字节 0..15 : ");
    for (int i = 0; i < 16; i++)
        printf("%02X ", cmd[i]);
    printf("\n  字节 40..51(CDW10/12 数据区): ");
    for (int i = 40; i < 52; i++)
        printf("%02X ", cmd[i]);
    printf("\n\n");

    printf("== 固件(控制器)解码 ==\n");
    decode_cmd(cmd, &opc, &cid, &nsid, &lba, &count);
    printf("  OPC  = 0x%02X (%s)\n", opc, opc == 0x02 ? "READ" : "其他");
    printf("  CID  = %u\n", cid);
    printf("  NSID = %u\n", nsid);
    printf("  LBA  = 0x%llX\n", lba);
    printf("  COUNT= %u 块\n", count);

    printf("\n要点: 固件核心工作之一就是“解析 64 字节命令 + 按字段执行”;\n");
    printf("      位操作是基本功, 真实协议字段更多、边界要严格对齐\n");
    return 0;
}
