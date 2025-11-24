#include <pthread.h>  // 引入 POSIX 线程库
#include <stdio.h>    // 引入标准输入输出库
#include <stdlib.h>   // 引入标准库，用于 atoi 函数

// 全局变量 sum，用于存储计算结果
// 注意：全局变量在所有线程间共享，这是线程间通信的一种简单方式
int sum;

// 线程函数声明
// 所有线程函数都必须遵循此格式：返回一个 void 指针，参数是一个 void 指针
void *runner(void *param);

int main(int argc, char *argv[]) {
    pthread_t tid;          // 线程 ID (Thread ID)，用于标识一个线程
    pthread_attr_t attr;    // 线程属性结构体，用于设置线程的属性

    // 1. 检查命令行参数数量是否正确
    if (argc != 2) {
        fprintf(stderr, "usage: %s <integer value>\n", argv[0]);
        return -1; // 返回非零值，表示程序异常退出
    }
    
    // 2. 检查输入的参数是否为非负整数
    if (atoi(argv[1]) < 0) {
        fprintf(stderr, "%d must be >= 0\n", atoi(argv[1]));
        return -1; // 返回非零值，表示程序异常退出
    }

    // 3. 初始化线程属性为默认值
    // 如果不初始化，直接使用 &attr 可能会导致未定义行为
    pthread_attr_init(&attr);   //这个函数用于初始化线程属性对象 attr，设置为默认属性
    
    // 4. 创建一个新线程
    // 参数说明：
    // &tid: 用于返回新创建线程的 ID
    // &attr: 线程属性，这里是默认属性
    // runner: 新线程要执行的函数入口地址
    // argv[1]: 传递给 runner 函数的参数，这里是一个字符串指针
    pthread_create(&tid, &attr, runner, argv[1]);
    
    // 5. 主线程等待新线程执行完毕
    // 如果没有这一步，主线程可能会在新线程计算完成前就打印 sum 的值（初始值 0）
    // 参数说明：
    // tid: 要等待的线程的 ID
    // NULL: 不关心线程的返回值，所以设为 NULL
    pthread_join(tid, NULL);

    // 6. 打印最终结果
    // 此时，sum 的值已经被 runner 线程计算完毕
    printf("sum = %d\n", sum);
    
    return 0; // 主函数返回 0，表示程序正常结束
}

/**
 * @brief 线程执行的函数，用于计算 1 到 n 的总和
 * 
 * @param param 从主线程传递过来的参数，类型为 void*，需要强制转换
 * @return void* 线程的返回值，这里返回 NULL
 */
void *runner(void *param) {
    int i;
    // 将传入的 void* 参数强制转换为 char*，然后用 atoi 函数转换为整数
    int upper = atoi(param);
    
    sum = 0; // 在计算前，将共享变量 sum 清零
    
    // 计算从 1 到 upper 的总和
    for (i = 1; i <= upper; i++) {
        sum += i;
    }
    
    // 线程退出，并返回一个空指针
    // 这个返回值可以被 pthread_join 的第二个参数捕获
    pthread_exit(0); 
}