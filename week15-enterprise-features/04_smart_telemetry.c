#include <stdio.h>
#include <string.h>

/*
 * SMART 健康监测与寿命估算。
 *
 * 企业级 SSD 持续上报健康数据(SMART/Telemetry), 供运维监控:
 *   - 温度 / 通电时间
 *   - 磨损: 平均擦写次数(PE count) -> 估算剩余寿命
 *   - 可靠性: 重分配扇区数、未校正 ECC 错误 -> 坏块/介质退化信号
 *   - 已用寿命百分比
 *
 * 本实验: 解析一组 SMART 属性, 判定健康状态, 估算剩余寿命。
 */

#define ATTR_NUM 7

typedef struct {
    int id;
    const char *name;
    unsigned long value;
    unsigned long threshold;   /* 超阈值则告警 */
    int higher_is_worse;       /* 1=值越高越危险 0=越低越危险 */
} smart_attr_t;

static smart_attr_t attrs[ATTR_NUM];

static void attrs_init(void)
{
    /* 用数组逐项初始化(避免行数过长) */
    attrs[0] = (smart_attr_t){ 1,  "重分配扇区数(Reallocated)",    12,   10, 1 };
    attrs[1] = (smart_attr_t){ 9,  "通电时间(小时)",              43800, 0,  0 };
    attrs[2] = (smart_attr_t){ 177,"平均擦写次数",                50,    0,  1 };
    attrs[3] = (smart_attr_t){ 187,"未校正读取错误",              3,     5,  1 };
    attrs[4] = (smart_attr_t){ 194,"温度(摄氏度)",                58,    70, 1 };
    attrs[5] = (smart_attr_t){ 231,"已用寿命百分比",              20,    100,1 };
    attrs[6] = (smart_attr_t){ 232,"剩余寿命百分比",              80,    0,  0 };
}

int main(void)
{
    attrs_init();

    printf("== SMART 健康状态 ==\n");
    int alarms = 0;
    for (int i = 0; i < ATTR_NUM; i++) {
        int alarmed = 0;
        if (attrs[i].threshold > 0) {
            if (attrs[i].higher_is_worse && attrs[i].value > attrs[i].threshold)
                alarmed = 1;
            if (!attrs[i].higher_is_worse && attrs[i].value < attrs[i].threshold)
                alarmed = 1;
        }
        if (alarmed) alarms++;
        printf("  [%3d] %-24s = %-6lu %s\n",
               attrs[i].id, attrs[i].name, attrs[i].value,
               alarmed ? "!! 告警" : "");
    }

    /* 寿命估算 */
    unsigned long pe_avg = attrs[2].value;        /* 平均擦写 */
    unsigned long pe_limit = 3000;                /* TLC 约 3000 次 */
    double used_pct = 100.0 * pe_avg / pe_limit;
    double remain_pct = 100.0 - used_pct;
    if (remain_pct < 0) remain_pct = 0;

    printf("\n== 寿命估算 ==\n");
    printf("  平均擦写 %lu / 上限 %lu -> 已用 %.0f%%, 剩余约 %.0f%%\n",
           pe_avg, pe_limit, used_pct, remain_pct);
    printf("  通电 %lu 小时, 温度 %lu 度\n", attrs[1].value, attrs[4].value);

    printf("\n== 健康判定 ==\n");
    if (alarms == 0)
        printf("  状态: 健康, 可继续服役\n");
    else
        printf("  状态: 有 %d 项告警! 建议关注/备份/更换\n", alarms);

    printf("\n要点: SMART 是运维的“仪表盘”, 企业级固件定期上报并触发告警;\n");
    return 0;
}
