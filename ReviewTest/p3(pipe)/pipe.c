#include <sys/types.h>  // 包含进程类型相关定义（如pid_t）
#include <sys/wait.h>   // 为了使用 waitpid() 函数，必须包含此头文件
#include <stdio.h>      // 标准输入输出（如printf、fprintf）
#include <unistd.h>     // 包含pipe、fork、close等系统调用
#include <string.h>     // 字符串操作（如strlen）

#define BUFFER_SIZE 25  // 定义缓冲区大小
#define READ_END 0      // 管道读端标识
#define WRITE_END 1     // 管道写端标识

int main(void) {
    char write_msg[BUFFER_SIZE] = "Greetings";  // 父进程要写入管道的消息
    char read_msg[BUFFER_SIZE];                 // 子进程从管道读取的消息存储区
    int fd[2];                                  // 管道的文件描述符数组
    pid_t pid;                                  // 用于存储进程ID
    int status;                                 // 用于存储子进程的退出状态

    /* 创建管道 */
    if (pipe(fd) == -1) {
        fprintf(stderr, "Pipe failed");
        return 1;
    }

    /* 创建子进程 */
    pid = fork();

    if (pid < 0) { /* 进程创建失败 */
        fprintf(stderr, "Fork Failed");
        return 1;
    }

    if (pid > 0) { /* 父进程执行分支 */
        /* 关闭管道未使用的读端 */
        close(fd[READ_END]);

        /* 向管道写端写入消息 */
        write(fd[WRITE_END], write_msg, strlen(write_msg) + 1);

        /* 关闭管道写端 */
        close(fd[WRITE_END]);

        // ======================================================
        // 【插入位置】在这里增加回收子进程的代码
        // ======================================================
        printf("Parent process (PID: %d) is waiting for child process (PID: %d) to finish...\n", getpid(), pid);
        
        // waitpid() 会阻塞父进程，直到指定的子进程（pid）结束
        // &status 用于存储子进程的退出状态信息
        // 0 表示等待方式为阻塞
        waitpid(pid, &status, 0);
        printf("Parent process: Child process has finished.\n");
        // ======================================================

    } else { /* 子进程执行分支 */
        /* 关闭管道未使用的写端 */
        close(fd[WRITE_END]);

        /* 从管道读端读取消息 */
        read(fd[READ_END], read_msg, BUFFER_SIZE);
        printf("Child process (PID: %d) read: %s\n", getpid(), read_msg);

        /* 关闭管道读端 */
        close(fd[READ_END]);
        
        printf("Child process (PID: %d) has completed execution and is exiting.\n", getpid());
    }

    return 0;  // 程序正常退出
}