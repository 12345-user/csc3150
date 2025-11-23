#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    pid_t pId;

    printf("Process start to fork\n");
    pId = fork();

    if (pId == -1) {
        printf("Fail to fork\n");
        exit(1);
    }
    else if (pId == 0) {
        // Child process
        printf("I'm the Child Process\n");
        printf("C01,My pid is:%d,  My ppid is:%d\n", getpid(), getppid());
        sleep(10);
        printf("C02,My pid is:%d,  My ppid is:%d\n", getpid(), getppid());
        exit(0);
    }
    else {
        // Parent process
        sleep(3);
        printf("I'm the Parent Process\n");
        printf("P01,,My pid is:%d,  My ppid is:%d\n", getpid(), getppid());
        exit(0);
    }

    return 0;
}