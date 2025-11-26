#define _GNU_SOURCE                   // 启用 GNU 扩展以便使用 sched_setaffinity 等接口
#include <errno.h>                    // 提供 errno 以及错误描述
#include <sched.h>                    // 包含 CPU 亲和力相关的数据结构与 API
#include <stdint.h>                   // 提供固定宽度整数类型
#include <stdio.h>                    // 标准输入输出函数
#include <stdlib.h>                   // 提供通用工具函数如 exit/malloc
#include <string.h>                   // 内存操作函数
#include <time.h>                     // 获取高分辨率时间
#include <unistd.h>                   // 包含 sysconf 等 POSIX 接口

// 用于封装一次 workload 的统计结果，便于主流程读取
typedef struct {
    double elapsed_ms;               // 单次测试耗时 (毫秒)
    double checksum;                 // 防止编译器优化掉计算的简单累积
} workload_result_t;

// 统一的错误处理：打印日志并退出进程
// 参数 message: 触发失败时的上下文描述
static void fail_and_exit(const char *message) {
    fprintf(stderr, "[ERROR] %s: %s\n", message, strerror(errno));  // 输出错误信息和 errno 描述
    exit(EXIT_FAILURE);                                              // 非零退出保证测试失败被捕获
}

// 打印当前 CPU mask 的内容，便于观察亲和力设置
// header: 说明文字; set: 要打印的亲和力集合; cpu_cnt: 在线 CPU 数目，用于遍历
static void log_cpu_set(const char *header, const cpu_set_t *set, int cpu_cnt) {
    printf("[LOG] %s {", header);                                    // 先打印标题
    for (int i = 0; i < cpu_cnt; ++i) {                              // 遍历每个 CPU 下标
        if (CPU_ISSET(i, set)) {                                     // 如果该 CPU 位被设置
            printf(" %d", i);                                        // 打印 CPU 编号
        }
    }
    printf(" }\n");                                                  // 结束括号与换行
}

// 获得单调递增时钟，避免受系统时间调整影响
// 返回值: 以毫秒为单位的当前单调时间
static double monotonic_ms(void) {
    struct timespec ts;                                              // timespec 用于保存秒/纳秒
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {                  // 调用单调时钟
        fail_and_exit("clock_gettime failed");                       // 失败则退出
    }
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;                    // 转换为毫秒
}

// 简单的 CPU 忙循环，增加 ALU/FPU 压力
// iterations: 循环次数，越大压力越高
// 返回值: 累积的 checksum，防止被优化
static double busy_spin(size_t iterations) {
    volatile double acc = 0.0;                                       // 使用 volatile 防止优化
    for (size_t i = 0; i < iterations; ++i) {                        // 主循环
        acc += (double)(i % 97) * 0.25;                              // 引入加法与取模操作
        acc *= 1.000000119;                                          // 使用乘法增加 FPU 工作量
    }
    return acc;                                                      // 返回累积值
}

// 顺序访问 64MB buffer，模拟典型内存访问延迟场景
// buffer: 数据缓冲区; len: 元素个数
static double touch_memory(uint64_t *buffer, size_t len) {
    volatile double acc = 0.0;                                       // 累积访问结果
    for (size_t i = 0; i < len; i += 64 / sizeof(uint64_t)) {        // 步长为 64 字节，模拟 cache line
        buffer[i] += (uint64_t)i;                                    // 修改数据制造写入流
        acc += buffer[i] * 0.000001;                                 // 将结果累积
    }
    return acc;                                                      // 返回累积值
}

// 根据目标亲和 mask 运行 workload，返回耗时及累积校验
// label: 当前测试标签; target_set: 目标 CPU 集合; cpu_cnt: 在线 CPU 数
static workload_result_t run_workload(const char *label, const cpu_set_t *target_set, int cpu_cnt) {
    workload_result_t result = {0};                                  // 存储结果的结构体
    if (sched_setaffinity(0, sizeof(cpu_set_t), target_set) != 0) {  // 将当前进程绑定到目标 CPU
        fail_and_exit("sched_setaffinity failed");                   // 设置失败则退出
    }
    log_cpu_set(label, target_set, cpu_cnt);                         // 打印当前亲和力设置

    const size_t array_len = (64 * 1024 * 1024) / sizeof(uint64_t);  // 64 MB buffer 对应的元素数
    uint64_t *buffer = aligned_alloc(64, array_len * sizeof(uint64_t));  // 64 字节对齐以匹配 cache line
    if (!buffer) {                                                   // 分配失败处理
        fail_and_exit("aligned_alloc failed");
    }
    memset(buffer, 0, array_len * sizeof(uint64_t));                 // 初始化 buffer 保证一致性

    const double start_ms = monotonic_ms();                          // 记录开始时间
    double checksum = 0.0;                                           // 初始化 checksum
    for (int rounds = 0; rounds < 5; ++rounds) {                     // 重复 workload 多次，稳定结果
        checksum += touch_memory(buffer, array_len);                 // 内存压力测试
        checksum += busy_spin(array_len);                            // 计算压力测试
    }
    const double end_ms = monotonic_ms();                            // 记录结束时间

    free(buffer);                                                    // 释放动态内存

    result.elapsed_ms = end_ms - start_ms;                           // 计算耗时
    result.checksum = checksum;                                      // 记录 checksum
    printf("[LOG] %s completed in %.2f ms (checksum %.2f)\n",        // 输出日志
           label, result.elapsed_ms, result.checksum);
    return result;                                                   // 返回测试结果
}

// 主函数：串联亲和力测试流程
int main(void) {
    const long cpu_cnt = sysconf(_SC_NPROCESSORS_ONLN);              // 查询在线 CPU 数
    if (cpu_cnt < 1) {                                               // 若查询失败或无 CPU
        fprintf(stderr, "[WARN] Unable to detect online CPU cores, aborting test.\n");
        return EXIT_FAILURE;                                         // 立刻退出
    }
    printf("[LOG] Detected %ld online CPU cores.\n", cpu_cnt);        // 打印检测结果

    cpu_set_t original_set;                                          // 存放原始亲和设置
    if (sched_getaffinity(0, sizeof(cpu_set_t), &original_set) != 0) {  // 读取当前亲和力掩码
        fail_and_exit("sched_getaffinity failed");
    }
    log_cpu_set("Original affinity mask", &original_set, cpu_cnt);    // 打印原始设置，方便恢复

    cpu_set_t single_core_set;                                       // 单核亲和力集合
    CPU_ZERO(&single_core_set);                                      // 清空集合
    CPU_SET(0, &single_core_set);                                    // 只保留 CPU0，模拟弃核

    cpu_set_t multi_core_set;                                        // 多核亲和力集合
    CPU_ZERO(&multi_core_set);                                       // 清空集合
    for (int i = 0; i < cpu_cnt; ++i) {                              // 将所有在线 CPU 加入集合
        CPU_SET(i, &multi_core_set);
    }

    printf("[LOG] Starting single-core affinity test (simulated core drop).\n");  // 第一阶段：单核
    workload_result_t single = run_workload("Single-core affinity", &single_core_set, cpu_cnt);  // 执行单核测试

    printf("[LOG] Starting all-core affinity test (fully open).\n");  // 第二阶段：释放全部 CPU
    workload_result_t multi = run_workload("All-core affinity", &multi_core_set, cpu_cnt);    // 执行全核测试

    if (sched_setaffinity(0, sizeof(cpu_set_t), &original_set) != 0) { // 恢复原始亲和力
        fail_and_exit("restore original affinity failed");
    }
    printf("[LOG] Original CPU affinity restored.\n");                // 确认恢复

    printf("\n[RESULT] Single-core elapsed: %.2f ms\n", single.elapsed_ms);  // 输出单核耗时
    printf("[RESULT] All-core elapsed: %.2f ms\n", multi.elapsed_ms);        // 输出全核耗时
    printf("[RESULT] Delta: %.2f ms\n", single.elapsed_ms - multi.elapsed_ms); // 输出差值
    printf("[HINT] Dropping cores limits scheduling freedom and typically increases completion time.\n"); // 提示含义

    return EXIT_SUCCESS;                                             // 正常结束程序
}

