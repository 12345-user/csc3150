#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>     // 包含 fork(), getpid()
#include <sys/wait.h>   // 包含 waitpid()

// 全局变量，用于在线程间传递信息（这里用于通知线程a退出）
volatile int keep_running = 1;

/**
 * @brief 线程a的执行函数。
 *        它会持续打印信息，直到 keep_running 被设置为 0。
 */

//线程a的执行函数
void* thread_a_task(void* arg) {
    // 获取当前线程ID，pthread_self() 返回调用线程的 ID
    pthread_t tid = pthread_self();
    
    printf("[PID: %d, Thread a: %lu] Starting task. Will run until signaled to stop.\n", getpid(), (unsigned long)tid);
    
    // 简单的循环任务
    int count = 0;
    while (keep_running) {
        printf("[PID: %d, Thread a: %lu] Running task... Count: %d\n", getpid(), (unsigned long)tid, count++);
        sleep(1); // 休眠1秒
    }
    
    printf("[PID: %d, Thread a: %lu] Task stopped.\n", getpid(), (unsigned long)tid);
    pthread_exit(NULL);
}

/**
 * @brief 线程b的执行函数。
 *        它会调用 fork() 创建子进程，然后在父进程和子进程中观察状态。
 */
void* thread_b_task(void* arg) {
    pthread_t tid = pthread_self();
    pid_t pid;

    printf("\n[PID: %d, Thread b: %lu] Starting task. Preparing to call fork().\n", getpid(), (unsigned long)tid);
    sleep(2); // 等待线程a跑起来

    // 调用 fork() 创建子进程
    pid = fork();

    if (pid < 0) {
        // fork 失败
        perror("[PID: %d, Thread b] fork failed");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) {
        // --- 子进程 ---
        printf("[CHILD PID: %d] Fork successful. I am the child process.\n", getpid());
        printf("[CHILD PID: %d] In the child process, only the thread that called fork() (thread b) continues executing.\n", getpid());
        printf("[CHILD PID: %d] Thread a does NOT exist in the child process. Let's verify...\n", getpid());
        printf("[CHILD PID: %d] If thread a were running, we would see its 'Running task...' messages here.\n", getpid());
        printf("[CHILD PID: %d] Child process will exit after 5 seconds.\n", getpid());
        
        sleep(5); // 等待5秒，观察是否有线程a的输出
        
        printf("[CHILD PID: %d] Child process exiting now.\n", getpid());
        exit(EXIT_SUCCESS); // 子进程退出
    } 
    else {
        // --- 父进程 ---
        printf("[PARENT PID: %d] Fork successful. Child process ID is %d.\n", getpid(), pid);
        printf("[PARENT PID: %d] In the parent process, both threads (a and b) continue to run.\n", getpid());
        
        // 等待子进程结束
        printf("[PARENT PID: %d] Waiting for child process %d to exit...\n", getpid(), pid);
        waitpid(pid, NULL, 0);
        printf("[PARENT PID: %d] Child process %d has exited.\n", getpid(), pid);
        
        // 通知线程a停止
        printf("[PARENT PID: %d] Signaling thread a to stop.\n", getpid());
        keep_running = 0;
    }

    printf("[PID: %d, Thread b: %lu] Task completed.\n", getpid(), (unsigned long)tid);
    pthread_exit(NULL);
}

int main() {
    pthread_t thread_a, thread_b;

    printf("=== Test Program for fork() ===\n");
    printf("Parent Process ID: %d\n", getpid());
    
    // 创建线程a和线程b
    pthread_create(&thread_a, NULL, thread_a_task, NULL);
    pthread_create(&thread_b, NULL, thread_b_task, NULL);
    
    // 等待线程b完成
    pthread_join(thread_b, NULL);
    printf("[PARENT PID: %d] Thread b has finished.\n", getpid());

    // 等待线程a完成
    pthread_join(thread_a, NULL);
    printf("[PARENT PID: %d] Thread a has finished.\n", getpid());

    printf("=== Program exit ===\n");
    return 0;
}