#include <pthread.h>   // POSIX 线程库头文件，提供 pthread_xxx API（属于系统线程库，并非信号接口）
#include <sched.h>     // 调度策略与优先级相关的头文件，定义 SCHED_OTHER / SCHED_RR / SCHED_FIFO 等常量
#include <stdio.h>     // C 标准 I/O 库头文件，用于打印/错误输出（非系统调用，而是标准库封装）
#include <string.h>    // 提供 memset 等内存操作函数
#include <unistd.h>    // 提供 sleep/usleep 等 POSIX 接口

#define NUM_THREADS 5  // 要创建的线程数量，本程序固定为5

// 用于向线程入口函数传参的结构体
typedef struct {
    int index;                 // 线程在本次测试中的编号（0 ~ NUM_THREADS-1）
    const char *policy_name;   // 当前测试使用的调度策略名称（例如 "SCHED_FIFO"）
} thread_arg_t;

// 线程入口函数的前向声明，满足编译器在 main 之前就知道其签名
void *runner(void *param);

/*
 * main 函数：程序入口。
 * 本测试程序会依次对 SCHED_OTHER、SCHED_RR、SCHED_FIFO 三种调度策略进行测试，
 * 每种策略下创建 NUM_THREADS 个线程，在线程内部查询并打印自己的实际调度策略和优先级。
 * 所有日志均使用英文输出，方便在不同 locale 下阅读；注释保持中文，解释每个参数/函数是否属于库或系统调用。
 */
int main(int argc, char *argv[])
{
    (void)argc;                                   // 当前测试不使用命令行参数，避免未使用警告
    (void)argv;

    pthread_attr_t attr;                          // 线程属性对象，用于配置调度策略与优先级
    int i = 0;                                    // 通用循环计数器

    // 定义要测试的调度策略数组，每一项包括策略常量和其字符串名称
    struct {
        int policy;                               // 调度策略常量：SCHED_OTHER / SCHED_RR / SCHED_FIFO
        const char *name;                         // 对应策略的名称字符串
    } policies[] = {
        {SCHED_OTHER, "SCHED_OTHER"},             // 普通分时调度策略
        {SCHED_RR,    "SCHED_RR"},                // 实时轮转调度策略
        {SCHED_FIFO,  "SCHED_FIFO"}               // 实时先进先出调度策略
    };

    int num_policies = (int)(sizeof(policies) / sizeof(policies[0])); // 要测试的策略总数

    printf("[LOG] Starting scheduling policy test for %d policies, %d threads each.\n",
           num_policies, NUM_THREADS);

    // 外层循环：依次对每一种调度策略进行测试
    for (int p = 0; p < num_policies; ++p) {
        int current_policy = policies[p].policy;   // 当前要测试的策略常量
        const char *policy_name = policies[p].name; // 当前策略名称字符串

        printf("\n[LOG] ===== Testing policy %s (%d/%d) =====\n",
               policy_name, p + 1, num_policies);

        // 初始化线程属性对象为默认值
        // pthread_attr_init 属于 pthread 库函数，不是系统调用；内部可能封装内核逻辑
        if (pthread_attr_init(&attr) != 0) {
            fprintf(stderr, "[ERROR] pthread_attr_init failed, skip policy %s.\n",
                    policy_name);
            continue;
        }

        // 设置为显式调度属性：新线程将使用 attr 中指定的调度策略和优先级
        // PTHREAD_EXPLICIT_SCHED 常量属于 pthread 线程属性设置选项，由线程库定义
        if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
            fprintf(stderr, "[WARN] Failed to set inherit scheduler to EXPLICIT, "
                            "threads may inherit parent's scheduling.\n");
        }

        // 设置当前测试的调度策略
        // pthread_attr_setschedpolicy 同样属于 pthread 线程库，而非直接系统调用
        if (pthread_attr_setschedpolicy(&attr, current_policy) != 0) {
            fprintf(stderr, "[ERROR] Failed to set policy %s on pthread attributes.\n",
                    policy_name);
            pthread_attr_destroy(&attr);          // 销毁属性对象，进入下一个策略
            continue;
        }

        // 计算该策略支持的优先级范围（仅对实时策略有效）
        // sched_get_priority_min / max 属于实时调度库函数，最终会查询内核调度参数
        int min_prio = sched_get_priority_min(current_policy);
        int max_prio = sched_get_priority_max(current_policy);
        struct sched_param sp;                    // 用于承载要设置的优先级参数
        memset(&sp, 0, sizeof(sp));              // 初始化结构体为0，防止未定义字段

        if (min_prio != -1 && max_prio != -1) {
            // 取中间优先级，便于兼容大多数系统
            sp.sched_priority = (min_prio + max_prio) / 2;
            printf("[LOG] Policy %s priority range: min=%d, max=%d, chosen=%d.\n",
                   policy_name, min_prio, max_prio, sp.sched_priority);
        } else {
            // 对于 SCHED_OTHER 等非实时策略，返回值可能为 -1，此时直接使用0
            sp.sched_priority = 0;
            printf("[LOG] Policy %s does not use real-time priorities, using default priority 0.\n",
                   policy_name);
        }

        // 将优先级参数写入线程属性
        // pthread_attr_setschedparam 也是线程库函数，用于在用户态配置调度参数
        if (pthread_attr_setschedparam(&attr, &sp) != 0) {
            fprintf(stderr, "[WARN] Failed to set sched_param for policy %s, "
                            "threads may run with default priority.\n",
                    policy_name);
        }

        pthread_t tids[NUM_THREADS];              // 保存本轮测试创建的线程ID
        thread_arg_t args[NUM_THREADS];           // 向每个线程传递索引与策略名称

        printf("[LOG] Creating %d threads with policy %s.\n",
               NUM_THREADS, policy_name);

        // 创建若干线程并传入对应参数
        for (i = 0; i < NUM_THREADS; ++i) {
            args[i].index = i;                    // 设置线程在本轮中的编号
            args[i].policy_name = policy_name;    // 告知线程期望的调度策略名称

            // pthread_create:
            //  - 第1个参数 &tids[i]：输出型，保存新线程ID；
            //  - 第2个参数 &attr：线程属性（包含调度策略和优先级）；
            //  - 第3个参数 runner：线程入口函数指针；
            //  - 第4个参数 &args[i]：传入的用户参数，不是信号，仅为普通指针。
            int rc = pthread_create(&tids[i],     // 输出参数，保存线程ID
                                    &attr,        // 线程属性，指定调度策略和优先级
                                    runner,       // 线程入口函数
                                    &args[i]);    // 传递参数结构体地址
            if (rc != 0) {
                fprintf(stderr,
                        "[ERROR] Failed to create thread %d for policy %s, return code=%d.\n",
                        i, policy_name, rc);
                tids[i] = 0;                      // 标记此位置线程无效
            } else {
                printf("[LOG] Created thread %d for policy %s.\n", i, policy_name);
            }
        }

        // 等待所有线程结束
        for (i = 0; i < NUM_THREADS; ++i) {
            if (tids[i]) {                        // 仅等待成功创建的线程
                // pthread_join 用于等待指定线程结束，属于 pthread 线程库
                pthread_join(tids[i], NULL);
                printf("[LOG] Joined thread %d for policy %s.\n", i, policy_name);
            }
        }

        // 使用完属性对象后进行销毁，释放内部资源
        pthread_attr_destroy(&attr);

        printf("[LOG] Finished testing policy %s.\n", policy_name);
    }

    printf("\n[LOG] All policy tests completed.\n");
    return 0;                                     // 正常退出main
}

/*
 * runner:
 *   - 线程入口函数，签名由 pthread_create 规定：void* (*)(void*)
 *   - 参数 param：线程创建时传入的用户数据，这里为 thread_arg_t*，不是信号，仅为普通指针。
 *   - 在线程内部会调用 pthread_getschedparam 查询并打印自身的调度策略与优先级。
 */
void *runner(void *param)
{
    thread_arg_t *info = (thread_arg_t *)param;   // 将void*转换为具体的参数结构指针
    int policy = 0;                               // 用于接收线程当前调度策略编号
    struct sched_param sp;                        // 用于接收线程当前调度优先级

    // pthread_getschedparam:
    //   - 第1个参数 pthread_self()：获取当前线程ID；
    //   - 第2个参数 &policy：输出当前线程的调度策略；
    //   - 第3个参数 &sp：输出当前线程的优先级参数。
    if (pthread_getschedparam(pthread_self(), &policy, &sp) != 0) {
        fprintf(stderr,
                "[THREAD %d] Failed to get scheduling parameters for expected policy %s.\n",
                info ? info->index : -1,
                info ? info->policy_name : "UNKNOWN");
    } else {
        printf("[THREAD %d] Started under policy=%d (expected=%s), priority=%d.\n",
               info ? info->index : -1,
               policy,
               info ? info->policy_name : "UNKNOWN",
               sp.sched_priority);
    }

    // 模拟一定的工作负载：简单的忙循环+短暂sleep，便于观察不同策略下的调度行为
    for (int iter = 0; iter < 5; ++iter) {
        printf("[THREAD %d] Working... iteration %d under policy %s.\n",
               info ? info->index : -1,
               iter,
               info ? info->policy_name : "UNKNOWN");
        // 简单忙循环，增加一点CPU占用
        for (volatile long k = 0; k < 1000000; ++k) {
            // 空循环，什么也不做，仅消耗时间
        }
        usleep(1000);                             // usleep：暂停约1毫秒，属于POSIX库函数
    }

    printf("[THREAD %d] Finished work under policy %s, exiting.\n",
           info ? info->index : -1,
           info ? info->policy_name : "UNKNOWN");

    // pthread_exit(0)：显式结束线程并返回0，属于 pthread 库函数，不是系统信号
    pthread_exit(0);
}


