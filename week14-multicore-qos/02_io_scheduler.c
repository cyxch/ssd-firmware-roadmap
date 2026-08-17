#include <stdio.h>

/*
 * IO 调度器：FIFO vs 优先级队列（QoS 基础）。
 *
 * 问题: 固件里同时有普通读写 IO 和"紧急/高优先级"命令
 *       (Trim、管理命令、实时性要求高的 IO)。
 *       若用 FIFO, 高优先级命令会被堵在一大堆普通 IO 后面。
 *
 * 场景: 12 个作业同时入队(前 10 个普通, 后 2 个高优先级),
 *       每个处理耗时 1 单位。
 *       等待时间 = 该作业被服务的次序(位置)。
 *
 * 对比: FIFO 把高优先级排到队尾; 优先级调度让它们最先被服务。
 */

#define JOB_NUM 12

typedef struct {
    int id;
    int pri;      /* 1=高 2=普通 */
} job_t;

static job_t jobs[JOB_NUM];

static void init_jobs(void)
{
    for (int i = 0; i < JOB_NUM; i++) {
        jobs[i].id = i;
        jobs[i].pri = (i >= 10) ? 1 : 2;   /* id 10,11 是高优先级 */
    }
}

/* use_priority=1 用优先级调度; 返回高优先级作业的平均等待 */
static double run_scheduler(int use_priority)
{
    int order[JOB_NUM];
    int done = 0;
    int serviced[JOB_NUM] = {0};

    while (done < JOB_NUM) {
        int pick = -1;
        if (use_priority) {
            /* 优先级调度: 挑最高优先级(同级取 id 小) */
            int best_pri = 0x7fffffff;
            for (int i = 0; i < JOB_NUM; i++) {
                if (!serviced[i] && jobs[i].pri < best_pri) {
                    best_pri = jobs[i].pri;
                    pick = i;
                }
            }
        } else {
            /* FIFO: 按 id 顺序 */
            for (int i = 0; i < JOB_NUM; i++) {
                if (!serviced[i]) { pick = i; break; }
            }
        }
        serviced[pick] = 1;
        order[done++] = pick;
    }

    /* 统计高优先级作业的平均等待(等待 = 服务位置下标) */
    int n_hi = 0;
    long long wait_sum = 0;
    for (int i = 0; i < JOB_NUM; i++) {
        if (jobs[order[i]].pri == 1) {
            wait_sum += i;
            n_hi++;
        }
    }

    printf("  服务顺序: ");
    for (int i = 0; i < JOB_NUM; i++)
        printf("%s%d ", jobs[order[i]].pri == 1 ? "H" : "", order[i]);
    printf("\n");
    return (double)wait_sum / n_hi;
}

int main(void)
{
    init_jobs();

    printf("== 场景: 10 个普通 IO + 2 个高优先级 IO, 同时入队 ==\n\n");

    printf("FIFO 调度:\n");
    double w1 = run_scheduler(0);
    printf("  高优先级 IO 平均等待 %.1f 单位\n\n", w1);

    printf("优先级调度:\n");
    double w2 = run_scheduler(1);
    printf("  高优先级 IO 平均等待 %.1f 单位\n\n", w2);

    printf("结论: FIFO 让高优先级 IO 排在 10 个普通 IO 后面,\n");
    printf("      优先级调度把它们提到最前, 平均等待从 %.1f 降到 %.1f\n", w1, w2);
    return 0;
}
