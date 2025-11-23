#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

/* This fucntion is called when the module is loaded*/
int main(int argc, char *argv[])
{
    pid_t pid;

    /*fork a child process*/
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork Failed\n");
        return -1;
    }

    else if (pid == 0) {
        /*child process*/
        execlp("/bin/ls", "ls", NULL);
        fprintf(stderr, "execlp Failed\n");
        return 1;
    }

    else {
        /*parent process*/
        wait(NULL);
        printf("Child Complete\n");
    }
    return 0;
}

