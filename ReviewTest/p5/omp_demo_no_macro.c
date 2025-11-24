#include <stdio.h>      // 标准输入输出库
#include <stdlib.h>     // 标准库，用于内存分配、随机数生成等
#include <omp.h>        // OpenMP 并行编程库
#include <time.h>       // 时间库，用于随机数种子初始化

// 定义数组大小
#define ARRAY_SIZE 1000000  

/**
 * @brief 用随机数初始化整数数组
 * 
 * @param arr 指向待初始化数组的指针
 * @param size 数组大小
 */
//输入数组大小和数组指针，初始化数组，生成随机数
void initArray(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        // rand() 生成 [0, RAND_MAX] 的伪随机数，对 100 取模限制在 [0, 99]
        arr[i] = rand() % 100;
    }
}

/**
 * @brief 串行计算数组元素总和（单线程）
 * 
 * @param arr 指向整数数组的指针
 * @param size 数组大小
 * @return int 数组元素的总和
 */
//输入数组大小和数组指针，串行计算数组元素总和。串行计算
int serialSum(int *arr, int size) {
    // omp_get_wtime()：获取高精度壁钟时间（秒），用于性能计时，性能计时的作用是评估代码执行效率
    fprintf(stderr, "[%lf] Starting serial computation\n", omp_get_wtime());
    
    int sum = 0;
    // 单线程循环累加所有元素
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    fprintf(stderr, "[%lf] Serial computation completed. Sum: %d\n", omp_get_wtime(), sum);
    return sum;
}

/**
 * @brief 用 OpenMP 并行计算数组元素总和（多线程）
 * 
 * @param arr 指向整数数组的指针
 * @param size 数组大小
 * @return int 数组元素的总和
 */
//输入数组大小和数组指针，使用OpenMP并行计算数组元素总和。并行计算
int parallelSum(int *arr, int size) {
    fprintf(stderr, "[%lf] Starting parallel computation\n", omp_get_wtime());
    
    int sum = 0; // 全局总和，供所有线程共享更新

    // OpenMP 指令：创建并行区域，自动生成线程（默认数量 = CPU 核心数）
    #pragma omp parallel
    {
        int partialSum = 0; // 线程私有变量，存储当前线程的局部和（避免数据竞争）//同进程中每个线程私有的寄存器和栈空间数据

        // omp_get_thread_num()：获取当前线程 ID（从 0 开始）
        int threadId = omp_get_thread_num();
        // omp_get_num_threads()：获取并行区域的总线程数
        int numThreads = omp_get_num_threads();

        // 打印线程启动信息
        fprintf(stderr, "[%lf] Thread %d started. Total threads: %d\n", omp_get_wtime(), threadId, numThreads);

        // OpenMP 指令：并行化后续 for 循环，自动分配迭代任务给各线程
        #pragma omp for
        for (int i = 0; i < size; i++) {
            // 每个线程累加自己分配到的数组片段
            partialSum += arr[i];
        }

        // 打印当前线程的局部计算结果
        fprintf(stderr, "[%lf] Thread %d finished local computation. Local sum: %d\n", omp_get_wtime(), threadId, partialSum);

        // OpenMP 指令：定义临界区，确保同一时间只有一个线程执行（安全更新全局和）
        #pragma omp critical
        {
            sum += partialSum; // 局部和累加到全局和
            fprintf(stderr, "[%lf] Thread %d entering critical section. Global sum updated to: %d\n", omp_get_wtime(), threadId, sum);
        }
        // 临界区结束，其他线程可进入

    }
    // 并行区域结束，所有线程汇合，仅主线程继续执行

    fprintf(stderr, "[%lf] Parallel computation completed. Sum: %d\n", omp_get_wtime(), sum);
    return sum;
}

/**
 * @brief 程序主函数，负责整体流程控制
 * @return int 0 = 正常结束，非 0 = 异常退出
 */
int main() {
    int *arr = NULL;

    // 系统调用：malloc 从堆区动态分配内存（存储 ARRAY_SIZE 个整数）
    arr = (int *)malloc(ARRAY_SIZE * sizeof(int));
    if (arr == NULL) {
        // 内存分配失败（如内存不足），malloc 返回 NULL
        fprintf(stderr, "[%lf] Memory allocation failed (malloc returned NULL)\n", omp_get_wtime());
        return -1; // 非 0 返回值表示异常
    }
    fprintf(stderr, "[%lf] Array memory allocated successfully. Size: %lu bytes\n", omp_get_wtime(), (unsigned long)ARRAY_SIZE * sizeof(int));

    // srand()：初始化随机数生成器；time(NULL)：获取当前时间戳作为种子（保证每次运行随机数不同）
    srand(time(NULL));
    
    // 调用函数初始化数组
    initArray(arr, ARRAY_SIZE);
    fprintf(stderr, "[%lf] Array initialization completed\n", omp_get_wtime());

    // --- 串行计算测试 ---
    fprintf(stderr, "[%lf] Starting serial computation test\n", omp_get_wtime());
    int serialResult = serialSum(arr, ARRAY_SIZE);

    // --- 并行计算测试 ---
    fprintf(stderr, "[%lf] Starting parallel computation test\n", omp_get_wtime());
    int parallelResult = parallelSum(arr, ARRAY_SIZE);

    // --- 结果验证与输出 ---
    fprintf(stderr, "[%lf] Verification result: Serial sum = %d, Parallel sum = %d, %s\n", 
            omp_get_wtime(),
            serialResult, 
            parallelResult, 
            (serialResult == parallelResult) ? "Results match" : "Results DO NOT match!");

    // 系统调用：free 释放 malloc 分配的内存（避免内存泄漏）
    free(arr);
    fprintf(stderr, "[%lf] Array memory released\n", omp_get_wtime());

    fprintf(stderr, "[%lf] Program ended normally\n", omp_get_wtime());
    return 0; // 正常结束
}