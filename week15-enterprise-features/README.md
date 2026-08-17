# Week 15 · NVMe-oF、多命名空间与企业级 QoS

SSD 固件方向学习计划 · 第 15 周（企业级特性）

## 本周目标

- 理解 NVMe over Fabrics 远程访问架构与延迟代价
- 掌握多命名空间的概念与 (nsid, lba) 寻址
- 理解命名空间级 QoS 隔离的必要性
- 掌握 SMART 健康监测与寿命估算
- 串起多命名空间 x 优先级 x WRR 的企业级 QoS 控制面

## 目录结构

| 文件 | 说明 | 本机编译 |
|------|------|---------|
| `01_nvme_of.c` | NVMe-oF 架构与本地/远程延迟对比 | ✅ gcc 可运行 |
| `02_multiple_namespaces.c` | 多命名空间地址空间 | ✅ gcc 可运行 |
| `03_ns_qos_isolation.c` | 命名空间级 QoS 隔离 | ✅ gcc 可运行 |
| `04_smart_telemetry.c` | SMART 健康监测与寿命估算 | ✅ gcc 可运行 |
| `05_enterprise_qos_sim.c` | 企业级综合 QoS 调度 | ✅ gcc 可运行 |

## 编译运行

```bash
gcc -O2 -Wall -Wextra -o f 01_nvme_of.c && ./f
gcc -O2 -Wall -Wextra -o n 02_multiple_namespaces.c && ./n
gcc -O2 -Wall -Wextra -o q 03_ns_qos_isolation.c && ./q
gcc -O2 -Wall -Wextra -o s 04_smart_telemetry.c && ./s
gcc -O2 -Wall -Wextra -o e 05_enterprise_qos_sim.c && ./e
```

## 关键概念速查

- NVMe-oF：RDMA/FC/TCP 传输，本地命令语义 + 网络 RTT
- 命名空间：一块物理盘分成多个逻辑盘，独立 LBA/容量/块大小
- QoS 隔离：per-NS 配额/令牌桶，防一个租户饿死其他租户
- SMART：温度/PE/重分配扇区/ECC 错误，寿命 = 已用 PE / 上限
- 企业级 QoS = 隔离 + 权重带宽(WRR) + 延迟优先(优先级)

## 本周自测

- [ ] 能说出 NVMe-oF 三种传输及其延迟差异
- [ ] 能解释为什么 (nsid, lba) 才能唯一寻址
- [ ] 能说明无隔离时写风暴为什么饿死其他租户
- [ ] 能根据 SMART 数据估算剩余寿命
- [ ] 能画出企业级 QoS 的控制面要素
