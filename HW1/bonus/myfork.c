#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {
    pid_t pid;
    int status;
    
    printf("=== My Fork Demo ===\n");
    printf("Original process PID: %d\n", getpid());
    
    pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // 子进程
        printf("Child process PID: %d, Parent PID: %d\n", getpid(), getppid());
        printf("Child process is running...\n");
        sleep(2);
        printf("Child process exiting\n");
        exit(0);
    } else {
        // 父进程
        printf("Parent process PID: %d, Child PID: %d\n", getpid(), pid);
        printf("Parent waiting for child...\n");
        
        wait(&status);
        
        if (WIFEXITED(status)) {
            printf("Child exited with status: %d\n", WEXITSTATUS(status));
        }
        
        printf("Parent process finished\n");
    }
    
    return 0;
}
