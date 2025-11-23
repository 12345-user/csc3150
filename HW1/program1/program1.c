#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

int main(int argc, char *argv[]){
    pid_t pid;
    int status;
    
    if (argc != 2) {
        printf("Usage: %s <test_program>\n", argv[0]);
        exit(1);
    }
    
    printf("Process start to fork\n");
    
    /* fork a child process */
    pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }
    else if (pid == 0) {
        /* Child process */
        printf("I'm the Child Process, my pid = %d\n", getpid());
        printf("Child process start to execute test program:\n");
        
        /* execute test program */
        execl(argv[1], argv[1], NULL);
        perror("execl failed");
        exit(1);
    }
    else {
        /* Parent process */
        printf("I'm the Parent Process, my pid = %d\n", getpid());
        
        /* wait for child process terminates */
        wait(&status);
        printf("Parent process receives SIGCHLD signal\n");
        
        /* check child process' termination status */
        if (WIFEXITED(status)) {
            printf("Normal termination with EXIT STATUS = %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            printf("child process get ");
            switch(sig) {
                case SIGABRT: printf("SIGABRT"); break;
                case SIGALRM: printf("SIGALRM"); break;
                case SIGBUS: printf("SIGBUS"); break;
                case SIGFPE: printf("SIGFPE"); break;
                case SIGHUP: printf("SIGHUP"); break;
                case SIGILL: printf("SIGILL"); break;
                case SIGINT: printf("SIGINT"); break;
                case SIGKILL: printf("SIGKILL"); break;
                case SIGPIPE: printf("SIGPIPE"); break;
                case SIGQUIT: printf("SIGQUIT"); break;
                case SIGSEGV: printf("SIGSEGV"); break;
                case SIGSTOP: printf("SIGSTOP"); break;
                case SIGTERM: printf("SIGTERM"); break;
                case SIGTRAP: printf("SIGTRAP"); break;
                default: printf("SIGNAL %d", sig); break;
            }
            printf(" signal\n");
        }
        else if (WIFSTOPPED(status)) {
            printf("child process get SIGSTOP signal\n");
        }
    }
    
    return 0;
}
