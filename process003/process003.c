#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // 若使用 perror 需要该头文件

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;

    printf("Process start to fork\n");
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("I'm the Child Process:\n");
        sleep(10);
        printf("\tMy pid is:%d.  My ppid is:%d\n", getpid(), getppid());// 打印子进程的 PID 和 PPID
        exit(0);
    } else {
        // Parent process
        waitpid(pid, &status, 0); // 等待特定子进程结束
        printf("I'm the Parent Process:\n"); // 等待子进程结束后再打印
        printf("\tMy pid is:%d\n", getpid()); // 打印父进程的 PID
        printf("\tChild process exited with status %d\n", status); // 打印子进程的退出状态
        exit(0);
    }

    return 0;
}