#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main() {
    // 临时注册处理函数（可选，也可直接屏蔽）
    void sigint_handler(int signum) {
        printf("\nReceived SIGINT (signal %d), but will restore default behavior later.\n", signum);
    }
    //这个signal函数作用是注册SIGINT信号的处理函数为sigint_handler
    signal(SIGINT, sigint_handler);

    //这里定义信号集，用于屏蔽和恢复信号
    sigset_t mask, oldmask;
    //初始化信号集mask为空集
    sigemptyset(&mask);
    //向信号集mask中添加SIGINT信号
    sigaddset(&mask, SIGINT);

    // 屏蔽 SIGINT
    //sigprocmask函数用于改变进程的信号屏蔽字
    //SIG_BLOCK表示将mask中的信号添加到当前的信号屏蔽字中，从而屏蔽这些信号
    if (sigprocmask(SIG_BLOCK, &mask, &oldmask) == -1) {
        perror("sigprocmask failed");
        return 1;
    }
    printf("SIGINT blocked. Press Ctrl+C (ignored) for 5 seconds...\n");
    sleep(5);

    // 解除屏蔽 + 恢复 SIGINT 默认行为（关键）
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) == -1) {
        perror("sigprocmask failed");
        return 1;
    }
    signal(SIGINT, SIG_DFL);  // 恢复默认终止行为
    printf("SIGINT unblocked and restored to default. Press Ctrl+C to exit...\n");

    //如果恢复失败，则进入无限循环等待信号
    while (1) {
        sleep(1);
    }

    return 0;
}