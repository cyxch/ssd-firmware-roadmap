# Week 12 · NVMe 协议与 SSD 架构

SSD 固件方向学习计划 · 第 12 周（NVMe 协议与整机架构）

## 本周目标

- 理解 NVMe 相比 AHCI 的队列机制优势
- 掌握 SQ/CQ 完整流转与 doorbell 交互
- 理解 NVMe 命令（64 字节）的字段布局与解析
- 理解完成项（CQE）与中断聚合
- 建立 SSD 整机分层架构与数据通路的整体认识

## 目录结构

| 文件 | 说明 | 本机编译 |
|------|------|---------|
| `01_nvme_vs_ahci.c` | NVMe vs AHCI 队列机制与吞吐对比 | ✅ gcc 可运行 |
| `02_nvme_sq_cq_doorbell.c` | SQ/CQ 完整流转 + doorbell 模拟 | ✅ gcc 可运行 |
| `03_nvme_command_encode.c` | 64 字节命令编码/解码（位操作） | ✅ gcc 可运行 |
| `04_nvme_completion_cq_entry.c` | CQE 完成项 + 中断聚合对比 | ✅ gcc 可运行 |
| `05_ssd_architecture_sim.c` | SSD 整机分层流水线模拟 | ✅ gcc 可运行 |

## 编译运行

```bash
gcc -O2 -Wall -Wextra -o a 01_nvme_vs_ahci.c && ./a
gcc -O2 -Wall -Wextra -o q 02_nvme_sq_cq_doorbell.c && ./q
gcc -O2 -Wall -Wextra -o c 03_nvme_command_encode.c && ./c
gcc -O2 -Wall -Wextra -o e 04_nvme_completion_cq_entry.c && ./e
gcc -O2 -Wall -Wextra -o s 05_ssd_architecture_sim.c && ./s
```

## 关键概念速查

- NVMe：PCIe 上的协议，多队列（每核一队列），深度可达 64K
- SQ（提交队列）：主机写命令；CQ（完成队列）：控制器写完成项
- Doorbell（门铃）：内存映射寄存器，写入即通知对方
- NVMe 命令固定 64 字节：OPC/CID/NSID/PRP/CDW10~13
- CQE 固定 16 字节：含 SQ Head 指针，通知主机可再提交
- 中断聚合：攒满 N 个完成项才中断一次，降低 CPU 负载

## 本周自测

- [ ] 能说出 NVMe 比 AHCI 快的 3 个原因
- [ ] 能画出 SQ/CQ/doorbell 三方交互时序
- [ ] 能说出 64 字节命令中 OPC/CID/NSID 的作用
- [ ] 能解释中断聚合为什么能降 CPU 负载
- [ ] 能画出 主机→NVMe→FTL→NAND 的数据通路并指出瓶颈
