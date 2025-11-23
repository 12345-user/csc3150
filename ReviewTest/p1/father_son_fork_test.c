#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int main()
{
    pid_t pid;

    /* fork a child process */
    pid = fork();

    if (pid < 0) { /* error occurred */
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) { /* child process */
    if (execlp("/bin/ls","ls",NULL) == -1) {
        fprintf(stderr, "execlp failed: %s\n", strerror(errno));
        exit(1);
        }
    }
    else { /* parent process */
        /* parent will wait for the child to complete */
        wait(NULL); // 等待子进程结束
        printf("Child Complete\n");
    }

    return 0;
}