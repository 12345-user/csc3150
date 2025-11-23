#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
int main(int argc, char *argv[]) {
    pid_t pid;
    int status; // 用于存储子进程的退出状态

    printf("Process start to fork\n");
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("I'm the Child Process:\n");
        printf("\tI'm raising SIGCHLD signal!\n");
        raise(SIGCHLD); // 发送 SIGCHLD 信号给父进程
    } else {
        // Parent process
        wait(&status);
        printf("Parent process receives the signal\n");

        if (WIFEXITED(status)) {
            printf("Normal(NORMAL) termination with EXIT STATUS = %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Abnormal(termination) FAILED: %d\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("CHILD PROCESS STOPPED: %d\n", WSTOPSIG(status));
        } else {
            printf("CHILD PROCESS CONTINUED\n");
        }
        exit(0);
    }

    return 0;
}