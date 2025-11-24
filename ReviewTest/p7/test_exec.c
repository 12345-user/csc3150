#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>     // 包含 execlp(), getpid()
#include <errno.h>      // 包含 errno

/**
 * @brief 线程c的执行函数。
 *        它会持续打印信息，直到进程被 exec() 替换。
 */
void* thread_c_task(void* arg) {
    pthread_t tid = pthread_self();
    
    printf("[PID: %d, Thread c: %lu] Starting task. Will run until exec() is called.\n", getpid(), (unsigned long)tid);
    
    // 简单的循环任务
    int count = 0;
    while (1) { // 无限循环，直到进程被替换
        printf("[PID: %d, Thread c: %lu] Running task... Count: %d\n", getpid(), (unsigned long)tid, count++);
        sleep(1); // 休眠1秒
    }
    
    // 下面的代码永远不会被执行，因为 exec() 会替换整个进程
    printf("[PID: %d, Thread c: %lu] This message will never be printed.\n", getpid(), (unsigned long)tid);
    pthread_exit(NULL);
}

/**
 * @brief 线程d的执行函数。
 *        它会调用 execlp() 来用 'ls -l' 命令替换当前进程。
 */
void* thread_d_task(void* arg) {
    pthread_t tid = pthread_self();
    
    printf("\n[PID: %d, Thread d: %lu] Starting task. Preparing to call exec().\n", getpid(), (unsigned long)tid);
    sleep(3); // 等待线程c跑起来

    printf("[PID: %d, Thread d: %lu] Calling execlp(\"ls\", \"ls\", \"-l\", NULL) now...\n", getpid(), (unsigned long)tid);
    printf("[PID: %d, Thread d: %lu] After exec(), the current process image will be replaced by 'ls -l'.\n", getpid(), (unsigned long)tid);
    printf("[PID: %d, Thread d: %lu] Both thread c and thread d will cease to exist.\n", getpid(), (unsigned long)tid);
    
    // 调用 execlp 执行 'ls -l' 命令
    // execlp 会搜索 PATH 环境变量来找到 'ls' 程序
    execlp("ls", "ls", "-l", NULL);

    // 如果 execlp 调用成功，它永远不会返回
    // 如果返回了，说明调用失败
    perror("[PID: %d, Thread d] execlp failed");
    printf("[PID: %d, Thread d] If you see this, exec() failed. errno: %d\n", getpid(), errno);
    
    pthread_exit(NULL);
}

int main() {
    pthread_t thread_c, thread_d;

    printf("=== Test Program for exec() ===\n");
    printf("Process ID: %d\n", getpid());
    
    // 创建线程c和线程d
    pthread_create(&thread_c, NULL, thread_c_task, NULL);
    pthread_create(&thread_d, NULL, thread_d_task, NULL);
    
    // 等待线程d完成。但如果 exec() 成功，这个进程就不存在了，所以join不会执行
    //pthread_join(thread_d, NULL)函数的作用是等待线程d执行完毕。
    pthread_join(thread_d, NULL);
    // 等待线程c完成。同样，这行代码在 exec() 成功后不会执行
    pthread_join(thread_c, NULL);

    // 这行代码也不会被执行
    printf("=== Program exit (this will not be printed if exec succeeds) ===\n");
    return 0;
}